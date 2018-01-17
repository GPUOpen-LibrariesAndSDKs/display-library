///
///  Copyright (c) 2008 - 2012 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.cpp

#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include "eyefinity.h"
#include <stdio.h>

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int ( *ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )();
typedef int ( *ADL_FLUSH_DRIVER_DATA)(int);
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) ( LPAdapterInfo, int );
typedef int ( *ADL_DISPLAY_COLORCAPS_GET ) ( int, int, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_GET ) ( int, int, int, int *, int *, int *, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_SET ) ( int, int, int, int );
typedef int ( *ADL_DISPLAY_DISPLAYINFO_GET ) ( int, int *, ADLDisplayInfo **, int );
// SLS functions
typedef int ( *ADL_DISPLAY_SLSMAPINDEX_GET ) ( int, int, ADLDisplayTarget *, int * );
typedef int ( *ADL_DISPLAY_SLSMAPCONFIG_GET	) ( int, int, ADLSLSMap*, int*, ADLSLSTarget**, int*, ADLSLSMode**, int*, ADLBezelTransientMode**, int*, ADLBezelTransientMode**, int*, ADLSLSOffset**, int );
typedef int ( *ADL_DISPLAY_SLSMAPCONFIG_DELETE) (int, int);
typedef int ( *ADL_DISPLAY_SLSMAPCONFIG_CREATE) (int, ADLSLSMap, int, ADLSLSTarget*, int, int*, int);
typedef int ( *ADL_DISPLAY_SLSMAPCONFIG_REARRANGE) (int, int, int, ADLSLSTarget*, ADLSLSMap, int);
typedef int ( *ADL_DISPLAY_SLSMAPCONFIG_SETSTATE) (int, int, int);
typedef int ( *ADL_DISPLAY_SLSMAPINDEXLIST_GET) (int, int*, int**, int);
typedef int ( *ADL_DISPLAY_MODES_GET ) ( int, int, int*, ADLMode ** );
typedef int ( *ADL_DISPLAY_MODES_SET ) (int, int, int, ADLMode*);
typedef int ( *ADL_DISPLAY_BEZELOFFSET_SET) ( int, int ,int, LPADLSLSOffset, ADLSLSMap, int);
typedef int ( *ADL_DISPLAY_BEZELOFFSETSTEPPINGSIZE_GET) (int, int*, ADLBezelOffsetSteppingSize**);
//display map functions
typedef int ( *ADL_DISPLAY_DISPLAYMAPCONFIG_GET ) ( int, int*, ADLDisplayMap**, int*, ADLDisplayTarget**, int );
typedef int ( *ADL_DISPLAY_DISPLAYMAPCONFIG_SET) (int, int, ADLDisplayMap*, int, ADLDisplayTarget*);

// adapter functions
typedef int ( *ADL_DISPLAY_POSSIBLEMODE_GET ) (int, int*, ADLMode**);
typedef int ( *ADL_DISPLAY_NUMBEROFDISPLAYS_GET) (int, int*);
typedef int ( *ADL_ADAPTER_PRIMARY_SET ) (int);
typedef int ( *ADL_ADAPTER_PRIMARY_GET ) (int*);
typedef int ( *ADL_ADAPTER_ACTIVE_SET ) (int, int, int*);



HINSTANCE hDLL;

ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
ADL_FLUSH_DRIVER_DATA			 ADL_Flush_Driver_Data = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
ADL_DISPLAY_SLSMAPINDEX_GET		 ADL_Display_SLSMapIndex_Get = NULL;
ADL_DISPLAY_SLSMAPCONFIG_GET	 ADL_Display_SLSMapConfig_Get = NULL;
ADL_DISPLAY_SLSMAPCONFIG_DELETE  ADL_Display_SLSMapConfig_Delete = NULL;
ADL_DISPLAY_SLSMAPCONFIG_CREATE  ADL_Display_SLSMapConfig_Create = NULL;
ADL_DISPLAY_SLSMAPCONFIG_REARRANGE  ADL_Display_SLSMapConfig_Rearrange = NULL;
ADL_DISPLAY_SLSMAPCONFIG_SETSTATE ADL_Display_SLSMapConfig_SetState = NULL;
ADL_DISPLAY_MODES_GET			 ADL_Display_Modes_Get = NULL;
ADL_DISPLAY_POSSIBLEMODE_GET ADL_Display_PossibleMode_Get = NULL;
ADL_DISPLAY_MODES_SET			 ADL_Display_Modes_Set = NULL;
ADL_DISPLAY_DISPLAYMAPCONFIG_GET ADL_Display_DisplayMapConfig_Get = NULL;
ADL_DISPLAY_DISPLAYMAPCONFIG_SET ADL_Display_DisplayMapConfig_Set = NULL;
ADL_DISPLAY_BEZELOFFSET_SET		 ADL_Display_BezelOffset_Set = NULL;
ADL_DISPLAY_BEZELOFFSETSTEPPINGSIZE_GET ADL_Display_BezelOffsetSteppingSize_Get = NULL;
ADL_DISPLAY_NUMBEROFDISPLAYS_GET ADL_Display_NumberOfDisplays_Get = NULL;
ADL_DISPLAY_DISPLAYINFO_GET ADL_Display_DisplayInfo_Get = NULL;
ADL_ADAPTER_PRIMARY_SET ADL_Adapter_Primary_Set = NULL;
ADL_ADAPTER_PRIMARY_GET ADL_Adapter_Primary_Get = NULL;
ADL_ADAPTER_ACTIVE_SET ADL_Adapter_Active_Set = NULL;
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

