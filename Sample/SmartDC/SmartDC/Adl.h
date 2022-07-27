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

//Declaring pointers to ADL Methods. 
extern ADL2_MAIN_CONTROL_CREATE             ADL2_Main_Control_Create;
extern ADL2_MAIN_CONTROL_DESTROY            ADL2_Main_Control_Destroy;
extern ADL2_SMARTDC_CAPS                    ADL2_SmartDC_Caps;
extern ADL2_SMARTDC_STATUS_GET              ADL2_SmartDC_Status_Get;
extern ADL2_SMARTDC_STATUS_SET              ADL2_SmartDC_Status_Set;

void __stdcall ADL_Main_Memory_Free(void* lpBuffer);

//Initialize ADL. Retrieves and initialize ADL API pointers.
//Returns false if ADL initialization failed or some of the expected ADL APIs can't be found
extern int InitADL();
//Destroy ADL. ALD Calls can't be called after the method is invoked;
extern void DestroyADL();

extern void PrepareAPI();
extern ADL_CONTEXT_HANDLE context_;

#endif
