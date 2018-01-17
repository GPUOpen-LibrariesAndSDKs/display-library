#include <stdio.h>
#include <tchar.h>

///
///  Copyright (c) 2008 - 2016 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file Source.cpp


#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include "..\..\include\adl_structures.h"


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
typedef int ( *ADL_ADAPTERX2_CAPS) (int, int*);
typedef int ( *ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int ( *ADL2_OVERDRIVEN_CAPABILITIES_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNCapabilities*);
typedef int ( *ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int ( *ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int ( *ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int ( *ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int ( *ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceStatus*);
typedef int ( *ADL2_OVERDRIVEN_FANCONTROL_GET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int ( *ADL2_OVERDRIVEN_FANCONTROL_SET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int ( *ADL2_OVERDRIVEN_POWERLIMIT_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int ( *ADL2_OVERDRIVEN_POWERLIMIT_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int ( *ADL2_OVERDRIVEN_TEMPERATURE_GET) (ADL_CONTEXT_HANDLE, int, int, int*);
HINSTANCE hDLL;

ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
ADL_ADAPTERX2_CAPS ADL_AdapterX2_Caps = NULL;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get=NULL;
ADL2_OVERDRIVEN_CAPABILITIES_GET ADL2_OverdriveN_Capabilities_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET ADL2_OverdriveN_SystemClocks_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET ADL2_OverdriveN_SystemClocks_Set = NULL;
ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET ADL2_OverdriveN_PerformanceStatus_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_GET ADL2_OverdriveN_FanControl_Get =NULL;
ADL2_OVERDRIVEN_FANCONTROL_SET ADL2_OverdriveN_FanControl_Set=NULL;
ADL2_OVERDRIVEN_POWERLIMIT_GET ADL2_OverdriveN_PowerLimit_Get =NULL;
ADL2_OVERDRIVEN_POWERLIMIT_SET ADL2_OverdriveN_PowerLimit_Set=NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_OverdriveN_MemoryClocks_Get = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_OverdriveN_MemoryClocks_Set = NULL;
ADL2_OVERDRIVE_CAPS ADL2_Overdrive_Caps = NULL;
ADL2_OVERDRIVEN_TEMPERATURE_GET ADL2_OverdriveN_Temperature_Get = NULL;
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
	ADL_AdapterX2_Caps = (ADL_ADAPTERX2_CAPS) GetProcAddress( hDLL, "ADL_AdapterX2_Caps");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
	ADL2_OverdriveN_Capabilities_Get = (ADL2_OVERDRIVEN_CAPABILITIES_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_Capabilities_Get");
	ADL2_OverdriveN_SystemClocks_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_SystemClocks_Get");
	ADL2_OverdriveN_SystemClocks_Set = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_SystemClocks_Set");
	ADL2_OverdriveN_MemoryClocks_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKS_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_MemoryClocks_Get");
	ADL2_OverdriveN_MemoryClocks_Set = (ADL2_OVERDRIVEN_MEMORYCLOCKS_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_MemoryClocks_Set");
	ADL2_OverdriveN_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) GetProcAddress (hDLL,"ADL2_OverdriveN_PerformanceStatus_Get");
	ADL2_OverdriveN_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_FanControl_Get");
	ADL2_OverdriveN_FanControl_Set = (ADL2_OVERDRIVEN_FANCONTROL_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_FanControl_Set");
	ADL2_OverdriveN_PowerLimit_Get = (ADL2_OVERDRIVEN_POWERLIMIT_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_PowerLimit_Get");
	ADL2_OverdriveN_PowerLimit_Set = (ADL2_OVERDRIVEN_POWERLIMIT_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_PowerLimit_Set");
	ADL2_OverdriveN_Temperature_Get = (ADL2_OVERDRIVEN_TEMPERATURE_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_Temperature_Get");
	ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS) GetProcAddress (hDLL, "ADL2_Overdrive_Caps");
	if ( NULL == ADL_Main_Control_Create ||
		 NULL == ADL_Main_Control_Destroy ||
		 NULL == ADL_Adapter_NumberOfAdapters_Get||
		 NULL == ADL_Adapter_AdapterInfo_Get ||
		 NULL == ADL_AdapterX2_Caps ||
		NULL == ADL2_Adapter_Active_Get ||
		NULL == ADL2_OverdriveN_Capabilities_Get || 
		NULL == ADL2_OverdriveN_SystemClocks_Get ||
		NULL == ADL2_OverdriveN_SystemClocks_Set ||
		NULL == ADL2_OverdriveN_MemoryClocks_Get ||
		NULL == ADL2_OverdriveN_MemoryClocks_Set ||
		NULL == ADL2_OverdriveN_PerformanceStatus_Get ||
		NULL == ADL2_OverdriveN_FanControl_Get ||
		NULL == ADL2_OverdriveN_FanControl_Set ||
		NULL == ADL2_Overdrive_Caps
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
	
	FreeLibrary(hDLL);


}

int printODNSCLKParameters()
{
 	int  i, active = 0;;
	
	int iSupported,iEnabled,iVersion;
	 

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}
				else
				{
					//performance levels info
					ADLODNPerformanceLevels *odPerformanceLevels;

					int size = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1);
					void* performanceLevelsBuffer = new char[size];
					memset(performanceLevelsBuffer, 0, size);
					odPerformanceLevels = (ADLODNPerformanceLevels*)performanceLevelsBuffer;
					odPerformanceLevels->iSize = size;
					odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
					//get GPU clocks
					if (ADL_OK != ADL2_OverdriveN_SystemClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
					{
						PRINTF("ADL2_OverdriveN_GPUClocks_Get is failed\n");

					}

					PRINTF("--------------------------------------------\n");					
					PRINTF(" ADL2_OverdriveN_GPUClocks_Get Data\n");
					PRINTF("--------------------------------------------\n");					
					PRINTF("Number of Levels : %d \n" ,overdriveCapabilities.iMaximumNumberOfPerformanceLevels);
					PRINTF("Clocks: Minimum Range : %d, Maximum Range : %d, Step value: %d\n" ,overdriveCapabilities.sEngineClockRange.iMin,overdriveCapabilities.sEngineClockRange.iMax, overdriveCapabilities.sEngineClockRange.iStep);
					PRINTF("Voltage: Minimum Range : %d, Maximum Range : %d, Step value: %d\n" ,overdriveCapabilities.svddcRange.iMin,overdriveCapabilities.svddcRange.iMax, overdriveCapabilities.svddcRange.iStep);
					for (int i=0 ;i <overdriveCapabilities.iMaximumNumberOfPerformanceLevels;i++)
					{
						PRINTF("odPerformanceLevels->aLevels[%d].iClock : %d\n" , i, odPerformanceLevels->aLevels[i].iClock);
						PRINTF("odPerformanceLevels->aLevels[%d].iEnabled : %d\n", i, odPerformanceLevels->aLevels[i].iEnabled);
						PRINTF("odPerformanceLevels->aLevels[%d].iVddc : %d\n\n", i, odPerformanceLevels->aLevels[i].iVddc);	
					}			
					PRINTF("---------------------------------------------\n");					
				}				
				break;
			}			
		}					
	}

	
	
	return 0;
}

int setODNSCLKParameters(int level, int feature, int value)
{
 	int i, active = 0;;
 	int iSupported,iEnabled,iVersion;
	
	
	// Obtain the number of adapters for the system
    if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters ) )
	{
	       PRINTF("Cannot get the number of adapters!\n");
		   return 0;
	}

    if ( 0 < iNumberAdapters )
    {
        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
    }

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}
				else
				{
					//performance levels info
					ADLODNPerformanceLevels *odPerformanceLevels;

					int size = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1);
					void* performanceLevelsBuffer = new char[size];
					memset(performanceLevelsBuffer, 0, size);
					odPerformanceLevels = (ADLODNPerformanceLevels*)performanceLevelsBuffer;
					odPerformanceLevels->iSize = size;
					odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
					//get GPU clocks
					if (ADL_OK != ADL2_OverdriveN_SystemClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
					{
						PRINTF("ADL2_OverdriveN_GPUClocks_Get is failed\n");

					}

					if (feature == 1)
					{
						if (!(overdriveCapabilities.sEngineClockRange.iMin <= value && overdriveCapabilities.sEngineClockRange.iMax >= value))
						{
							PRINTF("Clock range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.sEngineClockRange.iMin,overdriveCapabilities.sEngineClockRange.iMax, overdriveCapabilities.sEngineClockRange.iStep);			
						}
						else if (level > overdriveCapabilities.iMaximumNumberOfPerformanceLevels || level < 0)
						{
							PRINTF("Performance level should be less than : %d\n" ,overdriveCapabilities.iMaximumNumberOfPerformanceLevels);			
						}
						else if (odPerformanceLevels->aLevels[level].iEnabled == 0)
						{
							PRINTF("Performance level (%d) is disabled\n" ,level);			
						}
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNSCLKParameters();
							odPerformanceLevels->aLevels[level].iClock = value;							
							odPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Manual;
							
							if (ADL_OK != ADL2_OverdriveN_SystemClocks_Set(context,lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
							{
								PRINTF("ADL2_OverdriveN_SystemClocks_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_SystemClocks_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNSCLKParameters();
							}
						}
					}
					else if (feature == 2)
					{
						if (!(overdriveCapabilities.svddcRange.iMin <= value && overdriveCapabilities.svddcRange.iMax >= value))
						{
							PRINTF("Clock range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.svddcRange.iMin,overdriveCapabilities.svddcRange.iMax, overdriveCapabilities.svddcRange.iStep);			
						}
						else if (level > overdriveCapabilities.iMaximumNumberOfPerformanceLevels || level < 0)
						{
							PRINTF("Performance level should be less than : %d\n" ,overdriveCapabilities.iMaximumNumberOfPerformanceLevels);			
						}
						else if (odPerformanceLevels->aLevels[level].iEnabled == 0)
						{
							PRINTF("Performance level (%d) is disabled\n" ,level);			
						}
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNSCLKParameters();
							odPerformanceLevels->aLevels[level].iVddc = value;							
							odPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Manual;
							
							if (ADL_OK != ADL2_OverdriveN_SystemClocks_Set(context,lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
							{
								PRINTF("ADL2_OverdriveN_SystemClocks_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_SystemClocks_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNSCLKParameters();
							}
						}				
					}						
				}
				break;
			}
		}					
	}

	
	
	return 0;
}

int printODNMCLKParameters()
{
	int i, active = 0;;
	int iSupported,iEnabled,iVersion;
	

	// Obtain the number of adapters for the system
    if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters ) )
	{
	       PRINTF("Cannot get the number of adapters!\n");
		   return 0;
	}

    if ( 0 < iNumberAdapters )
    {
        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
    }

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		
			if (lpAdapterInfo[ i ].iBusNumber > -1)
			{
				ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				 ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
						PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
					}
				else
				{
					//performance levels info
					ADLODNPerformanceLevels *odPerformanceLevels;

					int size = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1);
					void* performanceLevelsBuffer = new char[size];
					memset(performanceLevelsBuffer, 0, size);
					odPerformanceLevels = (ADLODNPerformanceLevels*)performanceLevelsBuffer;
					odPerformanceLevels->iSize = size;
					odPerformanceLevels->iMode = 0; //current
					odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
					
					//get GPU clocks
					if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
					{
						PRINTF("ADL2_OverdriveN_GPUClocks_Get is failed\n");

					}
					ADLODNPerformanceLevels *odPerformanceLevels_default;
					void* performanceLevelsBuffer_default = new char[size];
					memset(performanceLevelsBuffer_default, 0, size);
					odPerformanceLevels_default = (ADLODNPerformanceLevels*)performanceLevelsBuffer_default;
					odPerformanceLevels_default->iSize = size;
					odPerformanceLevels_default->iMode = 1; //Defaults
					odPerformanceLevels_default->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
					
					//get GPU clocks
					if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels_default))
					{
						PRINTF("ADL2_OverdriveN_GPUClocks_Get is failed\n");

					}

					PRINTF("-------------------------------------\n");					
					PRINTF("ADL2_OverdriveN_MemoryClocks_Get Data\n");
					PRINTF("-------------------------------------\n");
					PRINTF("Number of Levels : %d\n" ,overdriveCapabilities.iMaximumNumberOfPerformanceLevels);
					//PRINTF("Minimum Range : %d, Maximum Range : %d, Step value: %d\n" ,overdriveCapabilities.sMemoryClockRange.iMin,overdriveCapabilities.sMemoryClockRange.iMax, overdriveCapabilities.sMemoryClockRange.iStep);
					//PRINTF("Voltage: Minimum Range : %d, Maximum Range : %d, Step value: %d\n" ,overdriveCapabilities.svddcRange.iMin,overdriveCapabilities.svddcRange.iMax, overdriveCapabilities.svddcRange.iStep);
					
					for (int i=0 ;i <overdriveCapabilities.iMaximumNumberOfPerformanceLevels;i++)
					{
						PRINTF("odPerformanceLevels->aLevels[%d].iClock : %d\n" , i, odPerformanceLevels->aLevels[i].iClock);
						PRINTF("MCLK Minimum Range : %d, Maximum Range : %d, Step value: %d\n" ,odPerformanceLevels_default->aLevels[i].iClock,odPerformanceLevels_default->aLevels[i].iClock + 5000, overdriveCapabilities.sMemoryClockRange.iStep);				
						PRINTF("odPerformanceLevels->aLevels[%d].iEnabled : %d\n", i, odPerformanceLevels->aLevels[i].iEnabled);
						PRINTF("odPerformanceLevels->aLevels[%d].iVddc : %d\n", i, odPerformanceLevels->aLevels[i].iVddc);	
						PRINTF("Voltage: Minimum Range : %d, Maximum Range : %d, Step value: %d\n\n" ,overdriveCapabilities.svddcRange.iMin,overdriveCapabilities.svddcRange.iMax, overdriveCapabilities.svddcRange.iStep);
				
					}	
					PRINTF("-------------------------------------\n");					
					
				}
				break;
			}			
			}	
	}

	
	
	return 0;
}

int setODNMCLKParameters(int level, int feature, int value)
{
	int i, active = 0;;
	int iSupported,iEnabled,iVersion;
	
	
	// Obtain the number of adapters for the system
    if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters ) )
	{
	       PRINTF("Cannot get the number of adapters!\n");
		   return 0;
	}

    if ( 0 < iNumberAdapters )
    {
        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
    }

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}
				else
				{
					//performance levels info
					ADLODNPerformanceLevels *odPerformanceLevels;

					int size = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1);
					void* performanceLevelsBuffer = new char[size];
					memset(performanceLevelsBuffer, 0, size);
					odPerformanceLevels = (ADLODNPerformanceLevels*)performanceLevelsBuffer;
					odPerformanceLevels->iSize = size;
					odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
					//get GPU clocks
					if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
					{
						PRINTF("ADL2_OverdriveN_MemoryClocks_Get is failed\n");

					}

					if (feature == 1)
					{
						if (!(overdriveCapabilities.sMemoryClockRange.iMin <= value && overdriveCapabilities.sMemoryClockRange.iMax >= value))
						{
							PRINTF("Clock range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.sMemoryClockRange.iMin,overdriveCapabilities.sMemoryClockRange.iMax, overdriveCapabilities.sMemoryClockRange.iStep);			
						}
						else if (level > overdriveCapabilities.iMaximumNumberOfPerformanceLevels || level < 0)
						{
							PRINTF("Performance level should be less than : %d\n" ,overdriveCapabilities.iMaximumNumberOfPerformanceLevels);			
						}
						else if (odPerformanceLevels->aLevels[level].iEnabled == 0)
						{
							PRINTF("Performance level (%d) is disabled\n" ,level);			
						}
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNMCLKParameters();
							odPerformanceLevels->aLevels[level].iClock = value;							
							odPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Manual;
							
							if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Set(context,lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
							{
								PRINTF("ADL2_OverdriveN_MemoryClocks_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_MemoryClocks_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNMCLKParameters();
							}
						}
					}
					else if (feature == 2)
					{
						if (!(overdriveCapabilities.svddcRange.iMin <= value && overdriveCapabilities.svddcRange.iMax >= value))
						{
							PRINTF("Clock range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.svddcRange.iMin,overdriveCapabilities.svddcRange.iMax, overdriveCapabilities.svddcRange.iStep);			
						}
						else if (level > overdriveCapabilities.iMaximumNumberOfPerformanceLevels || level < 0)
						{
							PRINTF("Performance level should be less than : %d\n" ,overdriveCapabilities.iMaximumNumberOfPerformanceLevels);			
						}
						else if (odPerformanceLevels->aLevels[level].iEnabled == 0)
						{
							PRINTF("Performance level (%d) is disabled\n" ,level);			
						}
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNMCLKParameters();
							odPerformanceLevels->aLevels[level].iVddc = value;							
							odPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Manual;
							
							if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Set(context,lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
							{
								PRINTF("ADL2_OverdriveN_MemoryClocks_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_MemoryClocks_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNMCLKParameters();
							}
						}				
					}						
				}
				break;
			}
		}					
	}

		
	return 0;
}