int initializeADL()
{
	
	// Load the ADL dll
	{
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
	}

	// Get & validate function pointers
	{
		ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL_Main_Control_Create");
		ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
		ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");
		ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET) GetProcAddress(hDLL,"ADL_Adapter_AdapterInfo_Get");
		ADL_Display_SLSMapIndex_Get = (ADL_DISPLAY_SLSMAPINDEX_GET)GetProcAddress(hDLL,"ADL_Display_SLSMapIndex_Get");
		ADL_Display_SLSMapConfig_Get = (ADL_DISPLAY_SLSMAPCONFIG_GET)GetProcAddress(hDLL,"ADL_Display_SLSMapConfig_Get");
		ADL_Display_Modes_Get = (ADL_DISPLAY_MODES_GET)GetProcAddress(hDLL,"ADL_Display_Modes_Get");
		ADL_Display_PossibleMode_Get = (ADL_DISPLAY_POSSIBLEMODE_GET)GetProcAddress(hDLL,"ADL_Display_PossibleMode_Get");
		ADL_Display_Modes_Set = (ADL_DISPLAY_MODES_SET)GetProcAddress(hDLL,"ADL_Display_Modes_Set");
		ADL_Display_SLSMapConfig_Delete = (ADL_DISPLAY_SLSMAPCONFIG_DELETE)GetProcAddress(hDLL, "ADL_Display_SLSMapConfig_Delete");
		ADL_Display_SLSMapConfig_Create = (ADL_DISPLAY_SLSMAPCONFIG_CREATE)GetProcAddress(hDLL, "ADL_Display_SLSMapConfig_Create");
		ADL_Display_SLSMapConfig_Rearrange = (ADL_DISPLAY_SLSMAPCONFIG_REARRANGE)GetProcAddress(hDLL, "ADL_Display_SLSMapConfig_Rearrange");
		ADL_Display_SLSMapConfig_SetState = (ADL_DISPLAY_SLSMAPCONFIG_SETSTATE)GetProcAddress(hDLL, "ADL_Display_SLSMapConfig_SetState");
		ADL_Flush_Driver_Data = (ADL_FLUSH_DRIVER_DATA)GetProcAddress(hDLL, "ADL_Flush_Driver_Data");
		ADL_Display_DisplayMapConfig_Get = (ADL_DISPLAY_DISPLAYMAPCONFIG_GET)GetProcAddress(hDLL,"ADL_Display_DisplayMapConfig_Get");
		ADL_Display_DisplayMapConfig_Set = (ADL_DISPLAY_DISPLAYMAPCONFIG_SET)GetProcAddress(hDLL, "ADL_Display_DisplayMapConfig_Set");
		ADL_Display_BezelOffset_Set = (ADL_DISPLAY_BEZELOFFSET_SET) GetProcAddress(hDLL, "ADL_Display_BezelOffset_Set");
		ADL_Display_BezelOffsetSteppingSize_Get = (ADL_DISPLAY_BEZELOFFSETSTEPPINGSIZE_GET) GetProcAddress(hDLL, "ADL_Display_BezelOffsetSteppingSize_Get");
		ADL_Display_NumberOfDisplays_Get = (ADL_DISPLAY_NUMBEROFDISPLAYS_GET) GetProcAddress(hDLL, "ADL_Display_NumberOfDisplays_Get");
		ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET) GetProcAddress(hDLL,"ADL_Display_DisplayInfo_Get");
		ADL_Adapter_Primary_Set = (ADL_ADAPTER_PRIMARY_SET) GetProcAddress(hDLL,"ADL_Adapter_Primary_Set");
		ADL_Adapter_Primary_Get = (ADL_ADAPTER_PRIMARY_GET) GetProcAddress(hDLL,"ADL_Adapter_Primary_Get");
		ADL_Adapter_Active_Set = (ADL_ADAPTER_ACTIVE_SET) GetProcAddress(hDLL,"ADL_Adapter_Active_Set");
		if ( NULL == ADL_Main_Control_Create ||
			 NULL == ADL_Main_Control_Destroy ||
			 NULL == ADL_Adapter_NumberOfAdapters_Get ||
			 NULL == ADL_Adapter_AdapterInfo_Get ||
			 NULL == ADL_Display_SLSMapIndex_Get ||
			 NULL == ADL_Display_SLSMapConfig_Get ||
			 NULL == ADL_Display_Modes_Get ||
			 NULL == ADL_Display_Modes_Set ||
			 NULL == ADL_Display_DisplayMapConfig_Get ||
			 NULL == ADL_Display_SLSMapConfig_Delete ||
			 NULL == ADL_Display_SLSMapConfig_Create ||
			 NULL == ADL_Display_SLSMapConfig_Rearrange ||
			 NULL == ADL_Display_SLSMapConfig_SetState ||
			 NULL == ADL_Flush_Driver_Data ||
			 NULL == ADL_Display_DisplayMapConfig_Set ||
			 NULL == ADL_Display_BezelOffset_Set ||
			 NULL == ADL_Display_BezelOffsetSteppingSize_Get ||
			 NULL == ADL_Display_NumberOfDisplays_Get ||
			 NULL == ADL_Display_DisplayInfo_Get ||
			 NULL == ADL_Adapter_Primary_Set ||
			 NULL == ADL_Adapter_Primary_Get ||
			 NULL == ADL_Adapter_Active_Set ||
			 NULL == ADL_Display_PossibleMode_Get
			 )
		{
			PRINTF("Failed to get ADL function pointers\n");
			return FALSE;
		}
	}

	if ( ADL_OK != ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1) )
	{
		PRINTF("ADL_Main_Control_Create() failed\n");
		return FALSE;
	}
	
	return TRUE;
}	

