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
double DisplaySetting::GetLinkRate(int linkRateIndex)
{
    double res = -1;

    switch (linkRateIndex)
    {
    case DPLinkRate_RBR:
        res = 1.62;
        break;
    case DPLinkRate_2_16Gbps:
        res = 2.16;
        break;
    case DPLinkRate_2_43Gbps:
        res = 2.43;
        break;
    case DPLinkRate_HBR:
        res = 2.70;
        break;
    case DPLinkRate_4_32Gbps:
        res = 4.32;
        break;
    case DPLinkRate_HBR2:
        res = 5.40;
        break;
    case DPLinkRate_HBR3:
        res = 8.10;
        break;
    }
    return res;
}

bool DisplaySetting::GetDCESettings(const ADLDisplayID& displayID_, ADLDceSettings& dceSettings_)
{
    if (ADL_OK == ADL2_Display_DCE_Get(context_, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &dceSettings_))
    {
        return true;
    }
    return false;

}
bool DisplaySetting::SetDCESettings(const ADLDisplayID& displayID_, ADLDceSettings& dceSettings_)
{
    if (ADL_OK == ADL2_Display_DCE_Set(context_, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &dceSettings_))
    {
        return true;
    }
    return false;
}

bool DisplaySetting::SetIntegerScalingState(const ADLDisplayID& displayID_, bool OnOff)
{
			ADLDisplayProperty DisplayProperty;
			memset(&DisplayProperty, 0, sizeof(DisplayProperty));
			DisplayProperty.iSize = sizeof(DisplayProperty);
			DisplayProperty.iPropertyType = ADL_DL_DISPLAYPROPERTY_TYPE_INTEGER_SCALING;
			DisplayProperty.iCurrent = OnOff;

			if (ADL_OK != ADL2_Display_Property_Set(context_, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &DisplayProperty))
			{
				printf("Failed to set integer scaling");
				return false;
			}

			DisplayProperty.iCurrent = -1;
			if (ADL_OK != ADL2_Display_Property_Get(context_, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &DisplayProperty))
			{
				printf("not expected; something has failed ? ? ? ");
				return false;
			}
			OnOff = (1 == DisplayProperty.iCurrent);
			DisplayProperty.iCurrent = OnOff ? 1 : 0;

			return true;
		
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

