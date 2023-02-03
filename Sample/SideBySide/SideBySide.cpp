#include <iostream>
#include <string>
#include "SideBySide.h"

using namespace std;

//Memory allocating callback for ADL
//iSize_ - how many bytes to allocate
void* __stdcall ADL_Main_Memory_Alloc(int iSize_)
{
	void* lpBuffer = malloc(iSize_);
	return lpBuffer;
}

void __stdcall ADL_Main_Memory_Free(void** lpBuffer_)
{
	if (NULL != lpBuffer_ && NULL != *lpBuffer_)
	{
		free(*lpBuffer_);
		*lpBuffer_ = NULL;
	}
}


SideBySide::SideBySide()
	:AdlLibHandle(nullptr)
	,AdlContext(nullptr)
	,AllAdapterInfo(nullptr)
	,NumAdapters(0)
	,CompatibilityCheckDisabled(false)
	
{
	//By default we are loading default ADL from system32 folder.Note that on side - by - side systems it is expected that system32 folder will contain ADL  dll that is shipped with latest graphics driver version installed on this system
	//ADL dll from system32  is expected to be compatible with GPU that latest version of AMD graphics driver is installed for.
	LoadAdl(L"atiadlxx.dll");
}

SideBySide::SideBySide(bool compatibilityCheckDisabled)
	:AdlLibHandle(nullptr)
	,AdlContext(nullptr)
	,AllAdapterInfo(nullptr)
	,NumAdapters(0)
	,CompatibilityCheckDisabled(compatibilityCheckDisabled)
{	
	//By default we are loading default ADL from system32 folder.Note that on side - by - side systems it is expected that system32 folder will contain ADL  dll that is shipped with latest graphics driver version installed on this system
	//ADL dll from system32  is expected to be compatible with GPU that latest version of AMD graphics driver is installed for.
	LoadAdl(L"atiadlxx.dll");
}

SideBySide::~SideBySide()
{
	UnloadAdl();
}

//Side-by-side system is characterized by presence of multiple instances of AMD graphics driver installed for different AMD GPUs. 
//In order to detect side-by-side the unction will enumerate all GPUs while getting information about driver installed for each GPU.
//The function will return true if presence of more then one driver version, or multiple instances of driver binaries are detected
bool SideBySide::IsSydeBySideSystem()
{
	bool result = false;
	ADL2_DRIVER_PATH_GET driverPathGet = (ADL2_DRIVER_PATH_GET)GetProcAddress(AdlLibHandle, "ADL2_Driver_Path_Get");
	ADL2_GRAPHICS_VERSIONSX3_GET graphicsVersionGet = (ADL2_GRAPHICS_VERSIONSX3_GET)GetProcAddress(AdlLibHandle, "ADL2_Graphics_VersionsX3_Get");
	ADLVersionsInfoX2 lastDriverVersion = { {0}, {0}, {0}, {0}};	
	char lastDriverPath[ADL_MAX_PATH] = {0};

	if (driverPathGet != nullptr && graphicsVersionGet != nullptr)
	{
		//In order to detect side-by-side we enumerates all adapter looking for two instances with different driver versions or two instances with different install location of driver binaries.
		//Both conditions indicate presence of more then one AMD graphics driver on the system thus side-by-side is detected.
		for (int i = 0; i < NumAdapters; i++)
		{
			ADLVersionsInfoX2 nextDriverVersion = { {0}, {0}, {0}, {0} };
			if (graphicsVersionGet(AdlContext, AllAdapterInfo[i].iAdapterIndex, &nextDriverVersion) >= 0)
			{
				if (lastDriverVersion.strDriverVer[0] == 0)
					lastDriverVersion = nextDriverVersion;

				if (strcmp(lastDriverVersion.strDriverVer, nextDriverVersion.strDriverVer) != 0)
				{
					result = true;
					break;
				}
			}

			char nextDriverPath[ADL_MAX_PATH] = { 0 };
			if (driverPathGet(AdlContext, AllAdapterInfo[i].iAdapterIndex, ADL_MAX_PATH, nextDriverPath) == ADL_OK && nextDriverPath[0] != 0)
			{
				if (lastDriverPath[0] == 0)
					strcpy_s(lastDriverPath, nextDriverPath);

				if (strcmp(lastDriverPath, nextDriverPath) != 0)
				{
					result = true;
					break;
				}
			}
		}
	}

	return result;
}

//Prints GPUs that are compatible with currently loaded ADL. APIs inside currently loaded ADL dll can be safely used to communicate to compatible GPUs (designated by passing adapter index).
//The APIs will fail if called to communicate to incompatible GPU.
void SideBySide::PrintCompatibleGPUs()
{
	//Compatible AMD GPUs are designated with Vndor ID of 1002		
	PrintGPUs(1002);
}

//Prints GPUs that are incompatible with currently loaded ADL. APIs from currently loaded ADL dll will fail if called while passing adapter index that belongs to incompatible GPU.
//The failure is due to ADL internal compatibility check. The GPU is considered to be incompatible with loaded ADL if GPU's driver versions is different from ADL's one. 
//And AMD QA didn't certified the compatibility of this combination.
void SideBySide::PrintIncompatibleGPUs()
{
	//Incompatible GPUs are designated with Vndor ID of -1002		
	PrintGPUs(-1002);
}