int printODNFANParameters()
{
	int i, active = 0;;
	int iSupported,iEnabled,iVersion;
		 
	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}
				ADLODNFanControl odNFanControl;
				memset(&odNFanControl, 0, sizeof(ADLODNFanControl));
				
				if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
				{
					PRINTF("ADL2_OverdriveN_FanControl_Get is failed\n");
				}
				else
				{
					PRINTF("-----------------------------------------\n");					
					PRINTF("ADL2_OverdriveN_FanControl_Get Data\n");
					PRINTF("-----------------------------------------\n");
					PRINTF("Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.fanSpeed.iMin,overdriveCapabilities.fanSpeed.iMax, overdriveCapabilities.fanSpeed.iStep);				
					PRINTF("odNFanControl.iFanControlMode : %d\n" , odNFanControl.iFanControlMode);
					PRINTF("odNFanControl.iMinFanLimit : %d\n" , odNFanControl.iMinFanLimit);					
					PRINTF("odNFanControl.iTargetFanSpeed : %d\n" , odNFanControl.iTargetFanSpeed);
					
					PRINTF ("-------- FAN (Min performance) ---------\n");
					PRINTF("Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.minimumPerformanceClock.iMin,overdriveCapabilities.minimumPerformanceClock.iMax, overdriveCapabilities.minimumPerformanceClock.iStep);				
					PRINTF("odNFanControl.iMinPerformanceClock : %d\n" , odNFanControl.iMinPerformanceClock);
					
					PRINTF ("--------- FAN (Target Temp) -----------\n");
					PRINTF("Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.fanTemperature.iMin,overdriveCapabilities.fanTemperature.iMax, overdriveCapabilities.fanTemperature.iStep);									
					PRINTF("odNFanControl.iTargetTemperature : %d\n" , odNFanControl.iTargetTemperature);
					
					PRINTF("---------Fan Current Speed--------------\n");
					PRINTF("odNFanControl.iCurrentFanSpeed : %d\n" , odNFanControl.iCurrentFanSpeed);
					PRINTF("odNFanControl.iCurrentFanSpeedMode : %d\n" , odNFanControl.iCurrentFanSpeedMode);
					PRINTF("-----------------------------------------\n");					
					
				}
				break;
			}			
		}						
	}

	
	return 0;
}

