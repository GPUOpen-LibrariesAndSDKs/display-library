///
///  Copyright (c) 2008 - 2018 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.c

#include <windows.h>
#include <algorithm>
#include "eyefinity.h"

#define PRINTF printf

typedef int(*ADL2_ADAPTER_ADAPTERINFOX3_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);
typedef int(*ADL2_DISPLAY_DISPLAYMAPCONFIG_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplayMap, ADLDisplayMap**  lppDisplayMap,
    int* lpNumDisplayTarget, ADLDisplayTarget** lppDisplayTarget, int iOptions);

typedef int(*ADL2_DISPLAY_SLSMAPINDEXLIST_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumSLSMapIndexList, int** lppSLSMapIndexList,
    int iOptions);

typedef int(*ADL2_DISPLAY_SLSMAPCONFIGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iSLSMapIndex, ADLSLSMap* lpSLSMap, int*                                                            lpNumSLSTarget, ADLSLSTarget** lppSLSTarget, int* lpNumStandardMode, ADLSLSMode** lppStandardMode,
    int* lpNumStandardModeOffsets, ADLSLSOffset** lppStandardModeOffsets,
    int* lpNumBezelMode, ADLBezelTransientMode** lppBezelMode, int* lpNumTransientMode,
    ADLBezelTransientMode** lppTransientMode, int* lpNumSLSOffset, ADLSLSOffset** lppSLSOffset, int iOption);

typedef int(*ADL2_MAIN_CONTROL_DESTROY) (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_MAIN_CONTROLX2_CREATE)  (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel);

ADL2_ADAPTER_ADAPTERINFOX3_GET    ADL2_Adapter_AdapterInfoX3_Get = NULL;
ADL2_DISPLAY_DISPLAYMAPCONFIG_GET ADL2_Display_DisplayMapConfig_Get = NULL;
ADL2_DISPLAY_SLSMAPINDEXLIST_GET  ADL2_Display_SLSMapIndexList_Get = NULL;
ADL2_DISPLAY_SLSMAPCONFIGX2_GET   ADL2_Display_SLSMapConfigX2_Get = NULL;
ADL2_MAIN_CONTROLX2_CREATE        ADL2_Main_ControlX2_Create = NULL;
ADL2_MAIN_CONTROL_DESTROY         ADL2_Main_Control_Destroy = NULL;

static ADL_CONTEXT_HANDLE ADLContext_ = NULL;

static bool DisplaysMatch(const ADLDisplayID& one_, const ADLDisplayID& other_);
static int  FindSLSTarget(ADLDisplayID displayID_, int numDisplays_, const ADLSLSTarget* slsTargets_);
static int  GpuBDF(const int& busNo_, const int& devNo_, const int& funcNo_) { return ((busNo_ & 0xFF) << 8) | ((devNo_ & 0x1F) << 3) | (funcNo_ & 0x07); }

static int  GetPrimaryAdpaterId(char displayName[]);
static int  GetWidth(ADLMode& oneMode_);
static int  GetHeight(ADLMode& oneMode_);

static void* __stdcall ADL_Main_Memory_Alloc(int iSize_)
{
    void* lpBuffer = malloc(iSize_);
    return lpBuffer;
}

static void __stdcall ADL_Main_Memory_Free(void** lpBuffer_)
{
    if (NULL != lpBuffer_ && NULL != *lpBuffer_) {
        free(*lpBuffer_);
        *lpBuffer_ = NULL;
    }
}

ADL_CONTEXT_HANDLE GetADLContext()
{
    return ADLContext_;
}

