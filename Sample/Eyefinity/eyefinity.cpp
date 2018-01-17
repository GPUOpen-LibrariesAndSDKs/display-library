///
///  Copyright (c) 2008 - 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.c

#include <windows.h>
#include <stdio.h>
#include "..\..\include\adl_sdk.h"
#include "eyefinity.h"

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int ( *ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )();
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) ( LPAdapterInfo, int );
typedef int ( *ADL_DISPLAY_COLORCAPS_GET ) ( int, int, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_GET ) ( int, int, int, int *, int *, int *, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_SET ) ( int, int, int, int );
typedef int ( *ADL_DISPLAY_DISPLAYINFO_GET ) ( int, int *, ADLDisplayInfo **, int );
typedef int ( *ADL_DISPLAY_SLSMAPINDEX_GET ) ( int, int, ADLDisplayTarget *, int * );
typedef int ( *ADL_DISPLAY_SLSMAPCONFIG_GET	) ( int, int, ADLSLSMap*, int*, ADLSLSTarget**, int*, ADLSLSMode**, int*, ADLBezelTransientMode**, int*, ADLBezelTransientMode**, int*, ADLSLSOffset**, int );
typedef int ( *ADL_DISPLAY_MODES_GET ) ( int, int, int*, ADLMode ** );
typedef int ( *ADL_DISPLAY_DISPLAYMAPCONFIG_GET ) ( int, int*, ADLDisplayMap**, int*, ADLDisplayTarget**, int );

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

int atiEyefinityReleaseConfigInfo ( DisplayInfoStruct **lppDisplaysInfo )
{
	ADL_Main_Memory_Free( (void**)lppDisplaysInfo );

	return TRUE;
}

