#pragma once

#include "Adl.h"
#include "Registry.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

/*
* @namespace AMD
* @brief All functions and classes are in the AMD namespace
*/
//BEGIN_AMD_NAMESPACE
class Registry;

//ADL customed struct definition
typedef enum
{
    SCALING_PreserveAspectRatio = 0,
    SCALING_FullPanel,
    SCALING_Centered

} GPUSCALINGMODES;

// GPU Scaling class
class GPUScaling
{
private:
    ADL_CONTEXT_HANDLE context_;
    ADL_CONTEXT_HANDLE GetADLContext() { return context_; }
    std::shared_ptr<Registry> hkcuRegistry_;

public:
    GPUScaling(ADL_CONTEXT_HANDLE ctx);

public:
    bool SetGPUScalingEnable(const ADLDisplayID &displayID_, bool OnOff);
    bool GetGPUScalingEnable(const ADLDisplayID &displayID_, bool &OnOff);
    bool SetDisplayScalingMode(const ADLDisplayID &displayID_, GPUSCALINGMODES GPUScalingModes);
	GPUSCALINGMODES GetDisplayScalingMode(const ADLDisplayID &displayID_);
};

/*
* End AMD namepsace
*/
//END_AMD_NAMESPACE