int setODNFANParameters(int feature, int value)
{
	int  i, active = 0;;
	int iSupported,iEnabled,iVersion;
	

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}
				ADLODNFanControl odNFanControl;
				memset(&odNFanControl, 0, sizeof(ADLODNFanControl));
				
				if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
				{
					PRINTF("ADL2_OverdriveN_FanControl_Get is failed\n");
				}
				else
				{
					if (feature == 1 || feature == 2)
					{
						if (!(overdriveCapabilities.fanSpeed.iMin <= value && overdriveCapabilities.fanSpeed.iMax >= value))
						{
							PRINTF("Min & Target Fan speed range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.fanSpeed.iMin,overdriveCapabilities.fanSpeed.iMax, overdriveCapabilities.fanSpeed.iStep);										
						}	
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNFANParameters();
							odNFanControl.iMode = ADLODNControlType::ODNControlType_Manual;
							if (feature == 1) 
								odNFanControl.iMinFanLimit = value;
							else
								odNFanControl.iTargetFanSpeed = value;

							if (ADL_OK != ADL2_OverdriveN_FanControl_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
							{
								PRINTF("ADL2_OverdriveN_FanControl_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_FanControl_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNFANParameters();
							}							
						}
					}
					else if (feature == 3)
					{
						if (!(overdriveCapabilities.minimumPerformanceClock.iMin <= value && overdriveCapabilities.minimumPerformanceClock.iMax >= value))
						{
							PRINTF("Minimum performance range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.minimumPerformanceClock.iMin,overdriveCapabilities.minimumPerformanceClock.iMax, overdriveCapabilities.minimumPerformanceClock.iStep);			
						}	
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNFANParameters();
							odNFanControl.iMode = ADLODNControlType::ODNControlType_Manual;
							odNFanControl.iMinPerformanceClock = value;
							if (ADL_OK != ADL2_OverdriveN_FanControl_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
							{
								PRINTF("ADL2_OverdriveN_FanControl_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_FanControl_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNFANParameters();
							}
						}
					}	
					else if (feature == 4)
					{
						if (!(overdriveCapabilities.fanTemperature.iMin <= value && overdriveCapabilities.fanTemperature.iMax >= value))
						{
							PRINTF("fanTemperature range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.fanTemperature.iMin,overdriveCapabilities.fanTemperature.iMax, overdriveCapabilities.fanTemperature.iStep);			
						}	
						else
						{
							PRINTF ("***** Driver Values: Before Apply ******\n");
							printODNFANParameters();							
							odNFanControl.iMode = ADLODNControlType::ODNControlType_Manual;
							odNFanControl.iTargetTemperature = value;
							if (ADL_OK != ADL2_OverdriveN_FanControl_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
							{
								PRINTF("ADL2_OverdriveN_FanControl_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_FanControl_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNFANParameters();
							}							
						}
					}	
				}
				break;
			}		
		}						
	}

	
	
	return 0;
}

int printODNTEMPParameters()
{
	
	int  i, active = 0;;
	int iSupported,iEnabled,iVersion;
	int ret = 0;
	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}

				ADLODNPowerLimitSetting odNPowerControl;
				memset(&odNPowerControl, 0, sizeof(ADLODNPowerLimitSetting));
				
				if (ADL_OK != ADL2_OverdriveN_PowerLimit_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPowerControl))
				{
					PRINTF("ADL2_OverdriveN_PowerLimit_Get is failed\n");
				}
				else
				{
					int temp;
					ADL2_OverdriveN_Temperature_Get(context,lpAdapterInfo[ i ].iAdapterIndex,1, &temp);
					PRINTF("-------------------------------------------------\n");					
					PRINTF("ADL2_OverdriveN_PowerLimit_Get Data\n");
					PRINTF("-------------------------------------------------\n");
					PRINTF("odNPowerControl.iMode : %d\n" , odNPowerControl.iMode);
					PRINTF("Current temperature : %d\n", temp);
					PRINTF ("------------------   POWER   --------------------\n");
					PRINTF("Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.powerTuneTemperature.iMin,overdriveCapabilities.powerTuneTemperature.iMax, overdriveCapabilities.powerTuneTemperature.iStep);														
					PRINTF("odNPowerControl.iMaxOperatingTemperature : %d\n" , odNPowerControl.iMaxOperatingTemperature);
					PRINTF ("----------------- TDP LIMITS --------------------\n");
					PRINTF("Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.power.iMin,overdriveCapabilities.power.iMax, overdriveCapabilities.power.iStep);														
					PRINTF("odNPowerControl.iTDPLimit : %d\n" , odNPowerControl.iTDPLimit);
					PRINTF("-------------------------------------------------\n");					
					
				}
				break;
			}			
		}
	}

	return ret;
}

