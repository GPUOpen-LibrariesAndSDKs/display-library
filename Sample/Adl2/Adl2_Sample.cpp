// Adl2_Sample.cpp : Defines the entry point for the console application.
//

///
///  Copyright (c) 2012 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file Adl2_Sample.cpp
/// \brief Sample  application that demonstrates usage of ADL2 APIs in the application that contains multiple uncoordinated ADL transactions.
///
/// ADL2 APIs have been added to ADL to allow creation of the applications that contain multiple clients that call ADL2 APIs in uncoordinated manner without risking 
/// to destroy each other’s context. You can use legacy ADL and newer ADL2 APIs in the same application. The only limitation is that no more than one client that uses 
/// ADL APIs can exist at any given time in the same process. There is no limit to number of clients that use ADL2 APIs. This sample demonstrates how to author application 
/// that contains multiple ADL2 transactions and single legacy ADL transaction. 
/// Author: Ilia Blank

#if defined (LINUX)
#include "../../include/adl_sdk.h"
#include "../../include/customer/oem_structures.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <stdlib.h>	
#include <string.h>	//memeset
#include <unistd.h>	//sleep

#else
#include <windows.h>
#include <tchar.h>
#include "..\..\include\adl_sdk.h"
#endif

#include <stdio.h>

// Definitions of the used function pointers. Add more if you use other ADL APIs. Note that that sample will use mixture of legacy ADL and ADL2 APIs.
typedef int (*ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
typedef int (*ADL_MAIN_CONTROL_DESTROY )();
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
typedef int (*ADL2_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int (*ADL2_MAIN_CONTROL_DESTROY )( ADL_CONTEXT_HANDLE);
typedef int (*ADL2_ADAPTER_ACTIVE_GET ) (ADL_CONTEXT_HANDLE, int, int* );
typedef int (*ADL2_DISPLAY_MODES_GET )(ADL_CONTEXT_HANDLE, int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);

ADL_MAIN_CONTROL_CREATE				ADL_Main_Control_Create;
ADL_MAIN_CONTROL_DESTROY			ADL_Main_Control_Destroy;
ADL_ADAPTER_NUMBEROFADAPTERS_GET	ADL_Adapter_NumberOfAdapters_Get;
ADL2_MAIN_CONTROL_CREATE			ADL2_Main_Control_Create;
ADL2_MAIN_CONTROL_DESTROY			ADL2_Main_Control_Destroy;
ADL2_DISPLAY_MODES_GET				ADL2_Display_Modes_Get;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get;

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free ( void* lpBuffer )
{
    if ( NULL != lpBuffer )
    {
        free ( lpBuffer );
        lpBuffer = NULL;
    }
}

#if defined (LINUX)
// equivalent functions in linux
void * GetProcAddress( void * pLibrary, const char * name)
{
    return dlsym( pLibrary, name);
}

#endif

int InitADL ()
{
#if defined (LINUX)
    void *hDLL;		// Handle to .so library
#else
    HINSTANCE hDLL;		// Handle to DLL
#endif

	#if defined (LINUX)
    hDLL = dlopen( "libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
#else
    hDLL = LoadLibrary("atiadlxx.dll");
    if (hDLL == NULL)
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary("atiadlxy.dll");
#endif

	if (NULL == hDLL)
    {
        printf("ADL library not found!\n");
		return ADL_ERR;
    }

	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL_Main_Control_Create");
    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
    ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");

	ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL2_Main_Control_Create");
    ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL2_Main_Control_Destroy");
    ADL2_Display_Modes_Get = (ADL2_DISPLAY_MODES_GET) GetProcAddress(hDLL,"ADL2_Display_Modes_Get");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");

	if (NULL == ADL_Main_Control_Create ||
        NULL == ADL_Main_Control_Destroy ||
        NULL == ADL_Adapter_NumberOfAdapters_Get ||
		NULL == ADL2_Main_Control_Create ||
        NULL == ADL2_Main_Control_Destroy ||
        NULL == ADL2_Display_Modes_Get ||
		NULL == ADL2_Adapter_Active_Get)
	{
	    printf("ADL's API is missing!\n");
		return ADL_ERR; 
	}

	return ADL_OK;
}


//Retrieves active status of given adapter. Implements the retrieval as isolated ADL2 transaction.
//Note that the function can be nested inside another ADL2 transaction without interfering with it.
//In real application it would be much more efficient to share the same context with the parent transaction by passing the context handle in the function argument list.   
int GetAdapterActiveStatus (int adapterId, int& active)
{
	ADL_CONTEXT_HANDLE context = NULL;	
	active = 0;
    
	if (ADL_OK != ADL2_Main_Control_Create (ADL_Main_Memory_Alloc, 1, &context))
	{
		printf ("Failed to initialize nested ADL2 context");
		return ADL_ERR;
	}

	
	if (ADL_OK != ADL2_Adapter_Active_Get(context, adapterId , &active))
	{
		printf ("Failed to get adapter status");
	}

	if (ADL_OK != ADL2_Main_Control_Destroy (context))
	{
		printf ("Failed to destroy nested ADL2 context");
		return ADL_ERR;
	}
	return ADL_OK;
}

//Demonstrates execution of multiple nested ADL2 transactions that are executed on the same thread. 
//Uncoordinated ADL2 transactions can be also executed on separate thread. 
int PrintAdapterInfo (int adapterId)
{
	ADL_CONTEXT_HANDLE context = NULL;	
    
	if (ADL_OK != ADL2_Main_Control_Create (ADL_Main_Memory_Alloc, 1, &context))
	{
		printf ("Failed to initialize ADL2 context");
		return ADL_ERR;
	}

	int active = 0;

	//Invoking additional nested ADL2 based transaction on the same thread to demonstrate that multiple ADL2 transactions can be executed at the same time inside 
	//the process without interfering. Not the most efficient way to work with ADL. In real application it would be much more efficient to re-use  context of parent
	//transaction by passing it to GetAdapterActiveStatus.  
	if (ADL_OK == GetAdapterActiveStatus (adapterId, active))
	{
		printf ("*************************************************\n" );

		printf ("Adapter %d is %s\n", adapterId, (active)?"active":"not active" );
		if (active)
		{
			int numModes;			
			ADLMode* adlMode;

			if (ADL_OK == ADL2_Display_Modes_Get (context, adapterId, -1, &numModes, &adlMode))
			{
				if (numModes == 1)
				{
					printf ("Adapter %d resolution is %d by %d\n", adapterId, adlMode->iXRes, adlMode->iYRes );
					ADL_Main_Memory_Free (adlMode);
				}
			}
		}
	}

	if (ADL_OK != ADL2_Main_Control_Destroy (context))
	{
		printf ("Failed to destroy ADL2 context");
		return ADL_ERR;
	}
	return ADL_OK;
}

int main (int c,char* k[],char* s[])
{
	if (ADL_OK != InitADL ())
	{
		return 0;
	}

    // Initialize legacy ADL transaction.Note that applications still can mix ADL and ADL2 API providing that only single 
	// transaction that uses legacy ADL APIs exists at any given time in the process. Numer of ADL2 transactions is not limited.  
	// The second parameter is 1, which means:
    // retrieve adapter information only for adapters that are physically present and enabled in the system
    if ( ADL_OK != ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1) )
	{
	    printf("ADL Initialization Error!\n");
		return 0;
	}

	int  iNumberAdapters;

    // Obtain the number of adapters for the system
    if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters ) )
	{
	    printf("Cannot get the number of adapters!\n");
		return 0;
	}

    for (int adapterId = 0; adapterId < iNumberAdapters; adapterId++ )
    {
		if (ADL_OK != PrintAdapterInfo (adapterId))
			break;
	}

	//Finishing legacy ADL transaction
	 if ( ADL_OK != ADL_Main_Control_Destroy ())
	 {
		 printf ("Failed to destroy ADL context");
	 }

	return 0;
}