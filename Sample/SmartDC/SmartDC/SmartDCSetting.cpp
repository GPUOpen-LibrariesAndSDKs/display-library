
///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

#include "stdafx.h"
#include "SmartDCSetting.h"
#include "Adl.h"

SmartDCSetting::SmartDCSetting()
{
}

SmartDCSetting::~SmartDCSetting()
{
}

bool SmartDCSetting::GetSmartDCCaps(int &supported_, int &defaultVal)
{
    return ADL_OK == ADL2_SmartDC_Caps(context_, &supported_, &defaultVal) ? true : false;
}

bool SmartDCSetting::GetSmartDCMode(int &currentMode_)
{
    return ADL_OK == ADL2_SmartDC_Status_Get(context_, &currentMode_) ? true : false;
}

bool SmartDCSetting::SetSmartDCMode(int enable_, int &settedMode_)
{
    return ADL_OK == ADL2_SmartDC_Status_Set(context_, enable_, &settedMode_) ? true : false;
}
