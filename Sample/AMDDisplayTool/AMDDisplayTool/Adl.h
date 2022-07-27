#ifndef _DOPP_ADL_H_
#define _DOPP_ADL_H_
///  Copyright (c) 2019 - 2022 Advanced Micro Devices, Inc.
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///
/// \file Adl.h
/// \brief 
///This sample demonstrates usage of ADL APIs to achieve following task:
///1. 


//ADL_INCLUDE_DIR environment variable has to be set to location of ADL SDK header files in order to successfuly include adl headers.
#include "..\..\..\include\adl_sdk.h"
#include <vector>
#include <unordered_map>
using namespace std;

//Declaring ADL Methods. 
typedef int(*ADL2_MAIN_CONTROL_CREATE)								(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROLX2_CREATE)							(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*, ADLThreadingModel);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)								(ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET)						(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_PRIMARY_GET)								(ADL_CONTEXT_HANDLE, int* lpPrimaryAdapterIndex);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET)							(ADL_CONTEXT_HANDLE, LPAdapterInfo lpInfo, int iInputSize);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) 								(ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_DISPLAY_DISPLAYINFO_GET)							(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplays, ADLDisplayInfo ** lppInfo, int iForceDetect);
typedef int(*ADL2_ADAPTER_ADAPTERINFOX4_GET)						(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfoX2** lppAdapterInfo);

//DisplayScalling ADL2 calls
typedef int(*ADL2_DFP_GPUSCALINGENABLE_GET)							(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpSupport, int *lpCurrent, int *lpDefault);
typedef int(*ADL2_DFP_GPUSCALINGENABLE_SET)							(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iCurrent);
typedef int(*ADL2_DISPLAY_PRESERVEDASPECTRATIO_GET)                    (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int* lpSupport, int * lpCurrent, int * lpDefault);
typedef int(*ADL2_DISPLAY_PRESERVEDASPECTRATIO_SET)                    (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iCurrent);
typedef int(*ADL2_DISPLAY_IMAGEEXPANSION_GET)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int* lpSupport, int * lpCurrent, int * lpDefault);
typedef int(*ADL2_DISPLAY_IMAGEEXPANSION_SET)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iCurrent);

//Color Depth
typedef int(*ADL2_DISPLAY_COLORDEPTH_GET)                           (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpColorDepth);
typedef int(*ADL2_DISPLAY_COLORDEPTH_SET)                           (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iColorDepth);
typedef int(*ADL2_DISPLAY_SUPPORTEDCOLORDEPTH_GET)                  (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpColorDepth);

//Pixel Format
typedef int(*ADL2_DISPLAY_PIXELFORMAT_GET)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpPixelFormat);
typedef int(*ADL2_DISPLAY_PIXELFORMAT_SET)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iPixelFormat);
typedef int(*ADL2_DISPLAY_SUPPORTEDPIXELFORMAT_GET)                 (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpPixelFormat);
typedef int(*ADL2_DISPLAY_PIXELFORMATDEFAULT_GET)                   (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpDefPixelFormat);

//FreeSync ADL2 calls
typedef int(*ADL2_DISPLAY_FREESYNCSTATE_GET)                        (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpCurrent, int *lpDefault, int *lpMinRefreshRateInMicroHz, int *lpMaxRefreshRateInMicroHz);
typedef int(*ADL2_DISPLAY_FREESYNCSTATE_SET)                        (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iSetting, int iRefreshRateInMicroHz);

//Virtual Resolution ADL2 calls
typedef int(*ADL2_DISPLAY_PROPERTY_GET)                             (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);
typedef int(*ADL2_DISPLAY_PROPERTY_SET)                             (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);

//color temperature
typedef int(*ADL2_DISPLAY_COLORTEMPERATURESOURCE_GET)               (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int *lpSource);
typedef int(*ADL2_DISPLAY_COLOR_GET)                                (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iColorType, int* iCurrent, int* iDefault, int* iMin, int* iMax, int* iStep);
typedef int(*ADL2_DISPLAY_COLOR_SET)                                (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iColorType, int iCurrent);
typedef int(*ADL2_DISPLAY_COLORTEMPERATURESOURCE_SET)               (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, int iSource);

//CHILL
typedef int(*ADL2_CHILL_SETTINGS_SET)                               (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iEnabled);
typedef int(*ADL2_CHILL_SETTINGS_GET)                               (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpEnabled);

// - The GPU supports Turbo Sync
typedef int(*ADL2_TURBOSYNCSUPPORT_GET)                             (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iTurboSyncSupported);

typedef int(*ADL2_ADAPTER_REGVALUESTRING_SET)                       (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, const char* szSubKey, const char *szKeyName, int iSize, char *lpKeyValue);
typedef int(*ADL2_ADAPTER_REGVALUESTRING_GET)                       (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, const char* szSubKey, const char *szKeyName, int iSize, char *lpKeyValue);
typedef int(*ADL2_ADAPTER_REGVALUEINT_SET)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, const char* szSubKey, const char *szKeyName, int lpKeyValue);
typedef int(*ADL2_ADAPTER_REGVALUEINT_GET)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, const char* szSubKey, const char *szKeyName, int *lpKeyValue);


