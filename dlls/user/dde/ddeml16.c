/* -*- tab-width: 8; c-basic-offset: 8 -*- */

/*
 * DDEML library
 *
 * Copyright 1997 Alexandre Julliard
 * Copyright 1997 Len White
 * Copyright 1999 Keith Matthews
 * Copyright 2000 Corel
 * Copyright 2001 Eric Pouech
 */

#include <string.h>
#include "winbase.h"
#include "windef.h"
#include "wine/windef16.h"
#include "wingdi.h"
#include "winuser.h"
#include "winerror.h"
#include "dde.h"
#include "ddeml.h"
#include "debugtools.h"

DEFAULT_DEBUG_CHANNEL(ddeml);


typedef HDDEDATA CALLBACK (*PFNCALLBACK16)(UINT16,UINT16,HCONV,HSZ,HSZ,HDDEDATA,DWORD,DWORD);

typedef struct
{
    UINT16  cb;
    UINT16  wFlags;
    UINT16  wCountryID;
    INT16   iCodePage;
    DWORD   dwLangID;
    DWORD   dwSecurity;
} CONVCONTEXT16, *LPCONVCONTEXT16;

typedef struct
{
    DWORD          cb;
    DWORD          hUser;
    HCONV          hConvPartner;
    HSZ            hszSvcPartner;
    HSZ            hszServiceReq;
    HSZ            hszTopic;
    HSZ            hszItem;
    UINT16         wFmt;
    UINT16         wType;
    UINT16         wStatus;
    UINT16         wConvst;
    UINT16         wLastError;
    HCONVLIST      hConvList;
    CONVCONTEXT16  ConvCtxt;
} CONVINFO16, *LPCONVINFO16;


/******************************************************************************
 *            DdeInitialize16   (DDEML.2)
 */
UINT16 WINAPI DdeInitialize16(LPDWORD pidInst, PFNCALLBACK16 pfnCallback,
			      DWORD afCmd, DWORD ulRes)
{
     return (UINT16)DdeInitializeA(pidInst,(PFNCALLBACK)pfnCallback,
				   afCmd, ulRes);
}

/*****************************************************************
 *            DdeUninitialize16   (DDEML.3)
 */
BOOL16 WINAPI DdeUninitialize16(DWORD idInst)
{
     FIXME(" stub calling DdeUninitialize\n");
     return (BOOL16)DdeUninitialize(idInst);
}

/*****************************************************************
 * DdeConnectList16 [DDEML.4]
 */

HCONVLIST WINAPI DdeConnectList16(DWORD idInst, HSZ hszService, HSZ hszTopic,
				  HCONVLIST hConvList, LPCONVCONTEXT16 pCC)
{
     return DdeConnectList(idInst, hszService, hszTopic, hConvList, 
			   (LPCONVCONTEXT)pCC);
}

/*****************************************************************
 * DdeQueryNextServer16 [DDEML.5]
 */
HCONV WINAPI DdeQueryNextServer16(HCONVLIST hConvList, HCONV hConvPrev)
{
     return DdeQueryNextServer(hConvList, hConvPrev);
}

/*****************************************************************
 *            DdeDisconnectList (DDEML.6)
 */
BOOL16 WINAPI DdeDisconnectList16(HCONVLIST hConvList)
{
     return (BOOL16)DdeDisconnectList(hConvList);
}


/*****************************************************************
 *		DdeQueryString16 (DDEML.23)
 */
DWORD WINAPI DdeQueryString16(DWORD idInst, HSZ hsz, LPSTR lpsz, DWORD cchMax, INT16 codepage)
{
     FIXME("(%ld, 0x%x, %p, %ld, %d): stub \n", 
	   idInst, hsz, lpsz, cchMax, codepage);
     return 0;
}

/*****************************************************************
 *            DdeConnect16   (DDEML.7)
 */
HCONV WINAPI DdeConnect16(DWORD idInst, HSZ hszService, HSZ hszTopic,
			  LPCONVCONTEXT16 pCC16)
{
     CONVCONTEXT	cc;
     CONVCONTEXT*	pCC = NULL;
     
     if (pCC16) {
	  pCC = &cc;
	  cc.cb = sizeof(cc);
	  cc.wFlags = pCC16->wFlags;
	  cc.iCodePage = pCC16->iCodePage;
	  cc.dwLangID = pCC16->dwLangID;
	  cc.dwSecurity = pCC16->dwSecurity;
     }
     return DdeConnect(idInst, hszService, hszTopic, pCC);
}

/*****************************************************************
 *            DdeDisconnect16   (DDEML.8)
 */
BOOL16 WINAPI DdeDisconnect16(HCONV hConv)
{
     return (BOOL16)DdeDisconnect(hConv);
}

/*****************************************************************
 *            DdeSetUserHandle16 (DDEML.10)
 */
BOOL16 WINAPI DdeSetUserHandle16(HCONV hConv, DWORD id, DWORD hUser)
{
     FIXME("(%d,%ld,%ld): stub\n",hConv,id, hUser);
     return 0;
}

/*****************************************************************
 *            DdeCreateDataHandle16 (DDEML.14)
 */
