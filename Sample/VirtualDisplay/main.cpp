///
///  Copyright (c) 2008 - 2022 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.c

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "EDID.h"

int main (int c,char* args[],char* s[])
{
	
	int iAdapterIndex, iConnectorIndex, iEmulationMode, validArgs = 0;
	ADLDevicePort devicePort;
	
	
	if (initializeADL())
	{
		if (c== 1)
			printSyntax();
		else
		{
			switch (*(args[1]))
			{
			case 'i':
				printAdapaterInfo();
				break;
			case 'c':
				if (c == 3)
				{
					iAdapterIndex = atoi(args[2]);
					getEmulationStatus(iAdapterIndex);
				}
				else
					validArgs = -1;
				break;
			case 'm':
				if (c == 8)
				{
					iAdapterIndex = atoi(args[2]);
					int displayIndex = atoi(args[3]);
					int xRes = atoi(args[4]);
					int yRes = atoi(args[5]);
					float fRefreshRate = atof(args[6]);
					int force = atoi(args[7]);
					applyMode(iAdapterIndex, displayIndex, xRes, yRes, fRefreshRate, force);
				}
				else
					validArgs = -1;
				break;
			case 's':
				if (c == 5)
				{
					iAdapterIndex = atoi(args[2]);
					getDevicePort(args[3], &devicePort);
					saveEDIDData(iAdapterIndex, devicePort, args[4]);
				}
				else
					validArgs = -1;
				break;
			case 'd':
				if (c == 5)
				{
					iAdapterIndex = atoi(args[2]);
					getDevicePort(args[3], &devicePort);
					setConnectionData(iAdapterIndex, devicePort, 13, args[4]);
				}
				else
					validArgs = -1;
				break;
			
			case 'r':
				if (c == 4)
				{
					iAdapterIndex = atoi(args[2]);
					getDevicePort(args[3], &devicePort);
					removeEmulation(iAdapterIndex, devicePort);
				}
				else
					validArgs = -1;
				break;
			case 'a':
				if (c == 5)
				{
					iAdapterIndex = atoi(args[2]);
					getDevicePort(args[3], &devicePort);
					iEmulationMode = atoi(args[4]);
					setEmulation(iAdapterIndex, devicePort, iEmulationMode);
				}
				else
					validArgs = -1;
				break;
			default:
				printSyntax();
				break;
			}	

			if (validArgs == -1)
			{
				printf(" Enter valid arguments");
				printSyntax();
			}
		}
	}
	 return 0;
}

void printSyntax()
{
	printf ("This Application accepts below parameters\n");
	printf ("-----------------------------------------\n");
	printf ("System Information \t\t\t VirtualDisplay i\n");
	printf ("\t\t \t\t\t Ex: VirtualDisplay i\n\n\n");
	printf ("Get Emulation Information of All ports \t VirtualDisplay c <AdapterIndex> \n");
	printf ("\t\t \t\t\t Ex: VirtualDisplay c 0\n\n\n");
	printf ("Download Display EDID \t VirtualDisplay s <AdapterIndex> <RAD Address> <output filename>\n");
	printf ("\t\t \t\t\t Ex: VirtualDisplay s 0 0.1 test.bin\n\n\n");
	printf ("Emulate Virtual Display \t\t\t\t VirtualDisplay d <AdapterIndex> <RAD Address> <input filename> \n");
	printf ("\t\t \t\t\t Ex: VirtualDisplay d 0 0 test.bin\n\n\n");	
	printf ("Modify Emulation Mode \t\t\t VirtualDisplay a <AdapterIndex> <RAD Address> <Emulation Mode> \n");
	printf ("\t\t \t\t\t Ex: VirtualDisplay a 0 1 3 \n\n\n");
	printf ("Remove Emulation \t\t\t VirtualDisplay r <AdapterIndex> <RAD Address>\n");
	printf ("\t\t \t\t\t Ex: VirtualDisplay r 0 1\n\n\n");
	printf("Apply Display Mode \t\t\t VirtualDisplay m <AdapterIndex> <DisplayIndex> <iXRes> <iYRes> <fRefreshRate> <bForce>\n");
	printf("\t\t \t\t\t Ex: VirtualDisplay m 0 8 1028 1028 60 0\n\n\n");
	printf ("RAD Address Examples: X.X.X (first X refers to connector Index)\n" );
	printf ("\tNon MST Ports: 0, 1, 2\n");
	printf ("\tMST Ports: 0.1, 0.2, 0.2.1, 1.1, 2.3.1");
}

void getDevicePort(char* RAD, ADLDevicePort* devicePort)
{
	int Link =1;
	char *token;	
	char *search = ".";
	ADLDevicePort temp;
	token = strtok (RAD,".");
	temp.iConnectorIndex = atoi(token);					
	token = strtok (NULL,".");
	temp.aMSTRad.rad[0] = 0;
	while (token != NULL)
	{
		temp.aMSTRad.rad[Link++] = atoi(token);
		token = strtok (NULL,".");
	}	
	temp.aMSTRad.iLinkNumber = Link;
	*devicePort = temp;	
}