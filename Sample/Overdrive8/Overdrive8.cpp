#include <stdio.h>
#include <tchar.h>
#include <vector>

///
///  Copyright (c) 2018 - 2022 Advanced Micro Devices, Inc.
 
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
typedef int(*ADL2_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)(ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET)(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET)(ADL_CONTEXT_HANDLE, LPAdapterInfo lpInfo, int iInputSize);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int(*ADL2_ADAPTER_REGVALUEINT_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, const char* szSubKey, const char *szKeyName, int *lpKeyValue);
typedef int(*ADL2_ADAPTER_REGVALUEINT_SET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iDriverPathOption, char *szSubKey, char *szKeyName, int iKeyValue);

typedef int(*ADL2_OVERDRIVE8_INIT_SETTING_GET) (ADL_CONTEXT_HANDLE, int, ADLOD8InitSetting*);
typedef int(*ADL2_OVERDRIVE8_CURRENT_SETTING_GET) (ADL_CONTEXT_HANDLE, int, ADLOD8CurrentSetting*);
typedef int(*ADL2_OVERDRIVE8_SETTING_SET) (ADL_CONTEXT_HANDLE, int, ADLOD8SetSetting*, ADLOD8CurrentSetting*);
typedef int(*ADL2_NEW_QUERYPMLOGDATA_GET) (ADL_CONTEXT_HANDLE, int, ADLPMLogDataOutput*);

typedef int(*ADL2_OVERDRIVE8_INIT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpOverdrive8Capabilities, int *lpNumberOfFeatures, ADLOD8SingleInitSetting** lppInitSettingList);
typedef int(*ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int *lpNumberOfFeatures, int** lppCurrentSettingList);


typedef int(*ADL2_ADAPTER_PMLOG_SUPPORT_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLPMLogSupportInfo* pPMLogSupportInfo);
typedef int(*ADL2_ADAPTER_PMLOG_START) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLPMLogStartInput* pPMLogStartInput, ADLPMLogStartOutput* pPMLogStartOutput, ADL_D3DKMT_HANDLE pDevice);
typedef int(*ADL2_ADAPTER_PMLOG_STOP) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADL_D3DKMT_HANDLE pDevice);
typedef int(*ADL2_DESKTOP_DEVICE_CREATE) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADL_D3DKMT_HANDLE *pDevice);
typedef int(*ADL2_DESKTOP_DEVICE_DESTROY) (ADL_CONTEXT_HANDLE context, ADL_D3DKMT_HANDLE hDevice);
typedef int(*ADL2_WS_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);


HINSTANCE hDLL;

ADL2_MAIN_CONTROL_CREATE          ADL2_Main_Control_Create = NULL;
ADL2_MAIN_CONTROL_DESTROY         ADL2_Main_Control_Destroy = NULL;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET ADL2_Adapter_NumberOfAdapters_Get = NULL;
ADL2_ADAPTER_ADAPTERINFO_GET      ADL2_Adapter_AdapterInfo_Get = NULL;
ADL2_ADAPTER_ACTIVE_GET          ADL2_Adapter_Active_Get = NULL;
ADL2_OVERDRIVE_CAPS              ADL2_Overdrive_Caps = NULL;
ADL2_ADAPTER_REGVALUEINT_GET     ADL2_Adapter_RegValueInt_Get = NULL;
ADL2_ADAPTER_REGVALUEINT_SET     ADL2_Adapter_RegValueInt_Set = NULL;

ADL2_OVERDRIVE8_INIT_SETTING_GET ADL2_Overdrive8_Init_Setting_Get = NULL;
ADL2_OVERDRIVE8_CURRENT_SETTING_GET ADL2_Overdrive8_Current_Setting_Get = NULL;
ADL2_OVERDRIVE8_SETTING_SET ADL2_Overdrive8_Setting_Set = NULL;
ADL2_NEW_QUERYPMLOGDATA_GET ADL2_New_QueryPMLogData_Get = NULL;

ADL2_OVERDRIVE8_INIT_SETTINGX2_GET ADL2_Overdrive8_Init_SettingX2_Get = NULL;
ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET ADL2_Overdrive8_Current_SettingX2_Get = NULL;

ADL2_ADAPTER_PMLOG_SUPPORT_GET ADL2_Adapter_PMLog_Support_Get = NULL;
ADL2_ADAPTER_PMLOG_START ADL2_Adapter_PMLog_Start = NULL;
ADL2_ADAPTER_PMLOG_STOP ADL2_Adapter_PMLog_Stop = NULL;
ADL2_DESKTOP_DEVICE_CREATE ADL2_Desktop_Device_Create = NULL;
ADL2_DESKTOP_DEVICE_DESTROY ADL2_Desktop_Device_Destroy = NULL;
ADL2_WS_OVERDRIVE_CAPS ADL2_WS_Overdrive_Caps = NULL;

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
int SetOD8PlusRange(const ADLOD8InitSetting &odInitSetting, ADLOD8CurrentSetting &odCurrentSetting, int iAdapterIndex, int SettingId, int Reset, int value);
int GetOD8RangePrint(ADLOD8InitSetting odInitSetting, const ADLOD8CurrentSetting& odCurrentSetting, OverdriveRangeDataStruct &oneRangeData, int itemID_, int featureID_);
int GetOD8RangePrint(ADLOD8InitSetting odInitSetting, OverdriveRangeDataStruct &oneRangeData, int itemID_, int featureID_);
int printOD8();
int PMLogAllSensorStart(int adapterNumber, int sampleRate, ADLPMLogData** PMLogOutput, ADL_D3DKMT_HANDLE *hDevice);

char * ADLOD8FeatureControlStr[]
{
	"ADL_OD8_GFXCLK_LIMITS",
	"ADL_OD8_GFXCLK_CURVE",
	"ADL_OD8_UCLK_MAX",
	"ADL_OD8_POWER_LIMIT",
	"ADL_OD8_ACOUSTIC_LIMIT_SCLK FanMaximumRpm",
	"ADL_OD8_FAN_SPEED_MIN FanMinimumPwm",
	"ADL_OD8_TEMPERATURE_FAN FanTargetTemperature",
	"ADL_OD8_TEMPERATURE_SYSTEM MaxOpTemp",
	"ADL_OD8_MEMORY_TIMING_TUNE",
	"ADL_OD8_FAN_ZERO_RPM_CONTROL",
	"ADL_OD8_AUTO_UV_ENGINE",
	"ADL_OD8_AUTO_OC_ENGINE",
	"ADL_OD8_AUTO_OC_MEMORY",
	"ADL_OD8_FAN_CURVE ",
	"ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT",
    "ADL_OD8_GFXCLK_QUADRATIC_CURVE",
    "ADL_OD8_OPTIMIZED_GPU_POWER_MODE",
    "ADL_OD8_ODVOLTAGE_LIMIT",
	"ADL_OD8_POWER_GAUGE"
};

