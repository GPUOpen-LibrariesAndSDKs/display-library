///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///
/// \file Adl.cpp
/// \brief
///This sample demonstrates usage of ADL APIs to achieve following task:
///1. Retrieve the SmartDC capability information.
///2. Retrieve the current SmartDC mode is enabled/disbaled information
///3. Change the SmartDC mode 

#include "stdafx.h"
#include "Adl.h"
#include <windows.h>
#include <iostream>
#include "SmartDCSetting.h"

//Defining pointers to ADL Methods.
ADL2_MAIN_CONTROL_CREATE                                ADL2_Main_Control_Create = nullptr;
ADL2_MAIN_CONTROL_DESTROY                               ADL2_Main_Control_Destroy = nullptr;
ADL2_SMARTDC_CAPS                                       ADL2_SmartDC_Caps = nullptr;
ADL2_SMARTDC_STATUS_GET                                 ADL2_SmartDC_Status_Get = nullptr;
ADL2_SMARTDC_STATUS_SET                                 ADL2_SmartDC_Status_Set = nullptr;

ADL2_GPUCONNECT_CAPS                                    ADL2_GPUConnect_Caps = nullptr;
ADL2_GPUCONNECT_GETADAPTERINFO                          ADL2_GPUConnect_GetAdapterInfo = nullptr;
ADL2_GPUCONNECT_STATE_GET                               ADL2_GPUConnect_State_Get = nullptr;
ADL2_GPUCONNECT_STATE_SET                               ADL2_GPUConnect_State_Set = nullptr;
ADL2_GPUCONNECT_INUSEAPPLIST_GET                        ADL2_GPUConnect_InUseAppList_Get = nullptr;


ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CAPS                    ADL2_SmartShiftEco_AutoLowPower_Caps = nullptr;
ADL2_SMARTSHIFTECO_AUTOLOWPOWER_STATE_GET               ADL2_SmartShiftEco_AutoLowPower_State_Get = nullptr;
ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_GET              ADL2_SmartShiftEco_AutoLowPower_Enable_Get = nullptr;
ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_SET              ADL2_SmartShiftEco_AutoLowPower_Enable_Set = nullptr;
ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_GET              ADL2_SmartShiftEco_AutoLowPower_Config_Get = nullptr;
ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_SET              ADL2_SmartShiftEco_AutoLowPower_Config_Set = nullptr;

ADL2_SMARTSHIFTECO_INACTIVE_STATE_GET                   ADL2_SmartShiftEco_InActive_State_Get = nullptr;



// ADL module handle
HINSTANCE hDLL = NULL;
ADL_CONTEXT_HANDLE context_ = NULL;
std::map<int, int> AdapterIndexMap_;
//Memory allocating callback for ADL
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

//Memory freeing callback for ADL
void __stdcall ADL_Main_Memory_Free(void* lpBuffer)
{
    if (NULL != lpBuffer)
    {
        free(lpBuffer);
        lpBuffer = NULL;
    }
}