void deinitializeADL()
{
    FreeLibrary(hDLL);
}




int setAdapterDisplaysToEyefinity(int iAdapterIndex, int iRows, int iColumns,int iDisplayMapIndexes[], int iNumOfDisplays, int iSLSRearrange)
{
	int iRetVal = 1;

	int iSLSMapIndexOut;
    //int  iAdapterIndex;
	int i=0,j=0,iCurrentSLSTarget;
	int displayFound=0;
	ADLSLSTarget* pSLSTargets = NULL;
	ADLSLSMap slsMap;

	//to get the modes
	int iNumModes;
	ADLMode* pModes=NULL;

	//need for Get Display Mappings
	int iNumDisplayTarget = 0;
	ADLDisplayTarget *lpDisplayTarget = NULL;
	int iNumDisplayMap = 0;
	ADLDisplayMap *lpDisplayMap = NULL;
	int iSLSMapIndex = -1;
	int iSLSMapValue = 512;//Fill

	//Holds the refresh SLS info params
	ADLSLSMap lpCreatedSLSMap;
	ADLSLSTarget* lppCreatedSLSTargets = NULL;
	ADLSLSMode* lppCreatedSLSNativeModes = NULL;
	ADLBezelTransientMode* lppCreatedBezelModes = NULL;
	ADLBezelTransientMode* lppCreatedTransientModes = NULL;
	ADLSLSOffset* lppCreatedBezelOffsets = NULL;
	int iNumCreatedSLSTargets = 0;
	int iNumCreatedSLSNativeModes = 0;
	int iNumCreatedBezelModes = 0;
	int iNumCreatedTransientModes = 0;
	int iNumCreatedBezelOffsets = 0;
	LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";


	pSLSTargets = (ADLSLSTarget*) malloc(iNumOfDisplays * sizeof(ADLSLSTarget));	
	
 	//iAdapterIndex = getAdapterIndex(iDisplayMapIndexes[0]);
	
	if (iAdapterIndex != -1)
	{
		if (!iSLSRearrange)
		{
			//Makeing this Adapter as Primary Adapater
			setPrimaryAdapter(iAdapterIndex);

			//Disabling Extended Surfaces
			setAdapterDisplaysToDisable(iAdapterIndex);
		
			//set to Clone Disabled displays
			setAdapterDisplaysToClone(iAdapterIndex, iDisplayMapIndexes, iNumOfDisplays);
		}

		iRetVal = ADL_Display_Modes_Get(iAdapterIndex, -1, &iNumModes, &pModes);

		//Setting the SLSMAP Information
		memset(&slsMap, 0, sizeof(ADLSLSMap));		

		slsMap.grid.iSLSGridColumn = iColumns;
		slsMap.grid.iSLSGridRow = iRows;
		
		slsMap.iAdapterIndex = iAdapterIndex;

		slsMap.iSLSMapIndex = iSLSMapIndex; 
		slsMap.iNumNativeMode = 0;
		slsMap.iNumBezelMode = 0;
		slsMap.iOrientation = pModes[0].iOrientation;
		slsMap.iSLSMapValue = iSLSMapValue;

		iRetVal = ADL_Display_DisplayMapConfig_Get( iAdapterIndex,
														&iNumDisplayMap, &lpDisplayMap, 
														&iNumDisplayTarget, &lpDisplayTarget, 
														ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );
		if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
		{
			iRetVal = ADL_Display_SLSMapIndex_Get (iAdapterIndex, 
				iNumDisplayTarget,
				lpDisplayTarget,
				&iSLSMapIndex);

			if (ADL_OK == iRetVal && iSLSMapIndex != -1)
			{
				//This is a temporary workaround to enable SLS.
				//Set this variable to any value.
				SetEnvironmentVariable(EnvironmentVariable, "TRUE");

				iRetVal = ADL_Display_SLSMapConfig_Get ( iAdapterIndex, iSLSMapIndex, &lpCreatedSLSMap,
														  &iNumCreatedSLSTargets, &lppCreatedSLSTargets,
														  &iNumCreatedSLSNativeModes, &lppCreatedSLSNativeModes,
														  &iNumCreatedBezelModes, &lppCreatedBezelModes,
														  &iNumCreatedTransientModes, &lppCreatedTransientModes,
														  &iNumCreatedBezelOffsets, &lppCreatedBezelOffsets,
														  ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE );


				if (ADL_OK == iRetVal)
				{
					if (iSLSRearrange)
					{
						iColumns = lpCreatedSLSMap.grid.iSLSGridColumn;
						slsMap.grid.iSLSGridColumn = lpCreatedSLSMap.grid.iSLSGridColumn;
						iRows = lpCreatedSLSMap.grid.iSLSGridRow;
						slsMap.grid.iSLSGridRow = lpCreatedSLSMap.grid.iSLSGridRow;
					}
					slsMap.iSLSMapIndex = iSLSMapIndex; 
					slsMap.iNumNativeMode = iNumCreatedSLSNativeModes;
					slsMap.iNumBezelMode = iNumCreatedBezelModes;
				}

			}
		}		
		
		if (iSLSRearrange)
		{
			if (iNumOfDisplays != iNumCreatedSLSTargets)
			{
				PRINTF("Number of displays currently in SLS are not matched with number of displays provided for re arrange\n");
				return 0;
			}
			for (i=0; i<iNumCreatedSLSTargets;i++)
			{
				displayFound = 0;
				for (j=0;j<iNumCreatedSLSTargets;j++)
				{
					if (lppCreatedSLSTargets[j].displayTarget.displayID.iDisplayLogicalIndex == iDisplayMapIndexes[i])
					{
						displayFound = 1;
						break;
					}
				}
				if (!displayFound)
				{
					PRINTF("provided display is not particepating in SLS\n");
					return 0;
				}
			}
		}
		iCurrentSLSTarget=0;
		for (i=0; i<iRows; i++)
		{
			for (j=0; j<iColumns; j++)
			{
				
				pSLSTargets[iCurrentSLSTarget].iAdapterIndex = iAdapterIndex;
				memset(&(pSLSTargets[iCurrentSLSTarget].displayTarget), 0, sizeof(ADLDisplayTarget));

				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex = iDisplayMapIndexes[iCurrentSLSTarget];
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayPhysicalIndex = iDisplayMapIndexes[iCurrentSLSTarget];
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayPhysicalAdapterIndex = iAdapterIndex;

				pSLSTargets[iCurrentSLSTarget].iSLSMapIndex = iSLSMapIndex;

				memset(&(pSLSTargets[iCurrentSLSTarget].viewSize), 0, sizeof(ADLMode));
			
				pSLSTargets[iCurrentSLSTarget].viewSize.iAdapterIndex = iAdapterIndex;
				pSLSTargets[iCurrentSLSTarget].viewSize.iModeFlag = pModes[0].iModeFlag;
				pSLSTargets[iCurrentSLSTarget].viewSize.iOrientation = pModes[0].iOrientation;
				pSLSTargets[iCurrentSLSTarget].viewSize.fRefreshRate = pModes[0].fRefreshRate;
				pSLSTargets[iCurrentSLSTarget].viewSize.iColourDepth = pModes[0].iColourDepth;
				pSLSTargets[iCurrentSLSTarget].viewSize.iXPos = pModes[0].iXPos;
				pSLSTargets[iCurrentSLSTarget].viewSize.iYPos = pModes[0].iYPos;
				pSLSTargets[iCurrentSLSTarget].viewSize.iXRes = pModes[0].iXRes;
				pSLSTargets[iCurrentSLSTarget].viewSize.iYRes = pModes[0].iYRes;


				pSLSTargets[iCurrentSLSTarget].iSLSGridPositionX = j;
				pSLSTargets[iCurrentSLSTarget].iSLSGridPositionY = i;
				iCurrentSLSTarget++;
			}		
		}

		if (!iSLSRearrange)
		{
			iRetVal = ADL_Display_SLSMapConfig_Create (
					iAdapterIndex,
					slsMap,
					iNumOfDisplays,
					pSLSTargets,
					0, // bezel mode percent
					&iSLSMapIndexOut,
					ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_LANDSCAPE
					);
			if (iRetVal == ADL_OK)
				PRINTF("SLS created Successfully\n");			
			else
			{
					PRINTF("Unable to create SLS\n");
			}
		}
		else
		{
			iRetVal = ADL_Display_SLSMapConfig_Rearrange(iAdapterIndex,slsMap.iSLSMapIndex, iNumOfDisplays, pSLSTargets, lpCreatedSLSMap, ADL_DISPLAY_SLSMAPCONFIG_REARRANGE_OPTION_RELATIVETO_LANDSCAPE);
			if (iRetVal == ADL_OK)
				PRINTF("SLS Re-arranged Successfully\n");			
			else
				{
					PRINTF("Unable to re-arrange SLS\n");
				}
				
		}
	}

	ADL_Main_Memory_Free((void**)&pSLSTargets);
	ADL_Main_Memory_Free((void**)&lppCreatedSLSTargets);
	ADL_Main_Memory_Free((void**)&lppCreatedSLSNativeModes);
	ADL_Main_Memory_Free((void**)&lppCreatedBezelModes);
	ADL_Main_Memory_Free((void**)&lppCreatedTransientModes);
	ADL_Main_Memory_Free((void**)&lppCreatedBezelOffsets);
	ADL_Main_Memory_Free((void**)&pModes);
	ADL_Main_Memory_Free((void**)&lpDisplayMap);
	ADL_Main_Memory_Free((void**)&lpDisplayTarget);

	return iRetVal;
}

