#include <stdio.h>
#include <tchar.h>
#include <vector>

///
///  Copyright (c) 2018 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.



#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include "..\..\include\adl_structures.h"
#include <stdio.h>

// #define PRINTF
#define PRINTF printf

struct OverdriveRangeDataStruct
{
    //Minimum value
    int Min_;
    //Maximum value
    int Max_;
    //Expected value: similar to current value
    int ExpectedValue_;
    //Default value
    int DefaultValue_;
    //Actual value
    int ActualValue_;
    // If ActualValue can be got from the driver, ActualValueAvailable_ will be true
    bool ActualValueAvailable_;
    // If the disable/enable feature is supported by the driver, it is true.
    bool EnableDisableSupport_;
    // The enabled state
    bool Visible_;
};

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int(*ADL_ADAPTERX2_CAPS) (int, int*);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int(*ADL2_ADAPTER_REGVALUEINT_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, const char* szSubKey, const char *szKeyName, int *lpKeyValue);

typedef int(*ADL2_OVERDRIVE8_INIT_SETTING_GET) (ADL_CONTEXT_HANDLE, int, ADLOD8InitSetting*);
typedef int(*ADL2_OVERDRIVE8_CURRENT_SETTING_GET) (ADL_CONTEXT_HANDLE, int, ADLOD8CurrentSetting*);
typedef int(*ADL2_OVERDRIVE8_SETTING_SET) (ADL_CONTEXT_HANDLE, int, ADLOD8SetSetting*, ADLOD8CurrentSetting*);
typedef int(*ADL2_NEW_QUERYPMLOGDATA_GET) (ADL_CONTEXT_HANDLE, int, ADLPMLogDataOutput*);

typedef int(*ADL2_OVERDRIVE8_INIT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpOverdrive8Capabilities, int *lpNumberOfFeatures, ADLOD8SingleInitSetting** lppInitSettingList);
typedef int(*ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpNumberOfFeatures, int** lppCurrentSettingList);

HINSTANCE hDLL;

ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
ADL_ADAPTERX2_CAPS               ADL_AdapterX2_Caps = NULL;
ADL2_ADAPTER_ACTIVE_GET          ADL2_Adapter_Active_Get = NULL;
ADL2_OVERDRIVE_CAPS              ADL2_Overdrive_Caps = NULL;
ADL2_ADAPTER_REGVALUEINT_GET     ADL2_Adapter_RegValueInt_Get = NULL;

ADL2_OVERDRIVE8_INIT_SETTING_GET ADL2_Overdrive8_Init_Setting_Get = NULL;
ADL2_OVERDRIVE8_CURRENT_SETTING_GET ADL2_Overdrive8_Current_Setting_Get = NULL;
ADL2_OVERDRIVE8_SETTING_SET ADL2_Overdrive8_Setting_Set = NULL;
ADL2_NEW_QUERYPMLOGDATA_GET ADL2_New_QueryPMLogData_Get = NULL;

ADL2_OVERDRIVE8_INIT_SETTINGX2_GET ADL2_Overdrive8_Init_SettingX2_Get = NULL;
ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET ADL2_Overdrive8_Current_SettingX2_Get = NULL;

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}

ADL_CONTEXT_HANDLE context = NULL;
LPAdapterInfo   lpAdapterInfo = NULL;
int  iNumberAdapters;
int PrintFeatureName(int itemID_);
int SetOD8Range(const ADLOD8InitSetting &odInitSetting, ADLOD8CurrentSetting &odCurrentSetting, int iAdapterIndex, int SettingId, int Reset, int value);
int GetOD8RangePrint(ADLOD8InitSetting odInitSetting, ADLOD8CurrentSetting odCurrentSetting, OverdriveRangeDataStruct &oneRangeData, int itemID_, int featureID_);

