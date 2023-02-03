///  Copyright (c) 2018 - 2022 Advanced Micro Devices, Inc.
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///
/// \file Adl.cpp
/// \brief
///This sample demonstrates usage of ADL APIs


#include "stdafx.h"
#include "Adl.h"
#include <windows.h>
#include <iostream>
#include <codecvt>

//Defining pointers to ADL Methods.
ADL2_MAIN_CONTROL_CREATE                                            ADL2_Main_Control_Create = nullptr;
ADL2_MAIN_CONTROLX2_CREATE                                          ADL2_Main_ControlX2_Create = nullptr;
ADL2_MAIN_CONTROL_DESTROY                                           ADL2_Main_Control_Destroy = nullptr;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET                                   ADL2_Adapter_NumberOfAdapters_Get = nullptr;
ADL2_ADAPTER_PRIMARY_GET                                            ADL2_Adapter_Primary_Get = nullptr;
ADL2_ADAPTER_ADAPTERINFO_GET                                        ADL2_Adapter_AdapterInfo_Get = nullptr;
ADL2_ADAPTER_ACTIVE_GET                                             ADL2_Adapter_Active_Get = nullptr;
ADL2_ADAPTER_REGVALUESTRING_SET										ADL2_Adapter_RegValueString_Set = nullptr;
ADL2_ADAPTER_REGVALUESTRING_GET										ADL2_Adapter_RegValueString_Get = nullptr;
ADL2_ADAPTER_REGVALUEINT_SET                                        ADL2_Adapter_RegValueInt_Set = nullptr;
ADL2_ADAPTER_REGVALUEINT_GET                                        ADL2_Adapter_RegValueInt_Get = nullptr;
ADL2_DFP_GPUSCALINGENABLE_SET                                       ADL2_DFP_GPUScalingEnable_Set = nullptr;
ADL2_DFP_GPUSCALINGENABLE_GET                                       ADL2_DFP_GPUScalingEnable_Get = nullptr;
ADL2_DISPLAY_PRESERVEDASPECTRATIO_SET                               ADL2_Display_PreservedAspectRatio_Set = nullptr;
ADL2_DISPLAY_PRESERVEDASPECTRATIO_GET								ADL2_Display_PreservedAspectRatio_Get = nullptr;
ADL2_DISPLAY_IMAGEEXPANSION_SET                                     ADL2_Display_ImageExpansion_Set = nullptr;
ADL2_DISPLAY_IMAGEEXPANSION_GET                                     ADL2_Display_ImageExpansion_Get = nullptr;
ADL2_DISPLAY_DISPLAYINFO_GET                                        ADL2_Display_DisplayInfo_Get = nullptr;
ADL2_ADAPTER_ADAPTERINFOX4_GET                                      ADL2_Adapter_AdapterInfoX4_Get = nullptr;


ADL2_DISPLAY_FREESYNCSTATE_GET										ADL2_Display_FreeSyncState_Get = nullptr;
ADL2_DISPLAY_FREESYNCSTATE_SET										ADL2_Display_FreeSyncState_Set = nullptr;

ADL2_DISPLAY_PROPERTY_GET											ADL2_Display_Property_Get = nullptr;
ADL2_DISPLAY_PROPERTY_SET											ADL2_Display_Property_Set = nullptr;

ADL2_DISPLAY_COLORDEPTH_GET											ADL2_Display_ColorDepth_Get = nullptr;
ADL2_DISPLAY_COLORDEPTH_SET											ADL2_Display_ColorDepth_Set = nullptr;
ADL2_DISPLAY_SUPPORTEDCOLORDEPTH_GET								ADL2_Display_SupportedColorDepth_Get = nullptr;
ADL2_DISPLAY_PIXELFORMAT_GET										ADL2_Display_PixelFormat_Get = nullptr;
ADL2_DISPLAY_PIXELFORMAT_SET										ADL2_Display_PixelFormat_Set = nullptr;
ADL2_DISPLAY_SUPPORTEDPIXELFORMAT_GET								ADL2_Display_SupportedPixelFormat_Get = nullptr;
ADL2_DISPLAY_PIXELFORMATDEFAULT_GET									ADL2_Display_PixelFormatDefault_Get = nullptr;
ADL2_DISPLAY_COLORTEMPERATURESOURCE_GET								ADL2_Display_ColorTemperatureSource_Get = nullptr;
ADL2_DISPLAY_COLOR_GET												ADL2_Display_Color_Get = nullptr;
ADL2_DISPLAY_COLOR_SET												ADL2_Display_Color_Set = nullptr;
ADL2_DISPLAY_COLORTEMPERATURESOURCE_SET								ADL2_Display_ColorTemperatureSource_Set = nullptr;
ADL2_CHILL_SETTINGS_SET												ADL2_Chill_Settings_Set = nullptr;
ADL2_CHILL_SETTINGS_GET												ADL2_Chill_Settings_Get = nullptr;