int setBezelOffsets(int iAdapterIndex, int iHbezel, int iVbezel)
{
	int iRetVal = 1;

	int iSLSMapIndexOut;
	int i=0,j=0, iCurrentSLSTarget;

	//need for Get Display Mappings
	int iNumDisplayTarget = 0;
	ADLDisplayTarget *lpDisplayTarget = NULL;
	int iNumDisplayMap = 0;
	ADLDisplayMap *lpDisplayMap = NULL;
	int iSLSMapIndex = -1;

	//Holds the refresh SLS info params
	ADLSLSMap lpCreatedSLSMap;
	ADLSLSTarget* lppCreatedSLSTargets = NULL;
	ADLSLSMode* lppCreatedSLSNativeModes = NULL;
	ADLBezelTransientMode* lppCreatedBezelModes = NULL;
	ADLBezelTransientMode* lppCreatedTransientModes = NULL;
	ADLSLSOffset* lppCreatedBezelOffsets = NULL;
	int iNumCreatedSLSTargets = 0;
	int iNumCreatedSLSNativeModes = 0;
	int iNumCreatedBezelModes = 0;
	int iNumCreatedTransientModes = 0;
	int iNumCreatedBezelOffsets = 0;
	int iCurrentTransientMode;
	//For Bezel Width and Height Ref Values
	int iReferenceWidth = 0;
	int iReferenceHeight = 0;
	int iCurrentBezelOffset = 0;
	int iNumModes;

	int iSLSMapValue = 512;//Fill
	ADLMode *pDisplayMode = NULL;	

	ADLSLSOffset* lpAppliedBezelOffsets = NULL;
	ADLSLSOffset* lpTempAppliedBezelOffsets = NULL;
	LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";

	iRetVal = ADL_Display_DisplayMapConfig_Get( iAdapterIndex,
													&iNumDisplayMap, &lpDisplayMap, 
													&iNumDisplayTarget, &lpDisplayTarget, 
													ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
		iRetVal = ADL_Display_SLSMapIndex_Get (iAdapterIndex, 
			iNumDisplayTarget,
			lpDisplayTarget,
			&iSLSMapIndex);

		if (iSLSMapIndex != -1)
		{
			//This is a temporary workaround to enable SLS.
			//Set this variable to any value.
			SetEnvironmentVariable(EnvironmentVariable, "TRUE");

			iRetVal = ADL_Display_SLSMapConfig_Get ( iAdapterIndex, iSLSMapIndex, &lpCreatedSLSMap,
														&iNumCreatedSLSTargets, &lppCreatedSLSTargets,
														&iNumCreatedSLSNativeModes, &lppCreatedSLSNativeModes,
														&iNumCreatedBezelModes, &lppCreatedBezelModes,
														&iNumCreatedTransientModes, &lppCreatedTransientModes,
														&iNumCreatedBezelOffsets, &lppCreatedBezelOffsets,
														ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE );

			if (ADL_OK == iRetVal)
			{
				//iRetVal = ADL_Display_Modes_Get(iAdapterIndex, -1, &iNumModes, &pDisplayMode);

				//currentMode = pDisplayMode[iNumModes - 1];
				//for(iCurrentDisplayMode = 0; iCurrentDisplayMode < iNumModes; iCurrentDisplayMode++)
				//{
				//	// we need to figure out the positioning of this display in the SLS
				//	pDisplayMode[iCurrentDisplayMode].iXRes = currentMode.iXRes / lpCreatedSLSMap.grid.iSLSGridColumn;
				//	pDisplayMode[iCurrentDisplayMode].iYRes = currentMode.iYRes / lpCreatedSLSMap.grid.iSLSGridRow;
				//	iRetVal = ADL_Display_Modes_Set(iAdapterIndex, -1, 1, &pDisplayMode[iCurrentDisplayMode]);
				//	break;
				//}

				iRetVal = ADL_Display_SLSMapConfig_SetState(iAdapterIndex, iSLSMapIndex, 0);
			}		
		}
			
	}
	
	if (iRetVal == 0 && (0 != iHbezel || 0 != iVbezel))
	{
		iRetVal = ADL_Display_SLSMapConfig_Create (
			iAdapterIndex,
			lpCreatedSLSMap,
			iNumCreatedSLSTargets,
			lppCreatedSLSTargets,
			10, // bezel mode percent
			&iSLSMapIndexOut,
			ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_LANDSCAPE
			);

	

		iRetVal = ADL_Display_SLSMapConfig_Get ( iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, &lpCreatedSLSMap,
														&iNumCreatedSLSTargets, &lppCreatedSLSTargets,
														&iNumCreatedSLSNativeModes, &lppCreatedSLSNativeModes,
														&iNumCreatedBezelModes, &lppCreatedBezelModes,
														&iNumCreatedTransientModes, &lppCreatedTransientModes,
														&iNumCreatedBezelOffsets, &lppCreatedBezelOffsets,
														ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE );

		
		if (iNumCreatedTransientModes > 0)
		{
			ADLMode *pTransientModes = NULL;
			iRetVal = ADL_Display_Modes_Get(iAdapterIndex, -1, &iNumModes, &pTransientModes);

			//lppCreatedTransientModes = (ADLBezelTransientMode*) malloc(iNumCreatedTransientModes * sizeof(ADLBezelTransientMode));
			for(iCurrentTransientMode = 0; iCurrentTransientMode < iNumCreatedTransientModes; iCurrentTransientMode++)
			{
				// we need to figure out the positioning of this display in the SLS
				pTransientModes[iCurrentTransientMode].iXRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iXRes;
				pTransientModes[iCurrentTransientMode].iYRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iYRes;
				iRetVal = ADL_Display_Modes_Set(iAdapterIndex, -1, 1, &pTransientModes[iCurrentTransientMode]);
				break;
			}

			if (iRetVal == 0)
			{
				iRetVal = ADL_Display_SLSMapConfig_Get ( iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, &lpCreatedSLSMap,
														&iNumCreatedSLSTargets, &lppCreatedSLSTargets,
														&iNumCreatedSLSNativeModes, &lppCreatedSLSNativeModes,
														&iNumCreatedBezelModes, &lppCreatedBezelModes,
														&iNumCreatedTransientModes, &lppCreatedTransientModes,
														&iNumCreatedBezelOffsets, &lppCreatedBezelOffsets,
														ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE );

				if (iNumCreatedBezelOffsets > 0)
				{
					iReferenceWidth = lppCreatedBezelOffsets[iNumCreatedBezelOffsets - 1].iDisplayWidth;
					iReferenceHeight = lppCreatedBezelOffsets[iNumCreatedBezelOffsets - 1].iDisplayHeight;

					
					//Try applying Permanently
					lpAppliedBezelOffsets = (ADLSLSOffset*) malloc(iNumCreatedSLSTargets * sizeof(ADLSLSOffset));
					//Set the initial offsets to start setting the user offsets
					for(iCurrentSLSTarget = 0; iCurrentSLSTarget < iNumCreatedSLSTargets; iCurrentSLSTarget++)
					{
						for (iCurrentBezelOffset = 0; iCurrentBezelOffset< iNumCreatedBezelOffsets; iCurrentBezelOffset++)
						{
							if (lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelModeIndex == lppCreatedTransientModes[iNumCreatedTransientModes - 1].iSLSModeIndex &&
								lppCreatedBezelOffsets[iCurrentBezelOffset].displayID.iDisplayLogicalIndex == lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex)
							{
								lpAppliedBezelOffsets[iCurrentSLSTarget].iBezelOffsetX = lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelOffsetX - (lppCreatedSLSTargets[iCurrentSLSTarget].iSLSGridPositionX * iHbezel);
								lpAppliedBezelOffsets[iCurrentSLSTarget].iBezelOffsetY = lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelOffsetY - (lppCreatedSLSTargets[iCurrentSLSTarget].iSLSGridPositionY * iVbezel);

								lpAppliedBezelOffsets[iCurrentSLSTarget].iAdapterIndex = iAdapterIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].iSLSMapIndex = lpCreatedSLSMap.iSLSMapIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayLogicalIndex = lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayPhysicalAdapterIndex = iAdapterIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayPhysicalIndex = lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].iDisplayWidth = iReferenceWidth;
								lpAppliedBezelOffsets[iCurrentSLSTarget].iDisplayHeight = iReferenceHeight;
							}
						}
					}

					lpCreatedSLSMap.iSLSMapMask = 0;
					lpCreatedSLSMap.iSLSMapValue = iSLSMapValue;
					iRetVal = ADL_Display_BezelOffset_Set(iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, iNumCreatedSLSTargets, lpAppliedBezelOffsets, lpCreatedSLSMap, ADL_DISPLAY_BEZELOFFSET_COMMIT | ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE);
					if (iRetVal != 0)
					{
						PRINTF("Setting bezel offsets failed");
					}
				}
			}
		}
	}

	return iRetVal;
}

