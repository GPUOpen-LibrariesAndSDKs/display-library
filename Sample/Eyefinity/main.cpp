///
///  Copyright (c) 2008 - 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.c

#include <windows.h>
#include <stdio.h>
#include "eyefinity.h"

#define DISPLAY_NAME_LEN 32

int main (int c,char* k[],char* s[])
{
	char OSDisplayName[DISPLAY_NAME_LEN];	
    int iNumDisplaysInfo = 0;
	EyefinityInfoStruct eyefinityInfo = {0};
	DisplayInfoStruct *pDisplaysInfo = NULL;
	
	 // Get the default active display
 	int iDevNum = 0;
	int dwFlags = 0;
	DISPLAY_DEVICE displayDevice;

    memset(&OSDisplayName,'\0', DISPLAY_NAME_LEN);
	displayDevice.cb = sizeof(displayDevice);

	while ( EnumDisplayDevices(0, iDevNum, &displayDevice, 0) ) {
		if (0 != (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ) {
			memcpy(OSDisplayName, displayDevice.DeviceName, DISPLAY_NAME_LEN);
			break;
		}
		iDevNum++;
	}

	// Find out if this display has an Eyefinity config enabled
	if (TRUE == atiEyefinityGetConfigInfo( OSDisplayName, &eyefinityInfo, &iNumDisplaysInfo, &pDisplaysInfo )) {
		if (TRUE == eyefinityInfo.iSLSActive) {
			int iCurrentDisplaysInfo = 0;

			printf ( "\nEYEFINITY ENABLED for display name %s:\n", OSDisplayName);
			printf ( " SLS grid is %i displays wide by %i displays tall.\n", eyefinityInfo.iSLSGridWidth, eyefinityInfo.iSLSGridHeight );
			printf ( " SLS resolution is %ix%i pixels.\n", eyefinityInfo.iSLSWidth, eyefinityInfo.iSLSHeight );

			if (TRUE == eyefinityInfo.iBezelCompensatedDisplay ) {
				printf ( " SLS is bezel-compensated.\n" );
			}

			for ( iCurrentDisplaysInfo = 0; iCurrentDisplaysInfo < iNumDisplaysInfo; iCurrentDisplaysInfo++ ) {
				printf ( "\nDisplay %i\n", iCurrentDisplaysInfo);

				if ( TRUE == pDisplaysInfo[iCurrentDisplaysInfo].iPreferredDisplay ) {
					printf ( " Preferred/main monitor\n");
				}

				printf ( " SLS grid coord [%i,%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].iGridXCoord, pDisplaysInfo[iCurrentDisplaysInfo].iGridYCoord );
				printf ( " Base coord [%i,%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iXOffset, pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iYOffset );
				printf ( " Dimensions [%ix%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iWidth, pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iHeight );
				printf ( " Visible base coord [%i,%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iXOffset, pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iYOffset );
				printf ( " Visible dimensions [%ix%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iWidth, pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iHeight );
			}
		}
		else {
			printf ( "\nEYEFINITY DISABLED for display name %s.\n", OSDisplayName);
		}
	} else {
		printf ( "Eyefinity configuration query failed for display name %s.\n", OSDisplayName);
	}        
    if (iNumDisplaysInfo > 0) {
        atiEyefinityReleaseConfigInfo(&pDisplaysInfo);
    }
    return 0;
}