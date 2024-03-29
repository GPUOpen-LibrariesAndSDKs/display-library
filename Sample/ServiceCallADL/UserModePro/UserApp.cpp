// UserApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "UserApp.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include "..\Include\ShareMFile.h"
#define MAX_LOADSTRING 100
//#define _APP_DEBUG

// Global Variables:

HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HINSTANCE HDll_ = NULL;
ADL_CONTEXT_HANDLE ADLContext_ = NULL;
WCHAR  pstrFunctionName[MAX_PATH] = { 0 };

int adapterIndex = 0;
bool ADLDirty_ = false;



std::map<int, int> AdapterIndexMap_;
bool InitADL(void);
void DestroyADL();
// 
//  FUNCTION: MemoryCommunication
//
//  PURPOSE: Communtcation with service using memory.
//
//  PARAMETERS:
//    lpFunctionName - none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  Read request using shared memory first, using Service request parameters call ADL,
//  response to Service using shared memory write
//
template <class TData> bool MemoryCommunication()
{
	bool bRet = false;
	TData responseADLdata = TData();
	CShareMFile cShareMFile = CShareMFile();
	WCHAR strEventPath[MAX_PATH] = { 0 };
	WCHAR strShareMemoryPath[MAX_PATH] = { 0 };

	swprintf_s(strEventPath, ADL_EVENT_NAME, pstrFunctionName);
	swprintf_s(strShareMemoryPath, ADL_SHAREMENORY_NAME, pstrFunctionName);

	if (cShareMFile.ReadBuffer(strShareMemoryPath, (PVOID)&responseADLdata, sizeof(TData)))
	{
		HANDLE hEvent = NULL;
		responseADLdata.context = ADLContext_;
		if (true == responseADLdata.CallADLFunction())
		{
			if (responseADLdata.szData)
			{   
				// send structure point buffer and template structure to service 
				if (responseADLdata.buffer != NULL)
					cShareMFile.WriteBuffer(strShareMemoryPath, (PVOID)&responseADLdata, sizeof(TData), (PVOID *)&responseADLdata.buffer, responseADLdata.szData);
			}
			else
				cShareMFile.WriteBuffer(strShareMemoryPath, (PVOID)&responseADLdata,  sizeof(TData));
			// Send event to Service, when user process write buffer done
			hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, strEventPath);
			if (hEvent)
			{
				SetEvent(hEvent);
				CloseHandle(hEvent);
				bRet = true;
			}
		}
	}
	return bRet;
}


void* __stdcall ADL_Main_Memory_Alloc(int iSize_)
{
	void* lpBuffer = malloc(iSize_);
	return lpBuffer;
}
ADL_CONTEXT_HANDLE GetADLContext()
{
	return ADLContext_;
}
bool InitADL(void)
{
	bool result = ADL_ERR;

	//try to load ADL (32 or 64 bit)
	HDll_ = LoadLibrary(L"atiadlxx.dll");
	if (NULL == HDll_)
	{
		HDll_ = LoadLibrary(L"atiadlxy.dll");
	}

	// If successful: find the ADL function pointers
	if (NULL != HDll_)
	{
		//To get the functions of ADL (mandatory ones)
		ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(HDll_, "ADL2_Main_Control_Create");
		ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(HDll_, "ADL2_Main_Control_Destroy");
		ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(HDll_, "ADL2_Adapter_NumberOfAdapters_Get");
		ADL2_Adapter_Primary_Get = (ADL2_ADAPTER_PRIMARY_GET)GetProcAddress(HDll_, "ADL2_Adapter_Primary_Get");
		ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(HDll_, "ADL2_Adapter_AdapterInfo_Get");
		ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(HDll_, "ADL2_Adapter_Active_Get");
		// TODO: Add your ADL call point to get process address
		ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS)GetProcAddress(HDll_, "ADL2_Overdrive_Caps");
		ADL2_OverdriveN_CapabilitiesX2_Get = (ADL2_OVERDRIVEN_CAPABILITIESX2_GET)GetProcAddress(HDll_, "ADL2_OverdriveN_CapabilitiesX2_Get");
		ADL2_OverdriveN_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(HDll_, "ADL2_OverdriveN_FanControl_Get");
		if (nullptr == ADL2_Main_Control_Create ||
			nullptr == ADL2_Main_Control_Destroy ||
			nullptr == ADL2_Adapter_NumberOfAdapters_Get ||
			nullptr == ADL2_Adapter_Primary_Get ||
			nullptr == ADL2_Adapter_AdapterInfo_Get ||
			nullptr == ADL2_Adapter_Active_Get ||
			nullptr == ADL2_Overdrive_Caps||
			nullptr == ADL2_OverdriveN_CapabilitiesX2_Get ||
			nullptr == ADL2_OverdriveN_FanControl_Get
			
			)
		{
			MessageBox(NULL, L"ADL's API is missing", L"UserApp", MB_OK);
			return ADL_ERR;
		}
		if (ADL_OK != ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &ADLContext_))
		{
			printf("Failed to initialize ADL2 context");
			return ADL_ERR;
		}
		result = ADL_OK;
	}
	else
	{
		MessageBox(NULL, L"atiadlxx.dll load failed.", L"UserApp", MB_OK);

		return ADL_ERR;
	}

	return result;
}
//Destroy ADL. ADL calls can't be called after the method is invoked 
void DestroyADL()
{
	// call ADL main control destroy
	if (NULL != ADL2_Main_Control_Destroy)
		ADL2_Main_Control_Destroy(ADLContext_);

	// unload ADL library
	if (HDll_)
		FreeLibrary(HDll_);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _APP_DEBUG
	bool bSleep = true;
	while (bSleep)
		Sleep(100);
#endif

	//MessageBoxW(NULL, L"runnging", L"UMP", MB_OK);
	// Get ADL call Function in command line 
	BOOL bRet = FALSE;
	std::wstring strFunctionName = GetCommandLine();//L"FunctionName=GetAdapterInfo";//
	int nSubLength = strFunctionName.find(ADL_FUNCTION_NAME);
	if (-1 == nSubLength)
		return FALSE;
	strFunctionName = strFunctionName.substr(nSubLength + ADL_FUNCTION_NAME_LEN);
	swprintf_s(pstrFunctionName, L"%s", strFunctionName.c_str());
	// Initialization ADL
	if(ADL_OK != InitADL())
		return FALSE;
	// TODO: Please add your ADL call function in the following line
	if (-1 != strFunctionName.find(ExADLODNCapabilitiesX2::GetFunctionName()))
	{
		bRet = MemoryCommunication<ExADLODNCapabilitiesX2>();
	}
	else if (-1 != strFunctionName.find(ExADLOverdriveCaps::GetFunctionName()))
	{
		bRet = MemoryCommunication<ExADLOverdriveCaps>();
	}
	else if (-1 != strFunctionName.find(ExADLODNFanControl::GetFunctionName()))
	{
		bRet = MemoryCommunication<ExADLODNFanControl>();
	}
	else if (-1 != strFunctionName.find(ExADLAdapterInfo::GetFunctionName()))
	{
		bRet = MemoryCommunication<ExADLAdapterInfo>();
	}
	else
		goto CLEAN;
CLEAN:
	DestroyADL();
	return bRet;
}