int setODNTEMPParameters(int feature, int value)
{

	int  i, active = 0;;
	int iSupported,iEnabled,iVersion;	
	int ret = 0;

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}
				ADLODNPowerLimitSetting odNPowerControl;
				memset(&odNPowerControl, 0, sizeof(ADLODNPowerLimitSetting));
				
				if (ADL_OK != ADL2_OverdriveN_PowerLimit_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPowerControl))
				{
					PRINTF("ADL2_OverdriveN_PowerLimit_Get is failed\n");
				}
				else
				{
					if (feature == 1)
					{
						if (!(overdriveCapabilities.powerTuneTemperature.iMin <= value && overdriveCapabilities.powerTuneTemperature.iMax >= value))
						{
							PRINTF("Maximum OP Temp range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.powerTuneTemperature.iMin,overdriveCapabilities.powerTuneTemperature.iMax, overdriveCapabilities.powerTuneTemperature.iStep);			
						}	
						else
						{
							PRINTF ("****** Driver Values: Before Apply ******\n");
							printODNTEMPParameters();
							odNPowerControl.iMode = ADLODNControlType::ODNControlType_Manual;
							odNPowerControl.iMaxOperatingTemperature = value;
							if (ADL_OK != ADL2_OverdriveN_PowerLimit_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPowerControl))
							{
								PRINTF("ADL2_OverdriveN_PowerLimit_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_PowerLimit_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNTEMPParameters();
							}
						}
					}
					else if (feature == 2)
					{
						if (!(overdriveCapabilities.power.iMin <= value && overdriveCapabilities.power.iMax >= value))
						{
							PRINTF("TDP Limit range should be in Min : %d, Max : %d, Step : %d\n" ,overdriveCapabilities.power.iMin,overdriveCapabilities.power.iMax, overdriveCapabilities.power.iStep);			
						}	
						else
						{
							PRINTF ("****** Driver Values: Before Apply ******\n");
							printODNTEMPParameters();
							odNPowerControl.iMode = ADLODNControlType::ODNControlType_Manual;
							odNPowerControl.iTDPLimit = value;
							if (ADL_OK != ADL2_OverdriveN_PowerLimit_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPowerControl))
							{
								PRINTF("ADL2_OverdriveN_PowerLimit_Set is failed\n");
							}
							else
							{
								PRINTF("ADL2_OverdriveN_PowerLimit_Set is Success\n\n");
								PRINTF ("****** Driver Values: After Apply ******\n");
								printODNTEMPParameters();
							}
						}
					}				
				}
				break;
			}		
		}
	}
	return ret;
	
}