char *ADLOD8SettingIdStr[] =
{
	"OD8_GFXCLK_FMAX",
	"OD8_GFXCLK_FMIN",
	"OD8_GFXCLK_FREQ1",
	"OD8_GFXCLK_VOLTAGE1",
	"OD8_GFXCLK_FREQ2",
	"OD8_GFXCLK_VOLTAGE2",
	"OD8_GFXCLK_FREQ3",
	"OD8_GFXCLK_VOLTAGE3",
	"OD8_UCLK_FMAX",
	"OD8_POWER_PERCENTAGE",
	"OD8_FAN_MIN_SPEED",
	"OD8_FAN_ACOUSTIC_LIMIT",
	"OD8_FAN_TARGET_TEMP",
	"OD8_OPERATING_TEMP_MAX",
	"OD8_AC_TIMING",
	"OD8_FAN_ZERORPM_CONTROL",
	"OD8_AUTO_UV_ENGINE_CONTROL",
	"OD8_AUTO_OC_ENGINE_CONTROL",
	"OD8_AUTO_OC_MEMORY_CONTROL",
	"OD8_FAN_CURVE_TEMPERATURE_1",
	"OD8_FAN_CURVE_SPEED_1",
	"OD8_FAN_CURVE_TEMPERATURE_2",
	"OD8_FAN_CURVE_SPEED_2",
	"OD8_FAN_CURVE_TEMPERATURE_3",
	"OD8_FAN_CURVE_SPEED_3",
	"OD8_FAN_CURVE_TEMPERATURE_4",
	"OD8_FAN_CURVE_SPEED_4",
	"OD8_FAN_CURVE_TEMPERATURE_5",
	"OD8_FAN_CURVE_SPEED_5",
	"OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT",
    "RESERVED_1",
    "RESERVED_2",
    "RESERVED_3",
    "RESERVED_4",
    "OD8_UCLK_FMIN",
    "OD8_FAN_ZERO_RPM_STOP_TEMPERATURE",
    "OD8_OPTIMZED_POWER_MODE",
	"OD8_POWER_GAUGE",
	"OD8_COUNT"
};


