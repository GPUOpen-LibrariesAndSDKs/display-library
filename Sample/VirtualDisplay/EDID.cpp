///
///  Copyright (c) 2008 - 2022 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file EDID.c

#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include "EDID.h"
#include <stdio.h>

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL2_MAIN_CONTROL_CREATE) (ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY) (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_DISPLAY_POSSIBLEMODE_GET) (ADL_CONTEXT_HANDLE, int, int*, ADLMode**);
typedef int(*ADL2_DISPLAY_MODES_GET) (ADL_CONTEXT_HANDLE, int, int, int*, ADLMode**);
typedef int(*ADL2_DISPLAY_MODES_SET) (ADL_CONTEXT_HANDLE, int, int, int, ADLMode*);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET) (ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET) (ADL_CONTEXT_HANDLE, LPAdapterInfo, int);
typedef int(*ADL2_ADAPTERX2_CAPS) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_ADAPTER_EDIDMANAGEMENT_CAPS)(ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_ADAPTER_CONNECTIONSTATE_GET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, ADLConnectionState*);
typedef int(*ADL2_ADAPTER_SUPPORTEDCONNECTIONS_GET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, ADLSupportedConnections*);
typedef int(*ADL2_ADAPTER_CONNECTIONDATA_SET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, ADLConnectionData);
typedef int(*ADL2_ADAPTER_CONNECTIONDATA_GET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, int, ADLConnectionData*);
typedef int(*ADL2_ADAPTER_CONNECTIONDATA_REMOVE) (ADL_CONTEXT_HANDLE, int, ADLDevicePort);
typedef int(*ADL2_ADAPTER_EMULATIONMODE_SET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, int);
typedef int(*ADL2_ADAPTER_BOARDLAYOUT_GET) (ADL_CONTEXT_HANDLE, int, int*, int*, ADLBracketSlotInfo**, int*, ADLConnectorInfo**);
typedef int(*ADL2_DISPLAY_MODETIMINGOVERRIDEX2_GET) (ADL_CONTEXT_HANDLE, int, ADLDisplayID, ADLDisplayModeX2*, ADLDisplayModeInfo*);
typedef int(*ADL2_DISPLAY_MODETIMINGOVERRIDE_SET) (ADL_CONTEXT_HANDLE, int, int, ADLDisplayModeInfo*, int);
typedef int(*ADL2_ADAPTER_MODETIMINGOVERRIDE_CAPS) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_DISPLAY_MODETIMINGOVERRIDE_DELETE) (ADL_CONTEXT_HANDLE, int, ADLDisplayID, ADLDisplayModeX2*, int);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);
typedef int(*ADL2_DISPLAY_DISPLAYINFO_GET) (ADL_CONTEXT_HANDLE, int, int*, ADLDisplayInfo **, int);
typedef int(*ADL2_WORKSTATION_DISPLAYGLSYNCMODE_GET) (ADL_CONTEXT_HANDLE, int, int, ADLGlSyncMode* );

HINSTANCE hADLDll;

ADL2_MAIN_CONTROL_CREATE                 ADL2_Main_Control_Create = NULL;
ADL2_MAIN_CONTROL_DESTROY                ADL2_Main_Control_Destroy = NULL;
ADL2_DISPLAY_POSSIBLEMODE_GET            ADL2_Display_PossibleMode_Get = NULL;
ADL2_DISPLAY_MODES_SET                   ADL2_Display_Modes_Set = NULL;
ADL2_DISPLAY_MODES_GET                   ADL2_Display_Modes_Get = NULL;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET        ADL2_Adapter_NumberOfAdapters_Get = NULL;
ADL2_ADAPTER_ADAPTERINFO_GET             ADL2_Adapter_AdapterInfo_Get = NULL;
ADL2_ADAPTERX2_CAPS                      ADL2_AdapterX2_Caps = NULL;
ADL2_ADAPTER_EDIDMANAGEMENT_CAPS		 ADL2_Adapter_EDIDManagement_Caps = NULL;
ADL2_ADAPTER_CONNECTIONSTATE_GET         ADL2_Adapter_ConnectionState_Get = NULL;
ADL2_ADAPTER_SUPPORTEDCONNECTIONS_GET    ADL2_Adapter_SupportedConnections_Get = NULL;
ADL2_ADAPTER_CONNECTIONDATA_SET          ADL2_Adapter_ConnectionData_Set = NULL;
ADL2_ADAPTER_CONNECTIONDATA_GET          ADL2_Adapter_ConnectionData_Get = NULL;
ADL2_ADAPTER_CONNECTIONDATA_REMOVE       ADL2_Adapter_ConnectionData_Remove = NULL;
ADL2_ADAPTER_EMULATIONMODE_SET           ADL2_Adapter_EmulationMode_Set = NULL;
ADL2_DISPLAY_MODETIMINGOVERRIDEX2_GET    ADL2_Display_ModeTimingOverrideX2_Get = NULL;
ADL2_DISPLAY_MODETIMINGOVERRIDE_SET      ADL2_Display_ModeTimingOverride_Set = NULL;
ADL2_ADAPTER_MODETIMINGOVERRIDE_CAPS     ADL2_Adapter_ModeTimingOverride_Caps = NULL;
ADL2_DISPLAY_MODETIMINGOVERRIDE_DELETE   ADL2_Display_ModeTimingOverride_Delete = NULL;
ADL2_ADAPTER_BOARDLAYOUT_GET			 ADL2_Adapter_BoardLayout_Get = NULL;
ADL2_ADAPTER_ACTIVE_GET				     ADL2_Adapter_Active_Get = NULL;
ADL2_DISPLAY_DISPLAYINFO_GET             ADL2_Display_DisplayInfo_Get = NULL;
ADL2_WORKSTATION_DISPLAYGLSYNCMODE_GET   ADL2_Workstation_DisplayGLSyncMode_Get = NULL;
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

ADL_CONTEXT_HANDLE g_adlContext = NULL;

