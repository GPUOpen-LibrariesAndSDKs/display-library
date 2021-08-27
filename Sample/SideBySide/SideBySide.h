#pragma once


#include "windows.h"
#include <vector>
#include "..\..\include\adl_sdk.h"

#define ADL_CREATE_OPTIONS_DEFAULT 					0 /*!< Treat AMD incompatible GPU as non-AMD gpu by using new vendor id -1002 */
#define INTERPRET_INCOMPATIBLE_DRIVER_VERSION_AS_SUPPORTED (1 << 0) /*!< AMD incompatible GPU means the ADL version 
																		 from current loaded ADL and the ADL from the GPU driver store is different.
																		 Set this flag, Treat AMD incompatible GPU as AMD gpu.
																		 Applications may get unpredictable result when communicate with incompatible driver
																		 using incompatible ADL */
																		 /// @}


typedef int(*ADL2_MAIN_CONTROLX3_CREATE)                                (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel, int adlCreateOptions);
typedef int(*ADL2_DRIVER_PATH_GET)                                      (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iSize, char* lpDriverPath);
typedef int(*ADL2_GRAPHICS_VERSIONSX3_GET)                              (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLVersionsInfoX2 * lpVersionsInfo);
typedef int(*ADL2_MAIN_CONTROLX2_CREATE)                                (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)                                 (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_ADAPTERINFOX4_GET)                            (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfoX2** lppAdapterInfo);

class SideBySide {
protected:
	HINSTANCE          AdlLibHandle;
	ADL_CONTEXT_HANDLE AdlContext;
	AdapterInfoX2*     AllAdapterInfo;
	int				   NumAdapters;
	bool               CompatibilityCheckDisabled;
	
public:
	SideBySide();
	SideBySide(bool disableCompatibilityCheck);
	~SideBySide();

	//Side-by-side system is characterized by presence of multiple instances of AMD graphics driver installed for different AMD GPUs. 
	//In order to detect side-by-side the unction will enumerate all GPUs while getting information about driver installed for each GPU.
	//The function will return true if presence of more then one driver version, or multiple instances of driver binaries are detected
	bool IsSydeBySideSystem();

	//Prints GPUs that are compatible with currently loaded ADL. APIs inside currently loaded ADL dll can be safely used to communicate to compatible GPUs (designated by passing adapter index).
	//The APIs will fail if called to communicate to incompatible GPU.
	void PrintCompatibleGPUs();

	//Prints GPUs that are incompatible with currently loaded ADL. APIs from currently loaded ADL dll will fail if called while passing adapter index that belongs to incompatible GPU.
    //The failure is due to ADL internal compatibility check. The GPU is considered to be incompatible with loaded ADL if GPU's driver versions is different from ADL's one. 
	//And AMD QA didn't certified the compatibility of this combination.
	void PrintIncompatibleGPUs();
	
	//Return list of adapter info structures. Single structure for each GPU incompatible to currently loaded ADL.
	std::vector<AdapterInfoX2> GetIncompatibleGPUs();

	//Unloads currently loaded ADL and reloads it from the disk loacation that contains graphics driver binaries installed for this GPU.
	void ReloadAdl(AdapterInfoX2 incompatibleGPU);

private:
	void PrintOneGPU(AdapterInfoX2* adapterInfo);
	int GpuBDF(const int& busNo_, const int& devNo_, const int& funcNo_);
	void PrintGPUs(int vendorId);
	void UnloadAdl();
	void LoadAdl(const wchar_t* adlPath);
	int MainControlCreate();
	int MainControlCreateCompatibilityCheckDisabled();
};
