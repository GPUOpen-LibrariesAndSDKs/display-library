// SmartDC.cpp : Defines the entry point for the console application.
//

///
///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///

#include "stdafx.h"
#include "Adl.h"
#include <iostream>
#include "SmartDCSetting.h"


void printSyntax()
{
    printf("This Application accepts below parameters\n");
    printf("-----------------------------------------\n");
    printf("Read capabilities of SmartDC: GetSmartDCCaps \n");
    printf("Read SmartDC Current Mode: GetSmartDCMode \n");
    printf("Set SmartDC Mode: SetSmartDCMode \n");
    printf("Get Smartshift eco inactive state: GetSSEcoInActiveState \n");


    printf("Read capabilities of GPUConnect caps: GetGPUConnectCaps \n");
    printf("Get list of GPUConnect adapters info: GetGPUConnectAdaptersInfo \n");
    printf("Get state of GPUConnect: GetGPUConnectState: param1:bdf \n");
    printf("Set state of GPUConnect: SetGPUConnectState: param1:bdf, param2:state \n");
    printf("Get list of GPUConnect in use applications: GetGPUConnectInUseAppList: param1:bdf \n");

    printf("Read capabilities of Auto low power caps: GetSSEcoAutoLowPowerCaps \n");
    printf("Get state of Auto low power: GetSSEcoAutoLowPowerState \n");
    printf("Get enable state of Auto low power: GetSSEcoAutoLowPowerEnable \n");
    printf("Set enable state of Auto low power: SetSSEcoAutoLowPowerEnable: param1:enable, \n");
    printf("Get list of Auto low power configs: GetSSEcoAutoLowPowerConfig \n");
    printf("Set list of Auto low power configs: SetSSEcoAutoLowPowerConfig \n");
}

int GpuBDF(const int& busNo_, const int& devNo_, const int& funcNo_)
{
    return ((busNo_ & 0xFF) << 8) | ((devNo_ & 0x1F) << 3) | (funcNo_ & 0x07);
}