int initializeADL()
{
	
	// Load the ADL dll
	{
		hADLDll = LoadLibrary(TEXT("atiadlxx.dll"));
		if (hADLDll == NULL)
		{
			// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
			// Try to load the 32 bit library (atiadlxy.dll) instead
			hADLDll = LoadLibrary(TEXT("atiadlxy.dll"));
		}

		if (NULL == hADLDll)
		{
			PRINTF("Failed to load ADL library\n");
			return FALSE;
		}
	}
	{
		ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE)GetProcAddress(hADLDll, "ADL2_Main_Control_Create");
		ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hADLDll, "ADL2_Main_Control_Destroy");
		ADL2_Display_PossibleMode_Get = (ADL2_DISPLAY_POSSIBLEMODE_GET)GetProcAddress(hADLDll, "ADL2_Display_PossibleMode_Get");
		ADL2_Display_Modes_Get = (ADL2_DISPLAY_MODES_GET)GetProcAddress(hADLDll, "ADL2_Display_Modes_Get");
		ADL2_Display_Modes_Set = (ADL2_DISPLAY_MODES_SET)GetProcAddress(hADLDll, "ADL2_Display_Modes_Set");
		ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hADLDll, "ADL2_Adapter_NumberOfAdapters_Get");
		ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hADLDll, "ADL2_Adapter_AdapterInfo_Get");
		ADL2_AdapterX2_Caps = (ADL2_ADAPTERX2_CAPS)GetProcAddress(hADLDll, "ADL2_AdapterX2_Caps");
		ADL2_Adapter_ConnectionState_Get = (ADL2_ADAPTER_CONNECTIONSTATE_GET)GetProcAddress(hADLDll, "ADL2_Adapter_ConnectionState_Get");
		ADL2_Adapter_SupportedConnections_Get = (ADL2_ADAPTER_SUPPORTEDCONNECTIONS_GET)GetProcAddress(hADLDll, "ADL2_Adapter_SupportedConnections_Get");
		ADL2_Adapter_ConnectionData_Set = (ADL2_ADAPTER_CONNECTIONDATA_SET)GetProcAddress(hADLDll, "ADL2_Adapter_ConnectionData_Set");
		ADL2_Adapter_ConnectionData_Get = (ADL2_ADAPTER_CONNECTIONDATA_GET)GetProcAddress(hADLDll, "ADL2_Adapter_ConnectionData_Get");
		ADL2_Adapter_EmulationMode_Set = (ADL2_ADAPTER_EMULATIONMODE_SET)GetProcAddress(hADLDll, "ADL2_Adapter_EmulationMode_Set");
		ADL2_Adapter_ConnectionData_Remove = (ADL2_ADAPTER_CONNECTIONDATA_REMOVE)GetProcAddress(hADLDll, "ADL2_Adapter_ConnectionData_Remove");
		ADL2_Display_ModeTimingOverrideX2_Get = (ADL2_DISPLAY_MODETIMINGOVERRIDEX2_GET)GetProcAddress(hADLDll, "ADL2_Display_ModeTimingOverrideX2_Get");
		ADL2_Display_ModeTimingOverride_Set = (ADL2_DISPLAY_MODETIMINGOVERRIDE_SET)GetProcAddress(hADLDll, "ADL2_Display_ModeTimingOverride_Set");
		ADL2_Adapter_ModeTimingOverride_Caps = (ADL2_ADAPTER_MODETIMINGOVERRIDE_CAPS)GetProcAddress(hADLDll, "ADL2_Adapter_ModeTimingOverride_Caps");
		ADL2_Display_ModeTimingOverride_Delete = (ADL2_DISPLAY_MODETIMINGOVERRIDE_DELETE)GetProcAddress(hADLDll, "ADL2_Display_ModeTimingOverride_Delete");
		ADL2_Adapter_BoardLayout_Get = (ADL2_ADAPTER_BOARDLAYOUT_GET)GetProcAddress(hADLDll, "ADL2_Adapter_BoardLayout_Get");
		ADL2_Adapter_EDIDManagement_Caps = (ADL2_ADAPTER_EDIDMANAGEMENT_CAPS)GetProcAddress(hADLDll, "ADL2_Adapter_EDIDManagement_Caps");
        ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hADLDll, "ADL2_Adapter_Active_Get");
        ADL2_Display_DisplayInfo_Get = (ADL2_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hADLDll, "ADL2_Display_DisplayInfo_Get");
        ADL2_Workstation_DisplayGLSyncMode_Get = (ADL2_WORKSTATION_DISPLAYGLSYNCMODE_GET)GetProcAddress(hADLDll, "ADL2_Workstation_DisplayGLSyncMode_Get");
        
		if (NULL == ADL2_Main_Control_Create ||
			NULL == ADL2_Main_Control_Destroy ||
			NULL == ADL2_Display_PossibleMode_Get ||
			NULL == ADL2_Display_Modes_Get ||
			NULL == ADL2_Display_Modes_Set ||
			NULL == ADL2_Adapter_NumberOfAdapters_Get ||
			NULL == ADL2_Adapter_AdapterInfo_Get ||
			NULL == ADL2_AdapterX2_Caps ||
			NULL == ADL2_Adapter_ConnectionState_Get ||
			NULL == ADL2_Adapter_SupportedConnections_Get ||
			NULL == ADL2_Adapter_ConnectionData_Set ||
			NULL == ADL2_Adapter_ConnectionData_Remove ||
			NULL == ADL2_Adapter_EmulationMode_Set ||
			NULL == ADL2_Display_ModeTimingOverrideX2_Get ||
			NULL == ADL2_Display_ModeTimingOverride_Set ||
			NULL == ADL2_Adapter_ModeTimingOverride_Caps ||
			NULL == ADL2_Display_ModeTimingOverride_Delete ||
			NULL == ADL2_Adapter_BoardLayout_Get ||
            NULL == ADL2_Display_DisplayInfo_Get ||
            NULL == ADL2_Workstation_DisplayGLSyncMode_Get)
		{
			PRINTF("Failed to get ADL function pointers\n");
			return FALSE;
		}
	}
	if (ADL_OK != ADL2_Main_Control_Create(ADL_Main_Memory_Alloc, 1, &g_adlContext))
	{
		PRINTF("ADL_Main_Control_Create() failed\n");
		return FALSE;
	}
	
	return TRUE;
}	

void deinitializeADL()
{
    FreeLibrary(hADLDll);
}