int atiEyefinityGetConfigInfo ( char OSDisplayName[], EyefinityInfoStruct *lpEyefinityInfo, int *lpNumDisplaysInfo, DisplayInfoStruct **lppDisplaysInfo )
{
    HINSTANCE hDLL;

    ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
    ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
    ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
	ADL_DISPLAY_SLSMAPINDEX_GET		 ADL_Display_SLSMapIndex_Get = NULL;
	ADL_DISPLAY_SLSMAPCONFIG_GET	 ADL_Display_SLSMapConfig_Get = NULL;
	ADL_DISPLAY_MODES_GET			 ADL_Display_Modes_Get = NULL;
	ADL_DISPLAY_DISPLAYMAPCONFIG_GET ADL_Display_DisplayMapConfig_Get = NULL;

    int iNumberAdapters = 0;
	int iCurrentAdapter = 0;
    LPAdapterInfo lpAdapterInfo = NULL;

	int iNumDisplayTarget = 0;
	ADLDisplayTarget *lpDisplayTarget = NULL;

	int iNumDisplayMap = 0;
	ADLDisplayMap *lpDisplayMap = NULL;
	int iSLSMapIndex = 0;

	int iNumSLSTarget = 0;
	int iCurrentSLSTarget = 0;
	ADLSLSTarget *lpSLSTarget = NULL;

	int iNumNativeMode = 0;
	int iCurrentNativeMode = 0;
	ADLSLSMode *lpNativeMode = NULL;

	int iNumBezelMode = 0;
	int iCurrentBezelMode = 0;
	ADLBezelTransientMode *lpBezelMode = NULL;

	int iNumTransientMode = 0;
	ADLBezelTransientMode *lpTransientMode = NULL;

	int iNumSLSOffset = 0;
	int iCurrentSLSOffset = 0;
	ADLSLSOffset *lpSLSOffset = NULL;

	int iNumModes = 0;
	ADLMode *lpModes = NULL;

	ADLSLSMap SLSMap = {0};

	DisplayInfoStruct *lpDisplaysInfoCurrent = NULL;

	int iCurrentDisplayTarget = 0;
	int iEyefinityEnabled = 0;
	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
	int iDisplaysInPortraitMode = 0;
	int iPreferredGridIndexX = 0;
	int iPreferredGridIndexY = 0;
	int iBezelCompensatedDisplay = 0;
	int iReturnValue = TRUE;
	int iFoundMatch = FALSE;
	LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";

	// Validate input params
	if (NULL == lpEyefinityInfo ||
		NULL == lpNumDisplaysInfo ||
		NULL == lppDisplaysInfo )
	{
		return FALSE;
	}

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
		ADL_Display_DisplayMapConfig_Get = (ADL_DISPLAY_DISPLAYMAPCONFIG_GET)GetProcAddress(hDLL,"ADL_Display_DisplayMapConfig_Get");

		if ( NULL == ADL_Main_Control_Create ||
			 NULL == ADL_Main_Control_Destroy ||
			 NULL == ADL_Adapter_NumberOfAdapters_Get ||
			 NULL == ADL_Adapter_AdapterInfo_Get ||
			 NULL == ADL_Display_SLSMapIndex_Get ||
			 NULL == ADL_Display_SLSMapConfig_Get ||
			 NULL == ADL_Display_Modes_Get ||
			 NULL == ADL_Display_DisplayMapConfig_Get )
		{
			PRINTF("Failed to get ADL function pointers\n");
			return FALSE;
		}
	}

    // Initialize ADL. The second parameter is 1, which means:
    // retrieve adapter information only for adapters that are physically present and enabled in the system
    if ( ADL_OK != ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1) )
	{
		PRINTF("ADL_Main_Control_Create() failed\n");
		return FALSE;
	}

    // Obtain the number of adapters for the system
    if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters ) )
	{
		PRINTF("ADL_Adapter_NumberOfAdapters_Get() failed\n");
		return FALSE;
	}

	// Query the list of adapters & their info
    if ( 0 != iNumberAdapters )
    {
        lpAdapterInfo = (LPAdapterInfo)malloc ( sizeof (AdapterInfo) * iNumberAdapters );
		if ( NULL == lpAdapterInfo )
		{
			PRINTF("lpAdapterInfo allocation failed\n");
			return FALSE;
		}
        memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

        // Get the AdapterInfo structure for all adapters in the system
        if ( ADL_OK != ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters) )
		{
			PRINTF("ADL_Adapter_AdapterInfo_Get() failed\n");
			return FALSE;
		}
    }
	else
	{
		PRINTF("ADL_Adapter_NumberOfAdapters_Get() returned no adapters\n");
		return FALSE;
	}

    // For all available adapters in the system
    for ( iCurrentAdapter = 0; iCurrentAdapter < iNumberAdapters; iCurrentAdapter++ )
    {
		// If this adapter/display isn't the one the calling app is asking about,
		// jump ahead to the next one in the list
		if (0 != memcmp(lpAdapterInfo[iCurrentAdapter].strDisplayName,OSDisplayName,sizeof(OSDisplayName)))
		{
			continue;
		}

		// Get the list of display targets associated with this adapater
		if (ADL_OK != ADL_Display_DisplayMapConfig_Get( lpAdapterInfo[iCurrentAdapter].iAdapterIndex,
														&iNumDisplayMap, &lpDisplayMap, 
														&iNumDisplayTarget, &lpDisplayTarget, 
														ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO ) )
		{
			PRINTF("ADL_Display_DisplayMapConfig_Get() failed\n");
			iReturnValue = FALSE;
			break;
		}

		// If we don't have more than one display, Eyefinity cannot be on.
		if ( iNumDisplayTarget < 2 )
		{
			break;
		}

		// Get the Eyefinity/SLS display map index
		if ( ADL_OK != ADL_Display_SLSMapIndex_Get (lpAdapterInfo[iCurrentAdapter].iAdapterIndex, iNumDisplayTarget, lpDisplayTarget, &iSLSMapIndex) )
		{
			break;
		}

		//This is a temporary workaround to enable SLS.
		//Set this variable to any value.
		SetEnvironmentVariable(EnvironmentVariable, "TRUE");

		// Get the list of modes supported by the current Eyefinity/SLS index:
		// for now, we only care about native and bezel-compensated modes
		if ( ADL_OK != ADL_Display_SLSMapConfig_Get ( lpAdapterInfo[iCurrentAdapter].iAdapterIndex, iSLSMapIndex, &SLSMap,
													  &iNumSLSTarget, &lpSLSTarget,
													  &iNumNativeMode, &lpNativeMode,
													  &iNumBezelMode, &lpBezelMode,
													  &iNumTransientMode, &lpTransientMode,
													  &iNumSLSOffset, &lpSLSOffset,
													  ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE ) )
		{
			PRINTF("ADL_Display_SLSMapConfig_Get() failed\n");
			iReturnValue = FALSE;
			break;
		}

		// First check that the number of grid entries is equal to the number
		// of display targets associated with this adapter & SLS surface.
		if ( iNumDisplayTarget != (SLSMap.grid.iSLSGridColumn * SLSMap.grid.iSLSGridRow) )
		{
			PRINTF("Number of display targets returned is not equal to the SLS grid size.\n");
			iReturnValue = FALSE;
			break;
		}

		// Figure out which display is the "preferred" one for rendering of elements
		// such as UIs, HUDs, prerendered cinematics, etc. This is user selectable in
		// CCC, but the interface to do this is so obscure that most end users will never 
		// figure out that they can control this. To avoid problems with game menus
		// showing up on the wrong display, simply enforce which display should be the
		// "preferred" one for showing game UI & HUD elements.
		{
			// Try to use the center-most column for gaming UI & HUD elements
			switch ( SLSMap.grid.iSLSGridColumn )
			{
			case 0:
			case 1:
			case 2:
			default:
				iPreferredGridIndexX = 0;
				break;
			case 3:
			case 4:
				iPreferredGridIndexX = 1;
				break;
			case 5:
			case 6:
				iPreferredGridIndexX = 2;
				break;
			}

			// Always default to the bottom row for gaming UI & HUD elements
			iPreferredGridIndexY = SLSMap.grid.iSLSGridRow - 1;
		}

		// For all the display targets reported by ADL_Display_DisplayMapConfig_Get()
		for ( iCurrentDisplayTarget=0; iCurrentDisplayTarget<iNumDisplayTarget; iCurrentDisplayTarget++ )
		{
			SimpleRectStruct *lpDisplayRect = NULL;
			SimpleRectStruct *lpDisplayRectVisible = NULL;

			// Get their current display mode for this adapter/display combination
			{
				ADL_Main_Memory_Free((void**)&lpModes);
				if ( ADL_OK != ADL_Display_Modes_Get( lpAdapterInfo[iCurrentAdapter].iAdapterIndex,
													  lpDisplayTarget[iCurrentDisplayTarget].displayID.iDisplayLogicalIndex,
													  &iNumModes, &lpModes ) )
				{
					PRINTF("ADL_Display_Modes_Get() failed\n");
					iReturnValue = FALSE;
					break;
				}
			}

			// If Eyefinity is enabled for this adapter, then the display mode of an
			// attached display target will match one of the SLS display modes reported by
			// ADL_Display_SLSMapConfig_Get(). The match will either be with "native" SLS 
			// modes (which are not bezel-compensated), or with "bezel" SLS modes which are.
			// 
			// So, simply compare current display mode against all the ones listed for the
			// SLS native or bezel-compensated modes: if there is a match, then the mode
			// currently used by this adapter is an Eyefinity/SLS mode, and Eyefinity is enabled.
			{
				// First check the native SLS mode list
				for ( iCurrentNativeMode=0; iCurrentNativeMode<iNumNativeMode; iCurrentNativeMode++)
				{
					if ( lpModes->iXRes == lpNativeMode[iCurrentNativeMode].displayMode.iXRes &&
						 lpModes->iYRes == lpNativeMode[iCurrentNativeMode].displayMode.iYRes )
					{
						iEyefinityEnabled |= 1 << iCurrentDisplayTarget;
						break;
					}
				}

				// If no match was found, check the bezel-compensated SLS mode list
				if ( 0 == ( iEyefinityEnabled & (1 << iCurrentDisplayTarget) ) )
				{
					for ( iCurrentBezelMode=0; iCurrentBezelMode<iNumBezelMode; iCurrentBezelMode++)
					{
						if ( lpModes->iXRes == lpBezelMode[iCurrentBezelMode].displayMode.iXRes &&
							 lpModes->iYRes == lpBezelMode[iCurrentBezelMode].displayMode.iYRes )
						{
							iEyefinityEnabled |= 1 << iCurrentDisplayTarget;
							iBezelCompensatedDisplay = TRUE;
							break;
						}
					}
				}
			}

			// Eyefinity is enabled for this display
			if ( 0 != ( iEyefinityEnabled & (1 << iCurrentDisplayTarget) ) )
			{
				// If this is the first round through the display target loop,
				// make sure we have an array of DisplayInfo structs allocated.
				{
					if ( *lppDisplaysInfo == NULL )
					{
						*lppDisplaysInfo = (DisplayInfoStruct*)malloc ( sizeof(DisplayInfoStruct) * iNumDisplayTarget );
						if ( NULL == *lppDisplaysInfo )
						{
							PRINTF("ppDisplaysInfo allocation failed\n");
							iReturnValue = FALSE;
							break;
						}
						memset ( *lppDisplaysInfo,'\0', sizeof(DisplayInfoStruct) * iNumDisplayTarget );
					}
				}

				// Find the SLSTarget that maps to the current display target
				{
					for ( iCurrentSLSTarget=0; iCurrentSLSTarget<iNumSLSTarget; iCurrentSLSTarget++)
					{
						if ( lpDisplayTarget[iCurrentDisplayTarget].displayID.iDisplayLogicalIndex == lpSLSTarget[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex )
						{
							iFoundMatch = TRUE;
							break;
						}
					}

					if ( FALSE == iFoundMatch )
					{
						PRINTF("Failed to find lpSLSTarget array match\n");
						iReturnValue = FALSE;
						break;
					}
				}

				// Set some vars for later
				{
					iDisplayWidth = lpModes->iXRes;
					iDisplayHeight = lpModes->iYRes;
					if ( lpModes->iOrientation != 0 && lpModes->iOrientation != 180 )
					{
						iDisplaysInPortraitMode = TRUE;
					}
				}

				// Now fill out the DisplayInfo structure passed in
				// by the caller for this display
				{
					lpDisplaysInfoCurrent = &((*lppDisplaysInfo)[iCurrentDisplayTarget]);

					// Set grid coordinates
					lpDisplaysInfoCurrent->iGridXCoord = lpSLSTarget[iCurrentSLSTarget].iSLSGridPositionX;
					lpDisplaysInfoCurrent->iGridYCoord = lpSLSTarget[iCurrentSLSTarget].iSLSGridPositionY;

					
					// Set display rect dimensions & offset
					{
						// If the display is in portrait mode, flip the width and height since
						// ADL still reports resolutions as though the display is in landscape
						// mode.
						if ( FALSE == iDisplaysInPortraitMode )
						{
							lpDisplaysInfoCurrent->displayRect.iWidth = lpModes->iXRes / SLSMap.grid.iSLSGridColumn;
							lpDisplaysInfoCurrent->displayRect.iHeight = lpModes->iYRes / SLSMap.grid.iSLSGridRow;
						}
						else
						{
							lpDisplaysInfoCurrent->displayRect.iWidth = lpModes->iYRes / SLSMap.grid.iSLSGridColumn;
							lpDisplaysInfoCurrent->displayRect.iHeight = lpModes->iXRes / SLSMap.grid.iSLSGridRow;
						}

						lpDisplaysInfoCurrent->displayRect.iXOffset = lpDisplaysInfoCurrent->displayRect.iWidth * lpSLSTarget[iCurrentSLSTarget].iSLSGridPositionX;
						lpDisplaysInfoCurrent->displayRect.iYOffset = lpDisplaysInfoCurrent->displayRect.iHeight * lpSLSTarget[iCurrentSLSTarget].iSLSGridPositionY;
					}

					// Set visible display rect dimensions & offset
					if ( TRUE == iBezelCompensatedDisplay )
					{
						// Find the SLSOffset array entry that maps to the current display target and display mode
						{
							for ( iCurrentSLSOffset=0; iCurrentSLSOffset<iNumSLSOffset; iCurrentSLSOffset++)
							{
								if ( lpBezelMode[iCurrentBezelMode].iSLSModeIndex == lpSLSOffset[iCurrentSLSOffset].iBezelModeIndex &&
									 lpDisplayTarget[iCurrentDisplayTarget].displayID.iDisplayLogicalIndex == lpSLSOffset[iCurrentSLSOffset].displayID.iDisplayLogicalIndex )
								{
									iFoundMatch = TRUE;
									break;
								}
							}

							if ( FALSE == iFoundMatch )
							{
								PRINTF("Failed to find lpSLSOffset array match\n");
								iReturnValue = FALSE;
								break;
							}
						}

						lpDisplaysInfoCurrent->displayRectVisible.iWidth = lpSLSOffset[iCurrentSLSOffset].iDisplayWidth;
						lpDisplaysInfoCurrent->displayRectVisible.iHeight = lpSLSOffset[iCurrentSLSOffset].iDisplayHeight;
						lpDisplaysInfoCurrent->displayRectVisible.iXOffset = lpSLSOffset[iCurrentSLSOffset].iBezelOffsetX;
						lpDisplaysInfoCurrent->displayRectVisible.iYOffset = lpSLSOffset[iCurrentSLSOffset].iBezelOffsetY;
					}
					else
					{
						lpDisplaysInfoCurrent->displayRectVisible = lpDisplaysInfoCurrent->displayRect;
					}

					// Set the preferred display flag
					if ( iPreferredGridIndexX == lpSLSTarget[iCurrentSLSTarget].iSLSGridPositionX &&
						 iPreferredGridIndexY == lpSLSTarget[iCurrentSLSTarget].iSLSGridPositionY )
					{
						lpDisplaysInfoCurrent->iPreferredDisplay = TRUE;
					}
				}
			}
		}

		// Finally, fill out the Eyefinity info struct passed in
		if ( 0 != iEyefinityEnabled &&
			 TRUE == iReturnValue )
		{
			lpEyefinityInfo->iSLSActive = TRUE;
			lpEyefinityInfo->iSLSGridWidth = SLSMap.grid.iSLSGridColumn;
			lpEyefinityInfo->iSLSGridHeight = SLSMap.grid.iSLSGridRow;

			if ( FALSE == iDisplaysInPortraitMode )
			{
				lpEyefinityInfo->iSLSWidth = iDisplayWidth;
				lpEyefinityInfo->iSLSHeight = iDisplayHeight;
			}
			else
			{
				lpEyefinityInfo->iSLSWidth = iDisplayHeight;
				lpEyefinityInfo->iSLSHeight = iDisplayWidth;
			}

			if ( TRUE == iBezelCompensatedDisplay )
			{
				lpEyefinityInfo->iBezelCompensatedDisplay = TRUE;
			}

			*lpNumDisplaysInfo = iNumDisplayTarget;
		}

		// If we get this far, then we can just break out of the display loop 
		// since we've found and processed the display the caller inquired about.
		break;
	}

	ADL_Main_Memory_Free ((void**)&lpModes );
	ADL_Main_Memory_Free ((void**)&lpSLSTarget );
	ADL_Main_Memory_Free ((void**)&lpNativeMode );
	ADL_Main_Memory_Free ((void**)&lpBezelMode );
	ADL_Main_Memory_Free ((void**)&lpTransientMode );
	ADL_Main_Memory_Free ((void**)&lpSLSOffset );
	ADL_Main_Memory_Free ((void**)&lpDisplayMap );
	ADL_Main_Memory_Free ((void**)&lpDisplayTarget );
	ADL_Main_Memory_Free ((void**)&lpAdapterInfo );

    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
    if ( NULL != ADL_Main_Control_Destroy )
        ADL_Main_Control_Destroy ();

    FreeLibrary(hDLL);

	if ( FALSE == iReturnValue )
	{
		atiEyefinityReleaseConfigInfo( lppDisplaysInfo );

		*lpNumDisplaysInfo = 0;

		memset ( lpEyefinityInfo, '\0', sizeof(lpEyefinityInfo) );
	}

    return iReturnValue;
}