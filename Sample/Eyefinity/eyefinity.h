///
///  Copyright (c) 2008 - 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.h

#ifndef EYEFINITY_H_
#define EYEFINITY_H_

typedef struct _SimpleRectStruct {

	int iXOffset;
	int iYOffset;
	int iWidth;
	int iHeight;

} SimpleRectStruct;

typedef struct _EyefinityInfoStruct {

	int iSLSActive; // Indicates if Eyefinity is active for the operating system display
					// index passed into atiEyefinityGetConfigInfo(). 1 if enabled and 0 if disabled.

	int iSLSGridWidth;  // Contains width of the multi-monitor grid that makes up the Eyefinity Single Large Surface.
					    // For example, a 3 display wide by 2 high Eyefinity setup will return 3 for this entry.
	int iSLSGridHeight; // Contains height of the multi-monitor grid that makes up the Eyefinity Single Large Surface.
						// For example, a 3 display wide by 2 high Eyefinity setup will return 2 for this entry.

	int iSLSWidth;  // Contains width in pixels of the multi-monitor Single Large Surface. The value returned is
	                // a function of the width of the SLS grid, of the horizontal resolution of each display, and
	                // of whether or not bezel compensation is enabled.
	int iSLSHeight; // Contains height in pixels of the multi-monitor Single Large Surface. The value returned is
	                // a function of the height of the SLS grid, of the vertical resolution of each display, and
	                // of whether or not bezel compensation is enabled.

	int iBezelCompensatedDisplay; // Indicates if bezel compensation is used for the current SLS display area.
	                              // 1 if enabled, and 0 if disabled.

} EyefinityInfoStruct;

typedef struct _DisplayInfoStruct {

	int iGridXCoord; // Contains horizontal SLS grid coordinate of the display. The value is zero based with
	                 // increasing values from left to right of the overall SLS grid. For example, the left-most
	                 // display of a 3x2 Eyefinity setup will have the value 0, and the right-most will have
	                 // the value 2.
	int iGridYCoord; // Contains vertical SLS grid coordinate of the display. The value is zero based with
	                 // increasing values from top to bottom of the overall SLS grid. For example, the top
	                 // display of a 3x2 Eyefinity setup will have the value 0, and the bottom will have the
	                 // value 1.

	SimpleRectStruct displayRect;        // Contains the base offset and dimensions in pixels of the SLS rendering
	                                     // area associated with this display. If bezel compensation is enabled, this
	                                     // area will be larger than what the display can natively present to account
	                                     // for bezel area. If bezel compensation is disabled, this area will be equal
	                                     // to what the display can support natively. 

	SimpleRectStruct displayRectVisible; // Contains the base offset and dimensions in pixels of the SLS rendering area
	                                     // associated with this display that is visible to the end user. If bezel
	                                     // compensation is enabled, this area will be equal to what the display can
	                                     // natively, but smaller that the area described in the displayRect entry. If
	                                     // bezel compensation is disabled, this area will be equal to what the display
	                                     // can support natively and equal to the area described in the displayRect entry.
	                                     // Developers wishing to place UI, HUD, or other game assets on a given display
	                                     // so that it is visible and accessible to end users need to locate them inside
	                                     // of the region defined by this rect.

	int iPreferredDisplay; // Indicates whether or not this display is the preferred one for rendering of
	                       // game HUD and UI elements. Only one display out of the whole SLS grid will have
	                       // this be true if it is the preferred display and 0 otherwise. Developers wishing
	                       // to place specific UI, HUD, or other game assets on a given display so that it
	                       // is visible and accessible to end users need to locate them inside of the region
	                       // defined by this rect.

} DisplayInfoStruct;

// Description
//   Function used to query Eyefinity configuration state information relevant to ISVs. State info returned
//   includes: whether Eyefinity is enabled or not, SLS grid configuration, SLS dimensions, whether bezel
//   compensation is enabled or not, SLS grid coordinate for each display, total rendering area for each
//   display, visible rendering area for each display, and a preferred display flag.
//
// Input params
//   OSDisplayName - Operating system specific display name. On Windows operating systems,
//                     the value can be queried using the EnumDisplayDevices() API.
//
// Output params
// 	 pEyefinitiInfo       – This is a pointer to a EyefinitiInfoStruct structure that contains system Eyefinity
//                          configuration information.
//   lpNumDisplaysInfo    – Pointer to the number of DisplayInfoStruct structures stored in the returned
//                          lppDisplayInfoStruct array. The value returned is equal to the number of displays
//                          used for the Eyefinity setup. 
//   lppDisplayInfoStruct – Pointer to an array of DisplayInfoStruct structures that contains per display
//                          Eyefinity configuration information.
//
// Return code
//   1 - Success
//   0 - Failure
extern int atiEyefinityGetConfigInfo ( char OSDisplayName[], EyefinityInfoStruct *lpEyefinityInfo, int *lpNumDisplaysInfo, DisplayInfoStruct **lppDisplaysInfo );

// Description
//   Function used release the code from eyefinity.cpp that manages the ADL lib.
//
// Input params
//   lppDisplaysInfo - pointer to array of DisplayInfo structures that contain per display Eyefinity state info
//
// Output params
//   None
//
// Return code
//   1 - Success
//   0 - Failure
extern int atiEyefinityReleaseConfigInfo ( DisplayInfoStruct **lppDisplaysInfo );

#endif /* EYEFINITY_H_ */