int printAdapaterInfo()
{
	int  iNumberAdapters, iNumberofPorts = 0;
	int iAdapterIndex =0, i=0, j=0, k=0,iGPUIndex=0, iGPUfound=0, iBusNumber,iGPUCounter = 0, slotIndex=-1;
	int iEDIDSupported = 0;
	int igpuBusIndexes[4];
	int iValidFlags,iNumofSlots, iNumofConnectors;
	ADLBracketSlotInfo* barcketSlotInfo;
	ADLConnectorInfo* connectorInfo;

	LPAdapterInfo     lpAdapterInfo = NULL;
	// Obtain the number of adapters for the system
    if ( ADL_OK != ADL2_Adapter_NumberOfAdapters_Get (g_adlContext, &iNumberAdapters ) )
	{
	       PRINTF("Cannot get the number of adapters!\n");
		   return 0;
	}

    if ( 0 < iNumberAdapters )
    {
        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        ADL2_Adapter_AdapterInfo_Get (g_adlContext, lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
    }


	for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
	{
		igpuBusIndexes[iGPUIndex] = -1;
	}

	PRINTF (" ASIC information :\n");
	PRINTF (" --------------------------------------------------------- \n");
    // Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		iGPUfound = 0;
		iBusNumber = lpAdapterInfo[ i ].iBusNumber;
		for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
		{
			if(igpuBusIndexes[iGPUIndex] != -1 && igpuBusIndexes[iGPUIndex] == iBusNumber)
			{
				iGPUfound = 1;
				break;
			}
		}

		if (!iGPUfound)
		{
			PRINTF (" GPU #%d Information\n", iGPUCounter);
			PRINTF (" ----------------------------- \n");
			for (j = 0; j < iNumberAdapters; j++)
			{

				if (iBusNumber == lpAdapterInfo[j].iBusNumber)
				{
					iAdapterIndex = lpAdapterInfo[j].iAdapterIndex;
					int active;
					ADL2_Adapter_Active_Get(g_adlContext, iAdapterIndex, &active);
					PRINTF(" Adapter Index: #%d Active:%d\n", iAdapterIndex, active);
					if (active)
					{
						if (ADL_OK != ADL2_Adapter_EDIDManagement_Caps(g_adlContext, iAdapterIndex, &iEDIDSupported))
						{
							PRINTF(" ADL_Adapter_EDIDManagement_Caps failed \n");
						}
						else
						{
							if (iEDIDSupported == 1)
							{
								ADLAdapterCapsX2 temp;
								memset(&temp, 0, sizeof(temp));

								if (ADL_OK != ADL2_AdapterX2_Caps(g_adlContext, iAdapterIndex, reinterpret_cast<int*>(&temp)))
								{
									PRINTF(" ADL_AdapterX2_Caps Failed \n");
								}
								else
								{
									PRINTF(" EDID Management Feature Supported \n");
									PRINTF(" Adapter Index : %d \n", iAdapterIndex);
									if (temp.iNumConnectors > 0)
									{
										PRINTF(" No of Ports : %d \n", temp.iNumConnectors);

										ADL2_Adapter_BoardLayout_Get(g_adlContext, iAdapterIndex, &iValidFlags, &iNumofSlots, &barcketSlotInfo, &iNumofConnectors, &connectorInfo);

										if (iValidFlags != 15)

											PRINTF(" Your VBIOS is older and connector location metioned in below is may not same as ASIC physical layout ");

										PRINTF(" Details of Ports  \n");
										for (k = 0; k < iNumofConnectors; k++)
										{
											if (slotIndex != connectorInfo[k].iSlotIndex)
											{
												slotIndex = connectorInfo[k].iSlotIndex;
												PRINTF(" Slot Number : %d\n", slotIndex);
											}
											PRINTF(" \tConnector Index : %d\t", connectorInfo[k].iConnectorIndex);
											switch (connectorInfo[k].iType)
											{
											case 1:
												PRINTF(" Connector Type : VGA\n");
												break;
											case 2:
												PRINTF(" Connector Type : DVI_I\n");
												break;
											case 3:
												PRINTF(" Connector Type : DVI_D\n");
												break;
											case 8:
												PRINTF(" Connector Type : HDMI A\n");
												break;
											case 9:
												PRINTF(" Connector Type : HDMI B\n");
												break;
											case 10:
												PRINTF(" Connector Type : DP\n");
												break;
											case 12:
												PRINTF(" Connector Type : Mini DP\n");
												break;
											case 13:
												PRINTF(" Connector Type : Virtual\n");
												break;
											default:
												PRINTF(" Connector Type : UNKNOWN\n");
												break;
											}
										}



									}
									else
										PRINTF(" No Ports are allowed to emulate \n");

								}
							}
							else
							{
								PRINTF(" EDID Management Feature Not Supported on Adapter #%d\n", iAdapterIndex);
							}


						}
						break;
					}
				}
			}
			PRINTF("\n \n");
			igpuBusIndexes[iGPUCounter++] = iBusNumber;

		}
	}


	

	
	PRINTF (" --------------------------------------------------------- \n");
	PRINTF ("* if adapter index -1 means, Disabled Display\n");
	PRINTF (" --------------------------------------------------------- \n");
	return TRUE;
}