HDDEDATA WINAPI DdeCreateDataHandle16(DWORD idInst, LPBYTE pSrc, DWORD cb, 
				      DWORD cbOff, HSZ hszItem, UINT16 wFmt, 
				      UINT16 afCmd)
{
     return DdeCreateDataHandle(idInst, pSrc, cb, cbOff, hszItem, wFmt, afCmd);
}

/*****************************************************************
 *            DdeCreateStringHandle16   (DDEML.21)
 */
HSZ WINAPI DdeCreateStringHandle16(DWORD idInst, LPCSTR str, INT16 codepage)
{
     if  (codepage)
     {
	  return DdeCreateStringHandleA(idInst, str, codepage);
     } else {
	  TRACE("Default codepage supplied\n");
	  return DdeCreateStringHandleA(idInst, str, CP_WINANSI);
     }
}

/*****************************************************************
 *            DdeFreeStringHandle16   (DDEML.22)
 */
BOOL16 WINAPI DdeFreeStringHandle16(DWORD idInst, HSZ hsz)
{
     FIXME("idInst %ld hsz 0x%x\n",idInst,hsz);
     return (BOOL)DdeFreeStringHandle(idInst, hsz);
}

/*****************************************************************
 *            DdeFreeDataHandle16   (DDEML.19)
 */
BOOL16 WINAPI DdeFreeDataHandle16(HDDEDATA hData)
{
     return (BOOL)DdeFreeDataHandle(hData);
}

/*****************************************************************
 *            DdeKeepStringHandle16   (DDEML.24)
 */
BOOL16 WINAPI DdeKeepStringHandle16(DWORD idInst, HSZ hsz)
{
     return (BOOL)DdeKeepStringHandle(idInst, hsz);
}

/*****************************************************************
 *            DdeClientTransaction16  (DDEML.11)
 */
HDDEDATA WINAPI DdeClientTransaction16(LPVOID pData, DWORD cbData,
				       HCONV hConv, HSZ hszItem, UINT16 wFmt,
				       UINT16 wType, DWORD dwTimeout,
				       LPDWORD pdwResult)
{
     return DdeClientTransaction((LPBYTE)pData, cbData, hConv, hszItem,
				 wFmt, wType, dwTimeout, pdwResult);
}

/*****************************************************************
 *
 *            DdeAbandonTransaction16 (DDEML.12)
 *
 */
BOOL16 WINAPI DdeAbandonTransaction16(DWORD idInst, HCONV hConv, 
				      DWORD idTransaction)
{
     FIXME("empty stub\n");
     return TRUE;
}

/*****************************************************************
 * DdePostAdvise16 [DDEML.13]
 */
BOOL16 WINAPI DdePostAdvise16(DWORD idInst, HSZ hszTopic, HSZ hszItem)
{
     return (BOOL16)DdePostAdvise(idInst, hszTopic, hszItem);
}

/*****************************************************************
 *            DdeAddData16 (DDEML.15)
 */
HDDEDATA WINAPI DdeAddData16(HDDEDATA hData, LPBYTE pSrc, DWORD cb,
			     DWORD cbOff)
{
     return DdeAddData(hData, pSrc, cb, cbOff);
}

/*****************************************************************
 * DdeGetData16 [DDEML.16]
 */
DWORD WINAPI DdeGetData16(
     HDDEDATA hData,
     LPBYTE pDst,
     DWORD cbMax, 
     DWORD cbOff)
{
     return DdeGetData(hData, pDst, cbMax, cbOff);
}

/*****************************************************************
 *            DdeAccessData16 (DDEML.17)
 */
LPBYTE WINAPI DdeAccessData16(HDDEDATA hData, LPDWORD pcbDataSize)
{
     return DdeAccessData(hData, pcbDataSize);
}

/*****************************************************************
 *            DdeUnaccessData16 (DDEML.18)
 */
BOOL16 WINAPI DdeUnaccessData16(HDDEDATA hData)
{
     return DdeUnaccessData(hData);
}

/*****************************************************************
 *            DdeEnableCallback16 (DDEML.26)
 */
BOOL16 WINAPI DdeEnableCallback16(DWORD idInst, HCONV hConv, UINT16 wCmd)
{
     return DdeEnableCallback(idInst, hConv, wCmd);
}

/*****************************************************************
 *            DdeNameService16  (DDEML.27)
 */
HDDEDATA WINAPI DdeNameService16(DWORD idInst, HSZ hsz1, HSZ hsz2,
				 UINT16 afCmd)
{
     return DdeNameService(idInst, hsz1, hsz2, afCmd);
}

/*****************************************************************
 *            DdeGetLastError16  (DDEML.20)
 */
UINT16 WINAPI DdeGetLastError16(DWORD idInst)
{
     return (UINT16)DdeGetLastError(idInst);
}

/*****************************************************************
 *            DdeCmpStringHandles16 (DDEML.36)
 */
INT16 WINAPI DdeCmpStringHandles16(HSZ hsz1, HSZ hsz2)
{
     return DdeCmpStringHandles(hsz1, hsz2);
}

/******************************************************************
 *		DdeQueryConvInfo16 (DDEML.9)
 *
 */
UINT16 WINAPI DdeQueryConvInfo16(HCONV hconv, DWORD idTransaction, LPCONVINFO16 lpConvInfo)
{
     FIXME("stub.\n");
     return 0;
}