int initializeADL()
{
    
    // Load the ADL dll
    hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
    if (hDLL == NULL)
    {
        // A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
    }
    
    if (NULL == hDLL)
    {
        PRINTF("Failed to load ADL library\n");
        return FALSE;
    }
    ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
    ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
    ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
    ADL_AdapterX2_Caps = (ADL_ADAPTERX2_CAPS)GetProcAddress(hDLL, "ADL_AdapterX2_Caps");
    ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
    ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS)GetProcAddress(hDLL, "ADL2_Overdrive_Caps");
    ADL2_Adapter_RegValueInt_Get = (ADL2_ADAPTER_REGVALUEINT_GET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueInt_Get");

    ADL2_Overdrive8_Init_Setting_Get = (ADL2_OVERDRIVE8_INIT_SETTING_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Init_Setting_Get");
    ADL2_Overdrive8_Current_Setting_Get = (ADL2_OVERDRIVE8_CURRENT_SETTING_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Current_Setting_Get");
    ADL2_Overdrive8_Setting_Set = (ADL2_OVERDRIVE8_SETTING_SET)GetProcAddress(hDLL, "ADL2_Overdrive8_Setting_Set");
    ADL2_New_QueryPMLogData_Get = (ADL2_NEW_QUERYPMLOGDATA_GET)GetProcAddress(hDLL, "ADL2_New_QueryPMLogData_Get");

    ADL2_Overdrive8_Init_SettingX2_Get = (ADL2_OVERDRIVE8_INIT_SETTINGX2_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Init_SettingX2_Get");
    ADL2_Overdrive8_Current_SettingX2_Get = (ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Current_SettingX2_Get");

    if (NULL == ADL_Main_Control_Create ||
        NULL == ADL_Main_Control_Destroy ||
        NULL == ADL_Adapter_NumberOfAdapters_Get ||
        NULL == ADL_Adapter_AdapterInfo_Get ||
        NULL == ADL_AdapterX2_Caps ||
        NULL == ADL2_Adapter_Active_Get ||
        NULL == ADL2_Overdrive_Caps ||
        NULL == ADL2_Adapter_RegValueInt_Get ||
        NULL == ADL2_Overdrive8_Init_Setting_Get ||
        NULL == ADL2_Overdrive8_Current_Setting_Get ||
        NULL == ADL2_Overdrive8_Setting_Set ||
        NULL == ADL2_New_QueryPMLogData_Get ||
        NULL == ADL2_Overdrive8_Init_SettingX2_Get ||
        NULL == ADL2_Overdrive8_Current_SettingX2_Get
        )
    {
        PRINTF("Failed to get ADL function pointers\n");
        return FALSE;
    }
    
    if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
    {
        printf("Failed to initialize nested ADL2 context");
        return ADL_ERR;
    }
    
    return TRUE;
}

void deinitializeADL()
{
    ADL_Main_Control_Destroy();
    if (NULL != hDLL)
    {
        FreeLibrary(hDLL);
        hDLL = NULL;
    }
}

bool GetOD8OneRange(ADLOD8InitSetting initSettings, int featureID_)
{
    bool RangeSupport_ = (initSettings.overdrive8Capabilities & featureID_) ? true : false;
    return RangeSupport_;
}

int GetOD8InitSetting(int iAdapterIndex, ADLOD8InitSetting &odInitSetting)
{
    int ret = -1;
    memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
    odInitSetting.count = OD8_COUNT;
    int overdrive8Capabilities;
    int numberOfFeatures = OD8_COUNT;
    ADLOD8SingleInitSetting* lpInitSettingList = NULL;
    if (NULL != ADL2_Overdrive8_Init_SettingX2_Get)
    {
        ret = ADL2_Overdrive8_Init_SettingX2_Get(context, iAdapterIndex, &overdrive8Capabilities, &numberOfFeatures, &lpInitSettingList);
        if (0 == ret)
        {
            ret = -1;
            odInitSetting.count = numberOfFeatures > OD8_COUNT ? OD8_COUNT : numberOfFeatures;
            odInitSetting.overdrive8Capabilities = overdrive8Capabilities;
            for (int i = 0; i < odInitSetting.count; i++)
            {
                odInitSetting.od8SettingTable[i].defaultValue = lpInitSettingList[i].defaultValue;
                odInitSetting.od8SettingTable[i].featureID = lpInitSettingList[i].featureID;
                odInitSetting.od8SettingTable[i].maxValue = lpInitSettingList[i].maxValue;
                odInitSetting.od8SettingTable[i].minValue = lpInitSettingList[i].minValue;
            }
            ADL_Main_Memory_Free((void**)&lpInitSettingList);
        }
        else
        {
            PRINTF("ADL2_Overdrive8_Init_SettingX2_Get is failed\n");
            ADL_Main_Memory_Free((void**)&lpInitSettingList);
            return ADL_ERR;
        }
    }
    else
    {
        if (NULL != ADL2_Overdrive8_Init_Setting_Get)
        {
            ret = ADL2_Overdrive8_Init_Setting_Get(context, iAdapterIndex, &odInitSetting);
            if (0 == ret)
                ret = -1;
            else
            {
                PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                return ADL_ERR;
            }
        }
    }
    return ADL_OK;
}

int GetOD8CurrentSetting(int iAdapterIndex, ADLOD8CurrentSetting &odCurrentSetting)
{
    int ret = -1;
    memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
    odCurrentSetting.count = OD8_COUNT;

    int numberOfFeaturesCurrent = OD8_COUNT;
    int* lpCurrentSettingList = NULL;
    if (NULL != ADL2_Overdrive8_Current_SettingX2_Get)
    {
        ret = ADL2_Overdrive8_Current_SettingX2_Get(context, iAdapterIndex, &numberOfFeaturesCurrent, &lpCurrentSettingList);
        if (0 == ret)
        {
            ret = -1;
            odCurrentSetting.count = numberOfFeaturesCurrent > OD8_COUNT ? OD8_COUNT : numberOfFeaturesCurrent;
            for (int i = 0; i < odCurrentSetting.count; i++)
            {
                odCurrentSetting.Od8SettingTable[i] = lpCurrentSettingList[i];
            }
            ADL_Main_Memory_Free((void**)&lpCurrentSettingList);
        }
        else
        {
            PRINTF("ADL2_Overdrive8_Current_SettingX2_Get is failed\n");
            ADL_Main_Memory_Free((void**)&lpCurrentSettingList);
            return ADL_ERR;
        }
    }
    else
    {
        if (NULL != ADL2_Overdrive8_Current_Setting_Get)
        {
            ret = ADL2_Overdrive8_Current_Setting_Get(context, iAdapterIndex, &odCurrentSetting);
            if (0 == ret)
                ret = -1;
            else
            {
                PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                return ADL_ERR;
            }
        }

    }
    return ADL_OK;
}
int printOD8GPUClocksParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS ||
                        (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) == ADL_OD8_GFXCLK_CURVE)
                    {
                        //GPU clocks
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FREQ1, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FREQ2, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FREQ3, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FMIN, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FMAX, ADL_OD8_GFXCLK_CURVE);

                        PRINTF("ADLSensorType: PMLOG_CLK_GFXCLK\n");
                        PRINTF("PMLOG_CLK_GFXCLK.supported:%d\n", odlpDataOutput.sensors[PMLOG_CLK_GFXCLK].supported);
                        PRINTF("PMLOG_CLK_GFXCLK.value:%d\n", odlpDataOutput.sensors[PMLOG_CLK_GFXCLK].value);
                        PRINTF("-----------------------------------------\n");

                    }
                    else
                        PRINTF("OD8 Failed to get GPU clocks\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}


int SetOD8GPUClocksParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS ||
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) == ADL_OD8_GFXCLK_CURVE)
                {
                    if (OD8_GFXCLK_FMIN == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FMIN parameters failed.\n");
                    }
                    else if (OD8_GFXCLK_FMAX == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FMAX parameters failed.\n");
                    }
                    else if (OD8_GFXCLK_FREQ1 == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FREQ1 parameters failed.\n");
                    }
                    else if (OD8_GFXCLK_FREQ2 == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FREQ2 parameters failed.\n");
                    }
                    else if (OD8_GFXCLK_FREQ3 == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FREQ3 parameters failed.\n");
                    }
                    else
                        PRINTF("Set Error settingID.\n");
                }
                break;
            }
        }

    }
    return 0;
}