void getEmulationStatus(int iAdapterIndex)
{
	ADLAdapterCapsX2 temp;
	ADLDevicePort devicePort;
	ADLConnectionState connectionState;
	ADLConnectionData connectionData;
	ADLSupportedConnections supportedConnections;
	int i=0,j=0;
	if (ADL_OK == ADL2_AdapterX2_Caps(g_adlContext, iAdapterIndex, reinterpret_cast<int*>(&temp)))
	{
		for (i=0; i<temp.iNumConnectors; i++)
		{
			devicePort.iConnectorIndex = i;
			devicePort.aMSTRad.iLinkNumber = 1;
			devicePort.aMSTRad.rad[0] = 0; 

			PRINTF ("\n --------------------------------------------------------- \n");
			PRINTF (" Connector #%d Information :\n" , i);
			PRINTF (" --------------------------------------------------------- \n");
				
			#pragma region ADL_Adapter_SupportedConnections_Get
			if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(g_adlContext, iAdapterIndex, devicePort, &supportedConnections))
			{
				PRINTF (" Connections     | BItRate | No.OfLanes | 3D Caps | OutputBwidth | ColorDepth\n");
				PRINTF (" ______________________________________________________________________________ \n");
				for (j=0; j<14; j++)
				{
					if ((supportedConnections.iSupportedConnections & (1 <<j)) == (1 <<j))
					{
						switch (j)
						{
						case ADL_CONNECTION_TYPE_VGA:
							PRINTF (" VGA");
							break;
						case ADL_CONNECTION_TYPE_DVI:
							PRINTF (" DVI");
							break;
						case ADL_CONNECTION_TYPE_DVI_SL:
							PRINTF (" DVI_SL");
							break;
						case ADL_CONNECTION_TYPE_HDMI:
							PRINTF (" HDMI");
							break;
						case ADL_CONNECTION_TYPE_DISPLAY_PORT:
							PRINTF (" Display port\t\t");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_DVI_SL:
							PRINTF (" DP to DVI SL (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_DVI_DL:
							PRINTF (" DP to DVI DL (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_HDMI:
							PRINTF (" DP to HDMI (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_VGA:
							PRINTF (" DP to VGA (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_HDMI:
							PRINTF (" DP to HDMI (Passive Dongle)");
							break;
						case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_DVI:
							PRINTF (" DP to DVI (Passive Dongle)");
							break;
						case ADL_CONNECTION_TYPE_MST:
							PRINTF (" Display Branch\t\t");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE:
							PRINTF (" DP (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_VIRTUAL:
							PRINTF (" VIRTUAL");
							break;
						}

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_BITRATE) == ADL_CONNECTION_PROPERTY_BITRATE)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES) == ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_3DCAPS) == ADL_CONNECTION_PROPERTY_3DCAPS)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH) == ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_COLORDEPTH) == ADL_CONNECTION_PROPERTY_COLORDEPTH)
							PRINTF ("\t Y\n");
						else
							PRINTF ("\t N\n");
					}
				}
			}  
			#pragma endregion
			
			#pragma region ADL_Adapter_ConnectionState_Get
			PRINTF ("\n Connection Information:\n");
			PRINTF (" -----------------------\n");
				
			if (ADL_OK == ADL2_Adapter_ConnectionState_Get(g_adlContext, iAdapterIndex,devicePort, &connectionState))
			{
				if (connectionState.iDisplayIndex != -1)
				{
					if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_REAL_DEVICE_CONNECTED) == ADL_EMUL_STATUS_REAL_DEVICE_CONNECTED)
					{
						PRINTF (" Display Status : Real Display Connected\n");
					}
					else if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_EMULATED_DEVICE_USED) == ADL_EMUL_STATUS_EMULATED_DEVICE_USED)
					{
						PRINTF (" Display Status : Emulated Display is Used\n");
					}
					else if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_LAST_ACTIVE_DEVICE_USED) == ADL_EMUL_STATUS_LAST_ACTIVE_DEVICE_USED)
					{
						PRINTF (" Display Status : Last Active Display is Used\n");
					}
					PRINTF (" Display Index : %d\n", connectionState.iDisplayIndex);					
				}
				else
				{
					PRINTF (" Display Status : No Display Connected\n");
				}
				switch(connectionState.iEmulationMode)
				{
				case ADL_EMUL_MODE_OFF:
					PRINTF (" Emulation : OFF\n");
					break;
				case ADL_EMUL_MODE_ON_CONNECTED:
					PRINTF (" Emulation : When Display connected\n");
					break;
				case ADL_EMUL_MODE_ON_DISCONNECTED:
					PRINTF (" Emulation : When Display Disconnected\n");
					break;
				case ADL_EMUL_MODE_ALWAYS:
					PRINTF (" Emulation : Always\n");
					break;
				}	
				if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_EMULATED_DEVICE_PRESENT) == ADL_EMUL_STATUS_EMULATED_DEVICE_PRESENT)
				{
					PRINTF (" Emulated Device Present : YES\n");
				}
				else
				{
					PRINTF (" Emulated Device Present : NO\n");
				}

			}  
			#pragma endregion


			#pragma region ADL_Adapter_ConnectionData_Get
			connectionData.iActiveConnections = 0;
			connectionData.iNumberofPorts = 0;

			if (ADL_OK == ADL2_Adapter_ConnectionData_Get(g_adlContext, iAdapterIndex, devicePort, ADL_QUERY_CURRENT_DATA, &connectionData))
			{
				switch (connectionData.iConnectionType)
				{
				case ADL_CONNECTION_TYPE_VGA:
					PRINTF (" Connection : VGA\n");
					break;
				case ADL_CONNECTION_TYPE_DVI:
					PRINTF (" Connection : DVI\n");
					break;
				case ADL_CONNECTION_TYPE_DVI_SL:
					PRINTF (" Connection : DVI_SL\n");
					break;
				case ADL_CONNECTION_TYPE_HDMI:
					PRINTF (" Connection : HDMI\n");
					break;
				case ADL_CONNECTION_TYPE_DISPLAY_PORT:
					PRINTF (" Connection : Display port\n");
					break;
				case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_HDMI:
					PRINTF (" Connection : DP to HDMI (Passive Dongle)\n");
					break;
				case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_DVI:
					PRINTF (" Connection : DP to DVI (Passive Dongle)\n");
					break;
				case ADL_CONNECTION_TYPE_MST:
					PRINTF (" Connection : Display Branch\n");
					PRINTF (" No of MST Ports : %d\n", connectionData.iNumberofPorts);
					break;
				case ADL_CONNECTION_TYPE_ACTIVE_DONGLE:
					PRINTF (" Connection : DP (Active Dongle)\t\t\n");
					break;
				case ADL_CONNECTION_TYPE_VIRTUAL:
					PRINTF (" Connection : Virtual\n");
					break;

				}
				PRINTF (" Properties:\n");
					PRINTF (" -----------\n");

				if (connectionData.aConnectionProperties.iValidProperties == 0)
					PRINTF (" \tNo Properties\n");
				else 
				{
					if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_BITRATE) == ADL_CONNECTION_PROPERTY_BITRATE)
					{
						switch(connectionData.aConnectionProperties.iBitrate)
						{
						case ADL_LINK_BITRATE_1_62_GHZ:
							PRINTF (" \tBITRate : 1.62Ghz\n");
							break;
						case ADL_LINK_BITRATE_2_7_GHZ:
							PRINTF (" \tBITRate : 2.7Ghz\n");
							break;						
						case ADL_LINK_BITRATE_5_4_GHZ:
							PRINTF (" \tBITRate : 5.4Ghz\n");
							break;
						default:
							PRINTF (" \tBITRate : UnKnown\n");
							break;
						}					
					}
					if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES) == ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES)
					{
						PRINTF (" \tNo.Of Lanes : %d\n", connectionData.aConnectionProperties.iNumberOfLanes);

					}
					if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_3DCAPS) == ADL_CONNECTION_PROPERTY_3DCAPS)
					{
						if ((connectionData.aConnectionProperties.iStereo3DCaps & ADL_CONNPROP_S3D_ALTERNATE_TO_FRAME_PACK) == ADL_CONNPROP_S3D_ALTERNATE_TO_FRAME_PACK)
							PRINTF (" \t3D Capabilities : Alternate Frame Pack\n");										
						else
							PRINTF (" \t3D Capabilities : Off\n");										
					}
					if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH) == ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH)
					{
						switch(connectionData.aConnectionProperties.iOutputBandwidth)
						{
						case ADL_LINK_BITRATE_1_62_GHZ:
							PRINTF (" \tBITRate : 1.62Ghz\n");
							break;
						case ADL_LINK_BITRATE_2_7_GHZ:
							PRINTF (" \tBITRate : 2.7Ghz\n");
							break;						
						case ADL_LINK_BITRATE_5_4_GHZ:
							PRINTF (" \tBITRate : 5.4Ghz\n");
							break;
						default:
							PRINTF (" \tBITRate : UnKnown\n");
							break;
						}					
					}
					if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_COLORDEPTH) == ADL_CONNECTION_PROPERTY_COLORDEPTH)
					{
						switch(connectionData.aConnectionProperties.iColorDepth)
						{
						case ADL_COLORDEPTH_666:
							PRINTF (" \tBITRate : 6Bit\n");
							break;
						case ADL_COLORDEPTH_888:
							PRINTF (" \tBITRate : 8Bit\n");
							break;
						case ADL_COLORDEPTH_101010:
							PRINTF (" \tBITRate : 10bit\n");
							break;
						case ADL_COLORDEPTH_121212:
							PRINTF (" \tBITRate : 12Bit\n");
							break;
						case ADL_COLORDEPTH_141414:
							PRINTF (" \tBITRate : 14Bit\n");
							break;
						case ADL_COLORDEPTH_161616:
							PRINTF (" \tBITRate : 16Bit\n");
							break;
						default:
							PRINTF (" \tBITRate : UnKnown\n");
							break;
						}					
					}		
				}
			}		  
			#pragma endregion

			
			


			if (connectionData.iNumberofPorts > 0)
				
			{
				printChildMSTPorts(iAdapterIndex, devicePort, connectionData.iNumberofPorts, connectionData.iActiveConnections);
			}
		}
		PRINTF (" --------------------------------------------------------- \n");
	}
}

