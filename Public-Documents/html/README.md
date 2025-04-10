# AMD Display Library (ADL) Documentation
Prior to AMD Display Library (ADL), AMD provided several SDKs (PDL, DSP, CCC, COM) for external use under Windows. The goal of ADL is to provide a single SDK to eventually replace the other SDKs.

The majority of the APIs in ADL wrap the company’s private APIs thus allowing the library’s client to access information related to the graphics associated with the system. ADL is used strictly for exposing “Graphics Hardware” support. Other hardware (such as CPUs, Northbridge, etc…) or software subsystems (such as Radeon Software) will expose an interface via their own binaries, if they require one.

ADL binaries are delivered as part of the Catalyst display Driver package while the documentation, definitions and sample code are distributed via a web-download as a single ZIP package. 

---

### ADL 18.1 - Release Notes
* Deprecate ADL2_New_QueryPMLogData_Get
* Upgraded samples:
	* <a href="Overdrive8-example.html">Overdrive8</a>: Updated sample to illustrate how to use ADL2_Overdrive8_PMLog_ShareMemory_Read.
* Added samples:
	* <a href="PanelFirmwareUpdateModeControl-example.html">PanelFirmwareUpdateModeControl</a>: Added sample to illustrate how to control features of Edp Lowpower.

---

### ADL 18.0 - Release Notes
* Introduced ADL_PMLOG_TEMPERATURE_INTAKE in ADL_PMLOG_SENSORS

---

### ADL 17.1 - Release Notes
* Introduced write and read I2C via SMU API.

---

### ADL 17.0 - Release Notes
* Introduced API to retrieve the SmartDC active state information.
* Introduced Colordepth default get API.
* Deprecate AMD Dolby Vision HDR

---

### ADL 16.0 - Release Notes
* Added samples:
	* <a href="SmartDC-example.html">SmartDC</a>: Added sample to illustrate how to control SmartDC from ADL.
	* <a href="SideBySide-example.html">SideBySide</a>: Added sample to illustrate how to use Side by Side from ADL.

---

### ADL 15.0 - Release Notes
* SDK is renewed with improved layout and documentation.
* ADL APIs are further restructured based on specific functionality.
* <a href="group__WATTMANAPI.html">Wattman Related APIs</a>: Add note for ADL2_Overdrive_Caps.
* <a href="LatestAPIS.html">New APIs</a>:
	More than 100 ADL APIs are exposed to Public section.
* New samples:
	* <a href="AMDDisplayTool-example.html">AMDDisplayTool</a>: This user interface sample demonstrates configuring different Display specific features. The list includes Virtual Super Resolution, FreeSync, GPU Scaling, Color Depth, Pixel Format and Color (Brightness, Hue, Saturation and Contrast) specific features. This sample also provides way to configure global Chill toggle functionality. This UI interface is localized based on the default UI language if it is in the supported language list.
	* <a href="PXRunningApps-example.html">PXRunningApps</a>: This sample demonstrates listing currently running applications on PowerXpress/Hybrid Graphics platform.
  * Upgraded samples:
	* <a href="Overdrive8-example.html">Overdrive8</a>: Updated sample to illustrate how to set Auto Tuning.
* Bug Fixes:
	* SWDEV-196568 - ADL Call Error With WX7100
	* SWDEV-202786 - Navi 10 ADL Fan Percentage API ADLSensorType (PMLOG_FAN_PERCENTAGE) value incorrect
	* SWDEV-217167 - GPU activity always reported 65535 in RX 5500 with ADL SDK OD8
	* SWDEV-244219 - An error about "LoadLibrary failed with error 87:parameter error" popup when do multi display test with "Onboard VGA/DP/HDMI+add-on VGA".
	* SWDEV-256913 - Factory Reset "Reboot Later" Unexpectedly Changes GECC Setting
	* SWDEV-253674 - Add missed link rate for getting link info.

---

### ADL 14.0 - Release Notes
* Introduded new samples.
* Bug fixes.

--- 

### ADL 13.0 - Release Notes
* Introduced APIs to Get & Set the power management features(Chill, Delag, Boost & ImageSharpening)
* Introduced Overdrive 8 APIs 
* Introduced API to Get Vendor ID in Hex format
* Introduced API to Get Autotuning results
* Introduced API to Get Hybrid A+I Status
* Introduced API to Get PX configuration 
* Introduded new samples.
* Bug fixes.

--- 

### ADL 12.2 - Release Notes
* Bug fixes.

--- 

### ADL 12.1 - Release Notes
* Upgraded wattman sample to supporte power gauge
* Introduced sample for VSR
* Bug fixes

--- 

### ADL 12.0 - Release Notes
* Introduced APIS to Get/Set for wattman 3.0
* Upgraded wattman sample to supported wattman 3.0
* Introduced sample for accessing ADL in windows services
* Bug fixes.

--- 

### What is ADL and How to use it
* Please refer to Usage Document. See <a href="What is ADL and how to use it.doc">`What is ADL and how to use it.doc`</a>.

--- 

### Compatibility
* Supported AMD Products:
<div>
  <img src="169768-C_RADEON_FAMILY_BADGE_E_RGB.PNG" title="AMD Radeon Graphics" margin="5" height="100"/> <img src="18161301-A_AMDRadeonProMASTER_Badge_RGB.PNG" title="AMD FirePro Graphics" margin="5" height="100"/>  
</div>

---
* Supported Operating Systems:
<div>
  <img src="Windows7.PNG" margin="5" height="100" title="Windows 7" /> <img src="Windows10.PNG" margin="5" height="100" title="Windows 10" /> 
</div>

---
* Supported Driver:
	Requires Radeon Software Adrenalin Edition 25.3.1 or later.

---
### End User License Agreement
* Please refer to EULA for terms and conditions of the ADL SDK. See <a href="ADL SDK EULA.pdf">`ADL SDK EULA.pdf`</a>.