char *sensorType[] = {
"SENSOR_MAXTYPES",
"PMLOG_CLK_GFXCLK",
"PMLOG_CLK_MEMCLK",
"PMLOG_CLK_SOCCLK",
"PMLOG_CLK_UVDCLK1",
"PMLOG_CLK_UVDCLK2",
"PMLOG_CLK_VCECLK",
"PMLOG_CLK_VCNCLK",
"PMLOG_TEMPERATURE_EDGE",
"PMLOG_TEMPERATURE_MEM",
"PMLOG_TEMPERATURE_VRVDDC",
"PMLOG_TEMPERATURE_VRMVDD",
"PMLOG_TEMPERATURE_LIQUID",
"PMLOG_TEMPERATURE_PLX",
"PMLOG_FAN_RPM",
"PMLOG_FAN_PERCENTAGE",
"PMLOG_SOC_VOLTAGE",
"PMLOG_SOC_POWER",
"PMLOG_SOC_CURRENT",
"PMLOG_INFO_ACTIVITY_GFX",
"PMLOG_INFO_ACTIVITY_MEM",
"PMLOG_GFX_VOLTAGE",
"PMLOG_MEM_VOLTAGE",
"PMLOG_ASIC_POWER",
"PMLOG_TEMPERATURE_VRSOC",
"PMLOG_TEMPERATURE_VRMVDD0",
"PMLOG_TEMPERATURE_VRMVDD1",
"PMLOG_TEMPERATURE_HOTSPOT",
"PMLOG_TEMPERATURE_GFX",
"PMLOG_TEMPERATURE_SOC",
"PMLOG_GFX_POWER",
"PMLOG_GFX_CURRENT",
"PMLOG_TEMPERATURE_CPU",
"PMLOG_CPU_POWER",
"PMLOG_CLK_CPUCLK",
"PMLOG_THROTTLER_STATUS",
"PMLOG_CLK_VCN1CLK1",
"PMLOG_CLK_VCN1CLK2",
"PMLOG_SMART_POWERSHIFT_CPU",
"PMLOG_SMART_POWERSHIFT_DGPU",
"ADL_PMLOG_BUS_SPEED",
"ADL_PMLOG_BUS_LANES"
};

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
    ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL2_Main_Control_Create");
    ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");
    ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL2_Adapter_NumberOfAdapters_Get");
    ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfo_Get");
    ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
    ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS)GetProcAddress(hDLL, "ADL2_Overdrive_Caps");
    ADL2_Adapter_RegValueInt_Get = (ADL2_ADAPTER_REGVALUEINT_GET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueInt_Get");
    ADL2_Adapter_RegValueInt_Set = (ADL2_ADAPTER_REGVALUEINT_SET)GetProcAddress(hDLL, "ADL2_Adapter_RegValueInt_Set");

    ADL2_Overdrive8_Init_Setting_Get = (ADL2_OVERDRIVE8_INIT_SETTING_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Init_Setting_Get");
    ADL2_Overdrive8_Current_Setting_Get = (ADL2_OVERDRIVE8_CURRENT_SETTING_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Current_Setting_Get");
    ADL2_Overdrive8_Setting_Set = (ADL2_OVERDRIVE8_SETTING_SET)GetProcAddress(hDLL, "ADL2_Overdrive8_Setting_Set");
    ADL2_New_QueryPMLogData_Get = (ADL2_NEW_QUERYPMLOGDATA_GET)GetProcAddress(hDLL, "ADL2_New_QueryPMLogData_Get");

    ADL2_Overdrive8_Init_SettingX2_Get = (ADL2_OVERDRIVE8_INIT_SETTINGX2_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Init_SettingX2_Get");
    ADL2_Overdrive8_Current_SettingX2_Get = (ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET)GetProcAddress(hDLL, "ADL2_Overdrive8_Current_SettingX2_Get");

	ADL2_Adapter_PMLog_Support_Get = (ADL2_ADAPTER_PMLOG_SUPPORT_GET)GetProcAddress(hDLL, "ADL2_Adapter_PMLog_Support_Get");
	ADL2_Adapter_PMLog_Start = (ADL2_ADAPTER_PMLOG_START)GetProcAddress(hDLL, "ADL2_Adapter_PMLog_Start");
	ADL2_Adapter_PMLog_Stop = (ADL2_ADAPTER_PMLOG_STOP)GetProcAddress(hDLL, "ADL2_Adapter_PMLog_Stop");
	ADL2_Desktop_Device_Create = (ADL2_DESKTOP_DEVICE_CREATE)GetProcAddress(hDLL, "ADL2_Desktop_Device_Create");
	ADL2_Desktop_Device_Destroy = (ADL2_DESKTOP_DEVICE_DESTROY)GetProcAddress(hDLL, "ADL2_Desktop_Device_Destroy");
    ADL2_WS_Overdrive_Caps = (ADL2_WS_OVERDRIVE_CAPS)GetProcAddress(hDLL, "ADL2_WS_Overdrive_Caps");
    
 
    if (NULL == ADL2_Main_Control_Create ||
        NULL == ADL2_Main_Control_Destroy ||
        NULL == ADL2_Adapter_NumberOfAdapters_Get ||
        NULL == ADL2_Adapter_AdapterInfo_Get ||
        NULL == ADL2_Adapter_Active_Get ||
        NULL == ADL2_Overdrive_Caps ||
        NULL == ADL2_Adapter_RegValueInt_Get ||
        NULL == ADL2_Adapter_RegValueInt_Set ||
        NULL == ADL2_Overdrive8_Init_Setting_Get ||
        NULL == ADL2_Overdrive8_Current_Setting_Get ||
        NULL == ADL2_Overdrive8_Setting_Set ||
        NULL == ADL2_New_QueryPMLogData_Get ||
        NULL == ADL2_Overdrive8_Init_SettingX2_Get ||
        NULL == ADL2_Overdrive8_Current_SettingX2_Get ||
		NULL == ADL2_Adapter_PMLog_Support_Get ||
		NULL == ADL2_Adapter_PMLog_Start ||
		NULL == ADL2_Adapter_PMLog_Stop ||
		NULL == ADL2_Desktop_Device_Create ||
		NULL == ADL2_Desktop_Device_Destroy ||
        NULL == ADL2_WS_Overdrive_Caps)
    {
        PRINTF("Failed to get ADL function pointers\n");
        return FALSE;
    }
    
    if (ADL_OK != ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &context))
    {
        printf("Failed to initialize nested ADL2 context");
        return ADL_ERR;
    }
    
    return TRUE;
}

void deinitializeADL()
{
    ADL2_Main_Control_Destroy(context);
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
        if (ADL_OK == ADL2_Overdrive8_Init_SettingX2_Get(context, iAdapterIndex, &overdrive8Capabilities, &numberOfFeatures, &lpInitSettingList))
        {
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
            if (ADL_OK != ADL2_Overdrive8_Init_Setting_Get(context, iAdapterIndex, &odInitSetting))
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
                
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS &&
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
                        PRINTF("ADLSensorType: PMLOG_INFO_ACTIVITY_GFX-GPU activity percentage value\n");
                        PRINTF("PMLOG_INFO_ACTIVITY_GFX.supported:%d\n", odlpDataOutput.sensors[PMLOG_INFO_ACTIVITY_GFX].supported);
                        PRINTF("PMLOG_INFO_ACTIVITY_GFX.value:%d\n", odlpDataOutput.sensors[PMLOG_INFO_ACTIVITY_GFX].value);
                        PRINTF("-----------------------------------------\n");

                    }
                    else if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS &&
                        (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) != ADL_OD8_GFXCLK_CURVE) {
                        //GPU clocks
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FMIN, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_FMAX, ADL_OD8_GFXCLK_CURVE);

                        PRINTF("ADLSensorType: PMLOG_CLK_GFXCLK\n");
                        PRINTF("PMLOG_CLK_GFXCLK.supported:%d\n", odlpDataOutput.sensors[PMLOG_CLK_GFXCLK].supported);
                        PRINTF("PMLOG_CLK_GFXCLK.value:%d\n", odlpDataOutput.sensors[PMLOG_CLK_GFXCLK].value);
                        PRINTF("-----------------------------------------\n");
                        PRINTF("ADLSensorType: PMLOG_INFO_ACTIVITY_GFX-GPU activity percentage value\n");
                        PRINTF("PMLOG_INFO_ACTIVITY_GFX.supported:%d\n", odlpDataOutput.sensors[PMLOG_INFO_ACTIVITY_GFX].supported);
                        PRINTF("PMLOG_INFO_ACTIVITY_GFX.value:%d\n", odlpDataOutput.sensors[PMLOG_INFO_ACTIVITY_GFX].value);
                        PRINTF("-----------------------------------------\n");
                    }
                    else
                        PRINTF("OD8PLUS Failed to get GPU clocks\n");
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
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS &&
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
                else if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS &&
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) != ADL_OD8_GFXCLK_CURVE) {
                     if (OD8_GFXCLK_FMIN == SettingId)
                     {
                        if (ADL_OK == SetOD8PlusRange(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FMIN parameters failed.\n");
                     }
                     else if (OD8_GFXCLK_FMAX == SettingId)
                     {
                        if (ADL_OK == SetOD8PlusRange(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUClocksParameters();
                        }
                        else
                            PRINTF("Set OD8_GFXCLK_FMAX parameters failed.\n");
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
                    if (((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS) &&
                        ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) == ADL_OD8_GFXCLK_CURVE))
                    {
                        //GPU Voltage
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_VOLTAGE1, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_VOLTAGE2, ADL_OD8_GFXCLK_CURVE);
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_GFXCLK_VOLTAGE3, ADL_OD8_GFXCLK_CURVE);
                    }
                    else if (((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS) &&
                        ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) != ADL_OD8_GFXCLK_CURVE)) {
                        //GPU Voltage
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_OD_VOLTAGE, ADL_OD8_ODVOLTAGE_LIMIT);
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
                if (((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS) &&
                    ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) == ADL_OD8_GFXCLK_CURVE))
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
                else if (((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS) &&
                    ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) != ADL_OD8_GFXCLK_CURVE)) {
                     if (OD8_OD_VOLTAGE == SettingId)
                     {
                        if (ADL_OK == SetOD8PlusRange(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            printOD8GPUVoltageParameters();
                        }
                        else
                            PRINTF("Set OD8_OD_VOLTAGE parameters failed.\n");
                     }
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

                        if (odInitSetting.overdrive8Capabilities & odInitSetting.od8SettingTable[OD8_UCLK_FMIN].featureID)
                            GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_UCLK_FMIN, ADL_OD8_UCLK_MAX);

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
                    else if (OD8_UCLK_FMIN == SettingId)
                    {
                        if (ADL_OK == SetOD8PlusRange(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
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

						PRINTF("ADLSensorType: PMLOG_TEMPERATURE_MEM - Memory Temp\n");
						PRINTF("PMLOG_TEMPERATURE_MEM.supported:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_MEM].supported);
						PRINTF("PMLOG_TEMPERATURE_MEM.value:%d\n", odlpDataOutput.sensors[PMLOG_TEMPERATURE_MEM].value);
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

                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_UV_ENGINE) == ADL_OD8_AUTO_UV_ENGINE)
                {
                    OverdriveRangeDataStruct oneRangeData;
                    GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AUTO_UV_ENGINE_CONTROL, ADL_OD8_AUTO_UV_ENGINE);
                }
                else
                    PRINTF("OD8 Failed to get Auto Undervolt Setting\n");
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_OC_ENGINE) == ADL_OD8_AUTO_OC_ENGINE)
                {
                    OverdriveRangeDataStruct oneRangeData;
                    GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AUTO_OC_ENGINE_CONTROL, ADL_OD8_AUTO_OC_ENGINE);
                }
                else
                    PRINTF("OD8 Failed to get Auto Overclock Setting\n");
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_OC_MEMORY) == ADL_OD8_AUTO_OC_MEMORY)
                {
                    OverdriveRangeDataStruct oneRangeData;
                    GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_AUTO_OC_MEMORY_CONTROL, ADL_OD8_AUTO_OC_MEMORY);
                }
                else
                    PRINTF("OD8 Failed to get Auto Overclock Memory Setting\n");
            }
        }
    }
    return 0;
}

