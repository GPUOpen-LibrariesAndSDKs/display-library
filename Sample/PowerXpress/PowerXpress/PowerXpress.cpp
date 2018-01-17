// PowerXpress.cpp : Defines the entry point for the console application.
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
#include "../../include/customer/oem_structures.h"
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

// Definitions of the used function pointers. Add more if you use other ADL APIs.
typedef int(*ADL2_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)(ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET)(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_PRIMARY_GET)(ADL_CONTEXT_HANDLE context, int* lpPrimaryAdapterIndex);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET)(ADL_CONTEXT_HANDLE context, LPAdapterInfo lpInfo, int iInputSize);
typedef int(*ADL2_ADAPTER_ACTIVE_GET)(ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_GRAPHICS_VERSION_GET)(ADL_CONTEXT_HANDLE context, ADLVersionsInfo * lpVersionsInfo);
typedef int(*ADL2_GRAPHICS_VERSIONX2_GET)(ADL_CONTEXT_HANDLE context, ADLVersionsInfoX2 * lpVersionsInfo);
typedef int(*ADL2_ADAPTER_VIDEOBIOSINFO_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLBiosInfo *lpBiosInfo);
typedef int(*ADL2_DISPLAY_MODES_GET)(ADL_CONTEXT_HANDLE, int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);
typedef int(*ADL2_POWERXPRESS_ANCILLARYDEVICES_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpNumberOfAncillaryDevices, ADLBdf **lppAncillaryDevices);
typedef int(*ADL2_APPLICATIONPROFILES_HITLISTSX3_GET)(ADL_CONTEXT_HANDLE context, int iListType, int *lpNumApps, ADLApplicationDataX3 **lppAppList);
typedef int(*ADL2_ADAPTER_ASPECTS_GET)(ADL_CONTEXT_HANDLE context, int iAdapterIndex, char* lpAspects, int iSize);

ADL2_MAIN_CONTROL_CREATE			ADL2_Main_Control_Create = nullptr;
ADL2_MAIN_CONTROL_DESTROY			ADL2_Main_Control_Destroy = nullptr;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET	ADL2_Adapter_NumberOfAdapters_Get = nullptr;
ADL2_ADAPTER_PRIMARY_GET            ADL2_Adapter_Primary_Get = nullptr;
ADL2_ADAPTER_ADAPTERINFO_GET        ADL2_Adapter_AdapterInfo_Get = nullptr;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get = nullptr;
ADL2_GRAPHICS_VERSION_GET           ADL2_Graphics_Versions_Get = nullptr;
ADL2_GRAPHICS_VERSIONX2_GET         ADL2_Graphics_VersionsX2_Get = nullptr;
ADL2_ADAPTER_VIDEOBIOSINFO_GET      ADL2_Adapter_VideoBiosInfo_Get = nullptr;
ADL2_DISPLAY_MODES_GET				ADL2_Display_Modes_Get = nullptr;
ADL2_POWERXPRESS_ANCILLARYDEVICES_GET ADL2_PowerXpress_AncillaryDevices_Get = nullptr;
ADL2_APPLICATIONPROFILES_HITLISTSX3_GET ADL2_ApplicationProfiles_HitListsX3_Get = nullptr;
ADL2_ADAPTER_ASPECTS_GET ADL2_Adapter_Aspects_Get = nullptr;

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

#ifdef __linux__
// equivalent functions in linux
void * GetProcAddress(void * pLibrary, const char * name)
{
	return dlsym(pLibrary, name);
}

#endif

