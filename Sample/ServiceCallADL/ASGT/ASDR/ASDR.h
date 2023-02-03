///
///  Copyright (c) 2019 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

#ifndef _SERVICE_H
#define _SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif

#include "..\..\Include\GlobalDefines.h"
#include "..\..\Include\ShareMFile.h"

//////////////////////////////////////////////////////////////////////////////
//// 
////
// name of the executable
#define SZAPPNAME            "ASGT"
// internal name of the service
#define SZSERVICENAME        "ASGT"
// displayed name of the service
#define SZSERVICEDISPLAYNAME "ASGT"
// list of service dependencies - "dep1\0dep2\0\0"
#define SZDEPENDENCIES       ""
// adl call service function name

//
//////////////////////////////////////////////////////////////////////////////

    VOID ServiceStart();
    VOID ServiceStop();

    BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

    void AddToMessageLog(LPTSTR lpszMsg);

    #ifdef __cplusplus
}
#endif

#endif