int printOD8OptimizedPowerModeParameters()
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

                //Check Cap
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_OPTIMIZED_GPU_POWER_MODE) == ADL_OD8_OPTIMIZED_GPU_POWER_MODE)
                {
                    OverdriveRangeDataStruct oneRangeData;
                    GetOD8RangePrint(odInitSetting, odCurrentSetting, oneRangeData, OD8_OPTIMZED_POWER_MODE, ADL_OD8_OPTIMIZED_GPU_POWER_MODE);
                    if (odCurrentSetting.Od8SettingTable[OD8_OPTIMZED_POWER_MODE] == 1) {
                        int result = 0;
                        if (ADL_OK == ADL2_Adapter_RegValueInt_Get(context, lpAdapterInfo[i].iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", &result)) {
                            if (result == 1)
                                PRINTF("Asics is set back to Default mode, Balanced mode is the default state of OD8_OPTIMZED_POWER_MODE feature");
                        }
                        else {
                            ADL2_Adapter_RegValueInt_Set(context, lpAdapterInfo[i].iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", 1);
                            PRINTF("Asics is set back to Default mode, Balanced mode is the default state of OD8_OPTIMZED_POWER_MODE feature");
                        }

                    }
                }
                else
                    PRINTF("OD8 Failed to get Optimized Power mode Setting\n");
            }
        }
    }
    return 0;
}

int SetOD8OptimizedPowerModeParameters(int SettingId, int Reset, int value)
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
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_OPTIMIZED_GPU_POWER_MODE) == ADL_OD8_OPTIMIZED_GPU_POWER_MODE)
                {
                    if (OD8_OPTIMZED_POWER_MODE == SettingId)
                    {
                        //do not reset
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, Reset, value))
                        {
                            if(Reset == 1)
                                ADL2_Adapter_RegValueInt_Set(context, lpAdapterInfo[i].iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", 1);
                            printOD8OptimizedPowerModeParameters();
                        }
                        else
                            PRINTF("Set OD8_OPTIMZED_POWER_MODE parameters failed.\n");
                    }
                }
                break;
            }
        }

    }
    return 0;
}

int SetOD8TuningControl(int SettingId)
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
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_UV_ENGINE) == ADL_OD8_AUTO_UV_ENGINE ||
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_OC_ENGINE) == ADL_OD8_AUTO_OC_ENGINE ||
                    (odInitSetting.overdrive8Capabilities & ADL_OD8_AUTO_OC_MEMORY) == ADL_OD8_AUTO_OC_MEMORY)
                {
                    if (OD8_AUTO_UV_ENGINE_CONTROL == SettingId || OD8_AUTO_OC_ENGINE_CONTROL == SettingId || OD8_AUTO_OC_MEMORY_CONTROL == SettingId)
                    {
                        //do not reset
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, 0, 1))
                        {
                            printOD8TuningControlSettingParameters();
                        }
                        else
                            PRINTF("Set OD8 Tuning control parameters failed.\n");
                    }
                    else if (99 == SettingId)   // Set to auto default
                    {
                        ADLOD8SetSetting overdriveSetSetting;
                        memset(&overdriveSetSetting, 0, sizeof(ADLOD8SetSetting));
                        overdriveSetSetting.count = OD8_COUNT;

                        for (int i = 0; i < OD8_COUNT; i++)
                        {
                            overdriveSetSetting.od8SettingTable[i].requested = true;
                            overdriveSetSetting.od8SettingTable[i].reset = true;
                            overdriveSetSetting.od8SettingTable[i].value = odInitSetting.od8SettingTable[i].defaultValue;
                        }

                        if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &overdriveSetSetting, &odCurrentSetting))
                        {
                            ADL2_Adapter_RegValueInt_Set(context, lpAdapterInfo[i].iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", 1);
                            printOD8TuningControlSettingParameters();
                        }
                        else
                        {
                            PRINTF("Set OD8 to auto default failed.\n");
                        }

                    }
                    // Set to Manual
                    else if (98 == SettingId) {
                        ADLOD8SetSetting overdriveSetSetting;
                        memset(&overdriveSetSetting, 0, sizeof(ADLOD8SetSetting));
                        overdriveSetSetting.count = OD8_COUNT;

                        for (int i = 0; i < OD8_COUNT; i++)
                        {
                            overdriveSetSetting.od8SettingTable[i].requested = true;
                            overdriveSetSetting.od8SettingTable[i].reset = true;
                            overdriveSetSetting.od8SettingTable[i].value = odInitSetting.od8SettingTable[i].defaultValue;
                        }

                        if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, lpAdapterInfo[i].iAdapterIndex, &overdriveSetSetting, &odCurrentSetting))
                        {
                            SetOD8OptimizedPowerModeParameters(OD8_OPTIMZED_POWER_MODE, 0, 3);
                            ADL2_Adapter_RegValueInt_Set(context, lpAdapterInfo[i].iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", 0);
                            printOD8TuningControlSettingParameters();
                        }
                        else
                        {
                            PRINTF("Set OD8 to manual failed.\n");
                        }
                    }
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

int printOD8PowerGaugeParameters()
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
                
                //make ADL call for VEGA12
                ADLPMLogDataOutput odlpDataOutput;
                memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
                ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
                if (0 == ret)
                {
                    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_POWER_GAUGE) == ADL_OD8_POWER_GAUGE)
                    {
                        //Power reading
                        OverdriveRangeDataStruct oneRangeData;
                        GetOD8RangePrint(odInitSetting, oneRangeData, OD8_POWER_GAUGE, ADL_OD8_POWER_GAUGE);

                        PRINTF("ADLSensorType: PMLOG_ASIC_POWER\n");
                        PRINTF("PMLOG_ASIC_POWER.supported:%d\n", odlpDataOutput.sensors[PMLOG_ASIC_POWER].supported);
                        PRINTF("PMLOG_ASIC_POWER.value:%d\n", odlpDataOutput.sensors[PMLOG_ASIC_POWER].value);
                        PRINTF("-----------------------------------------\n");

                    }
                    else
                        PRINTF("OD8 Failed to get ASIC Power reading\n");
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

int printOD8WSAutoFanAcousticLimitParameters()
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
                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT) == ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT)
                {
                    //Power reading
                    OverdriveRangeDataStruct oneRangeData;
                    GetOD8RangePrint(odInitSetting, oneRangeData, OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT, ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT);
                }
                else
                    PRINTF("OD8 WS_AUTO_FAN_ACOUSTIC_LIMIT not support\n");

                break;
            }
        }

    }
    return 0;
}