int printOD8GPUVoltageParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                 
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS ||
                        (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) == ADL_OD8_GFXCLK_CURVE)
                    {
                        //GPU Voltage
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_VOLTAGE1, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_VOLTAGE2, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_VOLTAGE3, ADL_OD8_GFXCLK_CURVE);
                    }
                    else
                        PRINTF("OD8 Failed to get GPU voltages\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8GPUVoltageParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS ||
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) == ADL_OD8_GFXCLK_CURVE)
                {
                    if (OD8_GFXCLK_VOLTAGE1 == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUVoltageParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_VOLTAGE1 parameters failed.\n");
                    }
                    else if (OD8_GFXCLK_VOLTAGE2 == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUVoltageParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_VOLTAGE2 parameters failed.\n");
                    }
                    else if (OD8_GFXCLK_VOLTAGE3 == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUVoltageParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_VOLTAGE3 parameters failed.\n");
                    }
                    else
                        PRINTF("Set Error settingID.\n");
                }

                break;
            }
        }

    }
    return 0;
}

int printOD8MemoryClocksParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                 
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_UCLK_MAX) == ADL_OD8_UCLK_MAX)
                    {
                        //Memory Clocks
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_UCLK_FMAX, ADL_OD8_UCLK_MAX);

                        PRINTF("ADLSensorType: PMLOG_CLK_MEMCLK\n");
                        PRINTF("PMLOG_CLK_MEMCLK.supported:%d\n", odlpDataOutput.sensors[PMLOG_CLK_MEMCLK].supported);
                        PRINTF("PMLOG_CLK_MEMCLK.value:%d\n", odlpDataOutput.sensors[PMLOG_CLK_MEMCLK].value);
                        PRINTF("-----------------------------------------\n");

                    }
                    else
                        PRINTF("OD8 Failed to get Memory Clocks\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8MemoryClocksParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_UCLK_MAX) == ADL_OD8_UCLK_MAX)
                {
                    if (OD8_UCLK_FMAX == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8MemoryClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_UCLK_FMAX parameters failed.\n");
                    }
                    else
                        PRINTF("Set Error settingID.\n");
                }
                break;
            }
        }

    }
    return 0;
}

