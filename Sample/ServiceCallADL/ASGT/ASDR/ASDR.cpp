///
///  Copyright (c) 2019 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file ASDR.cpp

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <atlbase.h>
#include "ASDR.h"
#include <WtsApi32.h>
#include "..\..\Include\GlobalDefines.h"

#include "GetUserProcessData.h"


#define AMDVENDORID             (1002)
#define ADL_WARNING_NO_DATA      -100
#define PRINTF printf

using namespace std;
#ifdef RUN_ADL_AS_SERVICE
typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_FLUSH_DRIVER_DATA)(int);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int(*ADL_ADAPTERX2_CAPS) (int, int*);
typedef int(*ADL2_MAIN_CONTROL_CREATE)									(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int(*ADL2_OVERDRIVEN_CAPABILITIESX2_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNCapabilitiesX2*);
typedef int(*ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceStatus*);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_GET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_SET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_POWERLIMIT_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int(*ADL2_OVERDRIVEN_POWERLIMIT_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int(*ADL2_OVERDRIVEN_TEMPERATURE_GET) (ADL_CONTEXT_HANDLE, int, int, int*);
typedef int(*ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
typedef int(*ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
typedef int(*ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
typedef int(*ADL2_OVERDRIVEN_MEMORYCLOCKSX2_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
typedef int(*PADL_DISPLAY_MVPUSTATUS_GET)(int, ADLMVPUStatus  *);
HINSTANCE hDLL;
ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
ADL_ADAPTERX2_CAPS ADL_AdapterX2_Caps = NULL;
ADL2_MAIN_CONTROL_CREATE			ADL2_Main_Control_Create = NULL;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get = NULL;
ADL2_OVERDRIVEN_CAPABILITIESX2_GET ADL2_OverdriveN_CapabilitiesX2_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET ADL2_OverdriveN_SystemClocksX2_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_SET ADL2_OverdriveN_SystemClocksX2_Set = NULL;
ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET ADL2_OverdriveN_PerformanceStatus_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_GET ADL2_OverdriveN_FanControl_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_SET ADL2_OverdriveN_FanControl_Set = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_GET ADL2_OverdriveN_PowerLimit_Get = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_SET ADL2_OverdriveN_PowerLimit_Set = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET ADL2_OverdriveN_MemoryClocksX2_Get = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKSX2_SET ADL2_OverdriveN_MemoryClocksX2_Set = NULL;
ADL2_OVERDRIVE_CAPS ADL2_Overdrive_Caps = NULL;
ADL2_OVERDRIVEN_TEMPERATURE_GET ADL2_OverdriveN_Temperature_Get = NULL;
PADL_DISPLAY_MVPUSTATUS_GET pADL_Display_MVPUStatus_Get;
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}
void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
    if (NULL != *lpBuffer)
    {
        free(*lpBuffer);
        *lpBuffer = NULL;
    }
}
ADL_CONTEXT_HANDLE context = NULL;
LPAdapterInfo   lpAdapterInfo = NULL;
int  iNumberAdapters;
#endif
HANDLE m_hGPUFanSettingEvent;
HANDLE m_hEXFanSettingEvent;
HANDLE hThread1 = NULL;
HANDLE hThread2 = NULL;
#define SHARED_FANSETTINGDATAEVENT "Global\\userfansettingdataevent"
#define SHARED_FANSETTINGDATAEVENT_EX "Global\\userfansettingdataevent_EX"

SERVICE_STATUS          ssStatus;       
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;
BOOL                    bDebug = FALSE;
TCHAR                   szErr[256];
HANDLE  hServerStopEvent = NULL;
PROCESS_INFORMATION	processInfo;

DWORD WINAPI service_ctrl(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv);
VOID CmdInstallService();
LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize);
BOOL CheckIsVista();
bool bstop = 0;
BOOL IsVista = FALSE;

int maxThermalControllerIndex = 0;

void __cdecl main(int argc, char **argv)
{
    SERVICE_TABLE_ENTRY dispatchTable[] =
    {
       { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main},
       { NULL, NULL}
    };

    if ((argc > 1) &&
        ((*argv[1] == '-') || (*argv[1] == '/')))
    {
        if (_stricmp("install", argv[1] + 1) == 0)
        {
            CmdInstallService();
        }
        else
        {
            goto dispatch;
        }
        exit(0);
    }

dispatch:
    if (!StartServiceCtrlDispatcher(dispatchTable))
        AddToMessageLog(TEXT("StartServiceCtrlDispatcher failed."));
}

void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{

#ifdef _SERVICE_DEBUG
    bool bSleep = true;
    while (bSleep)
        Sleep(200);
#else
#ifdef RUN_ADL_AS_SERVICE
    Sleep(20000);
#endif
#endif
    DWORD drtn;


    sshStatusHandle = RegisterServiceCtrlHandlerEx(TEXT(SZSERVICENAME), service_ctrl, NULL);

    if (!sshStatusHandle)
        goto cleanup;

    ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwServiceSpecificExitCode = 0;


    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, 
        NO_ERROR,              
        3000))                 
        goto cleanup;

    hServerStopEvent = CreateEvent(
        NULL,    
        TRUE,    
        FALSE,   
        NULL);   

    if (hServerStopEvent == NULL)
        goto cleanup;


    IsVista = CheckIsVista();

    ServiceStart();
    if (ReportStatusToSCMgr(SERVICE_RUNNING,       
        NO_ERROR,              
        0))                    
        drtn = WAIT_TIMEOUT;
#ifndef RUN_ADL_AS_SERVICE
    DWORD dwThreadId = 0;
    // TODO: run user process 
    {
        // Get all adapter info
        ExADLAdapterInfo responseADLdata = ExADLAdapterInfo();
        responseADLdata.iAdapters = 0;
        GetUserProcessData<ExADLAdapterInfo>  getUserProcessData = GetUserProcessData<ExADLAdapterInfo>(ExADLAdapterInfo::GetFunctionName(), responseADLdata);
        ExADLAdapterInfo * responsePointADLdata = getUserProcessData.GetProcessData();
        int nAdapter = responsePointADLdata->szData / sizeof(AdapterInfo);
        // Read structure point after template structure 
        AdapterInfo* infos = (AdapterInfo*)((INT64)responsePointADLdata + sizeof(ExADLAdapterInfo));
        int iDevice = 0;
        for (int loopi = 0; loopi < nAdapter; loopi++)
        {
            AdapterInfo info = infos[loopi];
            // Get over drive is enabled
            ExADLOverdriveCaps exADLODCaps = ExADLOverdriveCaps();
            exADLODCaps.iAdapters = info.iAdapterIndex;
            GetUserProcessData<ExADLOverdriveCaps >  getUserProcessCaps = GetUserProcessData<ExADLOverdriveCaps >(ExADLOverdriveCaps::GetFunctionName(), exADLODCaps);
            if ((7 == exADLODCaps.iVersion) && (true == exADLODCaps.iSupported))
            {
                // Get over drive capabilities ( if not wait please change  GetUserProcessData::GetUserProcessData INFINITE value
                ExADLODNCapabilitiesX2 exCapabilities = ExADLODNCapabilitiesX2();
                exCapabilities.iAdapters = info.iAdapterIndex;;
                GetUserProcessData<ExADLODNCapabilitiesX2>  getCapabilities = GetUserProcessData<ExADLODNCapabilitiesX2>(ExADLODNCapabilitiesX2::GetFunctionName(), exCapabilities);
                ADLODNCapabilitiesX2 ODNCapabilitiesX2 = exCapabilities.sODNCapabilitiesX2;

                // Get over drive fan control 
                ExADLODNFanControl exADLdataFanControl = ExADLODNFanControl();
                exADLdataFanControl.iAdapters = info.iAdapterIndex;;
                GetUserProcessData<ExADLODNFanControl>  getDataFanControl = GetUserProcessData<ExADLODNFanControl>(ExADLODNFanControl::GetFunctionName(), exADLdataFanControl);
                ADLODNFanControl ODNFanControl = exADLdataFanControl.odNFanControl;
            }
        }
        if (responsePointADLdata)
        {
            delete responsePointADLdata;
            responsePointADLdata = NULL;
        }
    }

    while (1)
    {
        drtn = WaitForSingleObject(hServerStopEvent, INFINITE);
        if (WAIT_OBJECT_0 == drtn)
            break;
    }
#endif
cleanup:


    if (sshStatusHandle)
        (VOID)ReportStatusToSCMgr(
            SERVICE_STOPPED,
            dwErr,
            0);
    if (g_hTokenToCheck != NULL)
        CloseHandle(g_hTokenToCheck);


    return;
}

DWORD WINAPI service_ctrl(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl)
    {
    case SERVICE_CONTROL_STOP:
        ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
        ServiceStop();
        return 0;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_SESSIONCHANGE:
        if (dwEventType == WTS_SESSION_LOGON)
        {
            ServiceStart();
        }
        break;

    default:
        break;

    }

    ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
    return 0;
}

BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;


    if (!bDebug) // when debugging we don't report to the SCM
    {
        if (dwCurrentState == SERVICE_START_PENDING)
            ssStatus.dwControlsAccepted = 0;
        else
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE;

        ssStatus.dwCurrentState = dwCurrentState;
        ssStatus.dwWin32ExitCode = dwWin32ExitCode;
        ssStatus.dwWaitHint = dwWaitHint;

        if ((dwCurrentState == SERVICE_RUNNING) ||
            (dwCurrentState == SERVICE_STOPPED))
            ssStatus.dwCheckPoint = 0;
        else
            ssStatus.dwCheckPoint = dwCheckPoint++;


        // Report the status of the service to the service control manager.
        //
        if (!(fResult = SetServiceStatus(sshStatusHandle, &ssStatus)))
        {
            AddToMessageLog(TEXT("SetServiceStatus"));
        }
    }
    return fResult;
}



VOID AddToMessageLog(LPTSTR lpszMsg)
{
    TCHAR szMsg[(sizeof(SZSERVICENAME) / sizeof(TCHAR)) + 100];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[2];

    if (!bDebug)
    {
        dwErr = GetLastError();
        hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

        _stprintf_s(szMsg, (sizeof(SZSERVICENAME) / sizeof(TCHAR)) + 100, TEXT("%s error: %d"), TEXT(SZSERVICENAME), dwErr);
        lpszStrings[0] = szMsg;
        lpszStrings[1] = lpszMsg;

        if (hEventSource != NULL)
        {
            ReportEvent(hEventSource, 
                EVENTLOG_ERROR_TYPE,  
                0,                    
                0,                    
                NULL,                 
                2,                    
                0,                    
                (LPCSTR*)lpszStrings, 
                NULL);                

            (VOID)DeregisterEventSource(hEventSource);
        }
    }
}


