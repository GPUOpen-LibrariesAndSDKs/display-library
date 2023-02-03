#include "stdafx.h"
#include "GPUScaling.h"
#include "Registry.h"

/* ADL headers */
#include "Adl.h"
#include <string>
#include <xstring>
#include <sstream>

//BEGIN_AMD_NAMESPACE

const std::wstring CurrentImageScale_Key = L"CurrentImageScale";

GPUScaling::GPUScaling(ADL_CONTEXT_HANDLE ctx) 
    : context_(ctx)
    , hkcuRegistry_(new Registry(HKEY_CURRENT_USER, TEXT("SOFTWARE\\AMD\\CN\\DisplayScaling"), false))
{
}

bool GPUScaling::SetGPUScalingEnable(const ADLDisplayID &displayID_, bool OnOff)
{
    int iAdapterIndex = displayID_.iDisplayLogicalAdapterIndex;
    int iDisplayIndex = displayID_.iDisplayLogicalIndex;

    std::string GPUScalingEnabledKey;
    GPUScalingEnabledKey.append("GPUScaling");

    std::ostringstream oss;
    oss << iAdapterIndex;
    GPUScalingEnabledKey.append(oss.str());
    oss.str("");

    oss << iDisplayIndex;
    GPUScalingEnabledKey.append(oss.str());

    char *szSubkey = const_cast<char*>(GPUScalingEnabledKey.c_str());
    //printf("\nsubkey: %s\n", szSubkey);

    ADL2_Adapter_RegValueInt_Set(GetADLContext(), iAdapterIndex, ADL_REG_DEVICE_FUNCTION_1, NULL, szSubkey, OnOff);

    if (ADL_OK != ADL2_DFP_GPUScalingEnable_Set(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, OnOff))
    {
        return false;
    }

    return true;
}

bool GPUScaling::GetGPUScalingEnable(const ADLDisplayID &displayID_, bool &OnOff)
{
    int GPUScalingSupported = -1;
    int GPUScalingCurrent = -1;
    int GPUScalingDefault = -1;
    
    OnOff = false;
    
    int ret = ADL2_DFP_GPUScalingEnable_Get(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &(GPUScalingSupported), &(GPUScalingCurrent), &(GPUScalingDefault));
    
    if (ret == ADL_OK) {
        OnOff = (GPUScalingCurrent == 1) ? true : false;
    }

    return (ret == ADL_OK) ? GPUScalingCurrent : false;
}

bool GPUScaling::SetDisplayScalingMode(const ADLDisplayID &displayID_, GPUSCALINGMODES GPUScalingModes)
{
    switch (GPUScalingModes){
    case SCALING_PreserveAspectRatio:
        ADL2_Display_PreservedAspectRatio_Set(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, 1);
        break;
    case SCALING_FullPanel:
        ADL2_Display_ImageExpansion_Set(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, 1);
        break;
    case SCALING_Centered:
        ADL2_Display_ImageExpansion_Set(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, 0);
        break;
    default:
        return false;
    }

    std::wstring KeyIn = CurrentImageScale_Key
        + L"," + std::to_wstring(displayID_.iDisplayLogicalAdapterIndex)
        + L"," + std::to_wstring(displayID_.iDisplayLogicalIndex);
    
    wchar_t *theKey = (wchar_t *)KeyIn.c_str();
    if (E_FAIL != hkcuRegistry_->Write(theKey, GPUScalingModes))
        return true;
    else
        return false;
}

GPUSCALINGMODES GPUScaling::GetDisplayScalingMode(const ADLDisplayID &displayID_)
{
	int ARExpansionSupport = 1;
	int ARExpansionEnable = 1;
	int ARExpansionDefault = 1;
	bool ImageScaleSupported = false;        //assume no support

	ADL2_Display_PreservedAspectRatio_Get(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex,	&(ARExpansionSupport), &(ARExpansionEnable), &(ARExpansionDefault));

	int ScalingSupported = -1;
	int ScalingCurrent = -1;
	int ScalingDefault = -1;
	if (ADL_OK == ADL2_Display_ImageExpansion_Get(GetADLContext(), displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &(ScalingSupported), &(ScalingCurrent), &(ScalingDefault)))
	{
		ImageScaleSupported = (ScalingCurrent == 1);
	}

	GPUSCALINGMODES GPUScalingModes = SCALING_FullPanel;
	if (0 != ARExpansionSupport)//if non-native resolution, use ADL value,
	{
		if (ImageScaleSupported)
		{
			if (1 == ARExpansionEnable)
			{//Wide Screen
				GPUScalingModes = SCALING_PreserveAspectRatio;
			}
			else
			{//Full Screen
				GPUScalingModes = SCALING_FullPanel;
			}
		}
		else
		{
			GPUScalingModes = SCALING_Centered;
		}
	}

	return GPUScalingModes;
}

/*
* End AMD namepsace
*/
//END_AMD_NAMESPACE
