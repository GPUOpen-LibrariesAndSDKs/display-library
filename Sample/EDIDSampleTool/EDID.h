///
///  Copyright (c) 2008 - 2013 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file EDID.h

#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include <stdio.h>

extern int initializeADL();
extern void deinitializeADL();
extern void printSyntax();
extern void getEmulationStatus(int iAdapterIndex);
extern void saveEDIDData(int iAdapterIndex, ADLDevicePort devicePort, char* fileName);
extern void setConnectionData(int iAdapterIndex, ADLDevicePort devicePort, int iConnectionType, char* fileName);
extern void setBranchData(int iAdapterIndex, ADLDevicePort devicePort, int iConnectionType, int iNumberofMSTPorts);
extern void setEmulation(int iAdapterIndex, ADLDevicePort devicePort, int iEmulationMode);
extern void removeEmulation(int iAdapterIndex, ADLDevicePort devicePort);
extern int printAdapaterInfo();
extern void printChildMSTPorts(int iAdapterIndex, ADLDevicePort parentDevicePort, int numberOfMSTPorts, int ActivePorts);
extern void getDevicePort(char* RAD, ADLDevicePort* devicePort);