void CmdInstallService()
{
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;

    TCHAR szPath[512];

    if (GetModuleFileName(NULL, szPath, 512) == 0)
    {
        return;
    }

    schSCManager = OpenSCManager(
        NULL,                  
        NULL,                  
        SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE
    );
    if (schSCManager)
    {
        schService = CreateService(
            schSCManager,               
            TEXT(SZSERVICENAME),        
            TEXT(SZSERVICEDISPLAYNAME), 
            SERVICE_QUERY_STATUS,       
            SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,  
            SERVICE_AUTO_START,       
            SERVICE_ERROR_NORMAL,       
            szPath,                     
            NULL,                       
            NULL,                       
            TEXT(SZDEPENDENCIES),       
            NULL,                       
            NULL);                      

        if (schService)
        {
            CloseServiceHandle(schService);
        }
        CloseServiceHandle(schSCManager);
    }

}




#ifdef RUN_ADL_AS_SERVICE
int initializeADL()
{

    hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
    if (hDLL == NULL)
    {
        hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
    }
    if (NULL == hDLL)
    {
        PRINTF("Failed to load ADL library\n");
        return FALSE;
    }
    ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
    ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
    ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
    ADL_AdapterX2_Caps = (ADL_ADAPTERX2_CAPS)GetProcAddress(hDLL, "ADL_AdapterX2_Caps");
    ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL2_Main_Control_Create");
    ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
    ADL2_OverdriveN_CapabilitiesX2_Get = (ADL2_OVERDRIVEN_CAPABILITIESX2_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_CapabilitiesX2_Get");
    ADL2_OverdriveN_SystemClocksX2_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_SystemClocksX2_Get");
    ADL2_OverdriveN_SystemClocksX2_Set = (ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_SystemClocksX2_Set");
    ADL2_OverdriveN_MemoryClocksX2_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_MemoryClocksX2_Get");
    ADL2_OverdriveN_MemoryClocksX2_Set = (ADL2_OVERDRIVEN_MEMORYCLOCKSX2_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_MemoryClocksX2_Set");
    ADL2_OverdriveN_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_PerformanceStatus_Get");
    ADL2_OverdriveN_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_FanControl_Get");
    ADL2_OverdriveN_FanControl_Set = (ADL2_OVERDRIVEN_FANCONTROL_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_FanControl_Set");
    ADL2_OverdriveN_PowerLimit_Get = (ADL2_OVERDRIVEN_POWERLIMIT_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_PowerLimit_Get");
    ADL2_OverdriveN_PowerLimit_Set = (ADL2_OVERDRIVEN_POWERLIMIT_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_PowerLimit_Set");
    ADL2_OverdriveN_Temperature_Get = (ADL2_OVERDRIVEN_TEMPERATURE_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_Temperature_Get");
    ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS)GetProcAddress(hDLL, "ADL2_Overdrive_Caps");
    pADL_Display_MVPUStatus_Get = (PADL_DISPLAY_MVPUSTATUS_GET)GetProcAddress(hDLL, "ADL_Display_MVPUStatus_Get");
    if (NULL == ADL_Main_Control_Create ||
        NULL == ADL_Main_Control_Destroy ||
        NULL == ADL_Adapter_NumberOfAdapters_Get ||
        NULL == ADL_Adapter_AdapterInfo_Get ||
        NULL == ADL_AdapterX2_Caps ||
        NULL == ADL2_Main_Control_Create ||
        NULL == ADL2_Adapter_Active_Get ||
        NULL == ADL2_OverdriveN_CapabilitiesX2_Get ||
        NULL == ADL2_OverdriveN_SystemClocksX2_Get ||
        NULL == ADL2_OverdriveN_SystemClocksX2_Set ||
        NULL == ADL2_OverdriveN_MemoryClocksX2_Get ||
        NULL == ADL2_OverdriveN_MemoryClocksX2_Set ||
        NULL == ADL2_OverdriveN_PerformanceStatus_Get ||
        NULL == ADL2_OverdriveN_FanControl_Get ||
        NULL == ADL2_OverdriveN_FanControl_Set ||
        NULL == ADL2_Overdrive_Caps ||
        NULL == pADL_Display_MVPUStatus_Get
        )
    {
        PRINTF("Failed to get ADL function pointers\n");
        return FALSE;
    }
    if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
    {
        printf("Failed to initialize nested ADL2 context");
        return ADL_ERR;
    }
    return TRUE;
}
void deinitializeADL()
{
    ADL_Main_Control_Destroy();
    FreeLibrary(hDLL);
}
#endif
VOID ServiceStart()
{
#ifdef RUN_ADL_AS_SERVICE
    initializeADL();
    int iReturn;
    int  i, active = 0;;
    int iSupported, iEnabled, iVersion;
    int  iNumberAdapters = 0;
    if (ADL_OK != ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &context))
    {
        printf("Failed to initialize ADL2 context");
        return;
    }
    if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters))
    {
        return;
    }
    iReturn = ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters);
    if (0 < iNumberAdapters)
    {
        lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * iNumberAdapters);
        memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * iNumberAdapters);
        iReturn = ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * iNumberAdapters);
    }
    for (i = 0; i < iNumberAdapters; i++)
    {
        ADLODNFanControl odNFanControl;
        memset(&odNFanControl, 0, sizeof(ADLODNFanControl));
        iReturn = ADL2_OverdriveN_FanControl_Get(context, lpAdapterInfo[i].iAdapterIndex, &odNFanControl);
        if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context, lpAdapterInfo[i].iAdapterIndex, &odNFanControl))
        {
            PRINTF("ADL2_OverdriveN_FanControl_Get is failed\n");
        }
        ADLODNCapabilitiesX2 overdriveCapabilities;
        memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilitiesX2));
        iReturn = ADL2_OverdriveN_CapabilitiesX2_Get(context, lpAdapterInfo[i].iAdapterIndex, &overdriveCapabilities);
        ADLODNPerformanceLevelsX2 *odPerformanceLevels;
        int size = sizeof(ADLODNPerformanceLevelsX2) + sizeof(ADLODNPerformanceLevelX2)* (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1);
        void* performanceLevelsBuffer = new char[size];
        memset(performanceLevelsBuffer, 0, size);
        odPerformanceLevels = (ADLODNPerformanceLevelsX2*)performanceLevelsBuffer;
        odPerformanceLevels->iSize = size;
        odPerformanceLevels->iMode = 0; 
        odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
        iReturn = ADL2_OverdriveN_MemoryClocksX2_Get(context, lpAdapterInfo[i].iAdapterIndex, odPerformanceLevels);
        int iDisplayIdx = 0;
        ADLMVPUStatus stMVPInfo_ADL = { 0 };
        stMVPInfo_ADL.iSize = sizeof(ADLMVPUStatus);
        iReturn = pADL_Display_MVPUStatus_Get(iDisplayIdx, &stMVPInfo_ADL);
    }
#endif
}

VOID ServiceStop()
{
    if (hServerStopEvent)
        SetEvent(hServerStopEvent);
}


LPTSTR GetLastErrorText(LPTSTR lpszBuf, DWORD dwSize)
{
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        GetLastError(),
        LANG_NEUTRAL,
        (LPTSTR)&lpszTemp,
        0,
        NULL);

    if (!dwRet || ((long)dwSize < (long)dwRet + 14))
        lpszBuf[0] = TEXT('\0');
    else
    {
        if (NULL != lpszTemp)
        {
            lpszTemp[lstrlen(lpszTemp) - 2] = TEXT('\0');  
        }
    }

    if (NULL != lpszTemp)
        LocalFree((HLOCAL)lpszTemp);

    return lpszBuf;
}

BOOL CheckIsVista()
{
	OSVERSIONINFO   VerInfo;
	VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&VerInfo);

	if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if ((VerInfo.dwMajorVersion == 6) && (VerInfo.dwMinorVersion >= 0))
		{
			return TRUE;
		}
	}
	return FALSE;
}