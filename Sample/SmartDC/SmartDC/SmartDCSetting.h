///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

#pragma once

class SmartDCSetting
{
public:
    SmartDCSetting();
    ~SmartDCSetting();

    //Method to read capabilities of SmartDC
    //Arguments         :
    //supported_        : Value of the SmartDC mode support. 0: Unsupported; 1: Supported
    //defaultVal        : Value of the SmartDC default state. 0: Disalbed; 1: Enabled
    //Returns           : TRUE on success and FALSE otherwise
    bool GetSmartDCCaps(int &supported_, int &defaultVal);

    //Method to read Blockchain Mode
    //Arguments                     :
    //currentMode_                  : Value of current SmartDC Mode. 0: Disalbed; 1: Enabled
    //Returns                       : TRUE on success and FALSE otherwise
    bool GetSmartDCMode(int &currentMode_);

    //Method to set Blockchain Mode
    //Arguments                 :
    //enable_                   : Defines the value of the SmartDC mode
    //settedMode_               : Value after SmartDC Mode changed. 0: Disalbed; 1: Enabled
    //Returns                   : TRUE on success and FALSE otherwise
    bool SetSmartDCMode(int enable_, int &settedMode_);
};

