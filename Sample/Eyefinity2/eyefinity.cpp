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

ADL_CONTEXT_HANDLE ADLContext_ = NULL;

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL2_MAIN_CONTROLX2_CREATE)                               (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)                                (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_ADAPTERINFOX3_GET)                           (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);
typedef int(*ADL2_DISPLAY_DISPLAYINFO_GET)                             (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplays, ADLDisplayInfo ** lppInfo, int iForceDetect);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_VALID)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLSLSMap slsMap, int iNumDisplayTarget, ADLSLSTarget* lpSLSTarget, int* lpSupportedSLSLayoutImageMode, int* lpReasonForNotSupportSLS, int iOption);

// SLS functions
typedef int(*ADL2_DISPLAY_SLSMAPINDEX_GET) (ADL_CONTEXT_HANDLE, int, int, ADLDisplayTarget *, int *);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_GET) (ADL_CONTEXT_HANDLE, int, int, ADLSLSMap*, int*, ADLSLSTarget**, int*, ADLSLSMode**, int*, ADLBezelTransientMode**, int*, ADLBezelTransientMode**, int*, ADLSLSOffset**, int);

typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_DELETE)                         (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iSLSMapIndex);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_CREATE)                         (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLSLSMap SLSMap, int iNumTarget, ADLSLSTarget* lpSLSTarget, int iBezelModePercent, int *lpSLSMapIndex, int iOption);


typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_REARRANGE) (ADL_CONTEXT_HANDLE, int, int, int, ADLSLSTarget*, ADLSLSMap, int);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_SETSTATE) (ADL_CONTEXT_HANDLE, int, int, int);
typedef int(*ADL2_DISPLAY_SLSMAPINDEXLIST_GET) (ADL_CONTEXT_HANDLE, int, int*, int**, int);
typedef int(*ADL2_DISPLAY_MODES_GET) (ADL_CONTEXT_HANDLE, int, int, int*, ADLMode **);
typedef int(*ADL2_DISPLAY_MODES_SET) (ADL_CONTEXT_HANDLE, int, int, int, ADLMode*);
typedef int(*ADL2_DISPLAY_BEZELOFFSET_SET) (ADL_CONTEXT_HANDLE, int, int, int, LPADLSLSOffset, ADLSLSMap, int);

//display map functions
typedef int(*ADL2_DISPLAY_DISPLAYMAPCONFIG_GET)                        (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplayMap, ADLDisplayMap** lppDisplayMap, int* lpNumDisplayTarget, ADLDisplayTarget** lppDisplayTarget, int iOptions);
typedef int(*ADL2_DISPLAY_DISPLAYMAPCONFIG_SET) (ADL_CONTEXT_HANDLE, int, int, ADLDisplayMap*, int, ADLDisplayTarget*);

// adapter functions
typedef int(*ADL2_DISPLAY_POSSIBLEMODE_GET) (ADL_CONTEXT_HANDLE, int, int*, ADLMode**);
typedef int(*ADL2_ADAPTER_PRIMARY_SET) (ADL_CONTEXT_HANDLE, int);
typedef int(*ADL2_ADAPTER_PRIMARY_GET) (ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_ACTIVE_SET) (ADL_CONTEXT_HANDLE, int, int, int*);



HINSTANCE hDLL;

ADL2_MAIN_CONTROLX2_CREATE              ADL2_Main_ControlX2_Create = NULL;
ADL2_MAIN_CONTROL_DESTROY               ADL2_Main_Control_Destroy = NULL;
ADL2_ADAPTER_ADAPTERINFOX3_GET          ADL2_Adapter_AdapterInfoX3_Get = NULL;
ADL2_DISPLAY_DISPLAYINFO_GET            ADL2_Display_DisplayInfo_Get = NULL;
ADL2_DISPLAY_SLSMAPCONFIG_VALID         ADL2_Display_SLSMapConfig_Valid = NULL;