//Sample entry point
int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        if (ADL_OK == InitADL()) //Initialization of ADL
        {
            if (1 == argc)
            {
                printSyntax();
            }
            else
            {
                SmartDCSetting smartDC;
                //read capabilities of SmartDC
                if (_tcscmp(argv[1], _T("GetSmartDCCaps")) == 0)
                {
                    int supported = 0;
                    int defaultState = 0;
                    if (smartDC.GetSmartDCCaps(supported, defaultState))
                    {
                        std::cout << "\tSmartDC supported: " << (supported ? "Support" : "Unsupport") << std::endl;
                        std::cout << "\tSmartDC current status: " << (defaultState ? "Enabled" : "Disabled") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetSmartDCMode")) == 0)
                {
                    int currentState = 0;
                    if (smartDC.GetSmartDCMode(currentState))
                    {
                        std::cout << "\tSmartDC current status: " << (currentState ? "Enabled" : "Disabled") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("SetSmartDCMode")) == 0)
                {
                    int enablement = _wtoi(argv[2]);
                    int currentState = 0;
                    if (smartDC.SetSmartDCMode(enablement, currentState))
                    {
                        std::cout << "\tSmartDC current status: " << (currentState ? "Enabled" : "Disabled") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetGPUConnectCaps")) == 0)
                {
                    int supported = 0;
                    if (smartDC.GetGPUConnectCaps(&supported))
                    {
                        std::cout << "\tSmartShiftEco GPUConnect supported: " << (supported ? "true" : "false") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetGPUConnectAdaptersInfo")) == 0)
                {
                    list<ADL_GPUCONNECT_ADAPTER_INFO> adapterlist;
                    if (smartDC.GetGPUConnectAdaptersInfo(adapterlist))
                    {
                        std::cout << "\tSmartShiftEco GPUConnect adapter list: " << std::endl;
                        for (auto &it : adapterlist)
                        {
                            printf("Adapter bdf: %d, name: %s, mode: %d\n", GpuBDF(it.bdf.iBus, it.bdf.iDevice, it.bdf.iFunction), it.strAdapterName, it.supportedModes);
                        }
                    }
                }
                else if (_tcscmp(argv[1], _T("GetGPUConnectState")) == 0)
                {   
                    int bdf = _wtoi(argv[2]);
                    ADL_GPUCONNECT_POWER_STATE state = ADL_GPUCONNECT_STATE_UNKNOWN;
                    if (smartDC.GetGPUConnectState(bdf, &state))
                    {
                        std::cout << "\tGet SmartShiftEco GPUConnect state: " << (state == 1 ? "power off" : state == 2 ? "reduce gfx" : "power on") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("SetGPUConnectState")) == 0)
                {
                    int bdf = _wtoi(argv[2]);
                    int state = _wtoi(argv[3]);
                    if (smartDC.SetGPUConnectState(bdf, ADL_GPUCONNECT_POWER_STATE(state)))
                    {
                        std::cout << "\tSet SmartShiftEco GPUConnect state: " << (state == 1 ? "power off" : state == 2 ? "reduce gfx" : "power on") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetGPUConnectInUseAppList")) == 0)
                {
                    int bdf = _wtoi(argv[2]);
                    bool isOtherSessionUsingGPU_ = false;
                    list<ADL_GPU_APPLICATION_INFO> applications_;
                    if (smartDC.GetGPUConnectInUseAppList(bdf, &isOtherSessionUsingGPU_, applications_))
                    {
                        std::cout << "\tSmartShiftEco GPUConnect in use applictaions list: isOtherSessionUsingGPU_: " << isOtherSessionUsingGPU_ << std::endl;
                        for (auto& it : applications_)
                        {
                            printf("Application processID: %d, sessionID: %d, flags: %d, name: %ws, full path: %ws\n", it.processID, it.sessionID, it.flags, it.name, it.fullPath);
                        }
                    }
                }
                else if (_tcscmp(argv[1], _T("GetSSEcoAutoLowPowerCaps")) == 0)
                {
                    int support = -1;
                    if (smartDC.GetSSEcoAutoLowPowerCaps(&support))
                    {
                        std::cout << "\tSmartShiftEco auto low power caps: " << (support == 1 ? "support" : "not support") << std::endl;
                    }
                    else
                    {
                        std::cout << "\tFailed to get SmartShiftEco auto low power caps" << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetSSEcoAutoLowPowerState")) == 0)
                {
                    int state = 0;
                    if (smartDC.GetSSEcoAutoLowPowerState(&state))
                    {
                        std::cout << "\tSmartShiftEco auto low power state: " << (state == 1 ? "enable" : "disable") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("SetSSEcoAutoLowPowerEnable")) == 0)
                {
                    int enable = _wtoi(argv[2]);
                    if (smartDC.SetSSEcoAutoLowPowerEnable(enable))
                    {
                        std::cout << "\tSet SmartShiftEco auto low power enable state: " << (enable == 1 ? "true" : "false") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetSSEcoAutoLowPowerEnable")) == 0)
                {
                    int enable = 0;
                    if (smartDC.GetSSEcoAutoLowPowerEnable(&enable))
                    {
                        std::cout << "\tGet SmartShiftEco auto low power enable state: " << (enable == 1 ? "true" : "false") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetSSEcoAutoLowPowerConfig")) == 0)
                {
                    list<ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG> configs_;
                    if (smartDC.GetSSEcoAutoLowPowerConfig(configs_))
                    {
                        std::cout << "\tGet SmartShiftEco auto low power config: " << std::endl;
                        for (auto& it : configs_)
                        {
                            printf("Auto low power mode: %d, isDefault: %d, isSelected: %d, currentParam1: %d, defaultParam1: %d\n", it.mode, it.isDefault, it.isSelected, it.currentParam1, it.defaultParam1);
                        }
                    }
                }
                else if (_tcscmp(argv[1], _T("SetSSEcoAutoLowPowerConfig")) == 0)
                {
                    int count = 2;
                    ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG* configs = (ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG*)ADL_Main_Memory_Alloc(count * sizeof(ADL_SMARTSHIFT_AUTOLOWPOWER_CONFIG));
                    configs[0].mode = ADL_AUTOLOWPOWER_MODE_ACDC;
                    configs[0].isSelected = false;

                    configs[1].mode = ADL_AUTOLOWPOWER_MODE_BATTERYLEVEL;
                    configs[1].isSelected = true;
                    configs[1].currentParam1 = 40;
                    if (smartDC.SetSSEcoAutoLowPowerConfig(count, configs))
                    {
                        std::cout << "\tSet SmartShiftEco auto low power config success" << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("GetSSEcoInActiveState")) == 0)
                {
                    int state = -1;
                    if (smartDC.GetSSEcoInActiveState(&state))
                    {
                        std::cout << "\tSmartShiftEco inactive state get: " << state << std::endl;
                    }
                    else
                    {
                        std::cout << "\tFailed to get SmartShiftEco inactive state get" << std::endl;
                    }
                }
            }
        }
        DestroyADL();
    }
    catch (std::exception  ex)
    {
        std::cout << ex.what();
        return -1;
    }
    return 0;
}