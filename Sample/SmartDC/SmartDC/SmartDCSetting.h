///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

#pragma once
#include <list>
#include <string>
#include "Adl.h"

using namespace std;

typedef struct GPU_APP_INFO {
    unsigned long Pid;
    wstring AppName;
    wstring AppExePath;
}GPU_APP_INFO, * PGPU_APP_INFO;

class SmartDCSetting
{
public:
    SmartDCSetting();
    ~SmartDCSetting();

    //Method to read capabilities of SmartDC
    //Arguments         :
    //supported_        : Value of the SmartDC mode support. 0: Unsupported; 1: Supported
    //defaultVal        : Value of the SmartDC default state. 0: Disalbed; 1: Enabled
    //Returns           : TRUE on success and FALSE otherwise
    bool GetSmartDCCaps(int &supported_, int &defaultVal_);

    //Method to read SmartDC Mode
    //Arguments                     :
    //currentMode_                  : Value of current SmartDC Mode. 0: Disalbed; 1: Enabled
    //Returns                       : TRUE on success and FALSE otherwise
    bool GetSmartDCMode(int &currentMode_);

    //Method to set SmartDC Mode
    //Arguments                 :
    //enable_                   : Defines the value of the SmartDC mode
    //settedMode_               : Value after SmartDC Mode changed. 0: Disalbed; 1: Enabled
    //Returns                   : TRUE on success and FALSE otherwise
    bool SetSmartDCMode(int enable_, int &settedMode_);

    //Method to get SmartDC inactive state
    //Arguments                 :
    //state_                    : Defines the value of the SmartDC inactive state
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetSSEcoInActiveState(int* state_);

    //Method to get GPUConnect caps
    //Arguments                 :
    //supported_                : Value of the GPU connect support. 0: Unsupported; 1: Supported
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetGPUConnectCaps(int* supported_);

    //Method to get GPUConnect adapters info
    //Arguments                 :
    //adaptersInfo_             : Value of the GPUConnect adapters information
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetGPUConnectAdaptersInfo(list<ADL_GPUCONNECT_ADAPTER_INFO>& adaptersInfo_);

    //Method to get GPUConnect state
    //Arguments                 :
    //bdf_                      : Specify the GPU
    //state_                    : Value of the GPUConnect power state.
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetGPUConnectState(int bdf_, ADL_GPUCONNECT_POWER_STATE* state_);

    //Method to set GPUConnect state
    //Arguments                 :
    //bdf_                      : Specify the GPU
    //state_                    : Value of the GPUConnect power state.
    //Returns                   : TRUE on success and FALSE otherwise
    bool SetGPUConnectState(int bdf_, ADL_GPUCONNECT_POWER_STATE state_);

    //Method to get GPUConnect in use app list
    //Arguments                 :
    //bdf_                      : Specify the GPU
    //isOtherSessionUsingGPU_   : This pointer indicates if there are other user sessions uses GPU.
    //applicationsInfo_         : Value of the applications list which created context on bdf adapter.
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetGPUConnectInUseAppList(int bdf_, bool* isOtherSessionUsingGPU_, list<ADL_GPU_APPLICATION_INFO>& applicationsInfo_);

    //Method to get smartshifteco auto low power caps
    //Arguments                 :
    //supported_                : Value of the smartshifteco auto low power caps. 0: Unsupported; 1: Supported
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetSSEcoAutoLowPowerCaps(int* supported_);

    //Method to get smartshifteco auto low power state
    //Arguments                 :
    //state_                    : Value of the smartshifteco auto low power state, 0: disable; 1: enable
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetSSEcoAutoLowPowerState(int* state_);

    //Method to set smartshifteco auto low power enable
    //Arguments                 :
    //state_                    : Value of the smartshifteco auto dgpu connnect enable state, 0: disable; 1: enable
    //Returns                   : TRUE on success and FALSE otherwise
    bool SetSSEcoAutoLowPowerEnable(int enable_);

    //Method to set smartshifteco auto low power enable
    //Arguments                 :
    //state_                    : Value of the smartshifteco auto low power enable state, 0: disable; 1: enable
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetSSEcoAutoLowPowerEnable(int* enable_);

    //Method to get smartshifteco auto low power configs
    //Arguments                 :
    //configs_                  : Value of the smartshifteco auto low power configs.
    //Returns                   : TRUE on success and FALSE otherwise
    bool GetSSEcoAutoLowPowerConfig(list<ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG>& configs_);

    //Method to set smartshifteco auto low power configs
    //Arguments                 :
    //count_                    : Value of the number of auto low power configs to set.
    //configs_                  : Value of the smartshifteco auto low power configs
    //Returns                   : TRUE on success and FALSE otherwise
    bool SetSSEcoAutoLowPowerConfig(int count_, ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG* configs_);
};