bool InitADL()
{
    // Load the ADL dll
    HINSTANCE hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
    if (hDLL == NULL) {
        // A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary(TEXT("atiadlxy.dll"));    
        if (hDLL == NULL) {
            PRINTF("Failed to load ADL library\n");
            return false;
        }
    }

    // Get & validate function pointers    
    ADL2_Adapter_AdapterInfoX3_Get = (ADL2_ADAPTER_ADAPTERINFOX3_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfoX3_Get");
    ADL2_Display_DisplayMapConfig_Get = (ADL2_DISPLAY_DISPLAYMAPCONFIG_GET)GetProcAddress(hDLL, "ADL2_Display_DisplayMapConfig_Get");
    ADL2_Display_SLSMapIndexList_Get = (ADL2_DISPLAY_SLSMAPINDEXLIST_GET)GetProcAddress(hDLL, "ADL2_Display_SLSMapIndexList_Get");
    ADL2_Display_SLSMapConfigX2_Get = (ADL2_DISPLAY_SLSMAPCONFIGX2_GET)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfigX2_Get");
    ADL2_Main_ControlX2_Create = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(hDLL, "ADL2_Main_ControlX2_Create");
    ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");

    if (NULL == ADL2_Adapter_AdapterInfoX3_Get ||
        NULL == ADL2_Display_DisplayMapConfig_Get ||
        NULL == ADL2_Display_SLSMapIndexList_Get ||
        NULL == ADL2_Display_SLSMapConfigX2_Get ||
        NULL == ADL2_Main_ControlX2_Create) {
        PRINTF("Failed to get ADL function pointers\n");
        return false;
    }

    if (ADL_OK != ADL2_Main_ControlX2_Create(ADL_Main_Memory_Alloc, 1, &ADLContext_, ADL_THREADING_LOCKED)) {
        PRINTF("ADL_Main_Control_Create() failed\n");
        return false;
    }
    return true;
}

void DestoryADL()
{
    if (NULL != ADL2_Main_Control_Destroy)
        ADL2_Main_Control_Destroy(ADLContext_);
}

int atiEyefinityGetConfigInfo(char OSDisplayName[], EyefinityInfoStruct *lpEyefinityInfo, int *lpNumDisplaysInfo, DisplayInfoStruct **lppDisplaysInfo)
{
    LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";
    // This is a temporary workaround to enable SLS.
    // Set this variable to any value.
    SetEnvironmentVariable(EnvironmentVariable, "TRUE");

    if (NULL == lpEyefinityInfo ||
        NULL == lpNumDisplaysInfo ||
        NULL == lppDisplaysInfo) {
        return FALSE;
    }

    if (!InitADL()) {
        PRINTF("Failed to initiliaze ADL Library!\n");
        return FALSE;
    }

    int primaryIndex = GetPrimaryAdpaterId(OSDisplayName);
    if (primaryIndex < 0) {
        PRINTF("Failed to get primary adapter id!\n");
        return FALSE;
    }

    vector<TopologyDisplay> displays(0);
    displays.reserve(24);

    int numDesktops = 0, numDisplays = 0;
    ADLDisplayMap*    adlDesktops = NULL;
    ADLDisplayTarget* adlDisplays = NULL;

    int adlRet = ADL2_Display_DisplayMapConfig_Get(GetADLContext(), primaryIndex, &numDesktops, &adlDesktops, &numDisplays, \
        &adlDisplays, ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO);
    if (ADL_OK == adlRet || ADL_OK_WARNING == adlRet) {
        for (int deskIdx = 0; deskIdx < numDesktops; deskIdx++) {
            ADLDisplayMap oneAdlDesktop = adlDesktops[deskIdx];
            ADLDisplayID preferredDisplay{ 0 };

            //If discover a rotation: swap X and Y in the mode
            if (90 == oneAdlDesktop.displayMode.iOrientation || 270 == oneAdlDesktop.displayMode.iOrientation) {
                int oldXRes = oneAdlDesktop.displayMode.iXRes;
                oneAdlDesktop.displayMode.iXRes = oneAdlDesktop.displayMode.iYRes;
                oneAdlDesktop.displayMode.iYRes = oldXRes;
            }

            // By default non-SLS; one row, one column
            int rows = 1, cols = 1;
            // By default SLsMapIndex is -1 and SLS Mode is fill
            int slsMapIndex = -1, slsMode = ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL;

            int numAdapters = 0;
            AdapterInfo* allAdapterInfo = NULL;
            ADL2_Adapter_AdapterInfoX3_Get(GetADLContext(), -1, &numAdapters, &allAdapterInfo);
            if (NULL == allAdapterInfo)
                numAdapters = 0;

            for (int dispIdx = 0, foundDisplays = 0; dispIdx < numDisplays; dispIdx++) {
                ADLDisplayTarget oneAdlDisplay = adlDisplays[dispIdx];
                if (oneAdlDisplay.iDisplayMapIndex == oneAdlDesktop.iDisplayMapIndex) {
                    if (primaryIndex == oneAdlDisplay.displayID.iDisplayPhysicalAdapterIndex) {
                        //add a display in list. For SLS this info will be updated later
                        displays.push_back(TopologyDisplay(oneAdlDisplay.displayID, 0,
                            oneAdlDesktop.displayMode.iXRes, oneAdlDesktop.displayMode.iYRes, //size
                            0, 0,	 //offset in desktop
                            0, 0)); //grid location (0-based)

                                    // count it and bail out of we found enough
                        foundDisplays++;
                        if (foundDisplays == oneAdlDesktop.iNumDisplayTarget)
                            break;
                    }
                }
            }

            if (numDisplays > 1 && displays.size() > 0) {
                TopologyDisplay firstDisplay = displays[0];
                preferredDisplay = firstDisplay.DisplayID();

                lpEyefinityInfo->iSLSWidth = adlDesktops[0].displayMode.iXRes;
                lpEyefinityInfo->iSLSHeight = adlDesktops[0].displayMode.iYRes;

                int numSLSMaps = 0;
                int* slsMapIDxs = NULL;

                if (ADL_OK == ADL2_Display_SLSMapIndexList_Get(GetADLContext(), primaryIndex, &numSLSMaps, &slsMapIDxs, ADL_DISPLAY_SLSMAPINDEXLIST_OPTION_ACTIVE)) {

                    // Declare data describing the SLS before the loop
                    ADLSLSMap slsMap;
                    int numSLSTargets = 0;
                    ADLSLSTarget* slsTargets = NULL;
                    int numStandardModes = 0, numStandardModesOffsets = 0, numBezelModes = 0, numTransientModes = 0, numBezelModesOffsets = 0;

                    ADLSLSMode* standardModes = NULL;
                    ADLBezelTransientMode *bezelModes = NULL, *transientModes = NULL;
                    ADLSLSOffset *standardModesOffsets = NULL, *bezelTransientModesOffsets = NULL;

                    for (int slsMapIdx = 0; slsMapIdx < numSLSMaps; slsMapIdx++) {
                        bool isActiveSLS = false;
                        // We got the SLS OK and it has the same number of displays as the current desktop
                        if (ADL_OK == ADL2_Display_SLSMapConfigX2_Get(GetADLContext(), primaryIndex, slsMapIDxs[slsMapIdx], &slsMap, &numSLSTargets, &slsTargets,
                            &numStandardModes, &standardModes,
                            &numStandardModesOffsets, &standardModesOffsets,
                            &numBezelModes, &bezelModes, &numTransientModes,
                            &transientModes, &numBezelModesOffsets,
                            &bezelTransientModesOffsets, ADL_DISPLAY_SLSMAPCONFIG_GET_OPTION_RELATIVETO_CURRENTANGLE)
                            && numSLSTargets == oneAdlDesktop.iNumDisplayTarget) {

                            cols = slsMap.grid.iSLSGridColumn;
                            rows = slsMap.grid.iSLSGridRow;

                            lpEyefinityInfo->iSLSGridWidth = cols;
                            lpEyefinityInfo->iSLSGridHeight = rows;

                            bool displaysMatch = true;
                            // Match slsTargets and the ones in this desktop, which are in the displays vector
                            for (int j = 0; j < numSLSTargets; j++) {
                                // find the SLS display into the display map allocation; find_if using lambda (perf: O(n))
                                auto disp = std::find_if(displays.begin(), displays.end(),
                                    [&](const TopologyDisplay& oneDisplay) { return (DisplaysMatch(oneDisplay.DisplayID(), slsTargets[j].displayTarget.displayID)); });

                                if (disp == displays.end()) {
                                    displaysMatch = false;
                                    break;
                                }
                            }

                            // Found the SLS for this desktop; see if it is active by checking if current mode is an SLS one
                            if (displaysMatch) {
                                for (int slsModeIdx = 0; slsModeIdx < numStandardModes; slsModeIdx++) {
                                    if (standardModes[slsModeIdx].displayMode.iXRes == GetWidth(oneAdlDesktop.displayMode) &&
                                        standardModes[slsModeIdx].displayMode.iYRes == GetHeight(oneAdlDesktop.displayMode)) {
                                        isActiveSLS = true;
                                        // Ditch the displays and add new ones from standardModesOffsets for this standard mode for each SLS display
                                        displays.clear();

                                        for (int oneModeOffset = 0, foundDisplays = 0; oneModeOffset < numStandardModesOffsets; oneModeOffset++) {
                                            // this is offset for the matched mode												
                                            if (standardModesOffsets[oneModeOffset].iBezelModeIndex == standardModes[slsModeIdx].iSLSModeIndex) {
                                                int index = FindSLSTarget(standardModesOffsets[oneModeOffset].displayID, numSLSTargets, slsTargets);
                                                int angle = (index != -1) ? slsTargets[index].viewSize.iOrientation : 0;
                                                int row = (index != -1) ? slsTargets[index].iSLSGridPositionY : 0;
                                                int col = (index != -1) ? slsTargets[index].iSLSGridPositionX : 0;

                                                displays.push_back(TopologyDisplay(standardModesOffsets[oneModeOffset].displayID, angle,
                                                    standardModesOffsets[oneModeOffset].iDisplayWidth, standardModesOffsets[oneModeOffset].iDisplayHeight,
                                                    standardModesOffsets[oneModeOffset].iBezelOffsetX, standardModesOffsets[oneModeOffset].iBezelOffsetY,
                                                    row, col));//increase by one

                                                // count it and bail if we found enough displays
                                                foundDisplays++;
                                                if (foundDisplays == numSLSTargets)
                                                    break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                if (!isActiveSLS) {
                                    for (int slsModeIdx = 0; slsModeIdx < numBezelModes; slsModeIdx++) {
                                        if (bezelModes[slsModeIdx].displayMode.iXRes == GetWidth(oneAdlDesktop.displayMode) &&
                                            bezelModes[slsModeIdx].displayMode.iYRes == GetHeight(oneAdlDesktop.displayMode)) {

                                            lpEyefinityInfo->iBezelCompensatedDisplay = TRUE;
                                            isActiveSLS = true;
                                            displays.clear();

                                            for (int oneModeOffset = 0, foundDisplays = 0; oneModeOffset < numBezelModesOffsets; oneModeOffset++) {
                                                if (bezelTransientModesOffsets[oneModeOffset].iBezelModeIndex == bezelModes[slsModeIdx].iSLSModeIndex) {
                                                    int index = FindSLSTarget(bezelTransientModesOffsets[oneModeOffset].displayID, numSLSTargets, slsTargets);
                                                    int angle = (index != -1) ? slsTargets[index].viewSize.iOrientation : 0;
                                                    int row = (index != -1) ? slsTargets[index].iSLSGridPositionY : 0;
                                                    int col = (index != -1) ? slsTargets[index].iSLSGridPositionX : 0;

                                                    displays.push_back(TopologyDisplay(bezelTransientModesOffsets[oneModeOffset].displayID, angle,
                                                        bezelTransientModesOffsets[oneModeOffset].iDisplayWidth, bezelTransientModesOffsets[oneModeOffset].iDisplayHeight,
                                                        bezelTransientModesOffsets[oneModeOffset].iBezelOffsetX, bezelTransientModesOffsets[oneModeOffset].iBezelOffsetY,
                                                        row, col));

                                                    foundDisplays++;
                                                    if (foundDisplays == numSLSTargets)
                                                        break;
                                                }
                                            }
                                            // Found we are on bezel SLS mode
                                            break;
                                        }
                                    }
                                }

                                if (!isActiveSLS) {
                                    for (int slsModeIdx = 0; slsModeIdx < numTransientModes; slsModeIdx++) {
                                        if (transientModes[slsModeIdx].displayMode.iXRes == GetWidth(oneAdlDesktop.displayMode) &&
                                            transientModes[slsModeIdx].displayMode.iYRes == GetHeight(oneAdlDesktop.displayMode)) {

                                            isActiveSLS = true;
                                            displays.clear();

                                            for (int oneModeOffset = 0, foundDisplays = 0; oneModeOffset < numBezelModesOffsets; oneModeOffset++) {
                                                // this is offset for the matched mode
                                                if (bezelTransientModesOffsets[oneModeOffset].iBezelModeIndex == transientModes[slsModeIdx].iSLSModeIndex)
                                                {
                                                    int index = FindSLSTarget(bezelTransientModesOffsets[oneModeOffset].displayID, numSLSTargets, slsTargets);
                                                    int angle = (index != -1) ? slsTargets[index].viewSize.iOrientation : 0;
                                                    int row = (index != -1) ? slsTargets[index].iSLSGridPositionY : 0;
                                                    int col = (index != -1) ? slsTargets[index].iSLSGridPositionX : 0;

                                                    displays.push_back(TopologyDisplay(bezelTransientModesOffsets[oneModeOffset].displayID, angle,
                                                        bezelTransientModesOffsets[oneModeOffset].iDisplayWidth, bezelTransientModesOffsets[oneModeOffset].iDisplayHeight,
                                                        bezelTransientModesOffsets[oneModeOffset].iBezelOffsetX, bezelTransientModesOffsets[oneModeOffset].iBezelOffsetY,
                                                        row, col));

                                                    foundDisplays++;
                                                    if (foundDisplays == numSLSTargets)
                                                        break;
                                                }
                                            }
                                            // Found we are on SLS transient mode
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        ADL_Main_Memory_Free((void**)&slsTargets);
                        ADL_Main_Memory_Free((void**)&standardModes);
                        ADL_Main_Memory_Free((void**)&bezelModes);
                        ADL_Main_Memory_Free((void**)&transientModes);
                        ADL_Main_Memory_Free((void**)&standardModesOffsets);
                        ADL_Main_Memory_Free((void**)&bezelTransientModesOffsets);

                        if (isActiveSLS) {
                            lpEyefinityInfo->iSLSActive = TRUE;
                            slsMapIndex = slsMapIDxs[slsMapIdx];
                            slsMode = slsMap.iSLSMapValue;
                            break;
                        }
                    }
                }
                ADL_Main_Memory_Free((void**)&slsMapIDxs);
            }
        }
    }

    // Fill out the eyefinity display info
    if (displays.size() >= 1) {
        size_t numDisplay = displays.size();
         if (*lppDisplaysInfo == NULL) {
            *lppDisplaysInfo = (DisplayInfoStruct*)malloc(sizeof(DisplayInfoStruct) * numDisplay);
            if (NULL == *lppDisplaysInfo) {
                PRINTF("ppDisplaysInfo allocation failed\n");
            }
            memset(*lppDisplaysInfo, '\0', sizeof(DisplayInfoStruct) * numDisplay);
         }
         *lpNumDisplaysInfo = (int)displays.size();
         
         for (int i = 0; i < displays.size(); i ++) {         
             DisplayInfoStruct *lpDisplaysInfo = NULL;
             lpDisplaysInfo = &((*lppDisplaysInfo)[i]);

             auto disp = displays[i];
             lpDisplaysInfo->iGridXCoord = disp.Col();
             lpDisplaysInfo->iGridYCoord = disp.Row();

             lpDisplaysInfo->displayRect.iXOffset = disp.Left();
             lpDisplaysInfo->displayRect.iYOffset = disp.Top();             
             lpDisplaysInfo->displayRect.iWidth = disp.Width();
             lpDisplaysInfo->displayRect.iHeight = disp.Height();

             lpDisplaysInfo->displayRectVisible = lpDisplaysInfo->displayRect;
             lpDisplaysInfo->iPreferredDisplay = (i == 0) ? TRUE : FALSE;
         }
    }
    DestoryADL();
    return TRUE;
}

int GetWidth(ADLMode& oneMode_)
{
    return (90 == oneMode_.iOrientation || 270 == oneMode_.iOrientation) ? oneMode_.iYRes : oneMode_.iXRes;
}

int GetHeight(ADLMode& oneMode_)
{
    return (90 == oneMode_.iOrientation || 270 == oneMode_.iOrientation) ? oneMode_.iXRes : oneMode_.iYRes;
}

int FindSLSTarget(ADLDisplayID displayID_, int numDisplays_, const ADLSLSTarget* slsTargets_)
{
    int index = -1;
    if (NULL != slsTargets_) {
        for (int i = 0; i < numDisplays_; i++) {
            if (DisplaysMatch(slsTargets_[i].displayTarget.displayID, displayID_)) {
                index = i;
                break;
            }
        }
    }
    return index;
}

bool DisplaysMatch(const ADLDisplayID& one_, const ADLDisplayID& other_)
{
    bool match = (-1 != one_.iDisplayLogicalIndex && -1 != other_.iDisplayLogicalIndex) ? (one_.iDisplayLogicalIndex == other_.iDisplayLogicalIndex) :
        (-1 == one_.iDisplayLogicalIndex && -1 != other_.iDisplayLogicalIndex) ? (one_.iDisplayPhysicalIndex == other_.iDisplayLogicalIndex) :
        (-1 != one_.iDisplayLogicalIndex && -1 == other_.iDisplayLogicalIndex) ? (one_.iDisplayLogicalIndex == other_.iDisplayPhysicalIndex) :
        false;

    if (match) {
        match = (one_.iDisplayPhysicalAdapterIndex == other_.iDisplayPhysicalAdapterIndex);
        if (!match && NULL != ADL2_Adapter_AdapterInfoX3_Get) {
            int oneBDF, otherBDF;
            LPAdapterInfo adNfo = NULL;
            ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, one_.iDisplayPhysicalAdapterIndex, NULL, &adNfo);
            if (NULL != adNfo) {
                oneBDF = GpuBDF(adNfo->iBusNumber, adNfo->iDeviceNumber, adNfo->iFunctionNumber);
                ADL_Main_Memory_Free((void**)&adNfo);

                ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, other_.iDisplayPhysicalAdapterIndex, NULL, &adNfo);
                if (NULL != adNfo) {
                    otherBDF = GpuBDF(adNfo->iBusNumber, adNfo->iDeviceNumber, adNfo->iFunctionNumber);
                    ADL_Main_Memory_Free((void**)&adNfo);
                    match = (oneBDF == otherBDF);
                }
            }
        }
    }
    return match;
}

int GetPrimaryAdpaterId(char displayName[])
{
    int adlRet = ADL_ERR;
    int numAdapters = 0;
    AdapterInfo*   allAdapterInfo = NULL;

    adlRet = ADL2_Adapter_AdapterInfoX3_Get(GetADLContext(), -1, &numAdapters, &allAdapterInfo);
    if (ADL_OK != adlRet)
        return -1;

    int primaryIndex = -1;
    for (int i = 0; i < numAdapters; i++) {
        int vendorID = allAdapterInfo[i].iVendorID;
        if (vendorID != 1002)
            continue;

        if (strcmp(allAdapterInfo[i].strDisplayName, displayName) == 0) {
            primaryIndex = allAdapterInfo[i].iAdapterIndex;
            break;
        }
    }
    return primaryIndex;
}

int atiEyefinityReleaseConfigInfo(DisplayInfoStruct **lppDisplaysInfo)
{
    ADL_Main_Memory_Free((void**)lppDisplaysInfo);
    return TRUE;
}