int printOD8TemperatureSettingParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                 
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if (((odInitSetting.overdrive8Capabilities &  ADL_OD8_TEMPERATURE_SYSTEM) == ADL_OD8_TEMPERATURE_SYSTEM) ||
                        ((odInitSetting.overdrive8Capabilities &  ADL_OD8_TEMPERATURE_FAN) == ADL_OD8_TEMPERATURE_FAN) ||
                        ((odInitSetting.overdrive8Capabilities &  ADL_OD8_POWER_LIMIT) == ADL_OD8_POWER_LIMIT))
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_OPERATING_TEMP_MAX, ADL_OD8_TEMPERATURE_SYSTEM);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_FAN_TARGET_TEMP, ADL_OD8_TEMPERATURE_FAN);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_POWER_PERCENTAGE, ADL_OD8_POWER_LIMIT);

                        PRINTF("ADLSensorType: PMLOG_TEMPERATURE_EDGE - Current Temp\n");
                        PRINTF("PMLOG_TEMPERATURE_EDGE.supported:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].supported);
                        PRINTF("PMLOG_TEMPERATURE_EDGE.value:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].value);

                        PRINTF("ADLSensorType: PMLOG_TEMPERATURE_HOTSPOT - Junction Temp\n");
                        PRINTF("PMLOG_TEMPERATURE_HOTSPOT.supported:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_HOTSPOT].supported);
                        PRINTF("PMLOG_TEMPERATURE_HOTSPOT.value:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_HOTSPOT].value);

                        PRINTF("-----------------------------------------\n");

                    }
                    else
                        PRINTF("OD8 Failed to get Temperature Settings\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8TemperatureSettingParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if (((odInitSetting.overdrive8Capabilities &  ADL_OD8_TEMPERATURE_SYSTEM) == ADL_OD8_TEMPERATURE_SYSTEM) ||
                    ((odInitSetting.overdrive8Capabilities &  ADL_OD8_TEMPERATURE_FAN) == ADL_OD8_TEMPERATURE_FAN) ||
                    ((odInitSetting.overdrive8Capabilities &  ADL_OD8_POWER_LIMIT) == ADL_OD8_POWER_LIMIT))
                {
                    if (OD8_FAN_TARGET_TEMP == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8TemperatureSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_FAN_TARGET_TEMP parameters failed.\n");
                    }
                    else if (OD8_POWER_PERCENTAGE == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8TemperatureSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_POWER_PERCENTAGE parameters failed.\n");
                    }
                    else if (OD8_OPERATING_TEMP_MAX == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8TemperatureSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_OPERATING_TEMP_MAX parameters failed.\n");
                    }
                }
                break;
            }
        }

    }
    return 0;
}

int printOD8FanSettingParameters()
{
	int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;
    
	// Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ret = ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (ret == 0 && iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                 
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_ACOUSTIC_LIMIT_SCLK) == ADL_OD8_ACOUSTIC_LIMIT_SCLK ||
                        (odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_SPEED_MIN) == ADL_OD8_FAN_SPEED_MIN)
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_FAN_MIN_SPEED, ADL_OD8_FAN_SPEED_MIN);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_FAN_ACOUSTIC_LIMIT, ADL_OD8_ACOUSTIC_LIMIT_SCLK);

                        PRINTF("ADLSensorType: PMLOG_FAN_RPM\n");
                        PRINTF("PMLOG_FAN_RPM.supported:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].supported);
                        PRINTF("PMLOG_FAN_RPM.value:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].value);
                        PRINTF("-----------------------------------------\n");
                    }
                    // Fan Curve
                    else if ((odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_CURVE) == ADL_OD8_FAN_CURVE) 
                    {
                        PRINTF("ADLSensorType: PMLOG_FAN_RPM\n");
                        PRINTF("OD8_FAN_RPM:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].value);
                        PRINTF("-----------------------------------------\n");
                    }
                    else
                        PRINTF("OD8 Failed to get Fan Settings\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8FanSettingParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_ACOUSTIC_LIMIT_SCLK) == ADL_OD8_ACOUSTIC_LIMIT_SCLK ||
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_SPEED_MIN) == ADL_OD8_FAN_SPEED_MIN ||
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_CURVE) == ADL_OD8_FAN_CURVE)
                {
                    if (OD8_FAN_MIN_SPEED == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8FanSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_FAN_MIN_SPEED parameters failed.\n");
                    }
                    else if (OD8_FAN_ACOUSTIC_LIMIT == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8FanSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_FAN_ACOUSTIC_LIMIT parameters failed.\n");
                    }
                }
                break;
            }
        }
    }
    return 0;
}

int printOD8MemoryTimingSettingParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                 
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_MEMORY_TIMING_TUNE) == ADL_OD8_MEMORY_TIMING_TUNE)
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AC_TIMING, ADL_OD8_MEMORY_TIMING_TUNE);
                    }
                    else
                        PRINTF("OD8 Failed to get Memory Timing Settings\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8MemoryTimingSettingParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }

                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_MEMORY_TIMING_TUNE) == ADL_OD8_MEMORY_TIMING_TUNE)
                {

                    if (OD8_AC_TIMING == SettingId)
                    {
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8MemoryTimingSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_AC_TIMING parameters failed.\n");
                    }
                }
                break;
            }
        }

    }
    return 0;
}