int setResolution(int iAdapterIndex, int iXRes, int iYRes)
{
	int iNumModes, iNumModesPriv, iRetVal;
	ADLMode *pModes = NULL, *lppModesPriv = NULL;
	int iResfound = 0,i;
	if (iAdapterIndex != -1)
	{
		// Check if the mode is in the possible mode list.
		iRetVal = ADL_Display_PossibleMode_Get (iAdapterIndex, &iNumModesPriv, &lppModesPriv);
		if (NULL != lppModesPriv)
		{
			for ( i=0;i<iNumModesPriv;i++)
			{
				if( lppModesPriv[i].iXRes == iXRes && lppModesPriv[i].iYRes == iYRes)
				{			
					iResfound=1;
					break;
				}
			}			
		}

		if (iResfound)
		{
			iRetVal = ADL_Display_Modes_Get(iAdapterIndex, -1, &iNumModes, &pModes);
			if (iRetVal == ADL_OK)
			{
				pModes[0].iXRes = iXRes;
				pModes[0].iYRes = iYRes;
				iRetVal = ADL_Display_Modes_Set(iAdapterIndex, -1, 1, &pModes[0]);
				if (iRetVal != ADL_OK)
				{
					PRINTF("unable to set provided resolution \n");
				}
			}
		}
		else
		{
			PRINTF("not a valid resolution \n");
		}
	}
	else
	{
		PRINTF("not a valid adapter index \n");
	}
	ADL_Main_Memory_Free((void**)&pModes);
	ADL_Main_Memory_Free((void**)&lppModesPriv);
	return TRUE;
}

