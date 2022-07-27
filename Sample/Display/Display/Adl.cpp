///  Copyright (c) 2019 - 2022 Advanced Micro Devices, Inc.
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///
/// \file Adl.cpp
/// \brief
///This sample demonstrates usage of ADL APIs to achieve following task:
///1. Retrieve the Display caps information.
///2. Retrieve the display mode information
///3. Change the display mode 

#include "stdafx.h"
#include "Adl.h"
#include <windows.h>
#include <iostream>
#include <codecvt>
#include "displaySetting.h"

//Defining pointers to ADL Methods.
ADL2_MAIN_CONTROL_CREATE			ADL2_Main_Control_Create = nullptr;
ADL2_MAIN_CONTROL_DESTROY			ADL2_Main_Control_Destroy = nullptr;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET	ADL2_Adapter_NumberOfAdapters_Get = nullptr;
ADL2_ADAPTER_PRIMARY_GET            ADL2_Adapter_Primary_Get = nullptr;
ADL2_ADAPTER_ADAPTERINFO_GET        ADL2_Adapter_AdapterInfo_Get = nullptr;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get = nullptr;

ADL2_DISPLAY_DISPLAYINFO_GET                                        ADL2_Display_DisplayInfo_Get = nullptr;
ADL2_DISPLAY_PROPERTY_GET											ADL2_Display_Property_Get = nullptr;
ADL2_DISPLAY_PROPERTY_SET											ADL2_Display_Property_Set = nullptr;
ADL2_ADAPTER_MEMORYINFO3_GET         ADL2_Adapter_MemoryInfo3_Get = nullptr;
ADL2_DISPLAY_DCE_GET         ADL2_Display_DCE_Get = nullptr;
ADL2_DISPLAY_DCE_SET         ADL2_Display_DCE_Set = nullptr;
// ADL module handle
HINSTANCE hDLL = NULL;
ADL_CONTEXT_HANDLE context_ = NULL;
std::map<int, int> AdapterIndexMap_;
std::map<int, ADLDisplayID> DisplayID_;

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
        ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL2_Adapter_NumberOfAdapters_Get");
        ADL2_Adapter_Primary_Get = (ADL2_ADAPTER_PRIMARY_GET)GetProcAddress(hDLL, "ADL2_Adapter_Primary_Get");
        ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfo_Get");
        ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
        ADL2_Display_DisplayInfo_Get = (ADL2_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hDLL, "ADL2_Display_DisplayInfo_Get");

        ADL2_Display_Property_Get = (ADL2_DISPLAY_PROPERTY_GET)GetProcAddress(hDLL, "ADL2_Display_Property_Get");
        ADL2_Display_Property_Set = (ADL2_DISPLAY_PROPERTY_SET)GetProcAddress(hDLL, "ADL2_Display_Property_Set");
        ADL2_Adapter_MemoryInfo3_Get = (ADL2_ADAPTER_MEMORYINFO3_GET)GetProcAddress(hDLL, "ADL2_Adapter_MemoryInfo3_Get");
        ADL2_Display_DCE_Get = (ADL2_DISPLAY_DCE_GET)GetProcAddress(hDLL, "ADL2_Display_DCE_Get");
        ADL2_Display_DCE_Set = (ADL2_DISPLAY_DCE_SET)GetProcAddress(hDLL, "ADL2_Display_DCE_Set");

        if (nullptr == ADL2_Main_Control_Create ||
            nullptr == ADL2_Main_Control_Destroy ||
            nullptr == ADL2_Adapter_NumberOfAdapters_Get ||
            nullptr == ADL2_Adapter_Primary_Get ||
            nullptr == ADL2_Adapter_AdapterInfo_Get ||
            nullptr == ADL2_Adapter_Active_Get ||
            nullptr == ADL2_Display_Property_Get ||
            nullptr == ADL2_Display_Property_Set ||
            nullptr == ADL2_Display_DisplayInfo_Get ||
            nullptr == ADL2_Adapter_MemoryInfo3_Get ||
            nullptr == ADL2_Display_DCE_Get ||
            nullptr == ADL2_Display_DCE_Set
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

int GpuBDF(int busNo_, int devNo_, int funcNo_)
{
    return ((busNo_ & 0xFF) << 8) | ((devNo_ & 0x1F) << 3) | (funcNo_ & 0x07);
}
char * GetGPUVRAMNameFromID(int iVramVendorRevId)
{
    switch (iVramVendorRevId)
    {
        case ADLvRamVendor_SAMSUNG:
            return "Samsung";
        case ADLvRamVendor_INFINEON:
            return "Infineon";
        case ADLvRamVendor_ELPIDA:
            return "Elpida";
        case ADLvRamVendor_ETRON:
            return "Etron";
        case ADLvRamVendor_NANYA:
            return "Nanya";
        case ADLvRamVendor_HYNIX:
            return "Hynix";
        case ADLvRamVendor_MOSEL:
            return "Mosel";
        case ADLvRamVendor_WINBOND:
            return "Winbond";
        case ADLvRamVendor_ESMT:
            return "Esmt";
        case ADLvRamVendor_MICRON:
            return "Micron";
            default:
                return "Undefine";
    }
}
void PrepareAPI()
{
    int iNumberAdapters = 0;
    if (ADL_OK == ADL2_Adapter_NumberOfAdapters_Get(context_, &iNumberAdapters))
    {
        int primary = -1;
        ADL2_Adapter_Primary_Get(context_, &primary);

        AdapterInfo* infos = new AdapterInfo[iNumberAdapters];
        if (ADL_OK == ADL2_Adapter_AdapterInfo_Get(context_, infos, sizeof(AdapterInfo)*iNumberAdapters))
        {
            for (int i = 0; i < iNumberAdapters; ++i)
            {
                AdapterInfo info = infos[i];

                int bdf = GpuBDF(info.iBusNumber, info.iDeviceNumber, info.iFunctionNumber);
                if (info.iVendorID == 1002 && AdapterIndexMap_.find(bdf) == AdapterIndexMap_.end()) // Is AMD graphic
                {
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
                    printf(
                        "\tPrimary: %s\n",
                        (primary == i) ? "yes" : "no"
                    );
                    AdapterIndexMap_[bdf] = i;
                    ADLMemoryInfo3 adlMemInfo3;
                    if (ADL_OK == ADL2_Adapter_MemoryInfo3_Get(context_, info.iAdapterIndex, &adlMemInfo3))
                    {
                        printf("\tGPU Memory Size : %d MB, Memory Type %s\n", adlMemInfo3.iMemorySize / 1024 / 1024, adlMemInfo3.strMemoryType);
                        if (ADLvRamVendor_Unsupported == adlMemInfo3.iVramVendorRevId)
                            printf("\tGPU Vedio RAM vendor ID Unsupport, only support AMD dGPU now.\n");
                        else
                            printf("\tGPU Vedio RAM vendor ID : %d, Name : %s\n", adlMemInfo3.iVramVendorRevId, GetGPUVRAMNameFromID(adlMemInfo3.iVramVendorRevId));
                        printf("\tGPU Vedio RAM memory bandwidth : %d B\n", adlMemInfo3.iMemoryBandwidth / 1024);
                    }

                }
            }
        }
        delete[] infos;
    }
    return;
}

void Get_All_DisplayInfo(int adapterIndex)
{
    int numDisplays = 0;
    ADLDisplayInfo* allDisplaysBuffer = NULL;

    if (ADL_OK == ADL2_Display_DisplayInfo_Get(context_, adapterIndex, &numDisplays, &allDisplaysBuffer, 1)) {
        for (int i = 0; i < numDisplays; i++) {
            ADLDisplayInfo* oneDis = &(allDisplaysBuffer[i]);
            if (ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED == (oneDis->iDisplayInfoValue &       ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED)) {
                ADLDisplayID display(oneDis->displayID);
                DisplayID_.insert(std::pair<int, ADLDisplayID>(i, display));
            }

        }

    }

    ADL_Main_Memory_Free((void**)&allDisplaysBuffer);
}