int printOD8ZeroFanControlSettingParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                 
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_ZERO_RPM_CONTROL) == ADL_OD8_FAN_ZERO_RPM_CONTROL)
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_FAN_ZERORPM_CONTROL, ADL_OD8_FAN_ZERO_RPM_CONTROL);
                    }
                    else
                        PRINTF("OD8 Failed to get Zero Fan Control Settings\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }
    }
    return 0;
}

int printOD8TuningControlSettingParameters()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }

                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_UV_ENGINE) == ADL_OD8_AUTO_UV_ENGINE)
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AUTO_UV_ENGINE_CONTROL, ADL_OD8_AUTO_UV_ENGINE);
                    }
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_OC_ENGINE) == ADL_OD8_AUTO_OC_ENGINE)
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AUTO_OC_ENGINE_CONTROL, ADL_OD8_AUTO_OC_ENGINE);
                    }
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_OC_MEMORY) == ADL_OD8_AUTO_OC_MEMORY)
                    {
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AUTO_OC_MEMORY_CONTROL, ADL_OD8_AUTO_OC_MEMORY);
                    }
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }
    }
    return 0;
}

int SetOD8ZeroFanControlSettingParameters(int SettingId, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_ZERO_RPM_CONTROL) == ADL_OD8_FAN_ZERO_RPM_CONTROL)
                {
                    if (OD8_FAN_ZERORPM_CONTROL == SettingId)
                    {
                        //do not reset
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, 0, value))
                        {
                            printOD8ZeroFanControlSettingParameters();
                        }
                        else
                            PRINTF("Set OD8_FAN_ZERORPM_CONTROL parameters failed.\n");
                    }
                }
                break;
            }
        }

    }
    return 0;
}

int PrintFeatureName(int itemID_)
{
    switch (itemID_)
    {
    case OD8_GFXCLK_FREQ1:
        PRINTF("OD8_GFXCLK_FREQ1:");
        break;
    case OD8_GFXCLK_FREQ2:
        PRINTF("OD8_GFXCLK_FREQ2:");
        break;
    case OD8_GFXCLK_FREQ3:
        PRINTF("OD8_GFXCLK_FREQ3:");
        break;
    case OD8_GFXCLK_FMIN:
        PRINTF("OD8_GFXCLK_FMIN:");
        break;
    case OD8_GFXCLK_FMAX:
        PRINTF("OD8_GFXCLK_FMAX:");
        break;
    case OD8_GFXCLK_VOLTAGE1:
        PRINTF("OD8_GFXCLK_VOLTAGE1:");
        break;
    case OD8_GFXCLK_VOLTAGE2:
        PRINTF("OD8_GFXCLK_VOLTAGE2:");
        break;
    case OD8_GFXCLK_VOLTAGE3:
        PRINTF("OD8_GFXCLK_VOLTAGE3:");
        break;
    case OD8_UCLK_FMAX:
        PRINTF("OD8_UCLK_FMAX:");
        break;
    case OD8_OPERATING_TEMP_MAX:
        PRINTF("OD8_OPERATING_TEMP_MAX:");
        break;
    case OD8_FAN_TARGET_TEMP:
        PRINTF("OD8_FAN_TARGET_TEMP:");
        break;
    case OD8_POWER_PERCENTAGE:
        PRINTF("OD8_POWER_PERCENTAGE:");
        break;
    case OD8_FAN_MIN_SPEED:
        PRINTF("OD8_FAN_MIN_SPEED:");
        break;
    case OD8_FAN_ACOUSTIC_LIMIT:
        PRINTF("OD8_FAN_ACOUSTIC_LIMIT:");
        break;
    case OD8_AC_TIMING:
        PRINTF("OD8_AC_TIMING:");
        break;
    case OD8_FAN_ZERORPM_CONTROL:
        PRINTF("OD8_FAN_ZERORPM_CONTROL:");
        break;
    case OD8_FAN_CURVE_TEMPERATURE_1:
        PRINTF("OD8_FAN_CURVE_TEMPERATURE_1:");
        break;
    case OD8_FAN_CURVE_SPEED_1:
        PRINTF("OD8_FAN_CURVE_SPEED_1:");
        break;
    case OD8_FAN_CURVE_TEMPERATURE_2:
        PRINTF("OD8_FAN_CURVE_TEMPERATURE_2:");
        break;
    case OD8_FAN_CURVE_SPEED_2:
        PRINTF("OD8_FAN_CURVE_SPEED_2:");
        break;
    case OD8_FAN_CURVE_TEMPERATURE_3:
        PRINTF("OD8_FAN_CURVE_TEMPERATURE_3:");
        break;
    case OD8_FAN_CURVE_SPEED_3:
        PRINTF("OD8_FAN_CURVE_SPEED_3:");
        break;
    case OD8_FAN_CURVE_TEMPERATURE_4:
        PRINTF("OD8_FAN_CURVE_TEMPERATURE_4:");
        break;
    case OD8_FAN_CURVE_SPEED_4:
        PRINTF("OD8_FAN_CURVE_SPEED_4 = 26:");
        break;
    case OD8_FAN_CURVE_TEMPERATURE_5:
        PRINTF("OD8_FAN_CURVE_TEMPERATURE_5:");
        break;
    case OD8_FAN_CURVE_SPEED_5:
        PRINTF("OD8_FAN_CURVE_SPEED_5:");
        break;
    default:
        PRINTF("Found no featureID \n");
        return ADL_ERR;
    }
    return ADL_OK;
}

