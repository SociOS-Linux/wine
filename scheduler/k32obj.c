/*
 * KERNEL32 objects
 *
 * Copyright 1996 Alexandre Julliard
 */

#include <assert.h>
#include <string.h>
#include "winerror.h"
#include "k32obj.h"
#include "heap.h"
#include "process.h"
#include "server.h"


/* The declarations are here to avoid including a lot of unnecessary files */
extern const K32OBJ_OPS PROCESS_Ops;
extern const K32OBJ_OPS THREAD_Ops;

static const K32OBJ_OPS K32OBJ_NullOps =
{
    NULL     /* destroy */
};

static void K32OBJ_Destroy( K32OBJ *obj );

static const K32OBJ_OPS K32OBJ_DefaultOps =
{
    K32OBJ_Destroy     /* destroy */
};


const K32OBJ_OPS * const K32OBJ_Ops[K32OBJ_NBOBJECTS] =
{
    NULL,
    &K32OBJ_DefaultOps,     /* K32OBJ_SEMAPHORE */
    &K32OBJ_DefaultOps,     /* K32OBJ_EVENT */
    &K32OBJ_DefaultOps,     /* K32OBJ_MUTEX */
    &K32OBJ_NullOps,        /* K32OBJ_CRITICAL_SECTION */
    &PROCESS_Ops,           /* K32OBJ_PROCESS */
    &THREAD_Ops,            /* K32OBJ_THREAD */
    &K32OBJ_DefaultOps,     /* K32OBJ_FILE */
    &K32OBJ_DefaultOps,     /* K32OBJ_CHANGE */
    &K32OBJ_DefaultOps,     /* K32OBJ_CONSOLE */
    &K32OBJ_NullOps,        /* K32OBJ_SCREEN_BUFFER */
    &K32OBJ_DefaultOps,     /* K32OBJ_MEM_MAPPED_FILE */
    &K32OBJ_NullOps,        /* K32OBJ_SERIAL */
    &K32OBJ_NullOps,        /* K32OBJ_DEVICE_IOCTL */
    &K32OBJ_DefaultOps,     /* K32OBJ_PIPE */
    &K32OBJ_NullOps,        /* K32OBJ_MAILSLOT */
    &K32OBJ_DefaultOps,     /* K32OBJ_TOOLHELP_SNAPSHOT */
    &K32OBJ_NullOps         /* K32OBJ_SOCKET */
};

typedef struct _NE
{
    struct _NE *next;
    K32OBJ     *obj;
    UINT      len;
    char        name[1];
} NAME_ENTRY;

static NAME_ENTRY *K32OBJ_FirstEntry = NULL;


/***********************************************************************
 *           K32OBJ_IncCount
 */
void K32OBJ_IncCount( K32OBJ *ptr )
{
    assert( ptr->type && ((unsigned)ptr->type < K32OBJ_NBOBJECTS) );
    SYSTEM_LOCK();
    ptr->refcount++;
    SYSTEM_UNLOCK();
    assert( ptr->refcount > 0 );  /* No wrap-around allowed */
}


/***********************************************************************
 *           K32OBJ_DecCount
 */
void K32OBJ_DecCount( K32OBJ *ptr )
{
    NAME_ENTRY **pptr;

    assert( ptr->type && ((unsigned)ptr->type < K32OBJ_NBOBJECTS) );
    assert( ptr->refcount > 0 );
    SYSTEM_LOCK();
    if (--ptr->refcount)
    {
        SYSTEM_UNLOCK();
        return;
    }

    /* Check if the object has a name entry and free it */

    pptr = &K32OBJ_FirstEntry;
    while (*pptr && ((*pptr)->obj != ptr)) pptr = &(*pptr)->next;
    if (*pptr)
    {
        NAME_ENTRY *entry = *pptr;
        *pptr = entry->next;
        HeapFree( SystemHeap, 0, entry );
    }

    /* Free the object */

    if (K32OBJ_Ops[ptr->type]->destroy) K32OBJ_Ops[ptr->type]->destroy( ptr );
    SYSTEM_UNLOCK();
}


/***********************************************************************
 *           K32OBJ_Destroy
 *
 * Generic destroy functions for objects that don't need any special treatment.
 */
static void K32OBJ_Destroy( K32OBJ *obj )
{
    obj->type = K32OBJ_UNKNOWN;
    HeapFree( SystemHeap, 0, obj );
}


/***********************************************************************
 *           K32OBJ_IsValid
 *
 * Check if a pointer is a valid kernel object
 */