int setPrimaryAdapter(int iAdapterIndex)
{
	int iCurrentAdapterIndex = 0;
	if (ADL_OK == ADL_Adapter_Primary_Get(&iCurrentAdapterIndex))
	{
		if (iCurrentAdapterIndex != iAdapterIndex && CanSetPrimary(iAdapterIndex ,iCurrentAdapterIndex))
		{
			ADL_Adapter_Primary_Set(iAdapterIndex);
		}
	}
	return TRUE;
}

int setAdapterDisplaysToDisable(int iAdapterIndex)
{
	int iAdapterIndexes[6];
	int iDisplayIndexes[6];
	int iCount=0;
	int active;
	int i;
	getDisplayIndexesofOneGPU(iAdapterIndex, &iAdapterIndexes[0], &iDisplayIndexes[0], &iCount);

	for (i=0;i<iCount;i++)
	{
		if (iAdapterIndex != iAdapterIndexes[i] && iAdapterIndexes[i] != -1)
		{
			ADL_Adapter_Active_Set (iAdapterIndexes[i], 0, &active);
		}
	}
	return TRUE;
}

int setAdapterDisplaysToClone(int iAdapterIndex, int iDisplayIndexes[], int iDisplaysCount)
{
	int i;
	ADLDisplayMap mapArray;
    ADLDisplayTarget* pDisplayTargets = NULL;


	int iNumModes;
	ADLMode *pModes = NULL;
	pDisplayTargets = (ADLDisplayTarget*) malloc(iDisplaysCount * sizeof(ADLDisplayTarget));
	memset(&(mapArray), 0, sizeof(ADLDisplayMap));
	mapArray.iNumDisplayTarget = iDisplaysCount;
	mapArray.iDisplayMapIndex = 0;
	
	ADL_Display_Modes_Get(iAdapterIndex, -1, &iNumModes, &pModes);
	memset(&(mapArray.displayMode), 0, sizeof(ADLMode));
	mapArray.displayMode.iAdapterIndex = iAdapterIndex;
	mapArray.displayMode.iModeFlag = pModes[0].iModeFlag;
	mapArray.displayMode.iOrientation = pModes[0].iOrientation;
	mapArray.displayMode.fRefreshRate = pModes[0].fRefreshRate;
	mapArray.displayMode.iColourDepth = pModes[0].iColourDepth;
	mapArray.displayMode.iXPos = pModes[0].iXPos;
	mapArray.displayMode.iYPos = pModes[0].iYPos;
	mapArray.displayMode.iXRes = pModes[0].iXRes;
	mapArray.displayMode.iYRes = pModes[0].iYRes;

	for (i=0;i<iDisplaysCount;i++)
	{
		memset(&(pDisplayTargets[i].displayID), 0, sizeof(ADLDisplayID));
		memset(&(pDisplayTargets[i]), 0, sizeof(ADLDisplayTarget));
		pDisplayTargets[i].displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
		pDisplayTargets[i].displayID.iDisplayLogicalIndex = iDisplayIndexes[i];
		pDisplayTargets[i].iDisplayMapIndex = 0;
	}
	ADL_Display_DisplayMapConfig_Set(iAdapterIndex,1,&mapArray,iDisplaysCount,pDisplayTargets);

	ADL_Main_Memory_Free((void**)&pDisplayTargets);
	return TRUE;
}