int SetOD8Range(const ADLOD8InitSetting &odInitSetting, ADLOD8CurrentSetting &odCurrentSetting, int iAdapterIndex, int SettingId, int Reset, int value)
{
    ADLOD8SetSetting odSetSetting;
    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
    odSetSetting.count = OD8_COUNT;
    // setting
    for (int i = OD8_GFXCLK_FREQ1; i <= OD8_UCLK_FMAX; ++i)
    {
        odSetSetting.od8SettingTable[i].requested = 1;
        odSetSetting.od8SettingTable[i].value = odCurrentSetting.Od8SettingTable[i];
    }
    bool reset = true;
    if (SettingId <= OD8_FAN_CURVE_SPEED_5 && SettingId >= OD8_FAN_CURVE_TEMPERATURE_1)
    {
        reset = false;
    }

    for (int i = OD8_FAN_CURVE_TEMPERATURE_1; i <= OD8_FAN_CURVE_SPEED_5; ++i)
    {
        odSetSetting.od8SettingTable[i].reset = reset;
        odSetSetting.od8SettingTable[i].requested = 1;
        odSetSetting.od8SettingTable[i].value = odCurrentSetting.Od8SettingTable[i];
    }

    odSetSetting.od8SettingTable[SettingId].requested = 1;
    if (ADL_OK == PrintFeatureName(SettingId))
    {
        if (!(odInitSetting.od8SettingTable[SettingId].minValue <= value && odInitSetting.od8SettingTable[SettingId].maxValue >= value))
        {
            PrintFeatureName(SettingId);
            PRINTF("range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[SettingId].minValue, odInitSetting.od8SettingTable[SettingId].maxValue);
            return ADL_ERR;
        }
        else
        {
            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
            {
                odSetSetting.od8SettingTable[SettingId].value = value;
                if (OD8_GFXCLK_FMAX == SettingId )
                    odSetSetting.od8SettingTable[OD8_GFXCLK_FREQ3].value = value;
                else if (OD8_GFXCLK_FMIN == SettingId)
                    odSetSetting.od8SettingTable[OD8_GFXCLK_FREQ1].value = value;
            }
            else if (1 == Reset)
            {
                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
            }

            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, iAdapterIndex, &odSetSetting, &odCurrentSetting))
            {
                PrintFeatureName(SettingId);
                PRINTF("ADL2_Overdrive8_Setting_Set is Success\n\n");
                PRINTF("****** Driver Values: After Apply ******\n");
            }
            else
            {
                PrintFeatureName(SettingId);
                PRINTF("ADL2_Overdrive8_Setting_Set is failed\n");
                return ADL_ERR;
            }
        }
    }
    else
    {
        PRINTF("Found No Feature ID.\n");
        return ADL_ERR;
    }
    return ADL_OK;
}


int GetOD8RangePrint(ADLOD8InitSetting odInitSetting, ADLOD8CurrentSetting odCurrentSetting, OverdriveRangeDataStruct &oneRangeData, int itemID_, int featureID_)
{
    memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
    oneRangeData.Max_ = odInitSetting.od8SettingTable[itemID_].maxValue;
    oneRangeData.Min_ = odInitSetting.od8SettingTable[itemID_].minValue;
    oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[itemID_].defaultValue;
    if (OD8_GFXCLK_FMIN == itemID_ || OD8_GFXCLK_FMAX == itemID_ || OD8_UCLK_FMAX == itemID_ || OD8_AC_TIMING == itemID_ || OD8_FAN_ZERORPM_CONTROL == itemID_
         || OD8_AUTO_UV_ENGINE_CONTROL == itemID_ || OD8_AUTO_OC_ENGINE_CONTROL == itemID_ || OD8_AUTO_OC_MEMORY_CONTROL == itemID_)
        oneRangeData.Visible_ = true;
    else
        oneRangeData.Visible_ = ((odInitSetting.overdrive8Capabilities & featureID_) == featureID_) ? true : false;
    PRINTF("itemID:");
    PrintFeatureName(itemID_);
    if (oneRangeData.Visible_) {
        PRINTF(" is Visible\n");
        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[itemID_];
        PRINTF("actualValue:%d\n", oneRangeData.ExpectedValue_);
    }
    else 
        PRINTF(" is not Visible\n");
    PrintFeatureName(itemID_);
    PRINTF("maxValue:%d\n", oneRangeData.Max_);
    PRINTF("minValue:%d\n", oneRangeData.Min_);
    PRINTF("defaultValue:%d\n", oneRangeData.DefaultValue_);
    PRINTF("-----------------------------------------\n");
    return ADL_OK;
}