void printChildMSTPorts(int iAdapterIndex, ADLDevicePort parentDevicePort, int numberOfMSTPorts, int ActiveConnection)
{
	
	ADLDevicePort devicePort ;
	ADLConnectionState connectionState;
	ADLConnectionData connectionData;
	ADLSupportedConnections supportedConnections;
	int i,j, ActiveBit = 0, res = -1; 
	boolean active = FALSE;
	
	for (i=0; i<numberOfMSTPorts; i++)
		{
			active = FALSE;
			ActiveBit = i;
            if (((1 << i) & ActiveConnection) == (1 << i))
            {
                 active = TRUE;                        
            }
            else
            {
                 // we have check, whether last bit is active. last bit can active when internal hub in use.
                 if (i + 1 == numberOfMSTPorts)
                 {
					 if (((1 << 7) & ActiveConnection) == (1 << 7))
					{
                        active = TRUE;
                        ActiveBit = 7;
                    }
                 }
             }

				
			devicePort.iConnectorIndex = parentDevicePort.iConnectorIndex;
			devicePort.aMSTRad.iLinkNumber = parentDevicePort.aMSTRad.iLinkNumber + 1;
			memset(devicePort.aMSTRad.rad,0,ADL_MAX_RAD_LINK_COUNT);			
			memcpy(devicePort.aMSTRad.rad, parentDevicePort.aMSTRad.rad, devicePort.aMSTRad.iLinkNumber);
			devicePort.aMSTRad.rad[parentDevicePort.aMSTRad.iLinkNumber] = (char)(ActiveBit + 1);
            
			PRINTF ("\n\n MST Port %d", devicePort.iConnectorIndex);
			for (j=1;j<devicePort.aMSTRad.iLinkNumber;j++)
				PRINTF (".%d",devicePort.aMSTRad.rad[j]);
			PRINTF (" Information \n .................................... \n\n");
			     
			
			#pragma region ADL_Adapter_SupportedConnections_Get
			if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(g_adlContext, iAdapterIndex, devicePort, &supportedConnections))
			{
				PRINTF (" Connections     | BItRate | No.OfLanes | 3D Caps | OutputBwidth | ColorDepth\n");
				PRINTF (" ______________________________________________________________________________ \n");
				
				for (j=0; j<14; j++)
				{
					if ((supportedConnections.iSupportedConnections & (1 <<j)) == (1 <<j))
					{
						switch (j)
						{
						case ADL_CONNECTION_TYPE_VGA:
							PRINTF (" VGA");
							break;
						case ADL_CONNECTION_TYPE_DVI:
							PRINTF (" DVI");
							break;
						case ADL_CONNECTION_TYPE_DVI_SL:
							PRINTF (" DVI_SL");
							break;
						case ADL_CONNECTION_TYPE_HDMI:
							PRINTF (" HDMI");
							break;
						case ADL_CONNECTION_TYPE_DISPLAY_PORT:
							PRINTF (" Display port\t\t");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_DVI_SL:
							PRINTF (" DP to DVI SL (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_DVI_DL:
							PRINTF (" DP to DVI DL (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_HDMI:
							PRINTF (" DP to HDMI (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_VGA:
							PRINTF (" DP to VGA (Active Dongle)");
							break;
						case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_HDMI:
							PRINTF (" DP to HDMI (Passive Dongle)");
							break;
						case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_DVI:
							PRINTF (" DP to DVI (Passive Dongle)");
							break;
						case ADL_CONNECTION_TYPE_MST:
							PRINTF (" Display Branch\t\t");
							break;
						case ADL_CONNECTION_TYPE_ACTIVE_DONGLE:
							PRINTF (" DP (Active Dongle)\t");
							break;
						case ADL_CONNECTION_TYPE_VIRTUAL:
							PRINTF (" VIRTUAL");
							break;
						}

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_BITRATE) == ADL_CONNECTION_PROPERTY_BITRATE)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES) == ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_3DCAPS) == ADL_CONNECTION_PROPERTY_3DCAPS)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH) == ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH)
							PRINTF ("\t Y");
						else
							PRINTF ("\t N");

						if ((supportedConnections.iSupportedProperties[j] & ADL_CONNECTION_PROPERTY_COLORDEPTH) == ADL_CONNECTION_PROPERTY_COLORDEPTH)
							PRINTF ("\t Y\n");
						else
							PRINTF ("\t N\n");
					}
				}
			}  
			#pragma endregion

			if (active)
			{
				#pragma region ADL_Adapter_ConnectionState_Get
			
				PRINTF ("\n Connection Information:\n");
				PRINTF (" -----------------------\n");
				if (ADL_OK == ADL2_Adapter_ConnectionState_Get(g_adlContext, iAdapterIndex,devicePort, &connectionState))
				{
					if (connectionState.iDisplayIndex != -1)
					{
						if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_REAL_DEVICE_CONNECTED) == ADL_EMUL_STATUS_REAL_DEVICE_CONNECTED)
						{
							PRINTF (" Display Status : Real Display Connected\n");
						}
						else if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_EMULATED_DEVICE_USED) == ADL_EMUL_STATUS_EMULATED_DEVICE_USED)
						{
							PRINTF (" Display Status : Emulated Display is Used\n");
						}
						else if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_LAST_ACTIVE_DEVICE_USED) == ADL_EMUL_STATUS_LAST_ACTIVE_DEVICE_USED)
						{
							PRINTF (" Display Status : Last Active Display is Used\n");
						}
						PRINTF (" Display Index : %d\n", connectionState.iDisplayIndex);					
					}
					else
					{
						PRINTF (" Display Status : No Display Connected\n");
					}
					switch(connectionState.iEmulationMode)
					{
					case ADL_EMUL_MODE_OFF:
						PRINTF (" Emulation : OFF\n");
						break;
					case ADL_EMUL_MODE_ON_CONNECTED:
						PRINTF (" Emulation : When Display connected\n");
						break;
					case ADL_EMUL_MODE_ON_DISCONNECTED:
						PRINTF (" Emulation : When Display Disconnected\n");
						break;
					case ADL_EMUL_MODE_ALWAYS:
						PRINTF (" Emulation : Always\n");
						break;
					}	
					if ((connectionState.iEmulationStatus & ADL_EMUL_STATUS_EMULATED_DEVICE_PRESENT) == ADL_EMUL_STATUS_EMULATED_DEVICE_PRESENT)
					{
						PRINTF (" Emulated Device Present : YES\n");
					}
					else
					{
						PRINTF (" Emulated Device Present : NO\n");
					}

				}  
	#pragma endregion


				#pragma region ADL_Adapter_ConnectionData_Get
				connectionData.iActiveConnections = 0;
				connectionData.iNumberofPorts = 0;

				if (ADL_OK == ADL2_Adapter_ConnectionData_Get(g_adlContext, iAdapterIndex, devicePort, ADL_QUERY_CURRENT_DATA, &connectionData))
				{
					switch (connectionData.iConnectionType)
					{
					case ADL_CONNECTION_TYPE_VGA:
						PRINTF (" Connection : VGA\n");
						break;
					case ADL_CONNECTION_TYPE_DVI:
						PRINTF (" Connection : DVI\n");
						break;
					case ADL_CONNECTION_TYPE_DVI_SL:
						PRINTF (" Connection : DVI_SL\n");
						break;
					case ADL_CONNECTION_TYPE_HDMI:
						PRINTF (" Connection : HDMI\n");
						break;
					case ADL_CONNECTION_TYPE_DISPLAY_PORT:
						PRINTF (" Connection : Display port\n");
						break;
					case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_HDMI:
						PRINTF (" Connection : DP to HDMI (Passive Dongle)\n");
						break;
					case ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_DVI:
						PRINTF (" Connection : DP to DVI (Passive Dongle)\n");
						break;
					case ADL_CONNECTION_TYPE_MST:
						PRINTF (" Connection : Display Branch\n");
						PRINTF (" No of MST Ports : %d\n", connectionData.iNumberofPorts);
						break;
					case ADL_CONNECTION_TYPE_ACTIVE_DONGLE:
						PRINTF (" Connection : DP (Active Dongle)\n");
						break;
					case ADL_CONNECTION_TYPE_VIRTUAL:
						PRINTF (" Connection : Virtual\n");
						break;

					}
					PRINTF (" Properties:\n");
					PRINTF (" -----------\n");

					if (connectionData.aConnectionProperties.iValidProperties == 0)
						PRINTF (" \tNo Properties\n");
					else 
					{
						if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_BITRATE) == ADL_CONNECTION_PROPERTY_BITRATE)
						{
							switch(connectionData.aConnectionProperties.iBitrate)
							{
							case ADL_LINK_BITRATE_1_62_GHZ:
								PRINTF (" \tBITRate : 1.62Ghz\n");
								break;
							case ADL_LINK_BITRATE_2_7_GHZ:
								PRINTF (" \tBITRate : 2.7Ghz\n");
								break;							
							case ADL_LINK_BITRATE_5_4_GHZ:
								PRINTF (" \tBITRate : 5.4Ghz\n");
								break;
							default:
								PRINTF (" \tBITRate : UnKnown\n");
								break;
							}					
						}
						if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES) == ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES)
						{
							PRINTF (" \tNo.Of Lanes : %d\n", connectionData.aConnectionProperties.iNumberOfLanes);

						}
						if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_3DCAPS) == ADL_CONNECTION_PROPERTY_3DCAPS)
						{
							if ((connectionData.aConnectionProperties.iStereo3DCaps & ADL_CONNPROP_S3D_ALTERNATE_TO_FRAME_PACK) == ADL_CONNPROP_S3D_ALTERNATE_TO_FRAME_PACK)
								PRINTF (" \t3D Capabilities : Alternate Frame Pack\n");										
							else
								PRINTF (" \t3D Capabilities : Off\n");										
						}
						if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH) == ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH)
						{
							switch(connectionData.aConnectionProperties.iOutputBandwidth)
							{
							case ADL_LINK_BITRATE_1_62_GHZ:
								PRINTF (" \tBITRate : 1.62Ghz\n");
								break;
							case ADL_LINK_BITRATE_2_7_GHZ:
								PRINTF (" \tBITRate : 2.7Ghz\n");
								break;							
							case ADL_LINK_BITRATE_5_4_GHZ:
								PRINTF (" \tBITRate : 5.4Ghz\n");
								break;
							default:
								PRINTF (" \tBITRate : UnKnown\n");
								break;
							}					
						}
						if ((connectionData.aConnectionProperties.iValidProperties & ADL_CONNECTION_PROPERTY_COLORDEPTH) == ADL_CONNECTION_PROPERTY_COLORDEPTH)
						{
							switch(connectionData.aConnectionProperties.iColorDepth)
							{
							case ADL_COLORDEPTH_666:
								PRINTF (" \tBITRate : 6Bit\n");
								break;
							case ADL_COLORDEPTH_888:
								PRINTF (" \tBITRate : 8Bit\n");
								break;
							case ADL_COLORDEPTH_101010:
								PRINTF (" \tBITRate : 10bit\n");
								break;
							case ADL_COLORDEPTH_121212:
								PRINTF (" \tBITRate : 12Bit\n");
								break;
							case ADL_COLORDEPTH_141414:
								PRINTF (" \tBITRate : 14Bit\n");
								break;
							case ADL_COLORDEPTH_161616:
								PRINTF (" \tBITRate : 16Bit\n");
								break;
							default:
								PRINTF (" \tBITRate : UnKnown\n");
								break;
							}		
						}
					}				
				}	  
	#pragma endregion 


				if (connectionData.iNumberofPorts > 0)				
				{
					printChildMSTPorts(iAdapterIndex, devicePort, connectionData.iNumberofPorts, connectionData.iActiveConnections);
				}
			}
			else
			{
				PRINTF ("\n Connection is not active\n");
			}			

		}
	}

