#ifndef _ADL_H_
#define _ADL_H_
///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///
/// \file Adl.h


//ADL_INCLUDE_DIR environment variable has to be set to location of ADL SDK header files in order to successfuly include adl headers.
#include "..\..\..\include\adl_sdk.h"
#include "..\..\..\include\amd_only\amd_structures.h"

#include <map>
//Declaring ADL Methods. 
typedef int(*ADL2_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)(ADL_CONTEXT_HANDLE);

typedef int(*ADL2_SMARTDC_CAPS)(ADL_CONTEXT_HANDLE context, int *lpSupported, int *lpDefault);
typedef int(*ADL2_SMARTDC_STATUS_GET)(ADL_CONTEXT_HANDLE context, int *lpSmartDCState);
typedef int(*ADL2_SMARTDC_STATUS_SET)(ADL_CONTEXT_HANDLE context, int enable, int *lpSmartDCState);
typedef int(*ADL2_SMARTSHIFTECO_INACTIVE_STATE_GET)(ADL_CONTEXT_HANDLE context, int *state);

//smartshift eco 1.5 functions
typedef int(*ADL2_GPUCONNECT_CAPS)(ADL_CONTEXT_HANDLE context, int* lpSupported);
typedef int(*ADL2_GPUCONNECT_GETADAPTERINFO)(ADL_CONTEXT_HANDLE context, int* lpCount, ADL_GPUCONNECT_ADAPTER_INFO** lppAdapterInfo);
typedef int(*ADL2_GPUCONNECT_STATE_GET)(ADL_CONTEXT_HANDLE context, ADLBdf bdf, ADL_GPUCONNECT_POWER_STATE* lpState);
typedef int(*ADL2_GPUCONNECT_STATE_SET)(ADL_CONTEXT_HANDLE context, ADLBdf bdf, ADL_GPUCONNECT_POWER_STATE state);
typedef int(*ADL2_GPUCONNECT_INUSEAPPLIST_GET)(ADL_CONTEXT_HANDLE context, ADLBdf bdf, bool* lpIsOtherSessionUsingGPU, int* lpCount, ADL_GPU_APPLICATION_INFO** lppGPUApplications);

typedef int(*ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CAPS)(ADL_CONTEXT_HANDLE context, int* lpSupported);
typedef int(*ADL2_SMARTSHIFTECO_AUTOLOWPOWER_STATE_GET)(ADL_CONTEXT_HANDLE context, int* lpState);
typedef int(*ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_GET)(ADL_CONTEXT_HANDLE context, int* lpEnabled);
typedef int(*ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_SET)(ADL_CONTEXT_HANDLE context, int enable);
typedef int(*ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_GET)(ADL_CONTEXT_HANDLE context, int* lpCount, ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG** lppConfigs);
typedef int(*ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_SET)(ADL_CONTEXT_HANDLE context, int count, ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG* lpConfigs);


//Declaring pointers to ADL Methods. 
extern ADL2_MAIN_CONTROL_CREATE                         ADL2_Main_Control_Create;
extern ADL2_MAIN_CONTROL_DESTROY                        ADL2_Main_Control_Destroy;
extern ADL2_SMARTDC_CAPS                                ADL2_SmartDC_Caps;
extern ADL2_SMARTDC_STATUS_GET                          ADL2_SmartDC_Status_Get;
extern ADL2_SMARTDC_STATUS_SET                          ADL2_SmartDC_Status_Set;
extern ADL2_SMARTSHIFTECO_INACTIVE_STATE_GET            ADL2_SmartShiftEco_InActive_State_Get;

extern ADL2_GPUCONNECT_CAPS                             ADL2_GPUConnect_Caps;
extern ADL2_GPUCONNECT_GETADAPTERINFO                   ADL2_GPUConnect_GetAdapterInfo;
extern ADL2_GPUCONNECT_STATE_GET                        ADL2_GPUConnect_State_Get;
extern ADL2_GPUCONNECT_STATE_SET                        ADL2_GPUConnect_State_Set;
extern ADL2_GPUCONNECT_INUSEAPPLIST_GET                 ADL2_GPUConnect_InUseAppList_Get;


extern ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CAPS                 ADL2_SmartShiftEco_AutoLowPower_Caps;
extern ADL2_SMARTSHIFTECO_AUTOLOWPOWER_STATE_GET            ADL2_SmartShiftEco_AutoLowPower_State_Get;
extern ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_GET           ADL2_SmartShiftEco_AutoLowPower_Enable_Get;
extern ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_SET           ADL2_SmartShiftEco_AutoLowPower_Enable_Set;
extern ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_GET           ADL2_SmartShiftEco_AutoLowPower_Config_Get;
extern ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_SET           ADL2_SmartShiftEco_AutoLowPower_Config_Set;

void* __stdcall ADL_Main_Memory_Alloc(int iSize);
void __stdcall ADL_Main_Memory_Free(void* lpBuffer);

//Initialize ADL. Retrieves and initialize ADL API pointers.
//Returns false if ADL initialization failed or some of the expected ADL APIs can't be found
extern int InitADL();
//Destroy ADL. ALD Calls can't be called after the method is invoked;
extern void DestroyADL();

extern void PrepareAPI();
extern ADL_CONTEXT_HANDLE context_;

#endif
