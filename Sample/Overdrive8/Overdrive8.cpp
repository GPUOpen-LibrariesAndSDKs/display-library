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
    //Step value
    int Step_;
    //Expected value: similar to current value
    int ExpectedValue_;
    //Default value
    int DefaultValue_;
    //Actual value
    int ActualValue_;
    // The enabled state
    bool Enabled_;
    // If ActualValue can be got from the driver, ActualValueAvailable_ will be true
    bool ActualValueAvailable_;
    // If the Range feature is supported by the driver, it is true.
    bool RangeSupport_;
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_FREQ1];
                        oneRangeData.RangeSupport_ = (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) ? true : false;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_FREQ1\n");
                        PRINTF("OD8_GFXCLK_FREQ1.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_FREQ1.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_FREQ1.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_FREQ2];
                        oneRangeData.RangeSupport_ = (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) ? true : false;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_FREQ2\n");
                        PRINTF("OD8_GFXCLK_FREQ2.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_FREQ2.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_FREQ2.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ3].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ3].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ3].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_FREQ3];
                        oneRangeData.RangeSupport_ = (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) ? true : false;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_FREQ3\n");
                        PRINTF("OD8_GFXCLK_FREQ3.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_FREQ3.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_FREQ3.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_FMIN];
                        oneRangeData.RangeSupport_ = true;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_FMIN\n");
                        PRINTF("OD8_GFXCLK_FMIN.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_FMIN.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_FMIN.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_FMAX];
                        oneRangeData.RangeSupport_ = true;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_FMAX\n");
                        PRINTF("OD8_GFXCLK_FMAX.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_FMAX.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_FMAX.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_GFXCLK_FMIN == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_FMIN Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_FMIN].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_FMIN ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUClocksParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_FMIN ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_GFXCLK_FMAX == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_FMAX Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_FMAX].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_FMAX ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUClocksParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_FMAX ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_GFXCLK_FREQ1 == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_FREQ1 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ1].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_FREQ1 ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUClocksParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_FREQ1 ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_GFXCLK_FREQ2 == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_FREQ2 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_FREQ2 ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUClocksParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_FREQ2 ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_GFXCLK_FREQ3 == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ3].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ3].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_FREQ3 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ3].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_FREQ2].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_FREQ3 ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUClocksParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_FREQ3 ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }

                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_VOLTAGE1];
                        oneRangeData.RangeSupport_ = (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) ? true : false;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_VOLTAGE1\n");
                        PRINTF("OD8_GFXCLK_VOLTAGE1.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_VOLTAGE1.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_VOLTAGE1.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_VOLTAGE2];
                        oneRangeData.RangeSupport_ = (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) ? true : false;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_VOLTAGE2\n");
                        PRINTF("OD8_GFXCLK_VOLTAGE2.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_VOLTAGE2.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_VOLTAGE2.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_GFXCLK_VOLTAGE3];
                        oneRangeData.RangeSupport_ = (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) ? true : false;
                        oneRangeData.Step_ = 1;
                        PRINTF("FeatureID: OD8_GFXCLK_VOLTAGE3\n");
                        PRINTF("OD8_GFXCLK_VOLTAGE3.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_GFXCLK_VOLTAGE3.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_GFXCLK_VOLTAGE3.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_GFXCLK_FREQ1.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_GFXCLK_FREQ1.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_GFXCLK_VOLTAGE1 == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_VOLTAGE1 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE1].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_VOLTAGE1 ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUVoltageParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_VOLTAGE1 ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_GFXCLK_VOLTAGE2 == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_VOLTAGE2 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE2].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_VOLTAGE2 ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUVoltageParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_VOLTAGE2 ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_GFXCLK_VOLTAGE3 == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].minValue <= value && odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_VOLTAGE3 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].minValue, odInitSetting.od8SettingTable[OD8_GFXCLK_VOLTAGE3].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_GFXCLK_VOLTAGE3 ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8GPUVoltageParameters();
                            }
                            else
                            {
                                PRINTF("OD8_GFXCLK_VOLTAGE3 ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_UCLK_FMAX].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_UCLK_FMAX].minValue;
                        oneRangeData.Step_ = 1;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_UCLK_FMAX].defaultValue;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_UCLK_FMAX];
                        oneRangeData.RangeSupport_ = true;
                        oneRangeData.Visible_ = true;
                        oneRangeData.ActualValueAvailable_ = true;
                        oneRangeData.ActualValue_ = odCurrentSetting.Od8SettingTable[OD8_UCLK_FMAX];
                        PRINTF("FeatureID: OD8_UCLK_FMAX\n");
                        PRINTF("OD8_UCLK_FMAX.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_UCLK_FMAX.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_UCLK_FMAX.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_UCLK_FMAX.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_UCLK_FMAX.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");

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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_UCLK_FMAX == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_UCLK_FMAX].minValue <= value && odInitSetting.od8SettingTable[OD8_UCLK_FMAX].maxValue >= value))
                        {
                            PRINTF("OD8_GFXCLK_VOLTAGE1 Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_UCLK_FMAX].minValue, odInitSetting.od8SettingTable[OD8_UCLK_FMAX].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_UCLK_FMAX ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8MemoryClocksParameters();
                            }
                            else
                            {
                                PRINTF("OD8_UCLK_FMAX ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].defaultValue;
                        oneRangeData.Step_ = 1;
                        oneRangeData.Enabled_ = true;
                        oneRangeData.Visible_ = ((odInitSetting.overdrive8Capabilities &  ADL_OD8_TEMPERATURE_SYSTEM) == ADL_OD8_TEMPERATURE_SYSTEM) ? true : false;
                        if (oneRangeData.Visible_)
                        {
                            oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_OPERATING_TEMP_MAX];
                            PRINTF("OD8_OPERATING_TEMP_MAX.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        }
                        PRINTF("FeatureID: OD8_OPERATING_TEMP_MAX\n");
                        PRINTF("OD8_OPERATING_TEMP_MAX.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_OPERATING_TEMP_MAX.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_OPERATING_TEMP_MAX.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_OPERATING_TEMP_MAX.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].defaultValue;
                        oneRangeData.Step_ = 1;
                        oneRangeData.Enabled_ = true;
                        oneRangeData.Visible_ = ((odInitSetting.overdrive8Capabilities & ADL_OD8_TEMPERATURE_FAN) == ADL_OD8_TEMPERATURE_FAN) ? true : false;
                        if (oneRangeData.Visible_)
                        {
                            oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_FAN_TARGET_TEMP];
                            PRINTF("OD8_FAN_TARGET_TEMP.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        }
                        PRINTF("FeatureID: OD8_FAN_TARGET_TEMP\n");
                        PRINTF("OD8_FAN_TARGET_TEMP.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_FAN_TARGET_TEMP.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_FAN_TARGET_TEMP.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_FAN_TARGET_TEMP.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].defaultValue;
                        oneRangeData.Step_ = 1;
                        oneRangeData.Enabled_ = true;
                        oneRangeData.Visible_ = ((odInitSetting.overdrive8Capabilities &  ADL_OD8_POWER_LIMIT) == ADL_OD8_POWER_LIMIT) ? true : false;
                        if (oneRangeData.Visible_)
                        {
                            oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_POWER_PERCENTAGE];
                            PRINTF("OD8_POWER_PERCENTAGE.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        }
                        PRINTF("FeatureID: OD8_POWER_PERCENTAGE\n");
                        PRINTF("OD8_POWER_PERCENTAGE.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_POWER_PERCENTAGE.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_POWER_PERCENTAGE.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_POWER_PERCENTAGE.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("-----------------------------------------\n");

                        PRINTF("ADLSensorType: PMLOG_TEMPERATURE_EDGE\n");
                        PRINTF("PMLOG_TEMPERATURE_EDGE.supported:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].supported);
                        PRINTF("PMLOG_TEMPERATURE_EDGE.value:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].value);
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_FAN_TARGET_TEMP == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].minValue <= value && odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].maxValue >= value))
                        {
                            PRINTF("OD8_FAN_TARGET_TEMP Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].minValue, odInitSetting.od8SettingTable[OD8_FAN_TARGET_TEMP].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_FAN_TARGET_TEMP ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8TemperatureSettingParameters();
                            }
                            else
                            {
                                PRINTF("OD8_FAN_TARGET_TEMP ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_POWER_PERCENTAGE == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].minValue <= value && odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].maxValue >= value))
                        {
                            PRINTF("OD8_POWER_PERCENTAGE Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].minValue, odInitSetting.od8SettingTable[OD8_POWER_PERCENTAGE].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_POWER_PERCENTAGE ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8TemperatureSettingParameters();
                            }
                            else
                            {
                                PRINTF("OD8_POWER_PERCENTAGE ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_OPERATING_TEMP_MAX == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].minValue <= value && odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].maxValue >= value))
                        {
                            PRINTF("OD8_OPERATING_TEMP_MAX Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].minValue, odInitSetting.od8SettingTable[OD8_OPERATING_TEMP_MAX].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_OPERATING_TEMP_MAX ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8TemperatureSettingParameters();
                            }
                            else
                            {
                                PRINTF("OD8_OPERATING_TEMP_MAX ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }

                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
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
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (iVersion == 8)
            {
                //OD8 initial Status
                ADLOD8InitSetting odInitSetting;
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].defaultValue;
                        oneRangeData.Step_ = 1;
                        oneRangeData.Enabled_ = true;
                        oneRangeData.Visible_ = ((odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_SPEED_MIN) == ADL_OD8_FAN_SPEED_MIN) ? true : false;
                        if (oneRangeData.Visible_)
                        {
                            oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_FAN_MIN_SPEED];
                            PRINTF("OD8_FAN_MIN_SPEED.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        }
                        PRINTF("FeatureID: OD8_FAN_MIN_SPEED\n");
                        PRINTF("OD8_FAN_MIN_SPEED.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_FAN_MIN_SPEED.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_FAN_MIN_SPEED.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_FAN_MIN_SPEED.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("-----------------------------------------\n");

                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].maxValue;
                        oneRangeData.Min_ = odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].minValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].defaultValue;
                        oneRangeData.Step_ = 1;
                        oneRangeData.Enabled_ = true;
                        oneRangeData.Visible_ = ((odInitSetting.overdrive8Capabilities & ADL_OD8_ACOUSTIC_LIMIT_SCLK) == ADL_OD8_ACOUSTIC_LIMIT_SCLK) ? true : false;
                        if (oneRangeData.Visible_)
                        {
                            oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT];
                            PRINTF("OD8_FAN_ACOUSTIC_LIMIT.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        }
                        PRINTF("FeatureID: OD8_FAN_ACOUSTIC_LIMIT\n");
                        PRINTF("OD8_FAN_ACOUSTIC_LIMIT.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_FAN_ACOUSTIC_LIMIT.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_FAN_ACOUSTIC_LIMIT.step:%d\n", oneRangeData.Step_);
                        PRINTF("OD8_FAN_ACOUSTIC_LIMIT.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("-----------------------------------------\n");

                        PRINTF("ADLSensorType: PMLOG_FAN_RPM\n");
                        PRINTF("PMLOG_FAN_RPM.supported:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].supported);
                        PRINTF("PMLOG_FAN_RPM.value:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].value);
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_FAN_MIN_SPEED == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].minValue <= value && odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].maxValue >= value))
                        {
                            PRINTF("OD8_FAN_MIN_SPEED Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].minValue, odInitSetting.od8SettingTable[OD8_FAN_MIN_SPEED].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_FAN_MIN_SPEED ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8FanSettingParameters();
                            }
                            else
                            {
                                PRINTF("OD8_FAN_MIN_SPEED ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                    else if (OD8_FAN_ACOUSTIC_LIMIT == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].minValue <= value && odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].maxValue >= value))
                        {
                            PRINTF("OD8_FAN_ACOUSTIC_LIMIT Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].minValue, odInitSetting.od8SettingTable[OD8_FAN_ACOUSTIC_LIMIT].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_FAN_ACOUSTIC_LIMIT ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8FanSettingParameters();
                            }
                            else
                            {
                                PRINTF("OD8_FAN_ACOUSTIC_LIMIT ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }

                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.Min_ = 1;
                        oneRangeData.Max_ = odInitSetting.od8SettingTable[OD8_AC_TIMING].maxValue;
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_AC_TIMING].defaultValue;
                        oneRangeData.Enabled_ = true;
                        oneRangeData.Visible_ = true;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_AC_TIMING];
                        PRINTF("FeatureID: OD8_AC_TIMING\n");
                        PRINTF("OD8_AC_TIMING.maxValue:%d\n", oneRangeData.Max_);
                        PRINTF("OD8_AC_TIMING.minValue:%d\n", oneRangeData.Min_);
                        PRINTF("OD8_AC_TIMING.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_AC_TIMING.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_AC_TIMING == SettingId)
                    {
                        if (!(odInitSetting.od8SettingTable[OD8_AC_TIMING].minValue <= value && odInitSetting.od8SettingTable[OD8_AC_TIMING].maxValue >= value))
                        {
                            PRINTF("OD8_AC_TIMING Fan speed range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_AC_TIMING].minValue, odInitSetting.od8SettingTable[OD8_AC_TIMING].maxValue);
                            return ADL_ERR;
                        }
                        else
                        {
                            if (0 == Reset)//0 - do not reset , 1 - reset setting back to default
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = value;
                            }
                            else if (1 == Reset)
                            {
                                odSetSetting.od8SettingTable[SettingId].reset = Reset;
                                odSetSetting.od8SettingTable[SettingId].value = odInitSetting.od8SettingTable[SettingId].defaultValue;
                            }

                            if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                            {
                                PRINTF("OD8_AC_TIMING ADL2_Overdrive8_Setting_Set is Success\n\n");
                                PRINTF("****** Driver Values: After Apply ******\n");
                                printOD8MemoryTimingSettingParameters();
                            }
                            else
                            {
                                PRINTF("OD8_AC_TIMING ADL2_Overdrive8_Setting_Set is failed\n");
                                return ADL_ERR;
                            }
                        }
                    }
                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
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
                        memset(&oneRangeData, 0, sizeof(OverdriveRangeDataStruct));
                        oneRangeData.DefaultValue_ = odInitSetting.od8SettingTable[OD8_FAN_ZERORPM_CONTROL].defaultValue;
                        oneRangeData.Visible_ = true;
                        oneRangeData.ExpectedValue_ = odCurrentSetting.Od8SettingTable[OD8_FAN_ZERORPM_CONTROL];
                        oneRangeData.Enabled_ = true;
                        PRINTF("FeatureID: OD8_FAN_ZERORPM_CONTROL\n");
                        PRINTF("OD8_FAN_ZERORPM_CONTROL.defaultValue:%d\n", oneRangeData.DefaultValue_);
                        PRINTF("OD8_FAN_ZERORPM_CONTROL.actualValue:%d\n", oneRangeData.ExpectedValue_);
                        PRINTF("-----------------------------------------\n");
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
                memset(&odInitSetting, 0, sizeof(ADLOD8InitSetting));
                odInitSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Init_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odInitSetting);
                if (0 == ret)
                    ret = -1;
                else
                {
                    PRINTF("ADL2_Overdrive8_Init_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                //OD8 Current Status
                ADLOD8CurrentSetting odCurrentSetting;
                memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
                odCurrentSetting.count = OD8_COUNT;
                ret = ADL2_Overdrive8_Current_Setting_Get(context, lpAdapterInfo[i].iAdapterIndex, &odCurrentSetting);
                if (0 == ret)
                {
                    ret = -1;

                    ADLOD8SetSetting odSetSetting;
                    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
                    odSetSetting.count = OD8_COUNT;
                    odSetSetting.od8SettingTable[SettingId].requested = 1;

                    if (OD8_FAN_ZERORPM_CONTROL == SettingId)
                    {
                        odSetSetting.od8SettingTable[SettingId].value = value;

                        if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &odSetSetting, &odCurrentSetting))
                        {
                            PRINTF("OD8_FAN_ZERORPM_CONTROL ADL2_Overdrive8_Setting_Set is Success\n\n");
                            PRINTF("****** Driver Values: After Apply ******\n");
                            printOD8ZeroFanControlSettingParameters();
                        }
                        else
                        {
                            PRINTF("OD8_FAN_ZERORPM_CONTROL ADL2_Overdrive8_Setting_Set is failed\n");
                            return ADL_ERR;
                        }
                    }
                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_Setting_Get is failed\n");
                    return ADL_ERR;
                }

                break;
            }
        }

    }
    return 0;
}

int GetInitOD8Setting()
{

    ADLOD8InitSetting overdriveInitSetting;
    memset(&overdriveInitSetting, 0, sizeof(ADLOD8InitSetting));
    overdriveInitSetting.count = OD8_COUNT;
    int overdrive8Capabilities;
    int numberOfFeatures = OD8_COUNT;
    ADLOD8SingleInitSetting* lpInitSettingList = NULL;
    int iSupported, iEnabled, iVersion;
    int i, ret = ADL_FALSE;
    for (i = 0; i < iNumberAdapters; i++)
    {
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (NULL != ADL2_Overdrive8_Init_SettingX2_Get)
            {
                ret = ADL2_Overdrive8_Init_SettingX2_Get(context, lpAdapterInfo[i].iAdapterIndex, &overdrive8Capabilities, &numberOfFeatures, &lpInitSettingList);
                if (ADL_OK != ret)
                {
                    PRINTF("ADL2_Overdriven8_Init_SettingX2_get is failed\n");
                }
                else
                {
                    PRINTF("ADL2_Overdriven8_Init_SettingX2_get is Success\n\n");
                    PRINTF("****** Driver Values: After Apply ******\n");
                    overdriveInitSetting.count = numberOfFeatures > OD8_COUNT ? OD8_COUNT : numberOfFeatures;
                    overdriveInitSetting.overdrive8Capabilities = overdrive8Capabilities;
                    for (int j = 0; j < overdriveInitSetting.count; j++)
                    {
                        PRINTF("lpInitSettingList.defaultValue : %d\n", lpInitSettingList[j].defaultValue);
                        PRINTF("lpInitSettingList.featureID : %d\n", lpInitSettingList[j].featureID);
                        PRINTF("lpInitSettingList.maxValue : %d\n", lpInitSettingList[j].maxValue);
                        PRINTF("lpInitSettingList.minValue : %d\n", lpInitSettingList[j].minValue);
                    }
                }
                ADL_Main_Memory_Free((void**)&lpInitSettingList);
            }
        }
    }
    return ret;
}

int GetCurrentOD8Setting()
{

    ADLOD8InitSetting overdriveInitSetting;
    memset(&overdriveInitSetting, 0, sizeof(ADLOD8InitSetting));
    overdriveInitSetting.count = OD8_COUNT;
    int numberOfFeaturesCurrent = OD8_COUNT;
    int* lpCurrentSettingList = NULL;
    int iSupported, iEnabled, iVersion;
    int i, ret = ADL_FALSE;
    for (i = 0; i < iNumberAdapters; i++)
    {
        if (lpAdapterInfo[i].iBusNumber > -1)
        {
            ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (NULL != ADL2_Overdrive8_Current_SettingX2_Get)
            {
                ret = ADL2_Overdrive8_Current_SettingX2_Get(context, lpAdapterInfo[i].iAdapterIndex, &numberOfFeaturesCurrent, &lpCurrentSettingList);
                if (ADL_OK != ret)
                {
                    PRINTF("ADL2_Overdrive8_Current_SettingX2_Get is failed\n");
                }
                else
                {
                    PRINTF("ADL2_Overdrive8_Current_SettingX2_Get is Success\n\n");
                    PRINTF("****** Driver Values: After Apply ******\n");
                    overdriveInitSetting.count = numberOfFeaturesCurrent > OD8_COUNT ? OD8_COUNT : numberOfFeaturesCurrent;
                    for (int j = 0; j < overdriveInitSetting.count; j++)
                    {
                        PRINTF("lpCurrentSettingList Value : %d\n", lpCurrentSettingList[j]);
                    }
                }
                ADL_Main_Memory_Free((void**)&lpCurrentSettingList);
            }
        }
    }
    return ret;
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
            else if ('x' == *(argv[1]))
            {
                if (argc == 2)
                {
                    GetInitOD8Setting();
                    GetCurrentOD8Setting();
                }
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