BOOL K32OBJ_IsValid( K32OBJ *ptr, K32OBJ_TYPE type )
{
    if (IsBadReadPtr( ptr, sizeof(*ptr) )) return FALSE;
    return (ptr->type == type);
}


/***********************************************************************
 *           K32OBJ_AddName
 *
 * Add a name entry for an object. We don't check for duplicates here.
 * FIXME: should use some sort of hashing.
 */
BOOL K32OBJ_AddName( K32OBJ *obj, LPCSTR name )
{
    NAME_ENTRY *entry;
    UINT len;

    if (!name) return TRUE;  /* Anonymous object */
    len = strlen( name );
    SYSTEM_LOCK();
    if (!(entry = HeapAlloc( SystemHeap, 0, sizeof(NAME_ENTRY) + len )))
    {
        SYSTEM_UNLOCK();
        SetLastError( ERROR_OUTOFMEMORY );
        return FALSE;
    }
    entry->next = K32OBJ_FirstEntry;
    entry->obj  = obj;
    entry->len  = len;
    lstrcpyA( entry->name, name );
    K32OBJ_FirstEntry = entry;
    SYSTEM_UNLOCK();
    return TRUE;
}


/***********************************************************************
 *           K32OBJ_Create
 *
 * Create a named kernel object.
 * Returns NULL if there was an error _or_ if the object already existed.
 * The refcount of the object must be decremented once it is initialized.
 */
K32OBJ *K32OBJ_Create( K32OBJ_TYPE type, DWORD size, LPCSTR name, int server_handle,
                       DWORD access, SECURITY_ATTRIBUTES *sa, HANDLE *handle)
{
    BOOL inherit = (sa && (sa->nLength>=sizeof(*sa)) && sa->bInheritHandle);

    /* Check if the name already exists */

    K32OBJ *obj = K32OBJ_FindName( name );
    if (obj)
    {
        if (obj->type == type)
        {
            SetLastError( ERROR_ALREADY_EXISTS );
            *handle = HANDLE_Alloc( PROCESS_Current(), obj, access, inherit, server_handle );
        }
        else
        {
            SetLastError( ERROR_DUP_NAME );
            *handle = INVALID_HANDLE_VALUE;
            if (server_handle != -1) CLIENT_CloseHandle( server_handle );
        }
        K32OBJ_DecCount( obj );
        return NULL;
    }

    /* Create the object */

    SYSTEM_LOCK();
    if (!(obj = HeapAlloc( SystemHeap, 0, size )))
    {
        SYSTEM_UNLOCK();
        *handle = INVALID_HANDLE_VALUE;
        if (server_handle != -1) CLIENT_CloseHandle( server_handle );
        return NULL;
    }
    obj->type     = type;
    obj->refcount = 1;

    /* Add a name for it */

    if (!K32OBJ_AddName( obj, name ))
    {
        /* Don't call the destroy function, as the object wasn't
         * initialized properly */
        HeapFree( SystemHeap, 0, obj );
        SYSTEM_UNLOCK();
        *handle = INVALID_HANDLE_VALUE;
        if (server_handle != -1) CLIENT_CloseHandle( server_handle );
        return NULL;
    }

    /* Allocate a handle */

    *handle = HANDLE_Alloc( PROCESS_Current(), obj, access, inherit, server_handle );
    SYSTEM_UNLOCK();
    return obj;
}


/***********************************************************************
 *           K32OBJ_FindName
 *
 * Find the object referenced by a given name.
 * The reference count is incremented.
 */
K32OBJ *K32OBJ_FindName( LPCSTR name )
{
    NAME_ENTRY *entry;
    UINT len;

    if (!name) return NULL;  /* Anonymous object */
    len = strlen( name );
    SYSTEM_LOCK();
    entry = K32OBJ_FirstEntry;
    while (entry)
    {
        if ((len == entry->len) && !strcmp( name, entry->name))
        {
            K32OBJ *obj = entry->obj;
            K32OBJ_IncCount( obj );
            SYSTEM_UNLOCK();
            return entry->obj;
        }
        entry = entry->next;
    }
    SYSTEM_UNLOCK();
    return NULL;
}


/***********************************************************************
 *           K32OBJ_FindNameType
 *
 * Find an object by name and check its type.
 * The reference count is incremented.
 */
K32OBJ *K32OBJ_FindNameType( LPCSTR name, K32OBJ_TYPE type )
{
    K32OBJ *obj = K32OBJ_FindName( name );
    if (!obj) return NULL;
    if (obj->type == type) return obj;
    SetLastError( ERROR_DUP_NAME );
    K32OBJ_DecCount( obj );
    return NULL;
}
