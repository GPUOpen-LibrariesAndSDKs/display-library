
///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

#include "stdafx.h"
#include "SmartDCSetting.h"


SmartDCSetting::SmartDCSetting()
{
}

SmartDCSetting::~SmartDCSetting()
{
}

bool SmartDCSetting::GetSmartDCCaps(int &supported_, int &defaultVal_)
{
    return ADL_OK == ADL2_SmartDC_Caps(context_, &supported_, &defaultVal_) ? true : false;
}

bool SmartDCSetting::GetSmartDCMode(int &currentMode_)
{
    return ADL_OK == ADL2_SmartDC_Status_Get(context_, &currentMode_) ? true : false;
}

bool SmartDCSetting::SetSmartDCMode(int enable_, int &settedMode_)
{
    return ADL_OK == ADL2_SmartDC_Status_Set(context_, enable_, &settedMode_) ? true : false;
}

bool SmartDCSetting::GetGPUConnectCaps(int* supported_)
{
    return ADL_OK == ADL2_GPUConnect_Caps(context_, supported_);
}

bool SmartDCSetting::GetGPUConnectAdaptersInfo(list<ADL_GPUCONNECT_ADAPTER_INFO>& adaptersInfo_)
{
    bool ret = false;
    int count = 0;
    ADL_GPUCONNECT_ADAPTER_INFO* adapterInfo = nullptr;
    if (ADL_OK == ADL2_GPUConnect_GetAdapterInfo(context_, &count, &adapterInfo))
    {
        ret = true;
        for (int i = 0; i < count; i++)
        {
            adaptersInfo_.push_back(adapterInfo[i]);
        }

        if (adapterInfo)
            ADL_Main_Memory_Free(adapterInfo);
    }
    return ret;
}

bool SmartDCSetting::GetGPUConnectState(int bdf_, ADL_GPUCONNECT_POWER_STATE* state_)
{
    bool ret = false;
    ADLBdf adlbdf;
    adlbdf.iBus = bdf_ >> 8;
    adlbdf.iDevice = (bdf_ >> 3) & 0x1F;
    adlbdf.iFunction = bdf_ & 0x07;
    if (ADL_OK == ADL2_GPUConnect_State_Get(context_, adlbdf, state_))
    {
        ret = true;
    }
    return ret;
}

bool SmartDCSetting::SetGPUConnectState(int bdf_, ADL_GPUCONNECT_POWER_STATE state_)
{
    bool ret = false;
    ADLBdf adlbdf;
    adlbdf.iBus = bdf_ >> 8;
    adlbdf.iDevice = (bdf_ >> 3) & 0x1F;
    adlbdf.iFunction = bdf_ & 0x07;
    if (ADL_OK == ADL2_GPUConnect_State_Set(context_, adlbdf, state_))
    {
        ret = true;
    }
    return ret;
}

bool SmartDCSetting::GetGPUConnectInUseAppList(int bdf_, bool* isOtherSessionUsingGPU_, list<ADL_GPU_APPLICATION_INFO>& applicationsInfo_)
{
    bool ret = false;
    ADLBdf adlbdf;
    adlbdf.iBus = bdf_ >> 8;
    adlbdf.iDevice = (bdf_ >> 3) & 0x1F;
    adlbdf.iFunction = bdf_ & 0x07;
    ADL_GPU_APPLICATION_INFO* applicationInfo = nullptr;
    int count = 0;
    if (ADL_OK == ADL2_GPUConnect_InUseAppList_Get(context_, adlbdf, isOtherSessionUsingGPU_, &count, &applicationInfo))
    {
        ret = true;
        for (int i = 0; i < count; i++)
        {
            applicationsInfo_.push_back(applicationInfo[i]);
        }

        if (applicationInfo)
            ADL_Main_Memory_Free(applicationInfo);
    }
    return ret;
}

bool SmartDCSetting::GetSSEcoInActiveState(int* state)
{
    return ADL_OK == ADL2_SmartShiftEco_InActive_State_Get(context_, state);
}

bool SmartDCSetting::GetSSEcoAutoLowPowerCaps(int* supported_)
{
    return ADL_OK == ADL2_SmartShiftEco_AutoLowPower_Caps(context_, supported_);
}

bool SmartDCSetting::GetSSEcoAutoLowPowerState(int* state_)
{
    return ADL_OK == ADL2_SmartShiftEco_AutoLowPower_State_Get(context_, state_);
}

bool SmartDCSetting::SetSSEcoAutoLowPowerEnable(int enable_)
{
    return ADL_OK == ADL2_SmartShiftEco_AutoLowPower_Enable_Set(context_, enable_);
}

bool SmartDCSetting::GetSSEcoAutoLowPowerEnable(int* enable_)
{
    return ADL_OK == ADL2_SmartShiftEco_AutoLowPower_Enable_Get(context_, enable_);
}

bool SmartDCSetting::GetSSEcoAutoLowPowerConfig(list<ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG>& configs_)
{
    bool ret = false;

    int count = 0;
    ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG* config = nullptr;
    if (ADL_OK == ADL2_SmartShiftEco_AutoLowPower_Config_Get(context_, &count, &config))
    {
        ret = true;
        for (int i = 0; i < count; i++)
        {
            configs_.push_back(config[i]);
        }

        if (config)
            ADL_Main_Memory_Free(config);
    }
    return ret;
}

bool SmartDCSetting::SetSSEcoAutoLowPowerConfig(int count_, ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG* configs_)
{
    return ADL_OK == ADL2_SmartShiftEco_AutoLowPower_Config_Set(context_, count_, configs_);
}