void saveEDIDData(int iAdapterIndex, ADLDevicePort devicePort, char* fileName)
{
	ADLConnectionData connectionData;
	FILE *ptr_myfile;

	PRINTF (" --------------------------------------------------------- \n");
	if (ADL_OK == ADL2_Adapter_ConnectionData_Get(g_adlContext, iAdapterIndex, devicePort, ADL_QUERY_CURRENT_DATA, &connectionData))
	{
		ptr_myfile=fopen(fileName,"wb");
		fwrite(connectionData.EdidData, connectionData.iDataSize, 1, ptr_myfile);
		fclose(ptr_myfile);

		PRINTF (" Connection Data saved Successfully \n");	
	}
	else
		PRINTF (" Connection Data get Failed \n");		

	PRINTF (" --------------------------------------------------------- \n");
}

void setConnectionData(int iAdapterIndex, ADLDevicePort devicePort, int iConnectionType, char* FileName)
{
	ADLConnectionData connectionData;
	ADLSupportedConnections supportedConnections;
	FILE *ptr_myfile;

	PRINTF (" --------------------------------------------------------- \n");
	
	ptr_myfile=fopen(FileName,"rb");
	if (!ptr_myfile)
	{
		PRINTF(" Unable to open file!");			
	}
	else
	{
		fseek(ptr_myfile, 0L, SEEK_END);
		connectionData.iDataSize = ftell(ptr_myfile);
		fseek(ptr_myfile, 0L, SEEK_SET);
		fread(connectionData.EdidData ,connectionData.iDataSize,1,ptr_myfile);
	}

	fclose(ptr_myfile);
	
	
	if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(g_adlContext, iAdapterIndex, devicePort, &supportedConnections))
	{
		connectionData.aConnectionProperties.iValidProperties = 0;
		connectionData.aConnectionProperties.iBitrate = 0;
		connectionData.aConnectionProperties.iColorDepth =0;
		connectionData.aConnectionProperties.iNumberOfLanes =0;
		connectionData.aConnectionProperties.iOutputBandwidth =0;
		connectionData.aConnectionProperties.iStereo3DCaps =0;		
		if ((supportedConnections.iSupportedConnections & (1 <<iConnectionType)) == (1 <<iConnectionType))
		{
			connectionData.iConnectionType = iConnectionType;

			if ((supportedConnections.iSupportedProperties[iConnectionType] & ADL_CONNECTION_PROPERTY_BITRATE) == ADL_CONNECTION_PROPERTY_BITRATE)
			{
				connectionData.aConnectionProperties.iBitrate = ADL_LINK_BITRATE_DEF;
				connectionData.aConnectionProperties.iValidProperties |= ADL_CONNECTION_PROPERTY_BITRATE;
			}
			if ((supportedConnections.iSupportedProperties[iConnectionType] & ADL_CONNECTION_PROPERTY_COLORDEPTH) == ADL_CONNECTION_PROPERTY_COLORDEPTH)
			{
				connectionData.aConnectionProperties.iColorDepth = ADL_COLOR_DEPTH_DEF;
				connectionData.aConnectionProperties.iValidProperties |= ADL_CONNECTION_PROPERTY_COLORDEPTH;
			}
			if ((supportedConnections.iSupportedProperties[iConnectionType] & ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES) == ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES)
			{
				connectionData.aConnectionProperties.iNumberOfLanes  = ADL_LANECOUNT_DEF;
				connectionData.aConnectionProperties.iValidProperties |= ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES;
			}
			if ((supportedConnections.iSupportedProperties[iConnectionType] & ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH) == ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH)
			{
				connectionData.aConnectionProperties.iOutputBandwidth = ADL_LINK_BITRATE_DEF;
				connectionData.aConnectionProperties.iValidProperties |= ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH;
			}
			if ((supportedConnections.iSupportedProperties[iConnectionType] & ADL_CONNECTION_PROPERTY_3DCAPS) == ADL_CONNECTION_PROPERTY_3DCAPS)
			{
				connectionData.aConnectionProperties.iStereo3DCaps = 0;
				connectionData.aConnectionProperties.iValidProperties |= ADL_CONNECTION_PROPERTY_3DCAPS;
			}

			if (ADL_OK == ADL2_Adapter_ConnectionData_Set(g_adlContext, iAdapterIndex, devicePort, connectionData))
			{
				PRINTF (" Set Connection Data success on %d \n", devicePort.iConnectorIndex);
				if (ADL_OK == ADL2_Adapter_EmulationMode_Set(g_adlContext, iAdapterIndex, devicePort, 2))
					PRINTF(" Emulation Activation Successfully \n");
				else
					PRINTF(" Emulation Activation failed \n");
			}
			else
				PRINTF (" Set Connection Data failed on %d \n", devicePort.iConnectorIndex);
		}
		else
		{
			PRINTF (" Can't Emulate virtual display as it is not supported on this port \n");
		}

	}

	
	PRINTF (" --------------------------------------------------------- \n");
}