ADL2_TURBOSYNCSUPPORT_GET											ADL2_TurboSyncSupport_Get = nullptr;

// ADL module handle
HINSTANCE hDLL = NULL;
//ADL_CONTEXT_HANDLE context_ = NULL;

//Memory allocating callback for ADL
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

//Memory freeing callback for ADL
void __stdcall ADL_Main_Memory_Free(void** lpBuffer_)
{
    if (NULL != lpBuffer_ && NULL != *lpBuffer_)
    {
        free(*lpBuffer_);
        *lpBuffer_ = NULL;
    }
}

static void Get_All_DisplayInfo(int adapterIndex);
static void CleanUP();
static bool ADL_Inits(ADLApp_Context *pAppCtx);

// Global data
ADLApp_Context ADLApp_context;
unordered_map <int, ADLDisplayID*> DisplayID;

//Initialize ADL. Retrieves and initialize ADL API pointers.
//Returns false if ADL initialization failed or some of the expected ADL APIs can't be found
int InitADL()
{
    int result = ADL_OK;
    //SetDllDirectoryW(L"..\\lib");
    //Try to load ADL
    hDLL = LoadLibrary(L"atiadlxx.dll");
    if (hDLL == NULL)
    {
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary. Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary(L"atiadlxy.dll");
    }

    if (NULL != hDLL)
    {
		//To get the functions of ADL (mandatory ones)
		ADL2_Main_ControlX2_Create = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(hDLL, "ADL2_Main_ControlX2_Create");
		ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL2_Adapter_NumberOfAdapters_Get");
		ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL2_Main_Control_Create");
		ADL2_Main_ControlX2_Create = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(hDLL, "ADL2_Main_ControlX2_Create");
		ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");
		ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL2_Adapter_NumberOfAdapters_Get");
		ADL2_Adapter_Primary_Get = (ADL2_ADAPTER_PRIMARY_GET)GetProcAddress(hDLL, "ADL2_Adapter_Primary_Get");
		ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfo_Get");
		ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
		ADL2_Adapter_RegValueInt_Get = (ADL2_ADAPTER_REGVALUEINT_GET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueInt_Get");
		ADL2_Adapter_RegValueInt_Set = (ADL2_ADAPTER_REGVALUEINT_SET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueInt_Set");
		ADL2_Adapter_RegValueString_Get = (ADL2_ADAPTER_REGVALUESTRING_GET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueString_Get");
		ADL2_Adapter_RegValueString_Set = (ADL2_ADAPTER_REGVALUESTRING_SET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueString_Set");

		ADL2_DFP_GPUScalingEnable_Set = (ADL2_DFP_GPUSCALINGENABLE_SET)GetProcAddress(hDLL, "ADL2_DFP_GPUScalingEnable_Set");
		ADL2_DFP_GPUScalingEnable_Get = (ADL2_DFP_GPUSCALINGENABLE_GET)GetProcAddress(hDLL, "ADL2_DFP_GPUScalingEnable_Get");
		ADL2_Display_PreservedAspectRatio_Set = (ADL2_DISPLAY_PRESERVEDASPECTRATIO_SET)GetProcAddress(hDLL, "ADL2_Display_PreservedAspectRatio_Set");
		ADL2_Display_PreservedAspectRatio_Get = (ADL2_DISPLAY_PRESERVEDASPECTRATIO_GET)GetProcAddress(hDLL, "ADL2_Display_PreservedAspectRatio_Get");
		ADL2_Display_ImageExpansion_Set = (ADL2_DISPLAY_IMAGEEXPANSION_SET)GetProcAddress(hDLL, "ADL2_Display_ImageExpansion_Set");
		ADL2_Display_ImageExpansion_Get = (ADL2_DISPLAY_IMAGEEXPANSION_GET)GetProcAddress(hDLL, "ADL2_Display_ImageExpansion_Get");
		ADL2_Display_DisplayInfo_Get = (ADL2_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hDLL, "ADL2_Display_DisplayInfo_Get");
		ADL2_Adapter_AdapterInfoX4_Get = (ADL2_ADAPTER_ADAPTERINFOX4_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfoX4_Get");

        ADL2_Display_FreeSyncState_Get = (ADL2_DISPLAY_FREESYNCSTATE_GET)GetProcAddress(hDLL, "ADL2_Display_FreeSyncState_Get");
        ADL2_Display_FreeSyncState_Set = (ADL2_DISPLAY_FREESYNCSTATE_SET)GetProcAddress(hDLL, "ADL2_Display_FreeSyncState_Set");

		ADL2_Display_Property_Get = (ADL2_DISPLAY_PROPERTY_GET)GetProcAddress(hDLL, "ADL2_Display_Property_Get");
		ADL2_Display_Property_Set = (ADL2_DISPLAY_PROPERTY_SET)GetProcAddress(hDLL, "ADL2_Display_Property_Set");

        ADL2_Display_ColorDepth_Get = (ADL2_DISPLAY_COLORDEPTH_GET)GetProcAddress(hDLL, "ADL2_Display_ColorDepth_Get");
        ADL2_Display_ColorDepth_Set = (ADL2_DISPLAY_COLORDEPTH_SET)GetProcAddress(hDLL, "ADL2_Display_ColorDepth_Set");

        ADL2_Display_PixelFormat_Get = (ADL2_DISPLAY_PIXELFORMAT_GET)GetProcAddress(hDLL, "ADL2_Display_PixelFormat_Get");
        ADL2_Display_PixelFormat_Set = (ADL2_DISPLAY_PIXELFORMAT_SET)GetProcAddress(hDLL, "ADL2_Display_PixelFormat_Set");
        ADL2_Display_ColorTemperatureSource_Get = (ADL2_DISPLAY_COLORTEMPERATURESOURCE_GET)GetProcAddress(hDLL, "ADL2_Display_ColorTemperatureSource_Get");
        ADL2_Display_Color_Get = (ADL2_DISPLAY_COLOR_GET)GetProcAddress(hDLL, "ADL2_Display_Color_Get");
        ADL2_Display_Color_Set = (ADL2_DISPLAY_COLOR_SET)GetProcAddress(hDLL, "ADL2_Display_Color_Set");
        ADL2_Display_ColorTemperatureSource_Set = (ADL2_DISPLAY_COLORTEMPERATURESOURCE_SET)GetProcAddress(hDLL, "ADL2_Display_ColorTemperatureSource_Set");
        ADL2_Chill_Settings_Set = (ADL2_CHILL_SETTINGS_SET)GetProcAddress(hDLL, "ADL2_Chill_Settings_Set");
        ADL2_Chill_Settings_Get = (ADL2_CHILL_SETTINGS_GET)GetProcAddress(hDLL, "ADL2_Chill_Settings_Get");

		ADL2_TurboSyncSupport_Get = (ADL2_TURBOSYNCSUPPORT_GET)GetProcAddress(hDLL, "ADL2_TurboSyncSupport_Get");

		if (nullptr == ADL2_Main_Control_Create ||
			nullptr == ADL2_Main_ControlX2_Create ||
			nullptr == ADL2_Main_Control_Destroy ||
			nullptr == ADL2_Adapter_NumberOfAdapters_Get ||
			nullptr == ADL2_Adapter_Primary_Get ||
			nullptr == ADL2_Adapter_AdapterInfo_Get ||
			nullptr == ADL2_Adapter_Active_Get ||
			nullptr == ADL2_Adapter_RegValueString_Set ||
			nullptr == ADL2_Adapter_RegValueString_Get ||
			nullptr == ADL2_Adapter_RegValueInt_Get ||
			nullptr == ADL2_Adapter_RegValueInt_Set ||
			nullptr == ADL2_DFP_GPUScalingEnable_Set ||
			nullptr == ADL2_DFP_GPUScalingEnable_Get ||
			nullptr == ADL2_Display_PreservedAspectRatio_Set ||
			nullptr == ADL2_Display_PreservedAspectRatio_Get ||
			nullptr == ADL2_Display_ImageExpansion_Set ||
			nullptr == ADL2_Display_ImageExpansion_Get ||
			nullptr == ADL2_Display_DisplayInfo_Get ||
			nullptr == ADL2_Adapter_AdapterInfoX4_Get ||
			nullptr == ADL2_Display_Property_Get ||
			nullptr == ADL2_Display_Property_Set ||
			nullptr == ADL2_Display_FreeSyncState_Get ||
			nullptr == ADL2_Display_FreeSyncState_Set ||
            nullptr == ADL2_Display_ColorDepth_Get ||
            nullptr == ADL2_Display_ColorDepth_Set ||
            nullptr == ADL2_Display_PixelFormat_Get ||
            nullptr == ADL2_Display_PixelFormat_Set ||
            nullptr == ADL2_Display_ColorTemperatureSource_Get ||
            nullptr == ADL2_Display_Color_Get ||
            nullptr == ADL2_Display_Color_Set ||
            nullptr == ADL2_Display_ColorTemperatureSource_Set ||
            nullptr == ADL2_Chill_Settings_Set ||
            nullptr == ADL2_Chill_Settings_Get ||
			nullptr == ADL2_TurboSyncSupport_Get
        )
        {
            std::cout << "ADL's API is missing!" << std::endl;
            return ADL_ERR;
        }

/*        if (ADL_OK != ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &context_))
        {
            std::cout << "Failed to initialize ADL2 context" << std::endl;
            return ADL_ERR;
        }*/
    }

    return result;
}

//Destroy ADL. ALD calls can't be called after the method is invoked;
void DestroyADL()
{
	CleanUP();

    if (NULL != ADL2_Main_Control_Destroy)
    {
        ADL2_Main_Control_Destroy(ADLApp_context.adlContext);
    }

    if (hDLL)
    {
        FreeLibrary(hDLL);
    }
}

void PrepareAPI()
{
	ADL_Inits(&ADLApp_context);
	for (int i : ADLApp_context.vecAciveAdapter) {
		Get_All_DisplayInfo(i);
	}
}

static void Get_All_DisplayInfo(int adapterIndex)
{
	int numDisplays = 0;
	ADLDisplayInfo* allDisplaysBuffer = NULL;

	if (ADL_OK == ADL2_Display_DisplayInfo_Get(ADLApp_context.adlContext, adapterIndex, &numDisplays, &allDisplaysBuffer, 1)) {
		for (int i = 0; i < numDisplays; i++) {
			ADLDisplayInfo* oneDis = &(allDisplaysBuffer[i]);
			if ((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED == (oneDis->iDisplayInfoValue & ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED)) &&
				(oneDis->displayID.iDisplayLogicalAdapterIndex >= 0)) {
				ADLDisplayID *display = new ADLDisplayID(oneDis->displayID);
				DisplayID.insert(std::pair<int, ADLDisplayID*>(i, display));
			}
		}
	}

	ADL_Main_Memory_Free((void**)&allDisplaysBuffer);
}

static void CleanUP()
{
	if (DisplayID.size() > 0) {
		for (auto it = DisplayID.begin(); it != DisplayID.end(); it++)
			delete it->second;

		DisplayID.clear();
	}
}

static bool ADL_Inits(ADLApp_Context *pAppCtx)
{
	int adlStatus = ADL_OK;
	int numAdapters = 0;

	/* Initialize ADL interface */
	adlStatus = ADL2_Main_ControlX2_Create(ADL_Main_Memory_Alloc,
		0,
		&pAppCtx->adlContext,
		ADL_THREADING_LOCKED);
	if (adlStatus != ADL_OK)
	{
		/*printf("Unable to Initialize ADL. Error code = %s\n",
			GetADLErrorStr(adlStatus));*/
	}

	printf("ADL initialization successful.\n");

	/* Get number of graphics adapters */
	AdapterInfoX2* allAdapterInfoX2 = NULL;
	adlStatus = ADL2_Adapter_AdapterInfoX4_Get(pAppCtx->adlContext, -1, &numAdapters, &allAdapterInfoX2);

	if ((adlStatus != ADL_OK) || (numAdapters < 0))
	{
		/*printf("Unable to get Adapter count. Error code = %s",
			GetADLErrorStr(adlStatus));*/
	}

	//printf("Number of adapters available :%d", numAdapters);

	/* Get the active adapter index */
	for (int i = 0; i < numAdapters; i++) {

		int nAdapterStatus = -1;
		int nAdapterIdx = allAdapterInfoX2[i].iAdapterIndex;

		int ret = ADL2_Adapter_Active_Get(pAppCtx->adlContext, nAdapterIdx, &nAdapterStatus);
		if (ADL_OK == ret && nAdapterStatus == 1) {
			pAppCtx->vecAciveAdapter.push_back(nAdapterIdx);
		}
	}
	ADL_Main_Memory_Free((void**)&allAdapterInfoX2);
	return true;
}