ADL2_DISPLAY_SLSMAPINDEX_GET		 ADL2_Display_SLSMapIndex_Get = NULL;
ADL2_DISPLAY_SLSMAPCONFIG_GET	 ADL2_Display_SLSMapConfig_Get = NULL;
ADL2_DISPLAY_SLSMAPCONFIG_DELETE  ADL2_Display_SLSMapConfig_Delete = NULL;
ADL2_DISPLAY_SLSMAPCONFIG_CREATE  ADL2_Display_SLSMapConfig_Create = NULL;
ADL2_DISPLAY_SLSMAPCONFIG_REARRANGE  ADL2_Display_SLSMapConfig_Rearrange = NULL;
ADL2_DISPLAY_SLSMAPCONFIG_SETSTATE ADL2_Display_SLSMapConfig_SetState = NULL;
ADL2_DISPLAY_MODES_GET			 ADL2_Display_Modes_Get = NULL;
ADL2_DISPLAY_POSSIBLEMODE_GET ADL2_Display_PossibleMode_Get = NULL;
ADL2_DISPLAY_MODES_SET			 ADL2_Display_Modes_Set = NULL;
ADL2_DISPLAY_DISPLAYMAPCONFIG_GET ADL2_Display_DisplayMapConfig_Get = NULL;
ADL2_DISPLAY_DISPLAYMAPCONFIG_SET ADL2_Display_DisplayMapConfig_Set = NULL;
ADL2_DISPLAY_BEZELOFFSET_SET		 ADL2_Display_BezelOffset_Set = NULL;