//Initialize ADL. Retrieves and initialize ADL API pointers.
//Returns false if ADL initialization failed or some of the expected ADL APIs can't be found
int InitADL()
{
    int result = ADL_OK;
    SetDllDirectoryW(L"..\\lib");
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
        ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL2_Main_Control_Create");
        ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");
        ADL2_SmartDC_Caps = (ADL2_SMARTDC_CAPS)GetProcAddress(hDLL, "ADL2_SmartDC_Caps");
        ADL2_SmartDC_Status_Get = (ADL2_SMARTDC_STATUS_GET)GetProcAddress(hDLL, "ADL2_SmartDC_Status_Get");
        ADL2_SmartDC_Status_Set = (ADL2_SMARTDC_STATUS_SET)GetProcAddress(hDLL, "ADL2_SmartDC_Status_Set");

        ADL2_GPUConnect_Caps = (ADL2_GPUCONNECT_CAPS)GetProcAddress(hDLL, "ADL2_GPUConnect_Caps");
        ADL2_GPUConnect_GetAdapterInfo = (ADL2_GPUCONNECT_GETADAPTERINFO)GetProcAddress(hDLL, "ADL2_GPUConnect_GetAdapterInfo");
        ADL2_GPUConnect_State_Get = (ADL2_GPUCONNECT_STATE_GET)GetProcAddress(hDLL, "ADL2_GPUConnect_State_Get");
        ADL2_GPUConnect_State_Set = (ADL2_GPUCONNECT_STATE_SET)GetProcAddress(hDLL, "ADL2_GPUConnect_State_Set");
        ADL2_GPUConnect_InUseAppList_Get = (ADL2_GPUCONNECT_INUSEAPPLIST_GET)GetProcAddress(hDLL, "ADL2_GPUConnect_InUseAppList_Get");

        ADL2_SmartShiftEco_AutoLowPower_Caps = (ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CAPS)GetProcAddress(hDLL, "ADL2_SmartShiftEco_AutoLowPower_Caps");
        ADL2_SmartShiftEco_AutoLowPower_State_Get = (ADL2_SMARTSHIFTECO_AUTOLOWPOWER_STATE_GET)GetProcAddress(hDLL, "ADL2_SmartShiftEco_AutoLowPower_State_Get");
        ADL2_SmartShiftEco_AutoLowPower_Enable_Get = (ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_GET)GetProcAddress(hDLL, "ADL2_SmartShiftEco_AutoLowPower_Enable_Get");
        ADL2_SmartShiftEco_AutoLowPower_Enable_Set = (ADL2_SMARTSHIFTECO_AUTOLOWPOWER_ENABLE_SET)GetProcAddress(hDLL, "ADL2_SmartShiftEco_AutoLowPower_Enable_Set");
        ADL2_SmartShiftEco_AutoLowPower_Config_Get = (ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_GET)GetProcAddress(hDLL, "ADL2_SmartShiftEco_AutoLowPower_Config_Get");
        ADL2_SmartShiftEco_AutoLowPower_Config_Set = (ADL2_SMARTSHIFTECO_AUTOLOWPOWER_CONFIG_SET)GetProcAddress(hDLL, "ADL2_SmartShiftEco_AutoLowPower_Config_Set");

        ADL2_SmartShiftEco_InActive_State_Get = (ADL2_SMARTSHIFTECO_INACTIVE_STATE_GET)GetProcAddress(hDLL, "ADL2_SmartShiftEco_InActive_State_Get");

        if (nullptr == ADL2_Main_Control_Create ||
                nullptr == ADL2_Main_Control_Destroy ||
                nullptr == ADL2_SmartDC_Caps ||
                nullptr == ADL2_SmartDC_Status_Get ||
                nullptr == ADL2_SmartDC_Status_Set ||
                nullptr == ADL2_GPUConnect_Caps ||
                nullptr == ADL2_GPUConnect_GetAdapterInfo ||
                nullptr == ADL2_GPUConnect_State_Get ||
                nullptr == ADL2_GPUConnect_State_Set ||
                nullptr == ADL2_GPUConnect_InUseAppList_Get ||
                nullptr == ADL2_SmartShiftEco_AutoLowPower_Caps ||
                nullptr == ADL2_SmartShiftEco_AutoLowPower_State_Get ||
                nullptr == ADL2_SmartShiftEco_AutoLowPower_Enable_Get ||
                nullptr == ADL2_SmartShiftEco_AutoLowPower_Enable_Set ||
                nullptr == ADL2_SmartShiftEco_AutoLowPower_Config_Get ||
                nullptr == ADL2_SmartShiftEco_AutoLowPower_Config_Set ||
                nullptr == ADL2_SmartShiftEco_InActive_State_Get
                )
        {
            std::cout << "ADL's API is missing!" << std::endl;
            return ADL_ERR;
        }

        if (ADL_OK != ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &context_))
        {
            std::cout << "Failed to initialize ADL2 context" << std::endl;
            return ADL_ERR;
        }
    }

    return result;
}

//Destroy ADL. ALD calls can't be called after the method is invoked;
void DestroyADL()
{
    if (NULL != ADL2_Main_Control_Destroy)
    {
        ADL2_Main_Control_Destroy(context_);
    }

    if (hDLL)
    {
        FreeLibrary(hDLL);
    }
}