int disableAdapterEyefinityMapping(int iAdapterIndex)
{
	int iNumDisplayTarget = 0;
	ADLDisplayTarget *lpDisplayTarget = NULL;
	int iNumDisplayMap = 0;
	ADLDisplayMap *lpDisplayMap = NULL;
	int iSLSMapIndex = -1,iRetVal=-1;

	iRetVal = ADL_Display_DisplayMapConfig_Get( iAdapterIndex,
														&iNumDisplayMap, &lpDisplayMap, 
														&iNumDisplayTarget, &lpDisplayTarget, 
														ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
		iRetVal = ADL_Display_SLSMapIndex_Get (iAdapterIndex, 
			iNumDisplayTarget,
			lpDisplayTarget,
			&iSLSMapIndex);

		if (ADL_OK == iRetVal && iSLSMapIndex != -1)
		{
			iRetVal = ADL_Display_SLSMapConfig_SetState(iAdapterIndex, iSLSMapIndex, 0);
			if (iRetVal != ADL_OK)
			{
				PRINTF ("Unable to Disable SLS");
			}
		}
		else
		{
			PRINTF ("SLS is not created on this adapter");
		}
	}
	ADL_Main_Memory_Free((void**)&lpDisplayMap);
	return TRUE;
}

int printDisplayIndexes()
{
	int  iNumberAdapters, iNumDisplays;
    int  iAdapterIndex;
    int  iDisplayIndex[6];
	int iBusNumber;
	int i=0,j=0,k=0,l=0, iGPUfound=0, iDisplayFound=0, iGPUIndex=0,iCount=0, iGPUCounter=0;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
	int igpuBusIndexes[4];

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

	for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
	{
		igpuBusIndexes[iGPUIndex] = -1;
	}
	PRINTF (" Adapter and Displays Indexes <AdapterIndex, DisplayIndex> \n");
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
			for (l=0;l<6;l++)
			{
				iDisplayIndex[l] = -1;				
			}
			iCount =0;
			PRINTF (" Displays Connected to GPU #%d\n", iGPUCounter);
			PRINTF (" ----------------------------- \n");
			for (j = 0; j < iNumberAdapters; j++)
			{
				if (iBusNumber == lpAdapterInfo[ j ].iBusNumber)
				{
					iAdapterIndex = lpAdapterInfo[ j ].iAdapterIndex;
					ADL_Main_Memory_Free ((void**) &lpAdlDisplayInfo );
					if (ADL_OK != ADL_Display_DisplayInfo_Get (lpAdapterInfo[j].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
						continue;

					for ( k = 0; k < iNumDisplays; k++ )
					{
							//For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
						if (( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED  ) != 
							( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED 	&
								lpAdlDisplayInfo[ k ].iDisplayInfoValue ) )
							continue;   // Skip the not connected or not mapped displays         
					
						iDisplayFound = 0;
						for (l=0;l<iCount;l++)
						{
							if (lpAdlDisplayInfo[k].displayID.iDisplayLogicalIndex == iDisplayIndex[l])
							{
								iDisplayFound = 1;
								break;
							}
						}
						if (!iDisplayFound)
						{
							iDisplayIndex[iCount] = lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalIndex;
							(iCount)++;
							PRINTF(" {%d,%d} \t",lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalAdapterIndex, lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalIndex);
						}	
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

int getDisplayIndexesofOneGPU(int iCurrentAdapterIndex, int* lpAdpaterIndexes, int* lpDisplayIndexes, int* lpCount )
{
	int  iNumberAdapters, iNumDisplays;
    int  iAdapterIndex;
  	int iBusNumber;
	int i=0,j=0, found=0,k=0;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
	*lpCount = 0;
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

	for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[i].iAdapterIndex == iCurrentAdapterIndex)
		{
			iBusNumber = lpAdapterInfo[i].iBusNumber;
		}
	}
    // Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (iBusNumber != lpAdapterInfo[ i ].iBusNumber)
			continue;

		iAdapterIndex = lpAdapterInfo[ i ].iAdapterIndex;
		ADL_Main_Memory_Free ((void**) &lpAdlDisplayInfo );
		if (ADL_OK != ADL_Display_DisplayInfo_Get (lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
			continue;

        for ( j = 0; j < iNumDisplays; j++ )
        {
				//For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
            if (( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED  ) != 
                ( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED 	&
					lpAdlDisplayInfo[ j ].iDisplayInfoValue ) )
                continue;   // Skip the not connected or not mapped displays
         
			found =1;

			for (k=0;k<*lpCount;k++)
			{
				if (lpAdlDisplayInfo[ j].displayID.iDisplayLogicalIndex == lpDisplayIndexes[k])
				{
					found = 0;
					break;
				}
			}
			if (found)
			{
				lpAdpaterIndexes[*lpCount] = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex;
				lpDisplayIndexes[*lpCount] = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalIndex;
				(*lpCount)++;
			}			
		}
	}
	return TRUE;
}

int CanSetPrimary(int iAdapterIndex, int iCurrentPrimaryAdapterIndex)
{
	int  iNumberAdapters;
	int iBusNumber = 0, iCurrentPrimaryAdapterBusNumber = 0;
	int i;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
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

	//Finding Adapater Index for SLS creation.
    // Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (iAdapterIndex == lpAdapterInfo[ i ].iAdapterIndex)
		{
			iBusNumber = lpAdapterInfo[ i ].iBusNumber;
		}       
		if (iCurrentPrimaryAdapterIndex == lpAdapterInfo[ i ].iAdapterIndex)
		{
			iCurrentPrimaryAdapterBusNumber = lpAdapterInfo[ i ].iBusNumber;
		}
	}

	if (iCurrentPrimaryAdapterBusNumber == iBusNumber)
	{
		return 1;
	}

	return 0;
}



