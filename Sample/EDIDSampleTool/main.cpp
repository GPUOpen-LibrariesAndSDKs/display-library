///
///  Copyright (c) 2008 - 2013 Advanced Micro Devices, Inc.
 
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
	
	int iAdapterIndex, iConnectorIndex, iConnectionType, iEmulationMode, validArgs = 0;
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
				if (c == 6)
				{
					iAdapterIndex = atoi(args[2]);
					getDevicePort(args[3], &devicePort);
					iConnectionType = atoi(args[4]);
					setConnectionData(iAdapterIndex, devicePort, iConnectionType, args[5]);
				}
				else
					validArgs = -1;
				break;
			case 'b':
				if (c == 5)
				{
					iAdapterIndex = atoi(args[2]);
					getDevicePort(args[3], &devicePort);
					setBranchData(iAdapterIndex, devicePort, ADL_CONNECTION_TYPE_MST, atoi(args[4]));
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
	printf ("System Information \t\t\t EDIDSampleTool i\n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool i\n\n\n");
	printf ("Get Emulation Information of All ports \t EDIDSampleTool c <AdapterIndex> \n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool c 0\n\n\n");
	printf ("Store EDID Data in binary format \t EDIDSampleTool s <AdapterIndex> <RAD Address> <output filename>\n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool s 0 0.1 test.bin\n\n\n");
	printf ("Set EDID Data \t\t\t\t EDIDSampleTool d <AdapterIndex> <RAD Address> <connectionType> <input filename> \n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool d 0 0 4 test.bin\n\n\n");
	printf ("Branch emulation \t\t\t\t EDIDSampleTool b <AdapterIndex> <RAD Address> <No of child ports> \n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool b 0 0 2 \n\n\n");	
	printf ("Active Emulation \t\t\t EDIDSampleTool a <AdapterIndex> <RAD Address> <Emulation Mode> \n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool a 0 1 3 \n\n\n");
	printf ("Remove Emulation \t\t\t EDIDSampleTool r <AdapterIndex> <RAD Address>\n");
	printf ("\t\t \t\t\t Ex: EDIDSampleTool r 0 1\n\n\n");
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