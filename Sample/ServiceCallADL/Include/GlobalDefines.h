#ifndef _GLOBAL_DEFINES_H__ 
#define _GLOBAL_DEFINES_H__ 
#include "..\..\..\include\adl_sdk.h"

#define ADL_FUNCTION_NAME							L"FunctionName="
#define ADL_FUNCTION_NAME_LEN							13
#define ADL_EVENT_NAME								L"Global\\EventADL_%s"
#define ADL_SHAREMENORY_NAME						L"Global\\ShareMemoryADL_%s"
// Control the alignment of user-defined data to 64 byte
#define CACHE_LINE  64  
#define CACHE_ALIGN __declspec(align(CACHE_LINE))
#define SHARE_MEMORY_MAX_SIZE   32 * 4 * 1024
#ifndef RUN_ADL_AS_SERVICE

typedef int(*ADL2_MAIN_CONTROLX2_CREATE)                                (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel);

// XConnect, optional. Fills AdapterInfoX2
typedef int(*ADL2_ADAPTER_ADAPTERINFOX3_GET)                            (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);
typedef int(*ADL2_MAIN_CONTROL_REFRESH)                                 (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_MAIN_CONTROL_CREATE)									(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)                                 (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_NUMBEROFADAPTERS_GET)							(ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_PRIMARY_GET)									(ADL_CONTEXT_HANDLE, int* lpPrimaryAdapterIndex);
typedef int(*ADL2_ADAPTER_ADAPTERINFO_GET)								(ADL_CONTEXT_HANDLE, LPAdapterInfo lpInfo, int iInputSize);
typedef int(*ADL2_ADAPTER_ACTIVE_GET)									(ADL_CONTEXT_HANDLE, int, int*);
// TODO: Add your ADL call define ADL2 is thread safe.
typedef int(*ADL2_OVERDRIVE_CAPS)										(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_GET)							(ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_CAPABILITIESX2_GET)						(ADL_CONTEXT_HANDLE, int, ADLODNCapabilitiesX2*);


ADL2_MAIN_CONTROL_CREATE			ADL2_Main_Control_Create = nullptr;
ADL2_MAIN_CONTROL_DESTROY			ADL2_Main_Control_Destroy = nullptr;
ADL2_ADAPTER_NUMBEROFADAPTERS_GET	ADL2_Adapter_NumberOfAdapters_Get = nullptr;
ADL2_ADAPTER_PRIMARY_GET            ADL2_Adapter_Primary_Get = nullptr;
ADL2_ADAPTER_ADAPTERINFO_GET        ADL2_Adapter_AdapterInfo_Get = nullptr;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get = nullptr;

// TODO: Add ADL call function point
ADL2_OVERDRIVE_CAPS					ADL2_Overdrive_Caps = nullptr;
ADL2_OVERDRIVEN_FANCONTROL_GET		ADL2_OverdriveN_FanControl_Get = nullptr;
ADL2_OVERDRIVEN_CAPABILITIESX2_GET  ADL2_OverdriveN_CapabilitiesX2_Get = nullptr;



class CACHE_ALIGN BaseExternADL
{
public:
	INT64 iAdapters;
	ADL_CONTEXT_HANDLE context;
	INT64 szData;// ADL point buffer size
	void * buffer;// ADL point buffer
	BaseExternADL() : szData(0), buffer(0) {}// init data
	// 
	virtual INT64 CallADLFunction() 
	{
		return false;
	}
	// Used to identify which method to execute. 
	virtual LPCWSTR GetFunctionName()
	{
		return L"";
	}

};
// TODO: Add ADL call function Class 
/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N capabilities
///
/// This structure is used to store information about Overdrive N capabilities
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
class CACHE_ALIGN ExADLODNCapabilitiesX2 :public BaseExternADL
{
public:

	ADLODNCapabilitiesX2 	sODNCapabilitiesX2;
	ExADLODNCapabilitiesX2()
	{
		memset(&sODNCapabilitiesX2, 0, sizeof(ADLODNCapabilitiesX2));
	}
	INT64 CallADLFunction()
	{
		return (ADL_OK == ADL2_OverdriveN_CapabilitiesX2_Get(context, iAdapters, &sODNCapabilitiesX2));
	}
	static LPCWSTR GetFunctionName()
	{
		// Add call function in the following line
		return L"GetODNCapabilities";
	}
};

class CACHE_ALIGN ExADLOverdriveCaps :public BaseExternADL
{
public:

	int iSupported;
	int iEnabled;
	int iVersion;
	INT64 CallADLFunction()
	{
		return (ADL_OK == ADL2_Overdrive_Caps(context, iAdapters, &iSupported, &iEnabled, &iVersion));
	}
	static LPCWSTR GetFunctionName()
	{
		return L"GetOverdriveCaps";
	}
};

class CACHE_ALIGN ExADLODNFanControl :public BaseExternADL
{
public:

	ADLODNFanControl odNFanControl;
	INT64 CallADLFunction()
	{
		return (ADL_OK == ADL2_OverdriveN_FanControl_Get(context, iAdapters, &odNFanControl));
	}
	static LPCWSTR GetFunctionName()
	{
		return L"GetFanControl";
	}
};
class CACHE_ALIGN ExADLAdapterInfo :public BaseExternADL
{
public:
	INT64 CallADLFunction()
	{
		int iNumberAdapters = 0;
		szData = 0;
		// Get Adapter Number
		if (ADL_OK == ADL2_Adapter_NumberOfAdapters_Get(context, &iNumberAdapters))
		{
			int primary = -1;
			ADL2_Adapter_Primary_Get(context, &primary);
			buffer = new AdapterInfo[iNumberAdapters];
			szData = iNumberAdapters * sizeof(AdapterInfo);
			ZeroMemory(buffer, szData);
			// Get Adapater infor to buffer
			int ret = (ADL_OK == ADL2_Adapter_AdapterInfo_Get(context, (LPAdapterInfo)buffer, szData));
			return ret;
		}
		return false;
	}
	static LPCWSTR GetFunctionName()
	{
		return L"GetAdapterInfo";
	}

	~ExADLAdapterInfo()
	{

	}
};

#endif

#endif //_GLOBAL_DEFINES_H__ 

