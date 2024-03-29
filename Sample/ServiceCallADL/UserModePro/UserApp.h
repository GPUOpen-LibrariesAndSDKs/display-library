#pragma once

#include "resource.h"
#include "..\Include\GlobalDefines.h"
#include <map>

/// Defines the maximum string length
#define ADL_MAX_PATH                                    256
/// ADL function completed successfully
#define ADL_OK                    0
/// Generic Error. Most likely one or more of the Escape calls to the driver failed!
#define ADL_ERR                    -1
#define MAKE_NULL(a,b) a b = NULL;

typedef void *ADL_CONTEXT_HANDLE;
typedef void* (__stdcall *ADL_MAIN_MALLOC_CALLBACK)(int);
void* __stdcall ADL_Main_Memory_Alloc(int iSize_);
extern std::map<int, int> AdapterIndexMap_;

