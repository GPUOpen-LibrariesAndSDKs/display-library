# List of APIs
Below is the list based introducation order.

---

### APIs Intoduced in 15.0
int **ADL2\_Adapter\_AceDefaults\_Restore(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex)**
>ADL local interface Function to restore all the ACE's INF registry defaults.

int **ADL2\_Adapter\_AdapterInfoX4\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* numAdapters, AdapterInfoX2\*\* lppAdapterInfoX2)**<br>
>ADL local interface. Retrieves extended adapter information for given adapter or all OS-known adapters.

int **ADL2\_Adapter\_AdapterInfoX3\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* numAdapters, AdapterInfo\*\* lppAdapterInfo)**
>ADL local interface. Retrieves adapter information for given adapter or all OS-known adapters.

int **ADL2\_Adapter\_AdapterList\_Disable(ADL\_CONTEXT\_HANDLE context, int iNumAdapters, int \*lpAdapterIndexList, bool isSkipSaveDB = false)**
>Function to disable a list of logic adapters once.

int **ADL2\_Adapter\_BigSw\_Info\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* lpBigSwSupportMajor, int\* lpBigSwSupportMinor, int\* lpRedStoneSupport)**
>This function retrieves the Big Software & Red Stone support information for a specified adapter.
 
int **ADL2\_Adapter\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLAdapterCaps \*adapterCaps)**
>Function to retrieve adapter caps information.
  
int **ADL2\_Adapter\_ChipSetInfo\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLChipSetInfo \* lpChipSetInfo)**
>Function to retrieve chipset information.
	
int **ADL2\_Adapter\_Feature\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADL\_UIFEATURES\_GROUP iFeatureID, int \*iIsFeatureSupported)**
>Function to Checks the feature if given GPU (identified by adapter id) supports or not.
	 
int **ADL2\_Adapter\_HBC\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpHbcCapable)**
>Function to retrieve HBC Capability.
	 
int **ADL2\_Adapter\_Headless\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpHeadless)**
>Function to retrieve whether an adapter is Headless or not.

int **ADL2\_Adapter\_IsGamingDriver\_Info\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* lpCwgSupport, int\* lpIsGamingMode)**
>function retrieves the creator who game support information for a specified adapter.
	   
int **ADL2\_Adapter\_MemoryInfo2\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLMemoryInfo2 \*lpMemoryInfo2)**
>Function to retrieve memory information from the adapter. Version 2
	   
int **ADL2\_Adapter\_TRNG\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iTRNGSize, int iTRNGBufferSize, char \*lpTRNGBuffer)**
>Function to retrieve true Hardware generated random number.
		
int **ADL2\_Adapter\_Modes\_ReEnumerate(ADL\_CONTEXT\_HANDLE context)**
>Function to re-enumerate adapter modes. 
		 
int **ADL2\_Adapter\_VideoTheaterModeInfo\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* lpOverlayDisplayMode, int\* lpSavedSettings)**
>Function to get MM video theater mode info
		   
int **ADL2\_Adapter\_VideoTheaterModeInfo\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iOverlayDisplayMode, int iSavedSettings)**
>Function to set MM video theater mode info
			
int **ADL2\_Feature\_Settings\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADL\_UIFEATURES\_GROUP iFeatureID, int \*iCurrent)**
>Function to retrieve the current value of given feature on a given adapter
			
int **ADL2\_Feature\_Settings\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADL\_UIFEATURES\_GROUP iFeatureID, int iCurrent)**
>Function sets the current value  to driver of given adapter.

int **ADL2\_GcnAsicInfo\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLGcnInfo\* gcnInfo)**
>For given ASIC returns information about components of ASIC GCN architecture. Such as number of compute units, number of Tex (Texture filtering units)  , number of ROPs (render back-ends).
				
int **ADL2\_GPUVMPageSize\_Info\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* iVMPageSizeSupport, int\* iVMPageSizeType)**
>Function to gets the GPU VM PageSize info.
				
int **ADL2\_GPUVMPageSize\_Info\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iVMPageSizeType)**
>Function to sets the GPU VM PageSize info.
				 
int **ADL2\_RegisterEvent(ADL\_CONTEXT\_HANDLE context, int eventID, void\* evntHandle)**
>Allows the client to register to specific graphics driver event.
				 
int **ADL2\_UnRegisterEvent(ADL\_CONTEXT\_HANDLE context, int eventID, void\* evntHandle)**
>Allows the client to un-register from specific graphics driver event.
				 
int **ADL2\_RegisterEventX2(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int clientID, int eventID, void\* evntHandle)**
>Allows the client to register to specific graphics driver event.
				 
int **ADL2\_UnRegisterEventX2(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int clientID, int eventID, void\* evntHandle)**
>Allows the client to un-register from specific graphics driver event.
				  
int **ADL2\_PerGPU\_GDEvent\_Register(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int clientID, int eventID, void\* evntHandle)**
>Allows the client to register to specific graphics driver event for given GPU for each GPU
				
int **ADL2\_PerGPU\_GDEvent\_UnRegister(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int clientID, int eventID, void\* evntHandle)**
>Allows the client to un-register to specific graphics driver event for a given GPU or for each GPU
				 
int **ADL2\_MMD\_Features\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLFeatureCaps \*\* lppFeatureCaps, int * lpFeatureCount)**
>Function to obtain capabilities of Multi-Media features
				 