int SetOD8WSAutoFanAcousticLimitParameters(int SettingId, int value)
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
            ret = ADL2_WS_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
            if (ret == ADL_OK && iSupported > 0 && iVersion == 8)
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

                if ((odInitSetting.overdrive8Capabilities & ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT) == ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT)
                {
                    if (OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT == SettingId)
                    {
                        //do not reset
                        if (ADL_OK == SetOD8Range(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, SettingId, 0, value))
                        {
                            printOD8WSAutoFanAcousticLimitParameters();
                        }
                        else
                            PRINTF("Set OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT parameters failed.\n");
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
    case OD8_AUTO_UV_ENGINE_CONTROL:
        PRINTF("Undervolt GPU:");
        break;
    case OD8_AUTO_OC_ENGINE_CONTROL:
        PRINTF("Overclock GPU:");
        break;
    case OD8_AUTO_OC_MEMORY_CONTROL:
        PRINTF("Overclock VRAM:");
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
    case OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT:
        PRINTF("OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT:");
        break;
    case OD8_GFXCLK_CURVE_COEFFICIENT_A:
        PRINTF("OD8_GFXCLK_CURVE_COEFFICIENT_A:");
        break;
    case OD8_GFXCLK_CURVE_COEFFICIENT_B:
        PRINTF("OD8_GFXCLK_CURVE_COEFFICIENT_B:");
        break;
    case OD8_GFXCLK_CURVE_COEFFICIENT_C:
        PRINTF("OD8_GFXCLK_CURVE_COEFFICIENT_C:");
        break;
    case OD8_GFXCLK_CURVE_VFT_FMIN:
        PRINTF("OD8_GFXCLK_CURVE_VFT_FMIN:");
        break;
    case OD8_UCLK_FMIN:
        PRINTF("OD8_UCLK_FMIN:");
        break;
    case OD8_FAN_ZERO_RPM_STOP_TEMPERATURE:
        PRINTF("OD8_FAN_ZERO_RPM_STOP_TEMPERATURE:");
        break;
    case OD8_OPTIMZED_POWER_MODE:
        PRINTF("OD8_OPTIMZED_POWER_MODE:");
        break;
    case OD8_OD_VOLTAGE:
        PRINTF("OD8_PLUS_GFX_VOLTAGE:");
        break;
    case OD8_POWER_GAUGE:
        PRINTF("OD8_POWER_GAUGE:");
        break;
    default:
        PRINTF("Found no featureID \n");
        return ADL_ERR;
    }
    return ADL_OK;
}

int SetOD8Range(const ADLOD8InitSetting &odInitSetting, ADLOD8CurrentSetting &odCurrentSetting, int iAdapterIndex, int SettingId, int Reset, int value)
{
    // Check if this is a NAVI 21+ OD8 Asic
    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_LIMITS) == ADL_OD8_GFXCLK_LIMITS &&
        (odInitSetting.overdrive8Capabilities & ADL_OD8_GFXCLK_CURVE) != ADL_OD8_GFXCLK_CURVE) {
        return SetOD8PlusRange(odInitSetting, odCurrentSetting, iAdapterIndex, SettingId, Reset, value);
    }

    ADLOD8SetSetting odSetSetting;
    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
    odSetSetting.count = OD8_COUNT;
    // setting
    if (SettingId <= OD8_GFXCLK_FREQ1 && SettingId >= OD8_UCLK_FMAX)
    {
        for (int i = OD8_GFXCLK_FREQ1; i <= OD8_UCLK_FMAX; ++i)
        {
            odSetSetting.od8SettingTable[i].requested = 1;
            odSetSetting.od8SettingTable[i].value = odCurrentSetting.Od8SettingTable[i];
        }
    }
    bool reset = true;
    if (SettingId <= OD8_FAN_CURVE_SPEED_5 && SettingId >= OD8_FAN_CURVE_TEMPERATURE_1)
    {
        reset = false;
        for (int i = OD8_FAN_CURVE_TEMPERATURE_1; i <= OD8_FAN_CURVE_SPEED_5; ++i)
        {
            odSetSetting.od8SettingTable[i].reset = reset;
            odSetSetting.od8SettingTable[i].requested = 1;
            odSetSetting.od8SettingTable[i].value = odCurrentSetting.Od8SettingTable[i];
        }
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
                ADL2_Adapter_RegValueInt_Set(context, iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", 0);
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

int SetOD8PlusRange(const ADLOD8InitSetting &odInitSetting, ADLOD8CurrentSetting &odCurrentSetting, int iAdapterIndex, int SettingId, int Reset, int value)
{
    ADLOD8SetSetting odSetSetting;
    ADLOD8SetSetting odModeSetSetting;
    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
    memset(&odModeSetSetting, 0, sizeof(ADLOD8SetSetting));
    odSetSetting.count = OD8_COUNT;
    odModeSetSetting.count = OD8_COUNT;

    // Set Asic to Manual mode if it has Power mode
    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_OPTIMIZED_GPU_POWER_MODE) == ADL_OD8_OPTIMIZED_GPU_POWER_MODE) {
        odModeSetSetting.od8SettingTable[OD8_OPTIMZED_POWER_MODE].requested = 1;
        odModeSetSetting.od8SettingTable[OD8_OPTIMZED_POWER_MODE].value = 3;
        if (ADL_OK != ADL2_Overdrive8_Setting_Set(context, iAdapterIndex, &odModeSetSetting, &odCurrentSetting))
        {
            PrintFeatureName(SettingId);
            PRINTF("ADL2_Overdrive8_Setting_Set is failed when resetting Optimized Power mode to Manual\n");
            return ADL_ERR;
        }
    }

    if (SettingId <= OD8_GFXCLK_FMIN && SettingId >= OD8_GFXCLK_FMAX)
    {
        for (int i = OD8_GFXCLK_FMAX; i <= OD8_GFXCLK_FMIN; ++i)
        {
            odSetSetting.od8SettingTable[i].requested = 1;
            odSetSetting.od8SettingTable[i].value = odCurrentSetting.Od8SettingTable[i];
        }
    }
    else if (SettingId <= OD8_FAN_CURVE_SPEED_5 && SettingId >= OD8_FAN_CURVE_TEMPERATURE_1) {
        for (int i = OD8_FAN_CURVE_TEMPERATURE_1; i <= OD8_FAN_CURVE_SPEED_5; ++i)
        {
            odSetSetting.od8SettingTable[i].requested = 1;
            odSetSetting.od8SettingTable[i].value = odCurrentSetting.Od8SettingTable[i];
        }
    }
    else if (SettingId == OD8_UCLK_FMIN || SettingId == OD8_UCLK_FMAX) {
        odSetSetting.od8SettingTable[OD8_UCLK_FMAX].requested = 1;
        odSetSetting.od8SettingTable[OD8_UCLK_FMAX].value = odCurrentSetting.Od8SettingTable[OD8_UCLK_FMAX];
        odSetSetting.od8SettingTable[OD8_UCLK_FMIN].requested = 1;
        odSetSetting.od8SettingTable[OD8_UCLK_FMIN].value = odCurrentSetting.Od8SettingTable[OD8_UCLK_FMIN];
    }

    odSetSetting.od8SettingTable[SettingId].requested = 1;
    if (ADL_OK == PrintFeatureName(SettingId))
    {
        // The Max/Min range check doesn't apply to GFX Curve coefficients as they are meaningless numbers in INT
        if (!(odInitSetting.od8SettingTable[SettingId].minValue <= value && odInitSetting.od8SettingTable[SettingId].maxValue >= value) && (SettingId != OD8_OPTIMZED_POWER_MODE))
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

                int result = ADL2_Adapter_RegValueInt_Set(context, iAdapterIndex, 0x00000001, NULL, "IsAutoDefault", 0);
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

int SetOD8FanSpeedTogether(const ADLOD8InitSetting &odInitSetting, ADLOD8CurrentSetting &odCurrentSetting, int iAdapterIndex, int Reset, int value)
{
    ADLOD8SetSetting odSetSetting;
    ADLOD8SetSetting odModeSetSetting;
    memset(&odSetSetting, 0, sizeof(ADLOD8SetSetting));
    memset(&odModeSetSetting, 0, sizeof(ADLOD8SetSetting));
    odSetSetting.count = OD8_COUNT;
    odModeSetSetting.count = OD8_COUNT;

    // Set Asic to Manual mode if it has Power mode
    if ((odInitSetting.overdrive8Capabilities & ADL_OD8_OPTIMIZED_GPU_POWER_MODE) == ADL_OD8_OPTIMIZED_GPU_POWER_MODE) {
        if (odCurrentSetting.Od8SettingTable[OD8_OPTIMZED_POWER_MODE] != 3) {
            odModeSetSetting.od8SettingTable[OD8_OPTIMZED_POWER_MODE].requested = 1;
            odModeSetSetting.od8SettingTable[OD8_OPTIMZED_POWER_MODE].value = 3;
            if (ADL_OK != ADL2_Overdrive8_Setting_Set(context, iAdapterIndex, &odModeSetSetting, &odCurrentSetting))
            {
                PRINTF("ADL2_Overdrive8_Setting_Set is failed when resetting Optimized Power mode to Manual\n");
                return ADL_ERR;
            }
        }
    }

    if (!(odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_1].minValue <= value && odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_1].maxValue >= value))
    {
        PrintFeatureName(OD8_FAN_CURVE_SPEED_1);
        PRINTF("range should be in Min : %d, Max : %d\n", odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_1].minValue, odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_1].maxValue);
        return ADL_ERR;
    }
    else
    {
        for (int i = OD8_FAN_CURVE_SPEED_1; i <= OD8_FAN_CURVE_SPEED_5; i += 2)
        {
            odSetSetting.od8SettingTable[i].reset = Reset;
            odSetSetting.od8SettingTable[i].requested = 1;
            odSetSetting.od8SettingTable[i].value = value;
        }
        for (int j = OD8_FAN_CURVE_TEMPERATURE_1; j <= OD8_FAN_CURVE_TEMPERATURE_5; j += 2)
        {
            odSetSetting.od8SettingTable[j].reset = Reset;
            odSetSetting.od8SettingTable[j].requested = 1;
            odSetSetting.od8SettingTable[j].value = odCurrentSetting.Od8SettingTable[j];
        }

        if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, iAdapterIndex, &odSetSetting, &odCurrentSetting))
        {
            PrintFeatureName(OD8_FAN_CURVE_SPEED_1);
            PRINTF("ADL2_Overdrive8_Setting_Set is Success\n\n");
            PRINTF("****** Driver Values: After Apply ******\n");
        }
        else
        {
            PrintFeatureName(OD8_FAN_CURVE_SPEED_1);
            PRINTF("ADL2_Overdrive8_Setting_Set is failed\n");
            return ADL_ERR;
        }
    }
    
    return ADL_OK;
}


