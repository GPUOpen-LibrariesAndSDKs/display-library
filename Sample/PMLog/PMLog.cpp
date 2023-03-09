#include <stdio.h>
#include <tchar.h>

///
///  Copyright (c) 2008 - 2022 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file Source.cpp


#include <windows.h>
#include <sys\timeb.h>
#include <signal.h>

#include "..\..\include\adl_sdk.h"
#include "..\..\include\adl_structures.h"
#include "..\..\include\adl_defines.h"


#include <stdio.h>

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int ( *ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )();
typedef int ( *ADL_FLUSH_DRIVER_DATA)(int);
typedef int (*ADL2_ADAPTER_ACTIVE_GET ) (ADL_CONTEXT_HANDLE, int, int* );

typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) ( LPAdapterInfo, int );
typedef int(*ADL2_ADAPTER_PMLOG_SUPPORT_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLPMLogSupportInfo* pPMLogSupportInfo);
typedef int(*ADL2_ADAPTER_PMLOG_SUPPORT_START) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLPMLogStartInput* pPMLogStartInput, ADLPMLogStartOutput* pPMLogStartOutput, ADL_D3DKMT_HANDLE pDevice);
typedef int(*ADL2_ADAPTER_PMLOG_SUPPORT_STOP) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADL_D3DKMT_HANDLE pDevice);
typedef int(*ADL2_DEVICE_PMLOG_DEVICE_CREATE) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADL_D3DKMT_HANDLE *pDevice);
typedef int(*ADL2_DEVICE_PMLOG_DEVICE_DESTROY) (ADL_CONTEXT_HANDLE context, ADL_D3DKMT_HANDLE hDevice);
HINSTANCE hDLL;

