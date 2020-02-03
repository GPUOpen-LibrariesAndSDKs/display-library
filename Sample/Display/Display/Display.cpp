// Display.cpp : Defines the entry point for the console application.
//

///
///  Copyright (c) 2019 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///

/// \file Display.cpp
/// \brief Sample application that demonstrates usage of ADL2 display APIs in the application.

#include "stdafx.h"
#include "Adl.h"
#include <iostream>
#include <string>
#include <codecvt>
#include <Windows.h>
#include "displaySetting.h"

void printSyntax()
{
    printf("This Application accepts below parameters\n");
    printf("-----------------------------------------\n");
    printf("Method to read VSR capabilities of display \t display g\n");
    printf("\t\t \t Ex: display g\n\n\n");
    printf("Method to set VSR ON/OFF \t\t\t display s X;  X - (0: OFF, 1: ON)\n");
    printf("\t\t \t Ex: display s 0\n\n\n");
}

//Sample entry point
int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        if (ADL_OK == InitADL()) //Initialization of ADL
        {         
            if (1==argc)
            {
                printSyntax();
            }
            else
            {
                PrepareAPI();
                //get all the displays
                for (auto adapterIndex : AdapterIndexMap_) {
                    Get_All_DisplayInfo(adapterIndex.second);
                }
                DisplaySetting displaySetting;

                if (_tcscmp(argv[1], _T("g")) == 0)
                {
                    //read VSR of display
                    for (auto displayID : DisplayID_)
                    {
                        std::cout << "Display[" << displayID.second.iDisplayLogicalAdapterIndex << "]" << std::endl;
                        bool state = displaySetting.GetVirtualResolutionState(displayID.second);
                        std::cout << "\VIRTUAL SUPER RESOLUTION is : " << (state ? "On" : "Off") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("s")) == 0)
                {
                    int isOnOff = _wtoi(argv[2]);
                    for (auto displayID : DisplayID_)
                    {
                        displaySetting.SetVirtualResolutionState(displayID.second, isOnOff);
                    }
                }
            }
        }
        DestroyADL();
    }
    catch (std::exception  ex)
    {
        std::cout << ex.what();
        return -1;
    }
    return 0;
}