void setEmulation(int iAdapterIndex, ADLDevicePort devicePort, int iEmulationMode)
{
	
	PRINTF (" --------------------------------------------------------- \n");
	if (ADL_OK == ADL2_Adapter_EmulationMode_Set(g_adlContext, iAdapterIndex, devicePort, iEmulationMode))
		PRINTF (" Emulation Activation Successfully \n");				
	else
		PRINTF (" Emulation Activation failed \n");		
		
		PRINTF (" --------------------------------------------------------- \n");
}

void removeEmulation(int iAdapterIndex, ADLDevicePort devicePort)
{
	PRINTF (" --------------------------------------------------------- \n");
	if (ADL_OK == ADL2_Adapter_ConnectionData_Remove(g_adlContext, iAdapterIndex, devicePort))
		PRINTF (" Connection data & emulation removed Successfully \n");				
	else
		PRINTF (" Connection data & emulation remove failed \n");		

	PRINTF (" --------------------------------------------------------- \n");
}

void applyMode(int iAdapterIndex, int iDisplayIndex, int xRes, int yRes, float fRefreshRate, int force)
{

	PRINTF(" --------------------------------------------------------- \n");

	int iNumModes;
	LPADLMode lpADLModes;
	if (ADL_OK != ADL2_Display_Modes_Get(g_adlContext, iAdapterIndex, iDisplayIndex, &iNumModes, &lpADLModes))
	{
		PRINTF("ADL2_Display_Modes_Get failed!");
	}
	int numPossModes = 0;
	ADLMode* possModes = NULL;
	
	if (ADL_OK == ADL2_Display_PossibleMode_Get(g_adlContext, iAdapterIndex, &numPossModes, &possModes))
	{
		int iModeFound = 0;
		ADLMode mode;
		// Iterate and find the SLS mode
		for (int possModeIdx = 0; possModeIdx < numPossModes; possModeIdx++)
		{
            PRINTF("xRes:%d yRes:%d fRefreshRate:%f %d, %d, %d, %d\n", possModes[possModeIdx].iXRes, possModes[possModeIdx].iYRes, possModes[possModeIdx].fRefreshRate, possModes[possModeIdx].displayID.iDisplayLogicalAdapterIndex, possModes[possModeIdx].displayID.iDisplayLogicalIndex, possModes[possModeIdx].displayID.iDisplayPhysicalAdapterIndex, possModes[possModeIdx].displayID.iDisplayPhysicalIndex);
			if (possModes[possModeIdx].iXRes == xRes && possModes[possModeIdx].iYRes == yRes && possModes[possModeIdx].fRefreshRate == fRefreshRate )
			{
				iModeFound = 1;
				mode = possModes[possModeIdx];
				break;
			}
		}
		if (iModeFound == 0 && force == 1)
		{
			int iOverrideSupported = 0;
    		int numDisplays = 0;
            ADLDisplayInfo* allDisplaysBuffer = NULL;
            ADLGlSyncMode glSyncMode;
            glSyncMode.iControlVector = ADL_GLSYNC_PORT_UNKNOWN;
            if (ADL_OK == ADL2_Workstation_DisplayGLSyncMode_Get(g_adlContext, iAdapterIndex, iDisplayIndex, &glSyncMode))
            {            
                // Check the Override timming support on this adapter
                if(ADL_GLSYNC_PORT_UNKNOWN != glSyncMode.iControlVector)
                    PRINTF("FLGL is enabled now, FLGL not enabled on this display is the necessary prerequisite to support adding custom resolutions.");
            }
            if (ADL_OK == ADL2_Adapter_ModeTimingOverride_Caps(g_adlContext, iAdapterIndex, &iOverrideSupported))
			{
				if (ADL_FALSE == iOverrideSupported) 
					PRINTF("Requested adapter does not support adding custom resolutions!");					
				else if (ADL_OK == ADL2_Display_DisplayInfo_Get(g_adlContext, iAdapterIndex, &numDisplays, &allDisplaysBuffer, 1))
				{
                    // Check the Override timming support on this monitor
                    for (int i = 0; i < numDisplays; i++)
                    {
                        ADLDisplayInfo* oneDis = &(allDisplaysBuffer[i]);
                        if (iDisplayIndex == oneDis->displayID.iDisplayLogicalIndex)
                        {
                            if ((oneDis->iDisplayInfoValue  & ADL_DISPLAY_DISPLAYINFO_MODETIMING_OVERRIDESSUPPORTED) == ADL_DISPLAY_DISPLAYINFO_MODETIMING_OVERRIDESSUPPORTED)
                            {
					            ADLDisplayModeX2 customDisplayMode;
					            customDisplayMode.iWidth = xRes;
					            customDisplayMode.iHeight = yRes;
					            customDisplayMode.iRefreshRate = fRefreshRate;
					            customDisplayMode.iScanType = 0; // 0 = progressive / ADL_DL_TIMINGFLAG_INTERLACED = interlaced
					            customDisplayMode.iTimingStandard = ADL_DL_MODETIMING_STANDARD_CVT;

					            ADLDisplayModeInfo dspModeInfo = {};

					            if (ADL_OK != ADL2_Display_ModeTimingOverrideX2_Get(g_adlContext, iAdapterIndex, lpADLModes[0].displayID, &customDisplayMode, &dspModeInfo))
					            {
						            PRINTF("ADL_Display_ModeTimingOverrideX2_Get failed!");						
					            }
					            else
					            {
                                    if (ADL_OK != ADL2_Display_ModeTimingOverride_Set(g_adlContext, iAdapterIndex, iDisplayIndex, &dspModeInfo, 1))
						            {
							            PRINTF("ADL_Display_ModeTimingOverride_Set failed!");
						            }						
						            else
						            {
                                        mode = lpADLModes[0];
							            mode.iXRes = xRes;
							            mode.iYRes = yRes;
							            mode.fRefreshRate = fRefreshRate;
                            
							            iModeFound = 1;
						            }
					            }
                            }
                        }
                    }
			    }
			    else
				    PRINTF("ADL2_Display_DisplayInfo_Get failed! \n");
            }
            else
                PRINTF("ADL2_Adapter_ModeTimingOverride_Caps failed! \n");
		}

		if (iModeFound == 1)
		{
			mode.iColourDepth = 32;
			if (ADL_OK != ADL2_Display_Modes_Set(g_adlContext, iAdapterIndex, iDisplayIndex, 1, &mode))
			{
				PRINTF("ADL2_Display_Modes_Set failed!");
			}
			else
			{
				PRINTF("Mode Applied");
			}
		}

	}
	PRINTF(" --------------------------------------------------------- \n");
}