int printODNActivity()
{
	
	int   i, active = 0;;
	int iSupported,iEnabled,iVersion;
	int ret = 0;

	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}

				ADLODNPerformanceStatus odNPerformanceStatus;
				memset(&odNPerformanceStatus, 0, sizeof(ADLODNPerformanceStatus));
				
				if (ADL_OK != ADL2_OverdriveN_PerformanceStatus_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPerformanceStatus))
				{
					PRINTF("ADL2_OverdriveN_PerformanceStatus_Get is failed\n");
				}
				else
				{
					PRINTF("-------------------------------------------------\n");					
					PRINTF("ADL2_OverdriveN_PerformanceStatus_Get Data\n");
					PRINTF("-------------------------------------------------\n");
					PRINTF("odNPerformanceStatus.iCoreClock : %d\n" , odNPerformanceStatus.iCoreClock);
					PRINTF("odNPerformanceStatus.iCurrentBusLanes : %d\n" , odNPerformanceStatus.iCurrentBusLanes);
					PRINTF("odNPerformanceStatus.iCurrentBusSpeed : %d\n" , odNPerformanceStatus.iCurrentBusSpeed);
					PRINTF("odNPerformanceStatus.iCurrentCorePerformanceLevel : %d\n" , odNPerformanceStatus.iCurrentCorePerformanceLevel);
					PRINTF("odNPerformanceStatus.iCurrentDCEFPerformanceLevel : %d\n" , odNPerformanceStatus.iCurrentDCEFPerformanceLevel);
					PRINTF("odNPerformanceStatus.iCurrentGFXPerformanceLevel : %d\n" , odNPerformanceStatus.iCurrentGFXPerformanceLevel);
					PRINTF("odNPerformanceStatus.iCurrentMemoryPerformanceLevel : %d\n" , odNPerformanceStatus.iCurrentMemoryPerformanceLevel);
					PRINTF("odNPerformanceStatus.iDCEFClock : %d\n" , odNPerformanceStatus.iDCEFClock);
					PRINTF("odNPerformanceStatus.iGFXClock : %d\n" , odNPerformanceStatus.iGFXClock);
					PRINTF("odNPerformanceStatus.iGPUActivityPercent : %d\n" , odNPerformanceStatus.iGPUActivityPercent);
					PRINTF("odNPerformanceStatus.iMaximumBusLanes : %d\n" , odNPerformanceStatus.iMaximumBusLanes);
					PRINTF("odNPerformanceStatus.iMemoryClock : %d\n" , odNPerformanceStatus.iMemoryClock);
					PRINTF("odNPerformanceStatus.iUVDClock : %d\n" , odNPerformanceStatus.iUVDClock);
					PRINTF("odNPerformanceStatus.iUVDPerformanceLevel : %d\n" , odNPerformanceStatus.iUVDPerformanceLevel);
					PRINTF("odNPerformanceStatus.iVCEClock : %d\n" , odNPerformanceStatus.iVCEClock);
					PRINTF("odNPerformanceStatus.iVCEPerformanceLevel : %d\n" , odNPerformanceStatus.iVCEPerformanceLevel);
					PRINTF("odNPerformanceStatus.iVDDC : %d\n" , odNPerformanceStatus.iVDDC);
					PRINTF("odNPerformanceStatus.iVDDCI : %d\n" , odNPerformanceStatus.iVDDCI);					
					PRINTF("-------------------------------------------------\n");					
					
				}
				break;
			}			
		}
	}

	return ret;
}