int GetOD8RangePrint(ADLOD8InitSetting odInitSetting, const ADLOD8CurrentSetting& odCurrentSetting, OverdriveRangeDataStruct &oneRangeData, int itemID_, int featureID_)
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

int GetOD8RangePrint(ADLOD8InitSetting odInitSetting, OverdriveRangeDataStruct &oneRangeData, int itemID_, int featureID_)
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
                    PRINTF("PMLOG_FAN_RPM.supported:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].supported);
                    PRINTF("PMLOG_FAN_RPM.value:%d\n", odlpDataOutput.sensors[PMLOG_FAN_RPM].value);
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

int SetOD8AllFanCurvePointsPWM(int Reset, int value) {
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
                ret = SetOD8FanSpeedTogether(odInitSetting, odCurrentSetting, lpAdapterInfo[i].iAdapterIndex, Reset, value);
                if (ret == ADL_OK)
                    PrintfOD8FanCurve();
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
    PRINTF("Method to Set OD8 GPU Clocks: \t Overdrive8.exe c X Y Z; X - (0:OD8_GFXCLK_FMAX, 1:OD8_GFXCLK_FMIN, 2:OD8_GFXCLK_FREQ1, 4:OD8_GFXCLK_FREQ2, 6:OD8_GFXCLK_FREQ3); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s c 0 0 1100\n\n", exeName);
    PRINTF("Method to Set OD8+(Navi 21+) GPU Clocks: \t Overdrive8.exe c X Y Z; X - (0:OD8_GFXCLK_FMAX, 1:OD8_GFXCLK_FMIN); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s c 0 0 1250\n\n", exeName);
    //GPU Voltage
    PRINTF("Method to Read OD8 GPU Voltages: \t Overdrive8.exe v\n");
    PRINTF("Ex: %s v\n\n", exeName);
    PRINTF("Method to Set OD8 GPU Voltages: \t Overdrive8.exe v X Y Z; X - (3:OD8_GFXCLK_VOLTAGE1, 5:OD8_GFXCLK_VOLTAGE2, 7:OD8_GFXCLK_VOLTAGE3); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s v 3 0 900\n\n", exeName);
    PRINTF("Method to Set OD8+(Navi 21+) GPU Voltage: \t Overdrive8.exe v X Y Z; X - (37:OD8_OD_VOLTAGE); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s v 37 0 1000\n\n", exeName);
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
    PRINTF("Method to Set OD8 Fan Setting: \t Overdrive8.exe f X Y Z; X - (10:OD8_FAN_MIN_SPEED, 11:OD8_FAN_ACOUSTIC_LIMIT); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s f 10 0 1000\n\n", exeName);
    PRINTF("This Fan Setting feature only applys to OD8 WorkStation Asics. Trying to set it on other Asics will result in a failure.");
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
    //Fan Curve
    PRINTF("Method to Read OD8 Fan Curve: \t Overdrive8.exe r\n");
    PRINTF("Ex: %s r\n\n", exeName);
    PRINTF("Method to Set OD8 all Fan Curve PWM points: \t Overdrive8.exe r X Y; X - (0:do not reset , 1:reset setting back to default); Y - (value of setting)\n");
    PRINTF("EX: %s r 0 30\n\n", exeName);
    PRINTF("Method to Set OD8 Fan Curve: \t Overdrive8.exe r X Y Z; X - (19:OD8_FAN_CURVE_TEMPERATURE_1, 20:OD8_FAN_CURVE_SPEED_1, \n");
    PRINTF("\t\t\t21:OD8_FAN_CURVE_TEMPERATURE_2, 22:OD8_FAN_CURVE_SPEED_2, 23:OD8_FAN_CURVE_TEMPERATURE_3, 24:OD8_FAN_CURVE_SPEED_3,\n");
    PRINTF("\t\t\t25:OD8_FAN_CURVE_TEMPERATURE_4, 26:OD8_FAN_CURVE_SPEED_4, 27:OD8_FAN_CURVE_TEMPERATURE_5, 28:OD8_FAN_CURVE_SPEED_5); Y - (0:do not reset , 1:reset setting back to default); Z - (value of setting)\n");
    PRINTF("EX: %s r 20 0 30\n\n", exeName);
    PRINTF("The fan will not spin if the Asic is Zero RPM Mode enabled and Asic's temperature is lower than the temperature threshold(For example, Navi 21 is 55°C)");
    PRINTF("If user wants to observe immediate effect of fan curve changes, please make sure Zero RPM Mode(if supported) is disabled");
    //Power Gauge
    PRINTF("Method to Read OD8 ASIC Power: \t Overdrive8.exe p\n");
    PRINTF("Ex: %s p\n\n", exeName);
    //Tuning Control
    PRINTF("Method to Read OD8 Tuning Control: \t Overdrive8.exe u\n");
    PRINTF("Ex: %s u\n\n", exeName);
    PRINTF("Method to Set OD8 Tuning Control: \t Overdrive8.exe u X; X - (16:OD8_AUTO_UV_ENGINE_CONTROL, 17:OD8_AUTO_OC_ENGINE_CONTROL, 18:OD8_AUTO_OC_MEMORY_CONTROL, 99:AutoDefault, 98:Manual)");
    PRINTF("EX: %s u 16\n\n", exeName);
    //Optimized Power Mode
    PRINTF("Method to Read OD8 Optimized Power Mode: \t Overdrive8.exe o\n");
    PRINTF("Ex: %s o\n\n", exeName);
    PRINTF("Method to Set OD8 Optimized Power Mode: \t Overdrive8.exe o X Y Z; X - (36:OD8_SETTING_OPTIMZED_POWER_MODE); Y - (0:do not reset , \
            1:reset setting back to default); Z - (value of setting: 0: Quiet Mode, 1: Balance Mode, 3: Custom Mode, 4: Rage Mode [Depending on Asics, some mode may not be supported])\n");
    PRINTF("EX: %s o 36 0 1\n\n", exeName);
    //WS Auto Fan
    PRINTF("Method to Read OD8 workstation AUTO FAN ACOUSTIC LIMIT: \t Overdrive8.exe w\n");
    PRINTF("Ex: %s w\n\n", exeName);
    PRINTF("Method to Set OD8 workstation AUTO FAN ACOUSTIC LIMIT: \t Overdrive8.exe w X; X - (0:enable, 1:disable)");
    PRINTF("EX: %s w 16\n\n", exeName);
}



int PMLogDestroyD3DDevice(int adapterNumber, ADL_D3DKMT_HANDLE hDevice)
{
	if (ADL_OK != ADL2_Desktop_Device_Destroy(context, hDevice))
	{
		PRINTF("Err: Failed to destory D3D device\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int PMLogCreateD3DDevice(int adapterNumber, ADL_D3DKMT_HANDLE *hDevice)
{
	if (ADL_OK != ADL2_Desktop_Device_Create(context, lpAdapterInfo[adapterNumber].iAdapterIndex, hDevice))
	{
		PRINTF("Err: Failed to create D3D device\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int main(int argc, char* argv[])
{
    if (initializeADL())
    {
        if (argc > 1)
        {
            // Obtain the number of adapters for the system
            if (ADL_OK != ADL2_Adapter_NumberOfAdapters_Get(context, &iNumberAdapters))
            {
                PRINTF("Cannot get the number of adapters!\n");
                return 0;
            }
            if (0 < iNumberAdapters)
            {
                lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo)* iNumberAdapters);
                memset(lpAdapterInfo, '\0', sizeof(AdapterInfo)* iNumberAdapters);
                // Get the AdapterInfo structure for all adapters in the system
                ADL2_Adapter_AdapterInfo_Get(context, lpAdapterInfo, sizeof(AdapterInfo)* iNumberAdapters);
            }
			if ('a' == *(argv[1]))//all
			{
				printOD8();
			}
			else if ('c' == *(argv[1]))//GPU Clocks
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
            else if ('u' == *(argv[1]))//tuning control(auto features)
            {
                if (argc == 2)
                    printOD8TuningControlSettingParameters();
                else if (argc == 3)
                    SetOD8TuningControl(atoi(argv[2]));
                else
                    printHelp(argv[0]);
            }
            else if ('o' == *(argv[1]))//optimized power mode
            {
                if (argc == 2)
                    printOD8OptimizedPowerModeParameters();
                else if (argc == 5)
                    SetOD8OptimizedPowerModeParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('r' == *(argv[1]))//Fan curve
            {
                if (argc == 2)
                {
                    PrintfOD8FanCurve();
                }
                else if (argc == 4) {
                    SetOD8AllFanCurvePointsPWM(atoi(argv[2]), atoi(argv[3]));
                }
                else if (argc == 5)
                    SetOD8FanCurveSettingParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
                else
                    printHelp(argv[0]);
            }
            else if ('p' == *(argv[1]))//Power gauge
            {
                if (argc == 2)
                    printOD8PowerGaugeParameters();
                else
                    printHelp(argv[0]);
            }
            else if ('w' == *(argv[1]))//WS Auto Fan
            {
                if (argc == 2)
                    printOD8WSAutoFanAcousticLimitParameters();
                else if (argc == 3)
                    SetOD8WSAutoFanAcousticLimitParameters(OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT, atoi(argv[2]));
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

int GetPMLogSupport(int adapterNumber, ADLPMLogSupportInfo *adlPMLogSupportInfo)
{

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Get(context, lpAdapterInfo[adapterNumber].iAdapterIndex, adlPMLogSupportInfo))
	{
		PRINTF("Err: Failed to get PMLog Support for adapter number: %d\n", adapterNumber);
		return ADL_ERR;
	}

	return ADL_OK;
}

int IsSensorSupportedusingPMLogCall(ADLPMLogSupportInfo *adlPMLogSupportInfo, int item)
{
	int support = 0;
	if (item != 0)
	{
		for (int i = 0; i < ADL_PMLOG_MAX_SUPPORTED_SENSORS; i++)
		{
			if (adlPMLogSupportInfo->usSensors[i] == item)
			{
				support = 1;
				break;
			}
		}
	}
	return support;
}

int GetSensorValueFromPMLog(ADLPMLogData** PMLogOutput, int item)
{
	int value = -1;
	if (item != 0)
	{
 		for (int i = 0; i < ADL_PMLOG_MAX_SUPPORTED_SENSORS; i++)
		{
			if ((*PMLogOutput)->ulValues[i][0] == item)
			{
				value = (*PMLogOutput)->ulValues[i][1];
				break;
			}
		}
	}
		return value;
}

int PMLogAllSensorStart(int adapterNumber, int sampleRate,  ADLPMLogData** PMLogOutput, ADL_D3DKMT_HANDLE *hDevice)
{
	ADLPMLogSupportInfo adlPMLogSupportInfo;
	ADLPMLogStartInput adlPMLogStartInput;
	ADLPMLogStartOutput adlPMLogStartOutput;
	int i = 0;
	
	if(*hDevice == 0)
	{
		if (ADL_OK != PMLogCreateD3DDevice(adapterNumber, hDevice))
		{
			PRINTF("Err: Failed to create D3D Device, can not start PMLOG\n");
			return ADL_ERR;
		}
	}
	if (ADL_OK != GetPMLogSupport(adapterNumber, &adlPMLogSupportInfo))
	{
		PRINTF("Err: Failed to get supported sensors, can not start PMLOG\n");
		return ADL_ERR;
	}


	while (adlPMLogSupportInfo.usSensors[i] != ADL_SENSOR_MAXTYPES)
	{
		adlPMLogStartInput.usSensors[i] = adlPMLogSupportInfo.usSensors[i];
		i++;
	}

	adlPMLogStartInput.usSensors[i] = ADL_SENSOR_MAXTYPES;
	adlPMLogStartInput.ulSampleRate = sampleRate;

	if (ADL_OK != ADL2_Adapter_PMLog_Start(context, lpAdapterInfo[0].iAdapterIndex, &adlPMLogStartInput, &adlPMLogStartOutput, *hDevice))
	{
 		PRINTF("Failed to start PMLOG\n");
		return ADL_ERR;
	}


	 *PMLogOutput = (ADLPMLogData  *)(adlPMLogStartOutput.pLoggingAddress);

	return ADL_OK;
}

int printOD8()
{
	int i;
	int ret = -1;
	int iSupported = 0, iEnabled = 0, iVersion = 0;
//	getchar();
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
				else
				{
					PRINTF("odInitSetting.count=%d\n", odInitSetting.count);
					PRINTF("odInitSetting.Caps=%d\n", odInitSetting.overdrive8Capabilities);
					PRINTF("**************Caps**************\n");
					for (int j=0; j < sizeof(ADLOD8FeatureControlStr) / sizeof(*ADLOD8FeatureControlStr); ++j)
					{
						if (( odInitSetting.overdrive8Capabilities & (1 << j)) == (1 << j))
						{
							PRINTF(" feature %-50s support\n", ADLOD8FeatureControlStr[j]);
						}
						else
						{
							PRINTF(" feature %-50s Not support\n", ADLOD8FeatureControlStr[j]);
						}
					}

					PRINTF("\n************* Range **********\n");
					for (int j = 0; j < odInitSetting.count; ++j)
					{
						PRINTF("j=%-10d %-35s ID=%-10d def=%-10d min=%-10d max=%-10d\n",j, ADLOD8SettingIdStr[j],odInitSetting.od8SettingTable[j].featureID, odInitSetting.od8SettingTable[j].defaultValue, odInitSetting.od8SettingTable[j].minValue, odInitSetting.od8SettingTable[j].maxValue);
					}
				}

				//OD8 Current Status
				ADLOD8CurrentSetting odCurrentSetting;
				if (ADL_OK != GetOD8CurrentSetting(lpAdapterInfo[i].iAdapterIndex, odCurrentSetting))
				{
					PRINTF("Get Current Setting failed.\n");
					return ADL_ERR;
				}
				else
				{
					PRINTF("\n************* Settings **********\n");

					PRINTF("odCurrentSetting.connt=%d\n", odCurrentSetting.count);
					for (int j = 0; j < odCurrentSetting.count; j++)
					{
						PRINTF("j=%-10d %-35s current=%d\n", j, ADLOD8SettingIdStr[j],odCurrentSetting.Od8SettingTable[j]);
						
					}

				}

				ADL_D3DKMT_HANDLE hDevice = 0;

				if (hDevice == 0)
				{
					if (ADL_OK != PMLogCreateD3DDevice(lpAdapterInfo[i].iAdapterIndex, &hDevice))
					{
						PRINTF("Err: Failed to create D3D Device, can not start PMLOG\n");
						return ADL_ERR;
					}
				}

				ADLPMLogData* PMLogOutput=NULL;
				int ret = PMLogAllSensorStart(0, 1000, &PMLogOutput,&hDevice);
				system("cls");

				for (int k = 0; k < 200; k++)
				{
					ADLPMLogDataOutput odlpDataOutput;
					memset(&odlpDataOutput, 0, sizeof(ADLPMLogDataOutput));
					ret = ADL2_New_QueryPMLogData_Get(context, lpAdapterInfo[i].iAdapterIndex, &odlpDataOutput);
					ADLPMLogSupportInfo adlPMLogSupportInfo;
					memset(&adlPMLogSupportInfo, 0, sizeof(ADLPMLogSupportInfo));

					ret = GetPMLogSupport(lpAdapterInfo[i].iAdapterIndex, &adlPMLogSupportInfo);
					if (ADL_OK == ret)
					{
						PRINTF("\n************* PMLOG **********\n");
						PRINTF("odlpDataOutput.size=%d\n", odlpDataOutput.size);
						for (int j = 0; j < PMLOG_MAX_SENSORS_REAL; j++)
						{
							int itemSupport = IsSensorSupportedusingPMLogCall(&adlPMLogSupportInfo, j);
							int itemValue = itemSupport ? odlpDataOutput.sensors[j].value, GetSensorValueFromPMLog(&PMLogOutput, j) : -1;
          					PRINTF(" j=%-10d %-25s suport=%-10d vaue=%-10d newSupprt=%-10d  newaue=%-10d \n", j, sensorType[j], odlpDataOutput.sensors[j].supported, odlpDataOutput.sensors[j].value, itemSupport, itemValue);
						}
					}
					else
					{
						PRINTF("ADL2_New_QueryPMLogData_Get is failed\n"); 
						return ADL_ERR;
					}

					Sleep(500);
					system("cls");
				}


				if (ADL_OK != ADL2_Adapter_PMLog_Stop(context, lpAdapterInfo[0].iAdapterIndex, hDevice))
				{
					PRINTF("Failed to get PMLog Support\n");
					return ADL_ERR;
				}
				break;
			}
		}

	}
	return 0;
}