ADL2_ADAPTER_PRIMARY_SET ADL2_Adapter_Primary_Set = NULL;
ADL2_ADAPTER_PRIMARY_GET ADL2_Adapter_Primary_Get = NULL;
ADL2_ADAPTER_ACTIVE_SET ADL2_Adapter_Active_Set = NULL;
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
        ADL2_Main_ControlX2_Create = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(hDLL, "ADL2_Main_ControlX2_Create");
		ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL2_Main_Control_Destroy");
        ADL2_Adapter_AdapterInfoX3_Get = (ADL2_ADAPTER_ADAPTERINFOX3_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfoX3_Get");
        ADL2_Display_SLSMapConfig_Valid = (ADL2_DISPLAY_SLSMAPCONFIG_VALID)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Valid");
		ADL2_Display_SLSMapIndex_Get = (ADL2_DISPLAY_SLSMAPINDEX_GET)GetProcAddress(hDLL,"ADL2_Display_SLSMapIndex_Get");
		ADL2_Display_SLSMapConfig_Get = (ADL2_DISPLAY_SLSMAPCONFIG_GET)GetProcAddress(hDLL,"ADL2_Display_SLSMapConfig_Get");
		ADL2_Display_Modes_Get = (ADL2_DISPLAY_MODES_GET)GetProcAddress(hDLL,"ADL2_Display_Modes_Get");
		ADL2_Display_PossibleMode_Get = (ADL2_DISPLAY_POSSIBLEMODE_GET)GetProcAddress(hDLL,"ADL2_Display_PossibleMode_Get");
		ADL2_Display_Modes_Set = (ADL2_DISPLAY_MODES_SET)GetProcAddress(hDLL,"ADL2_Display_Modes_Set");
		ADL2_Display_SLSMapConfig_Delete = (ADL2_DISPLAY_SLSMAPCONFIG_DELETE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Delete");
		ADL2_Display_SLSMapConfig_Create = (ADL2_DISPLAY_SLSMAPCONFIG_CREATE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Create");
		ADL2_Display_SLSMapConfig_Rearrange = (ADL2_DISPLAY_SLSMAPCONFIG_REARRANGE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Rearrange");
		ADL2_Display_SLSMapConfig_SetState = (ADL2_DISPLAY_SLSMAPCONFIG_SETSTATE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_SetState");
		ADL2_Display_DisplayMapConfig_Get = (ADL2_DISPLAY_DISPLAYMAPCONFIG_GET)GetProcAddress(hDLL,"ADL2_Display_DisplayMapConfig_Get");
		ADL2_Display_DisplayMapConfig_Set = (ADL2_DISPLAY_DISPLAYMAPCONFIG_SET)GetProcAddress(hDLL, "ADL2_Display_DisplayMapConfig_Set");
		ADL2_Display_BezelOffset_Set = (ADL2_DISPLAY_BEZELOFFSET_SET) GetProcAddress(hDLL, "ADL2_Display_BezelOffset_Set");
		ADL2_Display_DisplayInfo_Get = (ADL2_DISPLAY_DISPLAYINFO_GET) GetProcAddress(hDLL,"ADL2_Display_DisplayInfo_Get");
		ADL2_Adapter_Primary_Set = (ADL2_ADAPTER_PRIMARY_SET) GetProcAddress(hDLL,"ADL2_Adapter_Primary_Set");
		ADL2_Adapter_Primary_Get = (ADL2_ADAPTER_PRIMARY_GET) GetProcAddress(hDLL,"ADL2_Adapter_Primary_Get");
		ADL2_Adapter_Active_Set = (ADL2_ADAPTER_ACTIVE_SET) GetProcAddress(hDLL,"ADL2_Adapter_Active_Set");
        if (NULL == ADL2_Main_ControlX2_Create ||
            NULL == ADL2_Main_Control_Destroy ||
            NULL == ADL2_Adapter_AdapterInfoX3_Get ||
            NULL == ADL2_Display_SLSMapConfig_Valid ||
			 NULL == ADL2_Display_SLSMapIndex_Get ||
			 NULL == ADL2_Display_SLSMapConfig_Get ||
			 NULL == ADL2_Display_Modes_Get ||
			 NULL == ADL2_Display_Modes_Set ||
			 NULL == ADL2_Display_DisplayMapConfig_Get ||
			 NULL == ADL2_Display_SLSMapConfig_Delete ||
			 NULL == ADL2_Display_SLSMapConfig_Create ||
			 NULL == ADL2_Display_SLSMapConfig_Rearrange ||
			 NULL == ADL2_Display_SLSMapConfig_SetState ||
			 NULL == ADL2_Display_DisplayMapConfig_Set ||
			 NULL == ADL2_Display_BezelOffset_Set ||
			 NULL == ADL2_Display_DisplayInfo_Get ||
			 NULL == ADL2_Adapter_Primary_Set ||
			 NULL == ADL2_Adapter_Primary_Get ||
			 NULL == ADL2_Adapter_Active_Set ||
			 NULL == ADL2_Display_PossibleMode_Get
			 )
		{
			PRINTF("Failed to get ADL function pointers\n");
			return FALSE;
		}
	}

    if (ADL_OK != ADL2_Main_ControlX2_Create(ADL_Main_Memory_Alloc, 1, &ADLContext_, ADL_THREADING_LOCKED))
	{
		PRINTF("ADL2_Main_ControlX2_Create() failed\n");
		return FALSE;
	}
	
	return TRUE;
}	

void deinitializeADL()
{
    if (NULL != ADL2_Main_Control_Destroy)
        ADL2_Main_Control_Destroy(ADLContext_);

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

    //ADLDisplayTarget* lpDisplayTarget = (ADLDisplayTarget*)ADL_Main_Memory_Alloc(iNumOfDisplays * sizeof(ADLDisplayTarget));
    //ZeroMemory(lpDisplayTarget, iNumOfDisplays * sizeof(ADLDisplayTarget));

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

        iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pModes);
        
		//Setting the SLSMAP Information
		memset(&slsMap, 0, sizeof(ADLSLSMap));

		slsMap.grid.iSLSGridColumn = iColumns;
		slsMap.grid.iSLSGridRow = iRows;
		
		slsMap.iAdapterIndex = iAdapterIndex;

		slsMap.iSLSMapIndex = iSLSMapIndex; 
		slsMap.iNumNativeMode = 0;
		slsMap.iNumBezelMode = 0;
		slsMap.iOrientation = pModes[0].iOrientation;
		slsMap.iSLSMapValue = 0;

        

        iRetVal = ADL2_Display_DisplayMapConfig_Get(ADLContext_, iAdapterIndex,
														&iNumDisplayMap, &lpDisplayMap, 
														&iNumDisplayTarget, &lpDisplayTarget, 
														ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );


		if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
		{
            iRetVal = ADL2_Display_SLSMapIndex_Get(ADLContext_, iAdapterIndex,
				iNumDisplayTarget,
				lpDisplayTarget,
				&iSLSMapIndex);

			if (ADL_OK == iRetVal && iSLSMapIndex != -1)
			{
				//This is a temporary workaround to enable SLS.
				//Set this variable to any value.
				SetEnvironmentVariable(EnvironmentVariable, "TRUE");

                iRetVal = ADL2_Display_SLSMapConfig_Get(ADLContext_, iAdapterIndex, iSLSMapIndex, &lpCreatedSLSMap,
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

                // this is used only for SLS builder; set "Enabled" in all other cases
                pSLSTargets[iCurrentSLSTarget].iSLSTargetValue = 0x0001;
                pSLSTargets[iCurrentSLSTarget].iSLSTargetMask = 0x0001;

				iCurrentSLSTarget++;
			}		
		}

        bool ok = false;
        
        int supportedLayoutModes = -1, reasonForNotSupport = -1, option = ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_CURRENTANGLE;
        // Validate Fit
        slsMap.iSLSMapValue |= ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FIT;
        int ret = ADL2_Display_SLSMapConfig_Valid(ADLContext_, iAdapterIndex, slsMap, iNumOfDisplays, pSLSTargets, &supportedLayoutModes, &reasonForNotSupport, option);
        if (ADL_OK == ret && 0 == reasonForNotSupport)
        {
            ok = true;
        }
        // Validate Fill
        if (!ok)
        {
            slsMap.iSLSMapValue ^= ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FIT;
            slsMap.iSLSMapValue |= ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL;
            int ret = ADL2_Display_SLSMapConfig_Valid(ADLContext_, iAdapterIndex, slsMap, iNumOfDisplays, pSLSTargets, &supportedLayoutModes, &reasonForNotSupport, option);
            if (ADL_OK == ret && 0 == reasonForNotSupport)
                ok = true;
        }

		if (!iSLSRearrange)
		{
            iRetVal = ADL2_Display_SLSMapConfig_Create(
                    ADLContext_,
					iAdapterIndex,
					slsMap,
					iNumOfDisplays,
					pSLSTargets,
					0, // bezel mode percent
					&iSLSMapIndexOut,
                    option
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
            iRetVal = ADL2_Display_SLSMapConfig_Rearrange(ADLContext_, iAdapterIndex, slsMap.iSLSMapIndex, iNumOfDisplays, pSLSTargets, lpCreatedSLSMap, option);
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

    iRetVal = ADL2_Display_DisplayMapConfig_Get(ADLContext_, iAdapterIndex,
													&iNumDisplayMap, &lpDisplayMap, 
													&iNumDisplayTarget, &lpDisplayTarget, 
													ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
        iRetVal = ADL2_Display_SLSMapIndex_Get(ADLContext_, iAdapterIndex,
			iNumDisplayTarget,
			lpDisplayTarget,
			&iSLSMapIndex);

		if (iSLSMapIndex != -1)
		{
			//This is a temporary workaround to enable SLS.
			//Set this variable to any value.
			SetEnvironmentVariable(EnvironmentVariable, "TRUE");

            iRetVal = ADL2_Display_SLSMapConfig_Get(ADLContext_, iAdapterIndex, iSLSMapIndex, &lpCreatedSLSMap,
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

                iRetVal = ADL2_Display_SLSMapConfig_SetState(ADLContext_, iAdapterIndex, iSLSMapIndex, 0);
			}		
		}
			
	}
	
	if (iRetVal == 0 && (0 != iHbezel || 0 != iVbezel))
	{
		iRetVal = ADL2_Display_SLSMapConfig_Create (
            ADLContext_,
			iAdapterIndex,
			lpCreatedSLSMap,
			iNumCreatedSLSTargets,
			lppCreatedSLSTargets,
			10, // bezel mode percent
			&iSLSMapIndexOut,
			ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_LANDSCAPE
			);

	

        iRetVal = ADL2_Display_SLSMapConfig_Get(ADLContext_, iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, &lpCreatedSLSMap,
														&iNumCreatedSLSTargets, &lppCreatedSLSTargets,
														&iNumCreatedSLSNativeModes, &lppCreatedSLSNativeModes,
														&iNumCreatedBezelModes, &lppCreatedBezelModes,
														&iNumCreatedTransientModes, &lppCreatedTransientModes,
														&iNumCreatedBezelOffsets, &lppCreatedBezelOffsets,
														ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE );

		
		if (iNumCreatedTransientModes > 0)
		{
			ADLMode *pTransientModes = NULL;
            iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pTransientModes);

			//lppCreatedTransientModes = (ADLBezelTransientMode*) malloc(iNumCreatedTransientModes * sizeof(ADLBezelTransientMode));
			for(iCurrentTransientMode = 0; iCurrentTransientMode < iNumCreatedTransientModes; iCurrentTransientMode++)
			{
				// we need to figure out the positioning of this display in the SLS
				pTransientModes[iCurrentTransientMode].iXRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iXRes;
				pTransientModes[iCurrentTransientMode].iYRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iYRes;
                iRetVal = ADL2_Display_Modes_Set(ADLContext_, iAdapterIndex, -1, 1, &pTransientModes[iCurrentTransientMode]);
				break;
			}

			if (iRetVal == 0)
			{
                iRetVal = ADL2_Display_SLSMapConfig_Get(ADLContext_, iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, &lpCreatedSLSMap,
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
                    iRetVal = ADL2_Display_BezelOffset_Set(ADLContext_, iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, iNumCreatedSLSTargets, lpAppliedBezelOffsets, lpCreatedSLSMap, ADL_DISPLAY_BEZELOFFSET_COMMIT | ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE);
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
        iRetVal = ADL2_Display_PossibleMode_Get(ADLContext_, iAdapterIndex, &iNumModesPriv, &lppModesPriv);
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
            iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pModes);
			if (iRetVal == ADL_OK)
			{
				pModes[0].iXRes = iXRes;
				pModes[0].iYRes = iYRes;
                iRetVal = ADL2_Display_Modes_Set(ADLContext_, iAdapterIndex, -1, 1, &pModes[0]);
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
    if (ADL_OK == ADL2_Adapter_Primary_Get(ADLContext_, &iCurrentAdapterIndex))
	{
		if (iCurrentAdapterIndex != iAdapterIndex && CanSetPrimary(iAdapterIndex ,iCurrentAdapterIndex))
		{
            ADL2_Adapter_Primary_Set(ADLContext_, iAdapterIndex);
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
            ADL2_Adapter_Active_Set(ADLContext_, iAdapterIndexes[i], 0, &active);
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
	
    ADL2_Display_Modes_Get(ADLContext_,iAdapterIndex, -1, &iNumModes, &pModes);
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
    ADL2_Display_DisplayMapConfig_Set(ADLContext_, iAdapterIndex, 1, &mapArray, iDisplaysCount, pDisplayTargets);

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

    iRetVal = ADL2_Display_DisplayMapConfig_Get(ADLContext_, iAdapterIndex,
														&iNumDisplayMap, &lpDisplayMap, 
														&iNumDisplayTarget, &lpDisplayTarget, 
														ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
        iRetVal = ADL2_Display_SLSMapIndex_Get(ADLContext_, iAdapterIndex,
			iNumDisplayTarget,
			lpDisplayTarget,
			&iSLSMapIndex);

		if (ADL_OK == iRetVal && iSLSMapIndex != -1)
		{
            iRetVal = ADL2_Display_SLSMapConfig_SetState(ADLContext_, iAdapterIndex, iSLSMapIndex, 0);
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
	int  iNumberAdapters=0, iNumDisplays;
    int  iAdapterIndex;
    int  iDisplayIndex[6];
	int iBusNumber;
	int i=0,j=0,k=0,l=0, iGPUfound=0, iDisplayFound=0, iGPUIndex=0,iCount=0, iGPUCounter=0;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
    int igpuBusIndexes[4];

	// Obtain the number of adapters for the system
    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
	{
	       PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
		   return 0;
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
                    if (ADL_OK != ADL2_Display_DisplayInfo_Get(ADLContext_, lpAdapterInfo[j].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
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

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
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

    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
    {
        PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
        return 0;
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
        if (ADL_OK != ADL2_Display_DisplayInfo_Get(ADLContext_, lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
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
    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	return TRUE;
}

int CanSetPrimary(int iAdapterIndex, int iCurrentPrimaryAdapterIndex)
{
	int  iNumberAdapters;
	int iBusNumber = 0, iCurrentPrimaryAdapterBusNumber = 0;
	int i;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;

    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
    {
        PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
        return 0;
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

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	if (iCurrentPrimaryAdapterBusNumber == iBusNumber)
	{
		return 1;
	}

	return 0;
}