int PrintfOD8FanCurve()
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if (!GetOD8OneRange(odInitSetting, ADL_OD8_FAN_CURVE))
                {
                    PRINTF("Not support Fan Curve.\n");
                    return ADL_ERR;
                }
                else
                {
                    PRINTF("The follwoing is Fan curve temperature and speed:\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_1 = 19\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_1 = 20\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_2 = 21\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_2 = 22\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_3 = 23\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_3 = 24\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_4 = 25\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_4 = 26\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_5 = 27\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_5 = 28 \n");
                }
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    OverdriveRangeDataStruct oneRangeData;
                    for (int index = OD8_FAN_CURVE_TEMPERATURE_1; index <= OD8_FAN_CURVE_SPEED_5; ++index)
                    {
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, index, ADL_OD8_FAN_CURVE);
                    }
                    PRINTF("ADLSensorType: PMLOG_CLK_GFXCLK\n");
                    PRINTF("PMLOG_CLK_GFXCLK.supported:%d\n", odlpDataOutput.sensors[PMLOG_CLK_GFXCLK].supported);
                    PRINTF("PMLOG_CLK_GFXCLK.value:%d\n", odlpDataOutput.sensors[PMLOG_CLK_GFXCLK].value);
                    PRINTF("-----------------------------------------\n");
                }
                else
                {
                    PRINTF("ADL2_New_QueryPMLogData_Get is failed\n");
                    return ADL_ERR;
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8FanCurveSettingParameters(int SettingId, int Reset, int value)
{
    int i;
    int ret = -1;
    int iSupported = 0, iEnabled = 0, iVersion = 0;

    // Repeat for all available adapters in the system
    for (i = 0; i < iNumberAdapters; i++)
    {
        PRINTF("-----------------------------------------\n");
        PRINTF("Adapter Index[%d]\n ", lpAdapterInfo[i].iAdapterIndex);
        PRINTF("-----------------------------------------\n");
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                if (ADL_OK != GetOD8InitSetting(lpAdapterInfo[i].iAdapterIndex, odInitSetting))
                {
                    PRINTF("Get Init Setting failed.\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
                {
                    PRINTF("Get Current Setting failed.\n");
                    return ADL_ERR;
                }
                if (!GetOD8OneRange(odInitSetting, ADL_OD8_FAN_CURVE))
                {
                    return ADL_ERR;
                }
                else
                {
                    PRINTF("The follwoing is Fan curve temperature and speed:\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_1 = 19\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_1 = 20\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_2 = 21\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_2 = 22\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_3 = 23\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_3 = 24\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_4 = 25\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_4 = 26\n");
                    PRINTF("OD8_FAN_CURVE_TEMPERATURE_5 = 27\n");
                    PRINTF("OD8_FAN_CURVE_SPEED_5 = 28 \n");
                }
                if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                    PrintfOD8FanCurve();
                break;
            }
        }

    }
    return 0;
}

void printHelp(char *exeName)
{
    PRINTF("-----------------------------------------\n");
    PRINTF("This Application accepts below parameters\n");
    PRINTF("-----------------------------------------\n");
    //GPU Clocks
    PRINTF("Method to Read OD8 GPU Clocks: \t Overdrive8.exe c\n");
    PRINTF("Ex: %s c\n\n", exeName);
    PRINTF("Method to Set OD8 GPU Clocks: \t Overdrive8.exe c X Y Z; X - (0:OD8_GFXCLK_FMIN, 1:OD8_GFXCLK_FMAX, 2:OD8_GFXCLK_FREQ1, 4:OD8_GFXCLK_FREQ2, 6:OD8_GFXCLK_FREQ3); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s c 0 0 10\n\n", exeName);
    //GPU Voltage
    PRINTF("Method to Read OD8 GPU Voltage: \t Overdrive8.exe v\n");
    PRINTF("Ex: %s v\n\n", exeName);
    PRINTF("Method to Set OD8 GPU Voltage: \t Overdrive8.exe v X Y Z; X - (3:OD8_GFXCLK_VOLTAGE1, 5:OD8_GFXCLK_VOLTAGE2, 7:OD8_GFXCLK_VOLTAGE3); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s v 3 0 10\n\n", exeName);
    //Memory Clock
    PRINTF("Method to Read OD8 Memory Clock: \t Overdrive8.exe m\n");
    PRINTF("Ex: %s m\n\n", exeName);
    PRINTF("Method to Set OD8 Memory Clocks: \t Overdrive8.exe m X Y Z; X - (8:OD8_UCLK_FMAX); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s m 8 0 10\n\n", exeName);
    //Temperature Settings
    PRINTF("Method to Read OD8 Temperature Settings: \t Overdrive8.exe t\n");
    PRINTF("Ex: %s t\n\n", exeName);
    PRINTF("Method to Set OD8 Temperature Settings: \t Overdrive8.exe t X Y Z; X - (9:OD8_POWER_PERCENTAGE, 12:OD8_FAN_TARGET_TEMP, 13:OD8_OPERATING_TEMP_MAX); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s t 9 0 10\n\n", exeName);
    //Fan Setting
    PRINTF("Method to Read OD8 Fan Setting: \t Overdrive8.exe f\n");
    PRINTF("Ex: %s f\n\n", exeName);
    PRINTF("Method to Set OD8 Fan Setting: \t Overdrive8.exe f X Y Z; X - (10:OD8_FAN_ACOUSTIC_LIMIT, 11:OD8_FAN_MIN_SPEED); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s f 10 0 10\n\n", exeName);
    //Memory Timing Setting
    PRINTF("Method to Read OD8 Memory Timing Setting: \t Overdrive8.exe s\n");
    PRINTF("Ex: %s s\n\n", exeName);
    PRINTF("Method to Set OD8 Memory Timing Setting: \t Overdrive8.exe s X Y Z; X - (14:OD8_AC_TIMING); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s s 14 0 10\n\n", exeName);
    //Fan Zero RPM
    PRINTF("Method to Read OD8 Fan Zero RPM: \t Overdrive8.exe z\n");
    PRINTF("Ex: %s z\n\n", exeName);
    PRINTF("Method to Set OD8 Fan Zero RPM: \t Overdrive8.exe z X Y ; X - (15:OD8_FAN_ZERORPM_CONTROL); Y - (value of setting)\n");
    PRINTF("EX: %s z 15 0\n\n", exeName);
}

int main(int argc, char* argv[])
{

    if (initializeADL())
    {
        if (argc > 1)
        {
            // Obtain the number of adapters for the system
            if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters))
            {
                PRINTF("Cannot get the number of adapters!\n");
                return 0;
            }
            if (0 < iNumberAdapters)
            {
                lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo)* iNumberAdapters);
                memset(lpAdapterInfo, '\0', sizeof(AdapterInfo)* iNumberAdapters);
                // Get the AdapterInfo structure for all adapters in the system
                ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo)* iNumberAdapters);
            }

            if ('c' == *(argv[1]))//GPU Clocks
            {
                if (argc == 2)
                    printOD8GPUClocksParameters();
                else if (argc == 5)
                    SetOD8GPUClocksParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('v' == *(argv[1]))//GPU Voltage
            {
                if (argc == 2)
                    printOD8GPUVoltageParameters();
                else if (argc == 5)
                    SetOD8GPUVoltageParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('m' == *(argv[1]))//Memory Clocks
            {
                if (argc == 2)
                    printOD8MemoryClocksParameters();
                else if (argc == 5)
                    SetOD8MemoryClocksParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('t' == *(argv[1]))//Temperature Setting
            {
                if (argc == 2)
                    printOD8TemperatureSettingParameters();
                else if (argc == 5)
                    SetOD8TemperatureSettingParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('f' == *(argv[1]))//Fan Setting
            {
                if (argc == 2)
                    printOD8FanSettingParameters();
                else if (argc == 5)
                    SetOD8FanSettingParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('s' == *(argv[1]))//Memory Timing Setting
            {
                if (argc == 2)
                    printOD8MemoryTimingSettingParameters();
                else if (argc == 5)
                    SetOD8MemoryTimingSettingParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('z' == *(argv[1]))//Fan Zero RPM
            {
                if (argc == 2)
                    printOD8ZeroFanControlSettingParameters();
                else if (argc == 4)
                    SetOD8ZeroFanControlSettingParameters(atoi(argv[2]), atoi(argv[3]));
                else
                    printHelp(argv[0]);
            }
            else if ('t' == *(argv[1]))//tuning control
            {
                if (argc == 2)
                    printOD8TuningControlSettingParameters();
            }
            else if ('r' == *(argv[1]))//Fan curve
            {
                if (argc == 2)
                {
                    PrintfOD8FanCurve();
                }
                else if (argc == 5)
                    SetOD8FanCurveSettingParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
        }
        else
            printHelp(argv[0]);
        
        ADL_Main_Memory_Free((void**)&lpAdapterInfo);
        
        deinitializeADL();
    }

    return 0;
}
