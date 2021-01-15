// RunningApps.cpp : Defines the entry point for the console application.
//

// Not tested on __linux__
// Do not use x86 sample build on x64 OS. Use x64 sample build on x64 OS.
// application under test (MyGame.exe) must be running when query is made for it to appear in ADL_PX40_INTEGRATED or ADL_PX40_DISCRETE
// application under test (MyGame.exe) will appear in ADL_PX40_INTEGRATED only for AMD integrated graphics
// application under test (MyGame.exe) will appear in ADL_PX40_DISCRETE only for AMD discrete graphics
// application under test (MyGame.exe) can request the high performance ADL_PX40_DISCRETE device using the following code:
//     extern "C" { _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; }

#include "stdafx.h"

#ifdef __linux__
#include "../../include/adl_sdk.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <stdlib.h>	
#include <string.h>	//memeset
#include <unistd.h>	//sleep

#elif _WIN32
#include <windows.h>
#include <wchar.h>
#include "..\..\..\include\adl_sdk.h"
#include "..\..\..\include\adl_defines.h"
#include "..\..\..\include\adl_structures.h"
#endif

#include <stdio.h>
#include <string>
using std::string;

// Definitions of the used function pointers. Add more if you use other ADL APIs.
typedef int(*ADL2_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)(ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET)(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET)(ADL_CONTEXT_HANDLE context, LPAdapterInfo lpInfo, int iInputSize);
typedef int(*ADL2_ADAPTER_ACTIVE_GET)(ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_SWITCHABLEGRAPHICS_APPLICATIONS_GET)(ADL_CONTEXT_HANDLE context, int iListType, int *lpNumApps, ADLSGApplicationInfo **lppAppList);
typedef int(*ADL2_ADAPTER_ASPECTS_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, char* lpAspects, int iSize);

ADL2_MAIN_CONTROL_CREATE			         ADL2_Main_Control_Create = nullptr;
ADL2_MAIN_CONTROL_DESTROY			         ADL2_Main_Control_Destroy = nullptr;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET	         ADL2_Adapter_NumberOfAdapters_Get = nullptr;
ADL2_ADAPTER_ADAPTERINFO_GET                 ADL2_Adapter_AdapterInfo_Get = nullptr;
ADL2_ADAPTER_ACTIVE_GET				         ADL2_Adapter_Active_Get = nullptr;
ADL2_SWITCHABLEGRAPHICS_APPLICATIONS_GET     ADL2_SwitchableGraphics_Applications_Get = nullptr;
ADL2_ADAPTER_ASPECTS_GET					 ADL2_Adapter_Aspects_Get = nullptr;

typedef enum _APP_LIST_TYPE
{
	RUNNING_APPS = 1,
	RECENT_RUNNING_APPS = 2,
} APP_LIST_TYPE;

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	void* lpBuffer = malloc(iSize);
	return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free(void* lpBuffer)
{
	if (NULL != lpBuffer)
	{
		free(lpBuffer);
		lpBuffer = NULL;
	}
}

int InitADL()
{

	HINSTANCE hDLL;		// Handle to DLL
	

	hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
	if (hDLL == NULL)
	{
		// A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
		// Try to load the 32 bit library (atiadlxy.dll) instead
		hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
	}

	if (NULL == hDLL)
	{
		fprintf(stderr, "ADL library not found! Please install atiadlxx.dll and atiadlxy.dll.\n");
		return ADL_ERR;
	}

	ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL2_Main_Control_Create");
	ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");
	ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL2_Adapter_NumberOfAdapters_Get");
	ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfo_Get");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
	ADL2_SwitchableGraphics_Applications_Get = (ADL2_SWITCHABLEGRAPHICS_APPLICATIONS_GET)GetProcAddress(hDLL, "ADL2_SwitchableGraphics_Applications_Get");
	ADL2_Adapter_Aspects_Get = (ADL2_ADAPTER_ASPECTS_GET)GetProcAddress(hDLL, "ADL2_Adapter_Aspects_Get");

	if (NULL == ADL2_Main_Control_Create ||
		NULL == ADL2_Main_Control_Destroy ||
		NULL == ADL2_Adapter_NumberOfAdapters_Get ||
		NULL == ADL2_Adapter_AdapterInfo_Get ||
		NULL == ADL2_Adapter_Active_Get ||
		NULL == ADL2_SwitchableGraphics_Applications_Get)
	{
		fprintf(stderr, "ADL APIs are missing!\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int printRunningApplications(ADL_CONTEXT_HANDLE context, int iListType)
{
	int numApps = 0;
	if (ADL2_SwitchableGraphics_Applications_Get)
	{
		ADLSGApplicationInfo* profiles = NULL;
		if (ADL_OK == ADL2_SwitchableGraphics_Applications_Get(context, iListType, &numApps, &profiles)) 
		{
			wprintf(L"\t[N] File Name, Path, Time, Version, GPU Affinity\n");
			for (int i = 0; i < numApps; ++i) 
			{
				ADLSGApplicationInfo profile = profiles[i];
				wprintf(L"File Name is %ls\n", profile.strFileName);
				wprintf(L"File path is %ls\n", profile.strFilePath);
				wprintf(L"GPU on which app is running is %ls\n", (profile.iGPUAffinity == 1) ? L"High Performance GPU" : L"Power Saving GPU");
			}
		}
	}
	return numApps;
}

int main(int c, char* k[], char* s[])
{
	if (ADL_OK != InitADL()) {
		wprintf(L"ADL initialization Failed\n");
		return 0;
	}

	ADL_CONTEXT_HANDLE context = NULL;
	if (ADL_OK == ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &context)) 
	{
		bool isPXPlatform = false;
		int iNumberAdapters = 0;
		if (ADL_OK == ADL2_Adapter_NumberOfAdapters_Get(context, &iNumberAdapters))
		{
			AdapterInfo* infos = new AdapterInfo[iNumberAdapters];
			if (ADL_OK == ADL2_Adapter_AdapterInfo_Get(context, infos, sizeof(AdapterInfo)*iNumberAdapters)) 
			{
				for (int i = 0; i < iNumberAdapters; ++i)
				{
					int active = 0;
					if (ADL_OK == ADL2_Adapter_Active_Get(context, i, &active))
					{

						if (TRUE || active) 
						{
							AdapterInfo info = infos[i];

							if (info.iVendorID == 1002)
							{
								if (ADL2_Adapter_Aspects_Get) 
								{
									int iSize = ADL_MAX_CHAR;
									char lpAspects[ADL_MAX_CHAR];

									char* result = NULL;
									if (ADL_OK == ADL2_Adapter_Aspects_Get(context, i, lpAspects, iSize)) 
									{
										result = _strdup(lpAspects);
										if (NULL != result)
										{
											char* thisAspect = NULL;
											char* nextAspect = NULL;

											thisAspect = strtok_s(result, ";,", &nextAspect);

											while (NULL != thisAspect)
											{
												string aspect = string(thisAspect);
												if (0 == aspect.compare("PowerXpress"))
												{
													isPXPlatform = true;
													break;
												}
												thisAspect = strtok_s(NULL, ";,", &nextAspect);
											}
										}
									}
								}
							}
						}
					}
					if (isPXPlatform)
						break;
				}
				
			}
		}

		if (isPXPlatform)
		{
			wprintf(L"Listing all the Running Applications:\n");
			printRunningApplications(context, RUNNING_APPS);
		}
		else
			wprintf(L"Unsupported platform was detected\n");

		ADL2_Main_Control_Destroy(context);
	}

	return 0;
}