int resetODSettings()
{
	
	int i, active = 0;;
	int iSupported,iEnabled,iVersion;
	int ret = 0;
	// Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[ i ].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context,lpAdapterInfo[ i ].iAdapterIndex, &iSupported, &iEnabled, &iVersion);
			if (iVersion == 7)
			{
				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));
    
				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &overdriveCapabilities))
				{
					PRINTF("ADL2_OverdriveN_Capabilities_Get is failed\n");				
				}

				//performance levels info
				ADLODNPerformanceLevels *odPerformanceLevels;

				int size = sizeof(ADLODNPerformanceLevels) + sizeof(ADLODNPerformanceLevel)* (overdriveCapabilities.iMaximumNumberOfPerformanceLevels - 1);
				void* performanceLevelsBuffer = new char[size];
				memset(performanceLevelsBuffer, 0, size);
				odPerformanceLevels = (ADLODNPerformanceLevels*)performanceLevelsBuffer;
				odPerformanceLevels->iSize = size;
				odPerformanceLevels->iNumberOfPerformanceLevels = overdriveCapabilities.iMaximumNumberOfPerformanceLevels;
				//GPU clocks reset
				if (ADL_OK != ADL2_OverdriveN_SystemClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
				{
					PRINTF("ADL2_OverdriveN_GPUClocks_Get is failed\n");
				}
				else
				{
					odPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Default;
					if (ADL_OK != ADL2_OverdriveN_SystemClocks_Set(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
					{
						PRINTF("ADL2_OverdriveN_SystemClocks_Set is failed\n");
					}
					printODNSCLKParameters();
				}

				//Mem clocks reset
				if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Get(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
				{
					PRINTF("ADL2_OverdriveN_MemoryClocks_Get is failed\n");
				}
				else
				{
					odPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Default;
					if (ADL_OK != ADL2_OverdriveN_MemoryClocks_Set(context, lpAdapterInfo[ i ].iAdapterIndex, odPerformanceLevels))
					{
						PRINTF("ADL2_OverdriveN_MemoryClocks_Set is failed\n");
					}
					printODNMCLKParameters();
				}

				//Fan Reset
				ADLODNFanControl odNFanControl;
				memset(&odNFanControl, 0, sizeof(ADLODNFanControl));
				
				if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
				{
					PRINTF("ADL2_OverdriveN_FanControl_Get is failed\n");
				}
				else
				{
					odNFanControl.iMode = ADLODNControlType::ODNControlType_Auto;
					if (ADL_OK != ADL2_OverdriveN_FanControl_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNFanControl))
					{
						PRINTF("ADL2_OverdriveN_FanControl_Set is failed\n");
					}
					printODNFANParameters();
				}

				//Temp Reset
				ADLODNPowerLimitSetting odNPowerControl;
				memset(&odNPowerControl, 0, sizeof(ADLODNPowerLimitSetting));
				
				if (ADL_OK != ADL2_OverdriveN_PowerLimit_Get(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPowerControl))
				{
					PRINTF("ADL2_OverdriveN_PowerLimit_Get is failed\n");
				}
				else
				{
					odNPowerControl.iMode = ADLODNControlType::ODNControlType_Auto;
					if (ADL_OK != ADL2_OverdriveN_PowerLimit_Set(context,lpAdapterInfo[ i ].iAdapterIndex, &odNPowerControl))
					{
						PRINTF("ADL2_OverdriveN_PowerLimit_Set is failed\n");
					}
					printODNTEMPParameters();
				}
				break;
			}		
		}
	}

	return ret;
}