int **ADL2\_MMD\_FeatureValues\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLFeatureValues \*\* lppFeatureValues, int * lpFeatureCount)**
>Function to obtain the values and state of a Multi-Media feature
				  
int **ADL2\_MMD\_FeatureValues\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLFeatureValues \* lpFeatureValues, int iFeatureCount, int ClientID)**
>Function to set the current value of a Multi-Media feature
				   
int **ADL2\_PageMigration\_Settings\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLVirtualSegmentSettingsOutput \*lpVirtualSegSettings)**
>Function to retrieve PageMigration Global Settings.

int **ADL2\_PageMigration\_Settings\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iEnabled, int iNewSize)**
>Function to update PageMigration Global Settings.
 
int **ADL2\_MGPUSLS\_Status\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iStatus)**
>This function communicates KMD that user changed MGPU SLS feature.
  
int **ADL2\_Adapter\_CrossfireX2\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADLCrossfireComb \*lpCrossfireComb, ADLCrossfireInfo \*lpCrossfireInfo)**
>Function to get current CrossfireX settings for both QUAD and Software crossfire.
   
int **ADL2\_ElmCompatibilityMode\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpSupported, int \*lpDefault)**
>Function to retrieve Compatibility Mode setting capability.
	
int **ADL2\_ElmCompatibilityMode\_Status\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpValue)**
>Function to retrieve Compatibility Mode status whether it is On or Off.
	  
int **ADL2\_ElmCompatibilityMode\_Status\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iValue)**
>Function to update Compatibility Mode status to either On or Off.
	   
int **ADL2\_Chill\_Settings\_Notify(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iChanged)**
>Function to notify DX via KMD that Chill settings have changed.
		
int **ADL2\_Chill\_Settings\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iEnabled)**
>Function to enable or disable Global Chill.
		
int **ADL2\_Chill\_Settings\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* lpEnabled)**
>Function to get the Global Chill capabilities.
		 
int **ADL2\_Chill\_Caps\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int\* iSupported, int\* iCheckCaps)**
>Function to get the Global Chill capabilities.

int **ADL2\_PerformanceTuning\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpSupported, int \*lpDefault)**
>Function to retrieve Performance Tuning capability.
  
int **ADL2\_PerfTuning\_Status\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpPTuningValue)**
>Function to retrieve Performance ui tunning status whether it is On or Off.
   
int **ADL2\_PerfTuning\_Status\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int lpPTuningValue)**
>Function to update Performance ui tunning status to either On or Off.
   
int **ADL2\_PPW\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpSupported, int \*lpDefault)**
>Function to retrieve Performance Per Watt setting capability.
 
int **ADL2\_PPW\_Status\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*lpFPWValue)**
>Function to retrieve Performance Per Watt status whether it is On or Off.
 
int **ADL2\_PPW\_Status\_Set(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int iFPWValue)**
>Function to update Performance Per Watt status to either On or Off.
  
int **ADL2\_Adapter\_FrameMetrics\_Caps(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \*iIsFrameMonitorSupported)**
>Function to Checks if given GPU (identified by adapter id) supports Frame Monitoring Metrics or not.
   
int **ADL2\_Adapter\_FrameMetrics\_Start(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int VidPnSourceId)**
>Function to start frame metrics monitoring on GPU (identified by adapter id).
   
int **ADL2\_Adapter\_FrameMetrics\_Stop(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int VidPnSourceId)**
>Function to stop frame metrics monitoring on GPU (identified by adapter id).
	 
int **ADL2\_Adapter\_FrameMetrics\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int VidPnSourceId, float \*iFramesPerSecond)**
>Function to retrieve frame metrics information on GPU (identified by adapter id).
	  
int **ADL2\_Adapter\_FrameMetrics\_FrameDuration\_Enable(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADL\_FRAME\_DURATION\_HANDLE\* frameDurationHandle)**
>Function to Enable frame metrics time stamp recording on any GPU.
	   
int **ADL2\_Adapter\_FrameMetrics\_FrameDuration\_Disable(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, ADL\_FRAME\_DURATION\_HANDLE\* frameDurationHandle)**
>Function to Disable frame metrics time stamp recording on any GPU.
	   
int **ADL2\_Adapter\_FrameMetrics\_FrameDuration\_Start(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int VidPnSourceId)**;
>Function to start frame metrics time stamp recording on GPU (identified by adapter id).
		
int **ADL2\_Adapter\_FrameMetrics\_FrameDuration\_Stop(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int VidPnSourceId)**;
>Function to stop frame metrics time stamp recording on GPU (identified by adapter id).
		  
int **ADL2\_Adapter\_FrameMetrics\_FrameDuration\_Get(ADL\_CONTEXT\_HANDLE context, ADL\_FRAME\_DURATION\_HANDLE frameDurationHandle, unsigned long long \* pFrameDurationsArr, unsigned int frameDurationsArrSize, unsigned int \*elementsCopied)**
>Function to retrieve frame metrics frame duration information on GPU (identified by adapter id).
	
int **ADL2\_Adapter\_VRAMUsage\_Get(ADL\_CONTEXT\_HANDLE context, int iAdapterIndex, int \* iVRAMUsageInMB)**
>Function to get the current GPU VRAM usage.