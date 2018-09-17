///
///  Copyright (c) 2008 - 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.h

#ifndef EYEFINITY_H_
#define EYEFINITY_H_

#include "..\..\include\adl_sdk.h"
#include <vector>

using std::vector;

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

class TopologyDisplay
{
private:
    ADLDisplayID DisplayID_;
    
    // Angle, relative to desktop
    int Angle_;

    // Size
    int Width_;
    int Height_;

    // Position relative to the Desktop
    int Left_;
    int Top_;

    //TODO: position in the SLS grid (row/column; 0-based)
    int Row_;
    int Col_;

    // the badge ID is a number that allows a single monitor/display to be identified to user
    int BadgeID_;

public:
    // Default c-tor
    TopologyDisplay() :
        DisplayID_(),
        Angle_(0),
        Width_(0),
        Height_(0),
        Left_(0),
        Top_(0),
        Row_(0),
        Col_(0),
        BadgeID_(0)
    {
    }

    // Parameter constructor
    // displayID - displayID
    // angle_	- angle for this display relative to the desktop
    // width_	- width for the display within its desktop
    // height_	- height for the display within its desktop
    // left_	- left position for the display relative to left of its desktop
    // top_		- top position for the display relative to top of its desktop
    // row_		- 0-based grid position (for SLS): row number
    // col_		- 0-based grid position (for SLS): column number
    // BadgeID_ - 0 : undefined, 0 < display badge number
    TopologyDisplay(ADLDisplayID displayID_, int angle_,
        int width_, int height_,
        int left_, int top_, int row_, int col_) :
        DisplayID_(displayID_),
        Angle_(angle_),
        Width_(width_),
        Height_(height_),
        Left_(left_),
        Top_(top_),
        Row_(row_),
        Col_(col_),
        BadgeID_(0)
    {
    }

    // Copy c-tor
    TopologyDisplay(const TopologyDisplay& other_) :
        DisplayID_(other_.DisplayID_),
        Angle_(other_.Angle_),
        Width_(other_.Width_),
        Height_(other_.Height_),
        Left_(other_.Left_),
        Top_(other_.Top_),
        Row_(other_.Row_),
        Col_(other_.Col_),
        BadgeID_(other_.BadgeID_)
    {
    }

    // Destructor
    ~TopologyDisplay()
    {
    }

    // move c-tor
    TopologyDisplay(TopologyDisplay&& other_) :
        DisplayID_(other_.DisplayID_),
        Angle_(other_.Angle_),
        Width_(other_.Width_),
        Height_(other_.Height_),
        Left_(other_.Left_),
        Top_(other_.Top_),
        Row_(other_.Row_),
        Col_(other_.Col_),
        BadgeID_(other_.BadgeID_)
    {
    }

    // Copy assign operator
    TopologyDisplay operator= (const TopologyDisplay& other_)
    {
        if (this != &other_)
        {
            //plain old data: copy other into us
            DisplayID_ = other_.DisplayID_;
            Angle_ = other_.Angle_;
            Width_ = other_.Width_;
            Height_ = other_.Height_;
            Left_ = other_.Left_;
            Top_ = other_.Top_;
            Row_ = other_.Row_;
            Col_ = other_.Col_;
            BadgeID_ = other_.BadgeID_;
        }
        return *this;
    }

    // Move assign operator
    TopologyDisplay operator= (const TopologyDisplay&& other_)
    {
        if (this != &other_)
        {
            DisplayID_ = other_.DisplayID_;
            Angle_ = other_.Angle_;
            Width_ = other_.Width_;
            Height_ = other_.Height_;
            Left_ = other_.Left_;
            Top_ = other_.Top_;
            Row_ = other_.Row_;
            Col_ = other_.Col_;
            BadgeID_ = other_.BadgeID_;
        }
        return *this;
    }
    //public getter methods
    // Get displayID
    ADLDisplayID DisplayID() const { return DisplayID_; }
    // Get Angle, relative to desktop
    int Angle() const { return Angle_; }
    // Get Width
    int Width() const { return Width_; }
    // Get Height
    int Height() const { return Height_; }
    // Get left position relative to the Desktop
    int Left() const { return Left_; }
    // Get top position relative to the Desktop
    int Top() const { return Top_; }
    //Get row position in the SLS grid; 0-based
    int Row() const { return Row_; }
    //Get col position in the SLS grid; 0-based
    int Col() const { return Col_; }
    //Get badge number of desktop/display; 0 undefined;0 < display badge number
    void SetBadgeID(int id) { BadgeID_ = id; }
    //Get badge number of desktop/display; 0 undefined;0 < display badge number
    int BadgeID() const { return BadgeID_; }
};

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
