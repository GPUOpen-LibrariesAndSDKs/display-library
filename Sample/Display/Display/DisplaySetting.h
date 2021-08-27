#pragma once

#include "Adl.h"
class DisplaySetting
{
public:
    DisplaySetting();
    ~DisplaySetting();

    //Method to read VSR states
    //Arguments     :
    //displayID_    : Defines the value of the displayID
    //Returns       : TRUE is on and FALSE otherwise
    bool GetVirtualResolutionState(const ADLDisplayID& displayID_);

    //Method to write VSR states
    bool SetVirtualResolutionState(const ADLDisplayID& displayID_, bool OnOff);
	bool SetIntegerScalingState(const ADLDisplayID& displayID_, bool OnOff);
    bool GetDCESettings(const ADLDisplayID& displayID_, ADLDceSettings& dceSettings_);
    bool SetDCESettings(const ADLDisplayID& displayID_, ADLDceSettings& dceSettings_);
    double GetLinkRate(int linkRateIndex);

};

