///
///  Copyright (c) 2008 - 2022 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file EDID.c

#include <windows.h>
#include "..\..\include\ADL_sdk.h"
#include "EDID.h"
#include <stdio.h>

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int ( *ADL2_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int ( *ADL2_MAIN_CONTROL_DESTROY )(ADL_CONTEXT_HANDLE);
typedef int ( *ADL2_FLUSH_DRIVER_DATA)(int);
typedef int ( *ADL2_ADAPTER_NUMBEROFADAPTERS_GET ) (ADL_CONTEXT_HANDLE, int* );
typedef int ( *ADL2_ADAPTER_ADAPTERINFO_GET ) (ADL_CONTEXT_HANDLE, LPAdapterInfo, int );
typedef int ( *ADL2_ADAPTER_EDIDMANAGEMENT_CAPS )(ADL_CONTEXT_HANDLE, int, int*);
typedef int ( *ADL2_ADAPTERX2_CAPS) (ADL_CONTEXT_HANDLE, int, ADLAdapterCapsX2* adapterCaps );
typedef int ( *ADL2_ADAPTER_CONNECTIONSTATE_GET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, ADLConnectionState*);
typedef int ( *ADL2_ADAPTER_CONNECTIONDATA_GET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, int , ADLConnectionData*);
typedef int ( *ADL2_ADAPTER_CONNECTIONDATA_SET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, ADLConnectionData);
typedef int ( *ADL2_ADAPTER_SUPPORTEDCONNECTIONS_GET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, ADLSupportedConnections*);
typedef int ( *ADL2_ADAPTER_EMULATIONMODE_SET) (ADL_CONTEXT_HANDLE, int, ADLDevicePort, int);
typedef int ( *ADL2_ADAPTER_CONNECTIONDATA_REMOVE) (ADL_CONTEXT_HANDLE, int, ADLDevicePort);
typedef int ( *ADL2_ADAPTER_BOARDLAYOUT_GET) (ADL_CONTEXT_HANDLE, int, int*, int*, ADLBracketSlotInfo**, int*, ADLConnectorInfo**);
typedef int ( *ADL2_DISPLAY_EDIDDATA_GET) (ADL_CONTEXT_HANDLE context, int, int, ADLDisplayEDIDData* lpEDIDData);

HINSTANCE hDLL;
ADL_CONTEXT_HANDLE context = NULL;

ADL2_MAIN_CONTROL_CREATE          ADL2_Main_Control_Create = NULL;
ADL2_MAIN_CONTROL_DESTROY         ADL2_Main_Control_Destroy = NULL;
ADL2_FLUSH_DRIVER_DATA			 ADL2_Flush_Driver_Data = NULL;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET ADL2_Adapter_NumberOfAdapters_Get = NULL;
ADL2_ADAPTER_ADAPTERINFO_GET      ADL2_Adapter_AdapterInfo_Get = NULL;
ADL2_ADAPTER_EDIDMANAGEMENT_CAPS ADL2_Adapter_EDIDManagement_Caps = NULL;
ADL2_ADAPTERX2_CAPS ADL2_AdapterX2_Caps = NULL;
ADL2_ADAPTER_CONNECTIONSTATE_GET ADL2_Adapter_ConnectionState_Get = NULL;
ADL2_ADAPTER_CONNECTIONDATA_GET ADL2_Adapter_ConnectionData_Get = NULL;
ADL2_ADAPTER_CONNECTIONDATA_SET ADL2_Adapter_ConnectionData_Set = NULL;
ADL2_ADAPTER_SUPPORTEDCONNECTIONS_GET ADL2_Adapter_SupportedConnections_Get = NULL;
ADL2_ADAPTER_EMULATIONMODE_SET ADL2_Adapter_EmulationMode_Set = NULL;
ADL2_ADAPTER_CONNECTIONDATA_REMOVE ADL2_Adapter_ConnectionData_Remove = NULL;
ADL2_ADAPTER_BOARDLAYOUT_GET ADL2_Adapter_BoardLayout_Get = NULL;
ADL2_DISPLAY_EDIDDATA_GET               ADL2_Display_EdidData_Get = NULL;
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

int unInitializeADL()
{
	if (ADL_OK != ADL2_Main_Control_Destroy(context))
	{
		PRINTF("Failed to destroy ADL2 context");
		return FALSE;
	}
}
int initializeADL()
{
	
	// Load the ADL dll
	{
		OutputDebugStringA("11::::atiadlxx.dll");
		hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
		if (hDLL == NULL)
		{
			OutputDebugStringA("11::::atiadlxy.dll");
			// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
			// Try to load the 32 bit library (atiadlxy.dll) instead
			hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
		}

		if (NULL == hDLL)
		{
			PRINTF("Failed to load ADL library\n");
			return FALSE;
		}
	}
	{
		ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL2_Main_Control_Create");
		ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL2_Main_Control_Destroy");
		ADL2_Adapter_NumberOfAdapters_Get = (ADL2_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL2_Adapter_NumberOfAdapters_Get");
		ADL2_Adapter_AdapterInfo_Get = (ADL2_ADAPTER_ADAPTERINFO_GET) GetProcAddress(hDLL,"ADL2_Adapter_AdapterInfo_Get");
		ADL2_Adapter_EDIDManagement_Caps = (ADL2_ADAPTER_EDIDMANAGEMENT_CAPS) GetProcAddress(hDLL, "ADL2_Adapter_EDIDManagement_Caps");
		ADL2_AdapterX2_Caps = (ADL2_ADAPTERX2_CAPS) GetProcAddress( hDLL, "ADL2_AdapterX2_Caps");
		ADL2_Adapter_ConnectionState_Get = (ADL2_ADAPTER_CONNECTIONSTATE_GET) GetProcAddress (hDLL, "ADL2_Adapter_ConnectionState_Get");
		ADL2_Adapter_ConnectionData_Get = (ADL2_ADAPTER_CONNECTIONDATA_GET) GetProcAddress(hDLL, "ADL2_Adapter_ConnectionData_Get");
		ADL2_Adapter_ConnectionData_Set = (ADL2_ADAPTER_CONNECTIONDATA_SET) GetProcAddress(hDLL, "ADL2_Adapter_ConnectionData_Set");
		ADL2_Adapter_SupportedConnections_Get = (ADL2_ADAPTER_SUPPORTEDCONNECTIONS_GET) GetProcAddress(hDLL, "ADL2_Adapter_SupportedConnections_Get");
		ADL2_Adapter_EmulationMode_Set = (ADL2_ADAPTER_EMULATIONMODE_SET) GetProcAddress( hDLL,"ADL2_Adapter_EmulationMode_Set");
		ADL2_Adapter_ConnectionData_Remove = (ADL2_ADAPTER_CONNECTIONDATA_REMOVE) GetProcAddress (hDLL, "ADL2_Adapter_ConnectionData_Remove");
		ADL2_Adapter_BoardLayout_Get = (ADL2_ADAPTER_BOARDLAYOUT_GET) GetProcAddress (hDLL, "ADL2_Adapter_BoardLayout_Get");
		ADL2_Display_EdidData_Get = (ADL2_DISPLAY_EDIDDATA_GET) GetProcAddress(hDLL, "ADL2_Display_EdidData_Get");
		if ( NULL == ADL2_Main_Control_Create ||
			 NULL == ADL2_Main_Control_Destroy ||
			 NULL == ADL2_Adapter_NumberOfAdapters_Get||
			 NULL == ADL2_Adapter_AdapterInfo_Get ||
			 NULL == ADL2_Adapter_EDIDManagement_Caps ||
			 NULL == ADL2_AdapterX2_Caps ||
			 NULL == ADL2_Adapter_ConnectionState_Get ||
			 NULL == ADL2_Adapter_ConnectionData_Get ||
			 NULL == ADL2_Adapter_ConnectionData_Set ||
			 NULL == ADL2_Adapter_SupportedConnections_Get ||
			 NULL == ADL2_Adapter_EmulationMode_Set ||
			 NULL == ADL2_Adapter_ConnectionData_Remove ||
			 NULL == ADL2_Adapter_BoardLayout_Get ||
			 NULL == ADL2_Display_EdidData_Get)
		{
			PRINTF("Failed to get ADL function pointers\n");
			return FALSE;
		}
	}
	if ( ADL_OK != ADL2_Main_Control_Create (ADL_Main_Memory_Alloc, 1, &context) )
	{
		PRINTF("ADL2_Main_Control_Create() failed\n");
		return FALSE;
	}
	
	return TRUE;
}	

void deinitializeADL()
{
    FreeLibrary(hDLL);
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
    if ( ADL_OK != ADL2_Adapter_NumberOfAdapters_Get (context, &iNumberAdapters ) )
	{
	       PRINTF("Cannot get the number of adapters!\n");
		   return 0;
	}

    if ( 0 < iNumberAdapters )
    {
        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        ADL2_Adapter_AdapterInfo_Get (context, lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
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
				if (iBusNumber == lpAdapterInfo[ j ].iBusNumber)
				{
					iAdapterIndex = lpAdapterInfo[ j ].iAdapterIndex;					
					if (ADL_OK != ADL2_Adapter_EDIDManagement_Caps(context, iAdapterIndex, &iEDIDSupported))
					{
						PRINTF (" ADL2_Adapter_EDIDManagement_Caps failed \n");
					}
					else
					{
						if (iEDIDSupported == 1)
						{
                        	ADLAdapterCapsX2 temp;
                            memset(&temp, 0, sizeof(temp));

							if (ADL_OK != ADL2_AdapterX2_Caps(context, iAdapterIndex, &temp))
							{
								PRINTF (" ADL2_AdapterX2_Caps Failed \n");
							}
							else
							{
								PRINTF (" EDID Management Feature Supported \n");
								PRINTF (" Adapter Index : %d \n", iAdapterIndex);
								if (temp.iNumConnectors >0)
								{
									PRINTF (" No of Ports : %d \n", temp.iNumConnectors);
									
									ADL2_Adapter_BoardLayout_Get(context, iAdapterIndex, &iValidFlags, &iNumofSlots, &barcketSlotInfo, &iNumofConnectors, &connectorInfo);
									
									if (iValidFlags != 15)
										
										PRINTF (" Your VBIOS is older and connector location metioned in below is may not same as ASIC physical layout ");
									
									PRINTF (" Details of Ports  \n");
									for (k=0; k<iNumofConnectors; k++)
									{
										if (slotIndex != connectorInfo[k].iSlotIndex)
										{
											slotIndex = connectorInfo[k].iSlotIndex;
											PRINTF (" Slot Number : %d\n", slotIndex);
										}
										PRINTF (" \tConnector Index : %d\t", connectorInfo[k].iConnectorIndex);
										switch(connectorInfo[k].iType)
										{
										case 1:
											PRINTF (" Connector Type : VGA\n");
											break;
										case 2:
											PRINTF (" Connector Type : DVI_I\n");
											break;
										case 3:
											PRINTF (" Connector Type : DVI_D\n");
											break;
										case 8:
											PRINTF (" Connector Type : HDMI A\n");
											break;
										case 9:
											PRINTF (" Connector Type : HDMI B\n");
											break;
										case 10:
											PRINTF (" Connector Type : DP\n");
											break;
										case 12:
											PRINTF (" Connector Type : Mini DP\n");
											break;
										case 13:
											PRINTF (" Connector Type : Virtual\n");
											break;
										default:
											PRINTF (" Connector Type : UNKNOWN\n");
											break;
										}										
									}

									

								}
								else
									PRINTF (" No Ports are allowed to emulate \n");

							}
						}
						else
						{
							PRINTF (" EDID Management Feature Not Supported on Adapter #%d\n" , iAdapterIndex);
						}
							

					}
					break;
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
	if (ADL_OK == ADL2_AdapterX2_Caps(context, iAdapterIndex, &temp))
	{
		for (i=0; i<temp.iNumConnectors; i++)
		{
			devicePort.iConnectorIndex = i;
			devicePort.aMSTRad.iLinkNumber = 1;
			devicePort.aMSTRad.rad[0] = 0; 

			PRINTF ("\n --------------------------------------------------------- \n");
			PRINTF (" Connector #%d Information :\n" , i);
			PRINTF (" --------------------------------------------------------- \n");
				
			#pragma region ADL2_Adapter_SupportedConnections_Get
			if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(context, iAdapterIndex, devicePort, &supportedConnections))
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
			
			#pragma region ADL2_Adapter_ConnectionState_Get
			PRINTF ("\n Connection Information:\n");
			PRINTF (" -----------------------\n");
				
			if (ADL_OK == ADL2_Adapter_ConnectionState_Get(context, iAdapterIndex,devicePort, &connectionState))
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


			#pragma region ADL2_Adapter_ConnectionData_Get
			connectionData.iActiveConnections = 0;
			connectionData.iNumberofPorts = 0;

			if (ADL_OK == ADL2_Adapter_ConnectionData_Get(context, iAdapterIndex, devicePort, ADL_QUERY_CURRENT_DATA, &connectionData))
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
			     
			
			#pragma region ADL2_Adapter_SupportedConnections_Get
			if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(context, iAdapterIndex, devicePort, &supportedConnections))
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
				#pragma region ADL2_Adapter_ConnectionState_Get
			
				PRINTF ("\n Connection Information:\n");
				PRINTF (" -----------------------\n");
				if (ADL_OK == ADL2_Adapter_ConnectionState_Get(context, iAdapterIndex,devicePort, &connectionState))
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


				#pragma region ADL2_Adapter_ConnectionData_Get
				connectionData.iActiveConnections = 0;
				connectionData.iNumberofPorts = 0;

				if (ADL_OK == ADL2_Adapter_ConnectionData_Get(context, iAdapterIndex, devicePort, ADL_QUERY_CURRENT_DATA, &connectionData))
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

	PRINTF(" --------------------------------------------------------- \n");
	if (ADL2_Adapter_ConnectionData_Get == NULL)
	{
		PRINTF(" Connection Data get NULLLLLL \n");
	}
	if (ADL_OK == ADL2_Adapter_ConnectionData_Get(context, iAdapterIndex, devicePort, ADL_QUERY_CURRENT_DATA, &connectionData))
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

void ReadEDIDDataByDisplayIndex(int iAdapterIndex, int iDisplayIndex)
{
	ADLDisplayEDIDData edidData;
	ZeroMemory(&edidData, sizeof(edidData));
	edidData.iSize = sizeof(edidData);

	PRINTF(" --------------------------------------------------------- \n");
	if (ADL2_Display_EdidData_Get == NULL)
	{
		PRINTF(" EDID API is NULLLLLL \n");
	}
	if (ADL_OK == ADL2_Display_EdidData_Get(context, iAdapterIndex, iDisplayIndex, &edidData))
	{
		for (int i = 0; i < ADL_MAX_EDIDDATA_SIZE; i+=16)
		{
			PRINTF("%08x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
				(unsigned char)edidData.cEDIDData[i], (unsigned char)edidData.cEDIDData[i + 1], (unsigned char)edidData.cEDIDData[i + 2], (unsigned char)edidData.cEDIDData[i + 3],
				(unsigned char)edidData.cEDIDData[i + 4], (unsigned char)edidData.cEDIDData[i + 5], (unsigned char)edidData.cEDIDData[i + 6], (unsigned char)edidData.cEDIDData[i + 7],
				(unsigned char)edidData.cEDIDData[i + 8], (unsigned char)edidData.cEDIDData[i + 9], (unsigned char)edidData.cEDIDData[i + 10], (unsigned char)edidData.cEDIDData[i + 11],
				(unsigned char)edidData.cEDIDData[i + 12], (unsigned char)edidData.cEDIDData[i + 13], (unsigned char)edidData.cEDIDData[i + 14], (unsigned char)edidData.cEDIDData[i + 15]);
		}
		PRINTF(" EDID feteched Successfully \n");
	}
	else
		PRINTF(" EDID feteched Failed %08x\n", ADL2_Display_EdidData_Get(context, iAdapterIndex, iDisplayIndex, &edidData));

	PRINTF(" --------------------------------------------------------- \n");
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
	
	
	if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(context, iAdapterIndex, devicePort, &supportedConnections))
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

			if (ADL_OK == ADL2_Adapter_ConnectionData_Set(context, iAdapterIndex, devicePort, connectionData))
			{
				PRINTF (" Set Connection Data success on %d \n", devicePort.iConnectorIndex);
			}
			else
				PRINTF (" Set Connection Data failed on %d \n", devicePort.iConnectorIndex);
		}
		else
		{
			PRINTF (" Provided connection type is not supported on this port \n");
		}

	}

	
	PRINTF (" --------------------------------------------------------- \n");
}

void setBranchData(int iAdapterIndex, ADLDevicePort devicePort, int iConnectionType, int iNumberofMSTPorts)
{
	ADLConnectionData connectionData;
	ADLSupportedConnections supportedConnections;
	FILE *ptr_myfile;
	
	PRINTF (" --------------------------------------------------------- \n");
	
	
	if (ADL_OK == ADL2_Adapter_SupportedConnections_Get(context, iAdapterIndex, devicePort, &supportedConnections))
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
			connectionData.iNumberofPorts = iNumberofMSTPorts; 
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

			if (ADL_OK == ADL2_Adapter_ConnectionData_Set(context, iAdapterIndex, devicePort, connectionData))
			{
				PRINTF (" Set Connection Data success on %d \n", devicePort.iConnectorIndex);
			}
			else
				PRINTF (" Set Connection Data failed on %d \n", devicePort.iConnectorIndex);
		}
		else
		{
			PRINTF (" Provided connection type is not supported on this port \n");
		}

	}

	
	PRINTF (" --------------------------------------------------------- \n");
}

void setEmulation(int iAdapterIndex, ADLDevicePort devicePort, int iEmulationMode)
{
	
	PRINTF (" --------------------------------------------------------- \n");
	if (ADL_OK == ADL2_Adapter_EmulationMode_Set(context, iAdapterIndex, devicePort, iEmulationMode))
		PRINTF (" Emulation Activation Successfully \n");				
	else
		PRINTF (" Emulation Activation failed \n");		

	PRINTF (" --------------------------------------------------------- \n");
}

void removeEmulation(int iAdapterIndex, ADLDevicePort devicePort)
{
	PRINTF (" --------------------------------------------------------- \n");
	if (ADL_OK == ADL2_Adapter_ConnectionData_Remove(context, iAdapterIndex, devicePort))
		PRINTF (" Connection data & emulation removed Successfully \n");				
	else
		PRINTF (" Connection data & emulation remove failed \n");		

	PRINTF (" --------------------------------------------------------- \n");
}