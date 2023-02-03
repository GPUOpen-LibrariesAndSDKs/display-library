// SmartDC.cpp : Defines the entry point for the console application.
//

///
///  Copyright (c) 2021 - 2022 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///

#include "stdafx.h"
#include "Adl.h"
#include <iostream>
#include "SmartDCSetting.h"


void printSyntax()
{
    printf("This Application accepts below parameters\n");
    printf("-----------------------------------------\n");
    printf("Method to read capabilities of SmartDC \t SmartDC c\n");
    printf("\t\t \t Ex: SmartDC c\n\n\n");
    printf("Method to read SmartDC Current Mode \t SmartDC g\n");
    printf("\t\t \t Ex: SmartDC g\n\n\n");
    printf("Method to set SmartDC Mode \t SmartDC s X;  X - (0: Disable, 1: Enabled)");
    printf("\t\t \t Ex: SmartDC s 0\n\n\n");
}

//Sample entry point
int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        if (ADL_OK == InitADL()) //Initialization of ADL
        {
            if (1 == argc)
            {
                printSyntax();
            }
            else
            {
                SmartDCSetting smartDC;
                //read capabilities of SmartDC
                if (_tcscmp(argv[1], _T("c")) == 0)
                {
                    int supported = 0;
                    int defaultState = 0;
                    if (smartDC.GetSmartDCCaps(supported, defaultState))
                    {
                        std::cout << "\tSmartDC supported: " << (supported ? "Support" : "Unsupport") << std::endl;
                        std::cout << "\tSmartDC current status: " << (defaultState ? "Enabled" : "Disabled") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("g")) == 0)
                {
                    int currentState = 0;
                    if (smartDC.GetSmartDCMode(currentState))
                    {
                        std::cout << "\tSmartDC current status: " << (currentState ? "Enabled" : "Disabled") << std::endl;
                    }
                }
                else if (_tcscmp(argv[1], _T("s")) == 0)
                {
                    int enablement = _wtoi(argv[2]);
                    int currentState = 0;
                    if (smartDC.SetSmartDCMode(enablement, currentState))
                    {
                        std::cout << "\tSmartDC current status: " << (currentState ? "Enabled" : "Disabled") << std::endl;
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