int main(int argc, char* argv[])
{
//	Sleep(30000);

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
				case 'g':
					if (argc == 2)
						printODNSCLKParameters();
					else if (argc == 5)
						setODNSCLKParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
					else 
						printf ("signature of SClock Set (ODNEXT.exe g X Y Z); X - Level, Y - (1: Clock, 2: Voltage), Z - Expected value should be in min max range");
					break;				
				case 'm':
					if (argc == 2)
						printODNMCLKParameters();
					else if (argc == 5)
						setODNMCLKParameters(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
					else 
						printf ("signature of Memory Set (ODNEXT.exe m X Y Z); X - Level, Y - (1: Clock, 2: Voltage), Z - Expected value should be in min max range");
					break;
					break;
				case 'f':
					if (argc == 2)
						printODNFANParameters();
					else if (argc == 4)
						setODNFANParameters(atoi(argv[2]), atoi(argv[3]));
					else 
						printf ("signature of Fan Set (ODNEXT.exe f X Y); X - (1: Minimum Fan Speed, 2: Target Fan Speed, 3: Minimum Performance, 4: Target Temp), Y - Expected value should be in min max range");
					break;
				case 't':
					if (argc == 2)
						printODNTEMPParameters();
					else if (argc == 4)
						setODNTEMPParameters(atoi(argv[2]), atoi(argv[3]));
					else 
						printf ("signature of Tempareture Set (ODNEXT.exe t X Y); X - (1: Maximum OP Temperature, 2: TDP LIMIT), Y - Expected value should be in min max range");
					break;
				case 'a':
					printODNActivity();
					break;
				case 'r':
					resetODSettings();
					break;				
				default:
					printf ("Available command line parameters: f- Fan, g-GPU clocks, t-temp, m-Memory Clocks, a- Activity, r-reset \n ");
					printf ("SClocks Get :  ODNEXT.exe g\n");
					printf ("SClocks Set :  ODNEXT.exe g X Y Z; X - Level, Y - (1: Clock, 2: Voltage), Z - Expected value should be in min max range \n");
					printf ("MClocks Get :  ODNEXT.exe m\n");
					printf ("MClocks Set :  ODNEXT.exe m X Y Z; X - Level, Y - (1: Clock, 2: Voltage), Z - Expected value should be in min max range\n");
					printf ("FAN Get :  ODNEXT.exe f\n");
					printf ("FAN Set :  ODNEXT.exe f X Y; X - (1: Minimum Fan Speed, 2: Target Fan Speed, 3: Minimum Performance, 4: Target Temp), Y - Expected value should be in min max range\n");
					printf ("Temp Get :  ODNEXT.exe t\n");
					printf ("Temp Set :  ODNEXT.exe t X Y ; X - (1: Maximum OP Temperature, 2: TDP LIMIT), Y - Expected value should be in min max range\n");
					printf ("Activity Get :  ODNEXT.exe a\n");
					printf ("OD Reset :  ODNEXT.exe r\n");
					break;
			}



		}
		else
		{
					printf ("Available command line parameters: f- Fan, g-GPU clocks, t-temp, m-Memory Clocks, a- Activity, r-reset \n");
					printf ("SClocks Get :  ODNEXT.exe g\n");
					printf ("SClocks Set :  ODNEXT.exe g X Y Z; X - Level, Y - (1: Clock, 2: Voltage), Z - Expected value should be in min max range \n");
					printf ("MClocks Get :  ODNEXT.exe m\n");
					printf ("MClocks Set :  ODNEXT.exe m X Y Z; X - Level, Y - (1: Clock, 2: Voltage), Z - Expected value should be in min max range\n");
					printf ("FAN Get :  ODNEXT.exe f\n");
					printf ("FAN Set :  ODNEXT.exe f X Y; X - (1: Minimum Fan Speed, 2: Target Fan Speed, 3: Minimum Performance, 4: Target Temp), Y - Expected value should be in min max range\n");
					printf ("Temp Get :  ODNEXT.exe t\n");
					printf ("Temp Set :  ODNEXT.exe t X Y ; X - (1: Maximum OP Temperature, 2: TDP LIMIT), Y - Expected value should be in min max range\n");
					printf ("Activity Get :  ODNEXT.exe a\n");
					printf ("OD Reset :  ODNEXT.exe r\n");
		}


		ADL_Main_Memory_Free((void**)&lpAdapterInfo);

		deinitializeADL();
	}


	return 0;
}