//Return list of adapter info structures. Single structure for each GPU incompatible to currently loaded ADL.
std::vector<AdapterInfoX2> SideBySide::GetIncompatibleGPUs()
{
	std::vector<AdapterInfoX2> result;
	int lastAddeddBdf = -1;
	for (int i = 0; i < NumAdapters; i++)
	{
		AdapterInfoX2* adapterInfo = &AllAdapterInfo[i];

		if (adapterInfo->iVendorID == -1002)
		{
			//adapters owned by the same GPU have the same BDF (buss, device, function) identificator.
			//They are also grouped together is the adapters list.
			int nextBdf = GpuBDF(adapterInfo->iBusNumber, adapterInfo->iDeviceNumber, adapterInfo->iFunctionNumber);
			if (lastAddeddBdf != nextBdf)
			{
				result.push_back(*adapterInfo);
				lastAddeddBdf = nextBdf;
			}
		}
	}
	return result;
}


//Unloads currently loaded ADL and reloads it from the disk loacation that contains graphics driver binaries installed for this GPU.
void SideBySide::ReloadAdl(AdapterInfoX2 incompatibleGPU)
{	
	ADL2_DRIVER_PATH_GET driverPathGet = (ADL2_DRIVER_PATH_GET)GetProcAddress(AdlLibHandle, "ADL2_Driver_Path_Get");

	char driverPath[ADL_MAX_PATH];
	if (driverPathGet(AdlContext, incompatibleGPU.iAdapterIndex, ADL_MAX_PATH, driverPath) == ADL_OK && driverPath[0] != 0)
	{
		string adlPath = string(driverPath) + string("\\atiadlxx.dll");
		wstring wAdlPath(adlPath.begin(), adlPath.end());
		UnloadAdl();
		LoadAdl(wAdlPath.c_str());
	}	
}


void SideBySide::PrintOneGPU(AdapterInfoX2* adapterInfo)
{
	const char* compatibility = (adapterInfo->iVendorID == 1002) ? "Compatible GPU:" : "Incompatible GPU:";
	std::cout << compatibility << adapterInfo->strAdapterName << "\n";
}

int SideBySide::GpuBDF(const int& busNo_, const int& devNo_, const int& funcNo_)
{
	return ((busNo_ & 0xFF) << 8) | ((devNo_ & 0x1F) << 3) | (funcNo_ & 0x07);
}

//Prints GPUs that are compatible with currently loaded ADL. APIs inside currently loaded ADL dll can be safely used to communicate to compatible GPUs (designated by passing adapter index).
//The APIs will fail if called to communicate to incompatible GPU.
void SideBySide::PrintGPUs(int vendorId)
{

	int lastPrintedBdf = -1;
	for (int i = 0; i < NumAdapters; i++)
	{
		AdapterInfoX2* adapterInfo = &AllAdapterInfo[i];
		
		if (adapterInfo->iVendorID == vendorId)
		{
			//adapters owned by the same GPU have the same BDF (buss, device, function) identificator.
			//They are also grouped together is the adapters list.
			int nextBdf = GpuBDF(adapterInfo->iBusNumber, adapterInfo->iDeviceNumber, adapterInfo->iFunctionNumber);
			if (lastPrintedBdf != nextBdf)
			{
				PrintOneGPU(adapterInfo);
				lastPrintedBdf = nextBdf;
			}
		}
	}
}

void SideBySide::UnloadAdl()
{
	if (AllAdapterInfo != nullptr)
	{
		ADL_Main_Memory_Free((void**)&AllAdapterInfo);
		AllAdapterInfo = nullptr;
		NumAdapters = 0;
	}

	if (AdlLibHandle != nullptr)
	{
		ADL2_MAIN_CONTROL_DESTROY adlDestroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(AdlLibHandle, "ADL2_Main_Control_Destroy");
		adlDestroy(AdlContext);
		AdlContext = nullptr;
		FreeLibrary(AdlLibHandle);
		AdlLibHandle = nullptr;
	}
}

void SideBySide::LoadAdl(const wchar_t* adlPath)
{
	//By default we are loading default ADL from system32 folder.Note that on side-by- side systems it is expected that system32 folder will contain ADL  dll that is shipped with latest graphics driver version installed on this system
    
	AdlLibHandle = LoadLibrary(adlPath);	
	ADL2_ADAPTER_ADAPTERINFOX4_GET adapterInfoGet = (ADL2_ADAPTER_ADAPTERINFOX4_GET)GetProcAddress(AdlLibHandle, "ADL2_Adapter_AdapterInfoX4_Get");

	if (AdlLibHandle != nullptr) {
		//Getting list of adapters as returned by adl loaded by default from system32
		int createResult = (CompatibilityCheckDisabled) ? MainControlCreateCompatibilityCheckDisabled() : MainControlCreate();
		if (createResult == ADL_OK)
		{
			//Getting list of adapters as returned by adl loaded by default from system32
			adapterInfoGet(AdlContext, -1, &NumAdapters, &AllAdapterInfo);
		}
	}
}

int SideBySide::MainControlCreate()
{
	ADL2_MAIN_CONTROLX2_CREATE adlCreate = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(AdlLibHandle, "ADL2_Main_ControlX2_Create");
	return adlCreate(ADL_Main_Memory_Alloc, 1, &AdlContext, ADL_THREADING_LOCKED);
}

int SideBySide::MainControlCreateCompatibilityCheckDisabled()
{
	//Note that similar effect can be achieved by setting ADLCREATEOPTIONS  environment variable to 1 prior to calling ADL2_Main_ControlX2_Create
	ADL2_MAIN_CONTROLX3_CREATE adlCreate = (ADL2_MAIN_CONTROLX3_CREATE)GetProcAddress(AdlLibHandle, "ADL2_Main_ControlX3_Create");
	return adlCreate(ADL_Main_Memory_Alloc, 1, &AdlContext, ADL_THREADING_LOCKED, INTERPRET_INCOMPATIBLE_DRIVER_VERSION_AS_SUPPORTED);
}