//Declaring pointers to ADL Methods. 
extern ADL2_MAIN_CONTROL_CREATE                 ADL2_Main_Control_Create;
extern ADL2_MAIN_CONTROLX2_CREATE               ADL2_Main_ControlX2_Create;
extern ADL2_MAIN_CONTROL_DESTROY                ADL2_Main_Control_Destroy;
extern ADL2_ADAPTER_NUMBEROFADAPTERS_GET        ADL2_Adapter_NumberOfAdapters_Get;
extern ADL2_ADAPTER_PRIMARY_GET                 ADL2_Adapter_Primary_Get;
extern ADL2_ADAPTER_ADAPTERINFO_GET             ADL2_Adapter_AdapterInfo_Get;
extern ADL2_ADAPTER_ACTIVE_GET                  ADL2_Adapter_Active_Get;
extern ADL2_ADAPTER_REGVALUEINT_SET             ADL2_Adapter_RegValueInt_Set;
extern ADL2_ADAPTER_REGVALUEINT_GET             ADL2_Adapter_RegValueInt_Get;
extern ADL2_ADAPTER_REGVALUESTRING_SET			ADL2_Adapter_RegValueString_Set;
extern ADL2_ADAPTER_REGVALUESTRING_GET			ADL2_Adapter_RegValueString_Get;

extern ADL2_DFP_GPUSCALINGENABLE_SET            ADL2_DFP_GPUScalingEnable_Set;
extern ADL2_DFP_GPUSCALINGENABLE_GET            ADL2_DFP_GPUScalingEnable_Get;
extern ADL2_DISPLAY_PRESERVEDASPECTRATIO_SET    ADL2_Display_PreservedAspectRatio_Set;
extern ADL2_DISPLAY_PRESERVEDASPECTRATIO_GET	ADL2_Display_PreservedAspectRatio_Get;
extern ADL2_DISPLAY_IMAGEEXPANSION_GET			ADL2_Display_ImageExpansion_Get;
extern ADL2_DISPLAY_IMAGEEXPANSION_SET          ADL2_Display_ImageExpansion_Set;
extern ADL2_DISPLAY_DISPLAYINFO_GET             ADL2_Display_DisplayInfo_Get;
extern ADL2_ADAPTER_ADAPTERINFOX4_GET           ADL2_Adapter_AdapterInfoX4_Get;

extern ADL2_DISPLAY_FREESYNCSTATE_GET           ADL2_Display_FreeSyncState_Get;
extern ADL2_DISPLAY_FREESYNCSTATE_SET           ADL2_Display_FreeSyncState_Set;

extern ADL2_DISPLAY_PROPERTY_GET                ADL2_Display_Property_Get;
extern ADL2_DISPLAY_PROPERTY_SET                ADL2_Display_Property_Set;

extern ADL2_DISPLAY_COLORDEPTH_GET              ADL2_Display_ColorDepth_Get;
extern ADL2_DISPLAY_COLORDEPTH_SET              ADL2_Display_ColorDepth_Set;
extern ADL2_DISPLAY_SUPPORTEDCOLORDEPTH_GET     ADL2_Display_SupportedColorDepth_Get;
extern ADL2_DISPLAY_PIXELFORMAT_GET             ADL2_Display_PixelFormat_Get;
extern ADL2_DISPLAY_PIXELFORMAT_SET             ADL2_Display_PixelFormat_Set;
extern ADL2_DISPLAY_SUPPORTEDPIXELFORMAT_GET    ADL2_Display_SupportedPixelFormat_Get;
extern ADL2_DISPLAY_PIXELFORMATDEFAULT_GET      ADL2_Display_PixelFormatDefault_Get;
extern ADL2_DISPLAY_COLORTEMPERATURESOURCE_GET  ADL2_Display_ColorTemperatureSource_Get;
extern ADL2_DISPLAY_COLOR_GET                   ADL2_Display_Color_Get;
extern ADL2_DISPLAY_COLOR_SET                   ADL2_Display_Color_Set;
extern ADL2_DISPLAY_COLORTEMPERATURESOURCE_SET  ADL2_Display_ColorTemperatureSource_Set;
extern ADL2_CHILL_SETTINGS_SET					ADL2_Chill_Settings_Set;
extern ADL2_CHILL_SETTINGS_GET					ADL2_Chill_Settings_Get;

extern ADL2_TURBOSYNCSUPPORT_GET				ADL2_TurboSyncSupport_Get;


void __stdcall ADL_Main_Memory_Free(void** lpBuffer);

//Initialize ADL. Retrieves and initialize ADL API pointers.
//Returns false if ADL initialization failed or some of the expected ADL APIs can't be found
extern int InitADL();
//Destroy ADL. ALD Calls can't be called after the method is invoked;
extern void DestroyADL();

extern void PrepareAPI();
//extern ADL_CONTEXT_HANDLE context_;
//extern int adapterIndex_;
typedef struct ADLApp_Context_t
{
	ADL_CONTEXT_HANDLE  adlContext;
	vector<int>         vecAciveAdapter;
} ADLApp_Context;

extern ADLApp_Context ADLApp_context;
extern unordered_map<int, ADLDisplayID*> DisplayID;
#endif
