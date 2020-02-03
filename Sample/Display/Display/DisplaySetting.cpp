#include "stdafx.h"
#include "DisplaySetting.h"

DisplaySetting::DisplaySetting()
{
}


DisplaySetting::~DisplaySetting()
{
}

bool DisplaySetting::GetVirtualResolutionState(const ADLDisplayID& displayID_)
{
    ADLDisplayProperty DisplayProperty;
    memset(&DisplayProperty, 0, sizeof(DisplayProperty));
    DisplayProperty.iSize = sizeof(DisplayProperty);
    DisplayProperty.iPropertyType = ADL_DL_DISPLAYPROPERTY_TYPE_DOWNSCALE;
    DisplayProperty.iDefault = -1;
    DisplayProperty.iCurrent = -1;

    if (ADL_OK == ADL2_Display_Property_Get(context_, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &DisplayProperty))
    {
        return DisplayProperty.iCurrent == 1;
    }
    return false;
}

bool DisplaySetting::SetVirtualResolutionState(const ADLDisplayID& displayID_, bool OnOff)
{
    ADLDisplayProperty DisplayProperty;
    memset(&DisplayProperty, 0, sizeof(DisplayProperty));
    DisplayProperty.iSize = sizeof(DisplayProperty);
    DisplayProperty.iPropertyType = ADL_DL_DISPLAYPROPERTY_TYPE_DOWNSCALE;
    DisplayProperty.iDefault = -1;
    DisplayProperty.iCurrent = -1;

    if (GetVirtualResolutionState(displayID_) == OnOff)
        return true;

    DisplayProperty.iCurrent = OnOff;
    if (ADL_OK == ADL2_Display_Property_Set(context_, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &DisplayProperty))
    {
        if (GetVirtualResolutionState(displayID_) == OnOff)
            return true;
    }

    return false;
}

