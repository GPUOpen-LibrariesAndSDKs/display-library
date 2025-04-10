// SmartMux.cpp : Defines the entry point for the console application.
//

///
///  Copyright (c) 2024 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
///

#include "stdafx.h"
#include "Adl.h"
#include <conio.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <thread>

// #define DEBUG_PRINTS

void printHelpMessage()
{
	printf("This Application accepts below parameters\n");
	printf("-----------------------------------------\n");
	printf("set  \t Control eDP Lowpower feature. Such as: [paused][p]\t[resume][r]\n");
	printf("get  \t Obtain eDP Lowpower feature status.\n");
}

int main(int argc, char* argv[])
{
	int retval = 0, argIdx = 1;
	int iAdapterIdxWithSAG = -1;
	LPAdapterInfo lpAdapterInfo = nullptr;

#ifdef DEBUG_PRINTS
	std::cout << "--[Hello World]--" << std::endl;
	std::cout << std::endl;
#endif // DEBUG_PRINTS
	try
	{
		if (argc < 2 || argc > 2)
		{
			std::cout << "Lack of Input parameter. Please keyin as below: " << std::endl;
			std::cout << std::endl;
			printHelpMessage();
			return retval;
		}

		if (ADL_OK != InitADL()) //Initialization of ADL
		{
#ifdef DEBUG_PRINTS
			std::cout << "--[Load Atiadlxx.dll Failed]--" << std::endl;
			std::cout << std::endl;
#endif
			return retval;
		}

		std::string argStr{ argv[argIdx++] };

		if (argc == 2)
		{

			if (argStr == "help")
			{

			}
			else if (argStr == "get")
			{
				ADL_PanelFWUpdateStatus statusMsg = { 0 };

				std::cout << "Obtain Feature Status:";
				do {
					ADL2_Display_GetPanelFWUpdateStatus(context_, 0, 0, &statusMsg);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					std::cout << ".";

				} while (statusMsg.ready_for_update == 0);

				if (statusMsg.ready_for_update == 1)
				{
					std::cout << "All Feature is Paused" << std::endl;
				}
			}
			else if (argStr == "set")
			{
				char tmp = 0;
				ADL_PanelFWUpdateMode	msg = { 0 };
				do {
					std::cout << "Enter Any Key to Send cmd:" << std::endl;
					tmp = getchar();
					switch (tmp) {
						case 'p':
							msg.pause_for_update = true;

							if (ADL_OK == ADL2_Display_SetPanelFWUpdateMode(context_, 0, 0, &msg))
							{
								std::cout << "[p] Send [Disable] eDP Lowpower Feature CMD to ADL" << std::endl;
							}
							else
							{
								std::cout << "No Respond !!Send [Disable] eDP Lowpower Feature CMD to ADL" << std::endl;
							}
							break;
						case 'r':
							msg.pause_for_update = false;

							if (ADL_OK == ADL2_Display_SetPanelFWUpdateMode(context_, 0, 0, &msg))
							{
								std::cout << "[r] Send [Disable] eDP Lowpower Feature CMD to ADL" << std::endl;
							}
							else
							{
								std::cout << "No Respond !!Send [Disable] eDP Lowpower Feature CMD to ADL" << std::endl;
							}
							break;
						default:
							std::cout << "not support input key. Only support: [paused][p]\t[resume][r]" << std::endl;
							break;
					}
					memset((void*)&msg, 0, sizeof(ADL_PanelFWUpdateMode));
				} while (tmp != 'p' && tmp != 'r');
			}
		}
		DestroyADL();
	}
	catch (std::exception  ex)
	{
		std::cout << ex.what();
		retval = -1;
	}

	std::cout << "--[End World]--" << std::endl;
	std::cout << std::endl;

	return retval;
}