ADL_D3DKMT_HANDLE hDevice = 0;

ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get=NULL;
ADL2_ADAPTER_PMLOG_SUPPORT_GET ADL2_Adapter_PMLog_Support_Get = NULL;
ADL2_ADAPTER_PMLOG_SUPPORT_START ADL2_Adapter_PMLog_Support_Start = NULL;
ADL2_ADAPTER_PMLOG_SUPPORT_STOP ADL2_Adapter_PMLog_Support_Stop = NULL;
ADL2_DEVICE_PMLOG_DEVICE_CREATE ADL2_Device_PMLog_Device_Create = NULL;
ADL2_DEVICE_PMLOG_DEVICE_DESTROY ADL2_Device_PMLog_Device_Destroy = NULL;
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
	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL_Main_Control_Create");
	ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
	ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET) GetProcAddress(hDLL,"ADL_Adapter_AdapterInfo_Get");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
	ADL2_Adapter_PMLog_Support_Get = (ADL2_ADAPTER_PMLOG_SUPPORT_GET)GetProcAddress(hDLL, "ADL2_Adapter_PMLog_Support_Get");
	ADL2_Adapter_PMLog_Support_Start = (ADL2_ADAPTER_PMLOG_SUPPORT_START)GetProcAddress(hDLL, "ADL2_Adapter_PMLog_Start");
	ADL2_Adapter_PMLog_Support_Stop = (ADL2_ADAPTER_PMLOG_SUPPORT_STOP)GetProcAddress(hDLL, "ADL2_Adapter_PMLog_Stop");
	ADL2_Device_PMLog_Device_Create = (ADL2_DEVICE_PMLOG_DEVICE_CREATE)GetProcAddress(hDLL, "ADL2_Device_PMLog_Device_Create");
	ADL2_Device_PMLog_Device_Destroy = (ADL2_DEVICE_PMLOG_DEVICE_DESTROY)GetProcAddress(hDLL, "ADL2_Device_PMLog_Device_Destroy");
	if ( NULL == ADL_Main_Control_Create ||
		 NULL == ADL_Main_Control_Destroy ||
		 NULL == ADL_Adapter_NumberOfAdapters_Get||
		 NULL == ADL_Adapter_AdapterInfo_Get ||
		 NULL == ADL2_Adapter_Active_Get ||
		 NULL == ADL2_Adapter_PMLog_Support_Get ||
		 NULL == ADL2_Adapter_PMLog_Support_Start ||
		 NULL == ADL2_Adapter_PMLog_Support_Stop ||
		 NULL == ADL2_Device_PMLog_Device_Create ||
		 NULL == ADL2_Device_PMLog_Device_Destroy
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

int PMLogDestroyD3DDevice(int adapterNumber, ADL_D3DKMT_HANDLE hDevice)
{
	if (ADL_OK != ADL2_Device_PMLog_Device_Destroy(context, hDevice))
	{
		PRINTF("Err: Failed to destory D3D device\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int PMLogCreateD3DDevice(int adapterNumber, ADL_D3DKMT_HANDLE *hDevice)
{
	if (ADL_OK != ADL2_Device_PMLog_Device_Create(context, lpAdapterInfo[adapterNumber].iAdapterIndex, hDevice))
	{
		PRINTF("Err: Failed to create D3D device\n");
		return ADL_ERR;
	}

	return ADL_OK;
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



int PrintAllSupportedSensors()
{
	ADLPMLogSupportInfo adlPMLogSupportInfo;
	int  i = 0, j = 0;

	// Repeat for all available adapters in the system
	for (i = 0; i < iNumberAdapters; i++)
	{
		if (lpAdapterInfo[i].iBusNumber > -1)
		{
			if (GetPMLogSupport(i, &adlPMLogSupportInfo) == ADL_ERR)
				return ADL_ERR;

			PRINTF("Adapter number %d supported sensors:\n", i);
			while (adlPMLogSupportInfo.usSensors[j] != ADL_SENSOR_MAXTYPES)
			{

				switch (adlPMLogSupportInfo.usSensors[j])
				{
					case ADL_PMLOG_CLK_GFXCLK:
						PRINTF("Graphics Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_MEMCLK:
						PRINTF("Memory Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_SOCCLK:
						PRINTF("SOC Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_UVDCLK1:
						PRINTF("UVD1 Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_UVDCLK2:
						PRINTF("UVD2 Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_VCECLK:
						PRINTF("VCE Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_VCNCLK:
						PRINTF("VCN Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_EDGE:
						PRINTF("EDGE Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_MEM:
						PRINTF("Memory Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_VRVDDC:
						PRINTF("VDDC VR Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_VRSOC:
						PRINTF("SOC VR Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_VRMVDD:
						PRINTF("MVDD VR Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_VRMVDD0:
						PRINTF("MVDD0 VR Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_VRMVDD1:
						PRINTF("MVDD1 VR Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_LIQUID:
						PRINTF("Liquid Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_PLX:
						PRINTF("PLX Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_FAN_RPM:
						PRINTF("Fan RPM Sensor Detected\n");
						break;
					case ADL_PMLOG_FAN_PERCENTAGE:
						PRINTF("Fan Percentage Sensor Detected\n");
						break;
					case ADL_PMLOG_SOC_VOLTAGE:
						PRINTF("SOC Voltage Sensor Detected\n");
						break;
					case ADL_PMLOG_SOC_POWER:
						PRINTF("SOC Power Sensor Detected\n");
						break;
					case ADL_PMLOG_SOC_CURRENT:
						PRINTF("SOC Current Sensor Detected\n");
						break;
					case ADL_PMLOG_INFO_ACTIVITY_GFX:
						PRINTF("GFX Activity Sensor Detected\n");
						break;
					case ADL_PMLOG_INFO_ACTIVITY_MEM:
						PRINTF("MEM Activity Sensor Detected\n");
						break;
					case ADL_PMLOG_GFX_VOLTAGE:
						PRINTF("GFX Voltage Sensor Detected\n");
						break;
					case ADL_PMLOG_MEM_VOLTAGE:
						PRINTF("MEM Voltage Sensor Detected\n");
						break;
					case ADL_PMLOG_ASIC_POWER:
						PRINTF("Asic Power Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_HOTSPOT:
						PRINTF("HOTSPOT Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_GFX:
						PRINTF("GFX Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_SOC:
						PRINTF("SOC Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_TEMPERATURE_CPU:
						PRINTF("CPU Temperature Sensor Detected\n");
						break;
					case ADL_PMLOG_GFX_POWER:
						PRINTF("GFX Power Sensor Detected\n");
						break;
					case ADL_PMLOG_GFX_CURRENT:
						PRINTF("GFX Current Sensor Detected\n");
						break;
					case ADL_PMLOG_CPU_POWER:
						PRINTF("CPU Power Sensor Detected\n");
						break;
					case ADL_PMLOG_CLK_CPUCLK:
						PRINTF("CPU Clock Sensor Detected\n");
						break;
					case ADL_PMLOG_THROTTLER_STATUS:
						PRINTF("Throttler Status Sensor Detected\n");
						break;
					case ADL_PMLOG_SMART_POWERSHIFT_CPU:
						PRINTF("Powershift CPU Sensor Detected\n");
						break;
					case ADL_PMLOG_SMART_POWERSHIFT_DGPU:
						PRINTF("Powershift DGPU Sensor Detected\n");
						break;
					case ADL_PMLOG_BUS_SPEED:
						PRINTF("Bus Speed Sensor Detected\n");
						break;
					case ADL_PMLOG_BUS_LANES:
						PRINTF("Bus Lanes Sensor Detected\n");
						break;
					case ADL_PMLOG_BOARD_POWER:
						PRINTF("Total Board Power Sensor Detected\n");
						break;
					case ADL_PMLOG_SSPAIRED_ASICPOWER:
						PRINTF("APU Power Sensor Detected\n");
						break;
					case ADL_PMLOG_SSTOTAL_POWERLIMIT:
						PRINTF("Total Power limit Sensor Detected\n");
						break;
					case ADL_PMLOG_SSAPU_POWERLIMIT:
						PRINTF("APU Power limit Sensor Detected\n");
						break;
					case ADL_PMLOG_SSDGPU_POWERLIMIT:
						PRINTF("DGPU Power limit Sensor Detected\n");
						break;
					case ADL_SENSOR_MAXTYPES:
						PRINTF("End of Sensor Detected\n");
						break;
					default:
						PRINTF("ER: Uknown sensor detected\n");
						break;
				}
				PRINTF("ID: %d\n", adlPMLogSupportInfo.usSensors[j]);
				j++;
			}

			j = 0;
			
		}

	}

	return ADL_OK;
}

void DisplayPMLogOutput(ADLPMLogData* PMLogOutput, int Duration)
{
	struct timeb startTime, currentTime;
	int diff;
	int i;

	ftime(&startTime);
	system("cls");
	do {
		i = 0;
		while (PMLogOutput->ulValues[i][0] != ADL_SENSOR_MAXTYPES)
		{

			switch (PMLogOutput->ulValues[i][0])
			{
			case ADL_PMLOG_CLK_GFXCLK:
				PRINTF("Graphics Clock Sensor: ");
				break;
			case ADL_PMLOG_CLK_MEMCLK:
				PRINTF("Memory Clock Sensor: ");
				break;
			case ADL_PMLOG_CLK_SOCCLK:
				PRINTF("SOC Clock Sensor: ");
				break;
			case ADL_PMLOG_CLK_UVDCLK1:
				PRINTF("UVD1 Clock Sensor: ");
				break;
			case ADL_PMLOG_CLK_UVDCLK2:
				PRINTF("UVD2 Clock Sensor: ");
				break;
			case ADL_PMLOG_CLK_VCECLK:
				PRINTF("VCE Clock Sensor: ");
				break;
			case ADL_PMLOG_CLK_VCNCLK:
				PRINTF("VCN Clock Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_EDGE:
				PRINTF("EDGE Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_MEM:
				PRINTF("Memory Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_VRVDDC:
				PRINTF("VDDC VR Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_VRSOC:
				PRINTF("SOC VR Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_VRMVDD:
				PRINTF("MVDD VR Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_VRMVDD0:
				PRINTF("MVDD0 VR Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_VRMVDD1:
				PRINTF("MVDD1 VR Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_LIQUID:
				PRINTF("Liquid Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_PLX:
				PRINTF("PLX Temperature Sensor: ");
				break;
			case ADL_PMLOG_FAN_RPM:
				PRINTF("Fan RPM Sensor: ");
				break;
			case ADL_PMLOG_FAN_PERCENTAGE:
				PRINTF("Fan Percentage Sensor: ");
				break;
			case ADL_PMLOG_SOC_VOLTAGE:
				PRINTF("SOC Voltage Sensor: ");
				break;
			case ADL_PMLOG_SOC_POWER:
				PRINTF("SOC Power Sensor: ");
				break;
			case ADL_PMLOG_SOC_CURRENT:
				PRINTF("SOC Current Sensor: ");
				break;
			case ADL_PMLOG_INFO_ACTIVITY_GFX:
				PRINTF("GFX Activity Sensor: ");
				break;
			case ADL_PMLOG_INFO_ACTIVITY_MEM:
				PRINTF("MEM Activity Sensor: ");
				break;
			case ADL_PMLOG_GFX_VOLTAGE:
				PRINTF("GFX Voltage Sensor: ");
				break;
			case ADL_PMLOG_MEM_VOLTAGE:
				PRINTF("MEM Voltage Sensor: ");
				break;
			case ADL_PMLOG_ASIC_POWER:
				PRINTF("Asic Power Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_HOTSPOT:
				PRINTF("HOTSPOT Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_GFX:
				PRINTF("GFX Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_SOC:
				PRINTF("SOC Temperature Sensor: ");
				break;
			case ADL_PMLOG_TEMPERATURE_CPU:
				PRINTF("CPU Temperature Sensor: ");
				break;
			case ADL_PMLOG_GFX_POWER:
				PRINTF("GFX Power Sensor: ");
				break;
			case ADL_PMLOG_GFX_CURRENT:
				PRINTF("GFX Current Sensor: ");
				break;
			case ADL_PMLOG_CPU_POWER:
				PRINTF("CPU Power Sensor: ");
				break;
			case ADL_PMLOG_CLK_CPUCLK:
				PRINTF("CPU Clock Sensor: ");
				break;
			case ADL_PMLOG_THROTTLER_STATUS:
				PRINTF("Throttler Status Sensor: ");
				break;
			case ADL_PMLOG_SMART_POWERSHIFT_CPU:
				PRINTF("Powershift CPU Sensor: ");
				break;
			case ADL_PMLOG_SMART_POWERSHIFT_DGPU:
				PRINTF("Powershift DGPU Sensor: ");
				break;
			case ADL_PMLOG_BUS_SPEED:
				PRINTF("Bus Speed Sensor: ");
				break;
			case ADL_PMLOG_BUS_LANES:
				PRINTF("Bus Lanes Sensor: ");
				break;
			case ADL_PMLOG_BOARD_POWER:
				PRINTF("Total Board Power Sensor: ");
				break;
			case ADL_PMLOG_SSPAIRED_ASICPOWER:
				PRINTF("APU Power Sensor: ");
				break;
			case ADL_PMLOG_SSTOTAL_POWERLIMIT:
				PRINTF("Total Power limit Sensor: ");
				break;
			case ADL_PMLOG_SSAPU_POWERLIMIT:
				PRINTF("APU Power limit Sensor: ");
				break;
			case ADL_PMLOG_SSDGPU_POWERLIMIT:
				PRINTF("DGPU Power limit Sensor: ");
				break;
			case ADL_SENSOR_MAXTYPES:
				break;
			default:
				PRINTF("Unknown Sensor Value: ");
				break;
			}

			PRINTF("%d\n", PMLogOutput->ulValues[i][1]);
			i++;
		}

 		Sleep(500);
		system("cls");
		ftime(&currentTime);
		diff = (int)(currentTime.time - startTime.time);
	} while (diff < Duration);
}


int PMLogAllSensorStart(int adapterNumber, int sampleRate, int Duration)
{
	ADLPMLogSupportInfo adlPMLogSupportInfo;
	ADLPMLogStartInput adlPMLogStartInput;
	ADLPMLogStartOutput adlPMLogStartOutput;
	ADLPMLogData* PMLogOutput;
	int i = 0;

	if (hDevice == 0)
	{
		if (ADL_OK != PMLogCreateD3DDevice(lpAdapterInfo[adapterNumber].iAdapterIndex, &hDevice))
		{
			PRINTF("Err: Failed to create D3D Device, can not start PMLOG\n");
			return ADL_ERR;
		}
	}

	if (ADL_OK != GetPMLogSupport(lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogSupportInfo))
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
	if (ADL_OK != ADL2_Adapter_PMLog_Support_Start(context, lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogStartInput, &adlPMLogStartOutput, hDevice))
	{
		PRINTF("Failed to start PMLOG\n");
		return ADL_ERR;
	}


	PMLogOutput = (ADLPMLogData  *)(adlPMLogStartOutput.pLoggingAddress);

	DisplayPMLogOutput(PMLogOutput, Duration);

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Stop(context, lpAdapterInfo[adapterNumber].iAdapterIndex, hDevice))
	{
		PRINTF("Failed to get PMLog Support\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int PMLogMclkStart(int adapterNumber, int sampleRate, int Duration)
{
	ADLPMLogSupportInfo adlPMLogSupportInfo;
	ADLPMLogStartInput adlPMLogStartInput;
	ADLPMLogStartOutput adlPMLogStartOutput;
	ADLPMLogData* PMLogOutput;
	int i = 0;

	if (hDevice == 0)
	{
		if (ADL_OK != PMLogCreateD3DDevice(lpAdapterInfo[adapterNumber].iAdapterIndex, &hDevice))
		{
			PRINTF("Err: Failed to create D3D Device, can not start PMLOG\n");
			return ADL_ERR;
		}
	}

	if (ADL_OK != GetPMLogSupport(lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogSupportInfo))
	{
		PRINTF("Err: Failed to get supported sensors, can not start PMLOG\n");
		return ADL_ERR;
	}


	adlPMLogStartInput.usSensors[0] = ADL_PMLOG_CLK_MEMCLK;


	adlPMLogStartInput.usSensors[1] = ADL_SENSOR_MAXTYPES;
	adlPMLogStartInput.ulSampleRate = sampleRate;

	PRINTF("Start MCLK PMGLOG\n");

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Start(context, lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogStartInput, &adlPMLogStartOutput, hDevice))
	{
		PRINTF("Failed to start MCLK PMGLOG\n");
		return ADL_ERR;
	}


	PMLogOutput = (ADLPMLogData  *)(adlPMLogStartOutput.pLoggingAddress);

	DisplayPMLogOutput(PMLogOutput, Duration);

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Stop(context, lpAdapterInfo[adapterNumber].iAdapterIndex, hDevice))
	{
		PRINTF("Failed to get PMLog Support\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int PMLogFanStart(int adapterNumber, int sampleRate, int Duration)
{
	ADLPMLogSupportInfo adlPMLogSupportInfo;
	ADLPMLogStartInput adlPMLogStartInput;
	ADLPMLogStartOutput adlPMLogStartOutput;
	ADLPMLogData* PMLogOutput;
	int i = 0;

	if (hDevice == 0)
	{
		if (ADL_OK != PMLogCreateD3DDevice(lpAdapterInfo[adapterNumber].iAdapterIndex, &hDevice))
		{
			PRINTF("Err: Failed to create D3D Device, can not start PMLOG\n");
			return ADL_ERR;
		}
	}

	if (ADL_OK != GetPMLogSupport(lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogSupportInfo))
	{
		PRINTF("Err: Failed to get supported sensors, can not start PMLOG\n");
		return ADL_ERR;
	}


	//adlPMLogStartInput.usSensors[0] = ADL_PMLOG_CLK_MEMCLK;
	adlPMLogStartInput.usSensors[0] = ADL_PMLOG_FAN_RPM;

	adlPMLogStartInput.usSensors[1] = ADL_SENSOR_MAXTYPES;
	adlPMLogStartInput.ulSampleRate = sampleRate;

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Start(context, lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogStartInput, &adlPMLogStartOutput, hDevice))
	{
		PRINTF("Failed to start MCLK PMGLOG\n");
		return ADL_ERR;
	}


	PMLogOutput = (ADLPMLogData  *)(adlPMLogStartOutput.pLoggingAddress);

	DisplayPMLogOutput(PMLogOutput, Duration);

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Stop(context, lpAdapterInfo[adapterNumber].iAdapterIndex, hDevice))
	{
		PRINTF("Failed to get PMLog Support\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

int PMLogGfxClkStart(int adapterNumber, int sampleRate, int Duration)
{
	ADLPMLogSupportInfo adlPMLogSupportInfo;
	ADLPMLogStartInput adlPMLogStartInput;
	ADLPMLogStartOutput adlPMLogStartOutput;
	ADLPMLogData* PMLogOutput;
	int i = 0;

	if (hDevice == 0)
	{
		if (ADL_OK != PMLogCreateD3DDevice(lpAdapterInfo[adapterNumber].iAdapterIndex, &hDevice))
		{
			PRINTF("Err: Failed to create D3D Device, can not start PMLOG\n");
			return ADL_ERR;
		}
	}

	if (ADL_OK != GetPMLogSupport(lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogSupportInfo))
	{
		PRINTF("Err: Failed to get supported sensors, can not start PMLOG\n");
		return ADL_ERR;
	}

	adlPMLogStartInput.usSensors[0] = ADL_PMLOG_CLK_GFXCLK;

	adlPMLogStartInput.usSensors[1] = ADL_SENSOR_MAXTYPES;
	adlPMLogStartInput.ulSampleRate = sampleRate;

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Start(context, lpAdapterInfo[adapterNumber].iAdapterIndex, &adlPMLogStartInput, &adlPMLogStartOutput, hDevice))
	{
		PRINTF("Failed to start MCLK PMGLOG\n");
		return ADL_ERR;
	}


	PMLogOutput = (ADLPMLogData  *)(adlPMLogStartOutput.pLoggingAddress);

	DisplayPMLogOutput(PMLogOutput, Duration);

	if (ADL_OK != ADL2_Adapter_PMLog_Support_Stop(context, lpAdapterInfo[adapterNumber].iAdapterIndex, hDevice))
	{
		PRINTF("Failed to get PMLog Support\n");
		return ADL_ERR;
	}

	return ADL_OK;
}

void deinitializeADL()
{
		
	ADL_Main_Control_Destroy();
	
	FreeLibrary(hDLL);


}

int main(int argc, char* argv[])
{
	int result = 0;

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
				lpAdapterInfo = (LPAdapterInfo)malloc(sizeof (AdapterInfo)* iNumberAdapters);
				memset(lpAdapterInfo, '\0', sizeof (AdapterInfo)* iNumberAdapters);

				// Get the AdapterInfo structure for all adapters in the system
				ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof (AdapterInfo)* iNumberAdapters);
			}

			switch (*(argv[1]))
			{
				case 'l':
					result = PrintAllSupportedSensors();
					break;
				case 's':
					 if (argc == 5)
						 PMLogAllSensorStart(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
					else
						printf("signature of PMLog all sensors (PMLog.exe s X Y Z); X - Adapter Number, Y - Sample Rate (ms), Z - Duration to Log");
					break;
				case 'm':
					if (argc == 5)
						PMLogMclkStart(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
					else
						printf("signature of PMLog MCLK sensor (PMLog.exe m X Y Z); X - Adapter Number, Y - Sample Rate (ms), Z - Duration to Log");
					break;
				case 'f':
					if (argc == 5)
						PMLogFanStart(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
					else
						printf("signature of PMLog fan sensor (PMLog.exe f X Y Z); X - Adapter Number, Y - Sample Rate (ms), Z - Duration to Log");
					break;
				case 'g':
					if (argc == 5)
						PMLogGfxClkStart(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
					else
						printf("signature of PMLog GFX clock sensor (PMLog.exe g X Y Z); X - Adapter Number, Y - Sample Rate (ms), Z - Duration to Log");
					break;
					
				default:
					printf ("Available command line parameters: l- List All sensors, s-start log all sensors, m-start log MCLK sensor, f-start fan sensor, g-start GFX clock sensor\n");
		
					break;
			}
		}
		else
		{
					printf ("Available command line parameters: l- List All sensors, s-start log all sensors, m-start log MCLK sensor, f-start fan sensor, g-start GFX clock sensor\n");
		}

		ADL_Main_Memory_Free((void**)&lpAdapterInfo);

		deinitializeADL();
	}

	return result;
}