int InitADL()
{
#ifdef __linux__
	void *hDLL;		// Handle to .so library
#else
	HINSTANCE hDLL;		// Handle to DLL
#endif

#ifdef __linux__
	hDLL = dlopen("libatiadlxx.so", RTLD_LAZY | RTLD_GLOBAL);
#else
	hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
	if (hDLL == NULL)
		// A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
		// Try to load the 32 bit library (atiadlxy.dll) instead
		hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
#endif

	if (NULL == hDLL)
	{
		fprintf(stderr, "ADL library not found! Please install atiadlxx.dll and atiadlxy.dll.\n");
		return ADL_ERR;
	}

	ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL2_Main_Control_Create");
	ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");
	ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL2_Adapter_NumberOfAdapters_Get");
	ADL2_Adapter_Primary_Get = (ADL2_ADAPTER_PRIMARY_GET)GetProcAddress(hDLL, "ADL2_Adapter_Primary_Get");
	ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfo_Get");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
	ADL2_Adapter_VideoBiosInfo_Get = (ADL2_ADAPTER_VIDEOBIOSINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_VideoBiosInfo_Get");
	ADL2_Graphics_Versions_Get = (ADL2_GRAPHICS_VERSION_GET)GetProcAddress(hDLL, "ADL2_Graphics_Versions_Get");
	ADL2_Graphics_VersionsX2_Get = (ADL2_GRAPHICS_VERSIONX2_GET)GetProcAddress(hDLL, "ADL2_Graphics_VersionsX2_Get");
	ADL2_Display_Modes_Get = (ADL2_DISPLAY_MODES_GET)GetProcAddress(hDLL, "ADL2_Display_Modes_Get");
	ADL2_PowerXpress_AncillaryDevices_Get = (ADL2_POWERXPRESS_ANCILLARYDEVICES_GET)GetProcAddress(hDLL, "ADL2_PowerXpress_AncillaryDevices_Get");
	ADL2_ApplicationProfiles_HitListsX3_Get = (ADL2_APPLICATIONPROFILES_HITLISTSX3_GET)GetProcAddress(hDLL, "ADL2_ApplicationProfiles_HitListsX3_Get");
	ADL2_Adapter_Aspects_Get = (ADL2_ADAPTER_ASPECTS_GET)GetProcAddress(hDLL, "ADL2_Adapter_Aspects_Get");

	if (NULL == ADL2_Main_Control_Create ||
		NULL == ADL2_Main_Control_Destroy ||
		NULL == ADL2_Adapter_NumberOfAdapters_Get ||
		NULL == ADL2_Adapter_Primary_Get ||
		NULL == ADL2_Adapter_AdapterInfo_Get ||
		NULL == ADL2_Adapter_Active_Get ||
		NULL == ADL2_Adapter_VideoBiosInfo_Get ||
		NULL == ADL2_Display_Modes_Get
		)
	{
		fprintf(stderr, "ADL APIs are missing!\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int printApplicationProfilesX3(ADL_CONTEXT_HANDLE context, int iListType) {
	int numApps = 0;
	if (ADL2_ApplicationProfiles_HitListsX3_Get) {
		ADLApplicationDataX3* profiles = NULL;
		if (ADL_OK == ADL2_ApplicationProfiles_HitListsX3_Get(context, iListType, &numApps, &profiles)) {
			wprintf(L"\t[N] File,Path,Time,Version,ProcessId\n");
			for (int i = 0; i < numApps; ++i) {
				ADLApplicationDataX3 profile = profiles[i];
				wprintf(L"\t[%i] %s,%s,%s,%s,%08x\n",
					i,
					profile.strFileName,
					profile.strPathName,
					profile.strTimeStamp,
					profile.strVersion,
					profile.iProcessId
				);
			}
		}
	}
	return numApps;
}

int main(int c, char* k[], char* s[])
{
	if (ADL_OK != InitADL()) {
		fprintf(stderr, "InitADL FAILED\n");
		return 0;
	}

	ADL_CONTEXT_HANDLE context = NULL;
	if (ADL_OK == ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &context)) {

		int iNumberAdapters = 0;
		if (ADL_OK == ADL2_Adapter_NumberOfAdapters_Get(context, &iNumberAdapters)) {
			int primary = -1;
			ADL2_Adapter_Primary_Get(context, &primary);

			AdapterInfo* infos = new AdapterInfo[iNumberAdapters];
			if (ADL_OK == ADL2_Adapter_AdapterInfo_Get(context, infos, sizeof(AdapterInfo)*iNumberAdapters)) {

				for (int i = 0; i < iNumberAdapters; ++i) {
					int active = 0;
					if (ADL_OK == ADL2_Adapter_Active_Get(context, i, &active)) {

						if (TRUE || active) {
							AdapterInfo info = infos[i];
							printf("Adapter[%i]\n"
								"\tAdapter Index: %i\n"
								"\tUDID: %s\n"
								"\tLocation: PCI bus %i, device %i, function %i\n"
								"\tVendorID: %04x\n"
								"\tAdapter Name: %s\n"
								"\tDisplay Name: %s\n"
								"\tPresent: %s\n",
								i,
								info.iAdapterIndex,
								info.strUDID,
								info.iBusNumber,
								info.iDeviceNumber,
								info.iFunctionNumber,
								info.iVendorID,
								info.strAdapterName,
								info.strDisplayName,
								(info.iPresent) ? "yes" : "no"
							);
#ifdef _WIN32							
							printf(
								"\tExist: %s\n"
								"\tDriver Path %s\n"
								"\tDriver Path X: %s\n"
								"\tPNP String: %s\n"
								"\tOS Display Index: %i\n",
								(info.iExist) ? "yes" : "no",
								info.strDriverPath,
								info.strDriverPathExt,
								info.strPNPString,
								info.iOSDisplayIndex
							);
#endif
							printf(
								"\tPrimary: %s\n"
								"\tActive: %s\n",
								(primary == i) ? "yes" : "no",
								(active) ? "yes" : "no"
							);

							if (ADL2_Graphics_VersionsX2_Get) {
								ADLVersionsInfoX2 versionsInfo;
								int ADLResult = ADL2_Graphics_VersionsX2_Get(context, &versionsInfo);
								if (ADL_OK == ADLResult || ADL_OK_WARNING == ADLResult) {
									printf("\tVersions\n"
										"\t\tCrimsonVersion: %s\n"
										"\t\tCatalystVersion: %s\n"
										"\t\tCatalystWebLink: %s\n"
										"\t\tDriverVersion: %s\n",
										versionsInfo.strCrimsonVersion,
										versionsInfo.strCatalystVersion,
										versionsInfo.strCatalystWebLink,
										versionsInfo.strDriverVer
									);
								}
							}
							else if (ADL2_Graphics_Versions_Get) {
								ADLVersionsInfo versionsInfo;
								int ADLResult = ADL2_Graphics_Versions_Get(context, &versionsInfo);
								if (ADL_OK == ADLResult || ADL_OK_WARNING == ADLResult) {
									printf("\tVersions\n"
										"\t\tCatalystVersion: %s\n"
										"\t\tDriverVersion: %s\n",
										versionsInfo.strCatalystVersion,
										versionsInfo.strDriverVer
									);
								}
							}

							ADLBiosInfo biosInfo;
							if (ADL_OK == ADL2_Adapter_VideoBiosInfo_Get(context, i, &biosInfo)) {
								printf("\tBIOS\n"
									"\t\tPart#: %s\n"
									"\t\tVersion: %s\n"
									"\t\tDate: %s\n",
									biosInfo.strPartNumber,
									biosInfo.strVersion,
									biosInfo.strDate
								);
							}

							int numModes;
							ADLMode* adlMode;
							if (ADL_OK == ADL2_Display_Modes_Get(context, i, -1, &numModes, &adlMode)) {
								printf("\tDisplay Modes\n");
								if (numModes == 1) {
									printf("\t\t%dx%d\n", adlMode->iXRes, adlMode->iYRes);
									ADL_Main_Memory_Free(adlMode);
								}
							}

							if (ADL2_Adapter_Aspects_Get) {								
								int iSize = ADL_MAX_CHAR;
								char lpAspects[ADL_MAX_CHAR];
								if (ADL_OK == ADL2_Adapter_Aspects_Get(context, i, lpAspects, iSize)) {
									printf("\tAspects: %s\n",lpAspects);
								}
							}							

							if (ADL2_PowerXpress_AncillaryDevices_Get) {
								int  numberOfAncillaryDevices = 0;
								ADLBdf* lpAncillaryDevices = nullptr;
								if (ADL_OK == ADL2_PowerXpress_AncillaryDevices_Get(context, i, &numberOfAncillaryDevices, &lpAncillaryDevices)) {
									printf("\tPowerXpress Ancillary Devices\n");
									for (int i = 0; i < numberOfAncillaryDevices; ++i) {
										ADLBdf bdf = lpAncillaryDevices[i];
										printf("\t\tLocation: PCI bus %i, device %i, function %i\n", bdf.iBus, bdf.iDevice, bdf.iFunction);
									}
								}
							}
						}
					}
				}
			}
			delete[] infos;
		}

		{
			printf("ADL_PX40_MRU\n");
			printApplicationProfilesX3(context, ADL_PX40_MRU);

			printf("ADL_PX40_MISSED\n");
			printApplicationProfilesX3(context, ADL_PX40_MISSED);

			printf("ADL_PX40_DISCRETE\n");
			printApplicationProfilesX3(context, ADL_PX40_DISCRETE);

			printf("ADL_PX40_INTEGRATED\n");
			printApplicationProfilesX3(context, ADL_PX40_INTEGRATED);

			printf("ADL_MMD_PROFILED\n");
			printApplicationProfilesX3(context, ADL_MMD_PROFILED);

			printf("ADL_PX40_TOTAL\n");
			printApplicationProfilesX3(context, ADL_PX40_TOTAL);
		}

		ADL2_Main_Control_Destroy(context);
	}

	return 0;
}