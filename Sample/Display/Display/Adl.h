#ifndef _ADL_H_
#define _ADL_H_
///  Copyright (c) 2019 - 2022 Advanced Micro Devices, Inc.
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///
/// \file Adl.h
/// \brief 
///This sample demonstrates usage of ADL APIs to achieve following task:
///1. Retrieve the display caps information.
///2. Retrieve the display mode information
///3. Change the display mode 

//ADL_INCLUDE_DIR environment variable has to be set to location of ADL SDK header files in order to successfuly include adl headers.
#include "..\..\..\include\adl_sdk.h"

#include <map>
//Declaring ADL Methods. 
typedef int(*ADL2_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)(ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET)(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_PRIMARY_GET)(ADL_CONTEXT_HANDLE, int* lpPrimaryAdapterIndex);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET)(ADL_CONTEXT_HANDLE, LPAdapterInfo lpInfo, int iInputSize);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);

typedef int(*ADL2_DISPLAY_DISPLAYINFO_GET)      (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplays, ADLDisplayInfo ** lppInfo, int iForceDetect);
//Virtual Resolution ADL2 calls
typedef int(*ADL2_DISPLAY_PROPERTY_GET)         (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);
typedef int(*ADL2_DISPLAY_PROPERTY_SET)         (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDisplayProperty * lpDisplayProperty);
typedef int(*ADL2_ADAPTER_MEMORYINFO3_GET)       (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLMemoryInfo3 *);
//Display Spec
typedef int(*ADL2_DISPLAY_DCE_GET)                                     (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDceSettings* lpDceSettings);
typedef int(*ADL2_DISPLAY_DCE_SET)                                     (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDisplayIndex, ADLDceSettings* lpDceSettings);


//Declaring pointers to ADL Methods. 
extern ADL2_MAIN_CONTROL_CREATE			    ADL2_Main_Control_Create;
extern ADL2_MAIN_CONTROL_DESTROY			ADL2_Main_Control_Destroy;
extern ADL2_ADAPTER_NUMBEROFADAPTERS_GET	ADL2_Adapter_NumberOfAdapters_Get;
extern ADL2_ADAPTER_PRIMARY_GET             ADL2_Adapter_Primary_Get;
extern ADL2_ADAPTER_ADAPTERINFO_GET         ADL2_Adapter_AdapterInfo_Get;
extern ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get;

extern ADL2_DISPLAY_PROPERTY_GET                ADL2_Display_Property_Get;
extern ADL2_DISPLAY_PROPERTY_SET                ADL2_Display_Property_Set;

extern ADL2_DISPLAY_DISPLAYINFO_GET             ADL2_Display_DisplayInfo_Get;
extern ADL2_ADAPTER_MEMORYINFO3_GET         ADL2_Adapter_MemoryInfo3_Get;

extern ADL2_DISPLAY_DCE_GET         ADL2_Display_DCE_Get;
extern ADL2_DISPLAY_DCE_SET         ADL2_Display_DCE_Set;

void __stdcall ADL_Main_Memory_Free(void** lpBuffer);

//Initialize ADL. Retrieves and initialize ADL API pointers.
//Returns false if ADL initialization failed or some of the expected ADL APIs can't be found
extern int InitADL();
//Destroy ADL. ALD Calls can't be called after the method is invoked;
extern void DestroyADL();

extern void PrepareAPI();
extern void Get_All_DisplayInfo(int adapterIndex);

extern ADL_CONTEXT_HANDLE context_;
extern std::map<int, int> AdapterIndexMap_;
extern std::map<int, ADLDisplayID> DisplayID_;
#endif
