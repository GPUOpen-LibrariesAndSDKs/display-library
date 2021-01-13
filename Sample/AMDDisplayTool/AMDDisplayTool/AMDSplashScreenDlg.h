
// AMDSplashScreenDlg.h : header file
//

#pragma once

#include <sstream> //Need for AString
#include <fstream>
#include "Adl.h"

#define AString std::wstring
#define AStringStream std::wstringstream

enum STRING_ENCODING
{
	STR_ASCII = 0, //Ascii string
	STR_UTF8,	  //utf-8 encoding
	STR_UTF16,	 //utf-16 lencoding
	STR_UTF16L,	//utf-16 little endian encoding
	STR_UTF16B,	//utf-16 big endian encoding
	STR_UTF32,	 //utf-32 encoding
	STR_UTF32L,	//utf-32 little endian encoding
	STR_UTF32B	 //utf-32 big endian encoding
};

//Define Color Data Struct. This will be shared by Color runtime and UI
//Range data means that the Color feature has min, max, step,expected parameters
struct DisplayColorBasicStruct
{
	int Default = 0;
	int Current = 0;
	int Min = 0;
	int Max = 0;
	int Step = 0;
	bool Support = false;
	bool SourceManual = false;

	DisplayColorBasicStruct(const DisplayColorBasicStruct &input_)
	{
		Default = input_.Default;
		Current = input_.Current;
		Min = input_.Min;
		Max = input_.Max;
		Step = input_.Step;
		Support = input_.Support;
		SourceManual = input_.SourceManual;
	}

	DisplayColorBasicStruct()
	{
	}

};

class FileOperation
{
public:
	FileOperation()
	{
	}

	~FileOperation()
	{
	}

	static FILE* open(const TCHAR* file, const TCHAR* mode);
	static AString getDirPath(AString szDirPath);
	static AString getCompleteDirPath(AString szDirName);
	static AString getInfFilefromDir(AString szDirName);

	static bool isValidFileRepositoryDir(AString szDirtoDel);
	static bool isFileExists(const TCHAR *fileName);
	static bool isDirExists(const TCHAR *dirName);
	static bool isDots(const TCHAR* str);

};

// CAMDSplashScreenDlg dialog
class CAMDSplashScreenDlg : public CDialogEx
{
	// Construction
public:
	CAMDSplashScreenDlg(CWnd* pParent = NULL);	// standard 

	virtual ~CAMDSplashScreenDlg();

private:
	ULONG_PTR m_gdiplusToken;

	BOOL ImageFromIDResource(UINT nID, LPCTSTR sTR, Image *&pImg);

	void InitTextControls();
	void DrawPictureImage(UINT nItemID, UINT nID, int nWidth = 0, int nHeight = 0);
	void DrawPictureControls();
	void AdjustTitlebarControls();
	void RedrawControls(UINT nItemID);
	void RefreshControls();

	AString GetStringFromFile(UINT id, const AString& defaultString);
	bool LoadFileToBuffer(const AString& file, AString& buffer);
	void Localization();
	bool ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCTSTR szOutputFilename);
	AString GetLanguageFileName();
	const wchar_t * AppendBackSlash(const wchar_t * file);
	const TCHAR* GetLanguageFileExt();
	void replaceNoCaseW(std::wstring & str, const wchar_t * oldStr, const wchar_t * newStr);
	int replaceOnce(std::wstring & str, const wchar_t * oldStr, const wchar_t * newStr, unsigned int start);
	bool GetXMLIDTransString(const AString& buffer, UINT id, AString& tranString);
	STRING_ENCODING GetFileEnconding(const AString& file);
	bool LoadBinaryFileW(const std::wstring& file, void** buffer, long& size);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AMDSPLASHSCREEN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	HINSTANCE m_hInstance;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

private:
	int GetVirtualResolutionState(const ADLDisplayID displayID_);
	bool SetVirtualResolutionState(const ADLDisplayID displayID_, bool OnOff);

	bool GetTurboSyncSupport(const ADLDisplayID displayID_);
	int GetRegistrySetting(const ADLDisplayID displayID_, const string &subkey_, const string &key_, string &value_);
	int SetRegistrySetting(const ADLDisplayID displayID_, const string &subkey_, const string &key_, const string &value_);

	DisplayColorBasicStruct GetColorTemperatureMode(const ADLDisplayID displayID_);
	bool SetColorTemperatureMode(const ADLDisplayID displayID_, bool OnOff);
	int GetFreeSyncState();
	bool SetFreeSyncState(bool enable_);
	bool SetColorDepth(int index);
	int GetColorDepth();
	bool SetPixelFormat(int index);
	int GetPixelFormat();
	void SetColor(int CurrentVal, bool OnOff, int option);
	DisplayColorBasicStruct GetColorVal(const ADLDisplayID displayID_, int option);

	int GetChillState(const ADLDisplayID displayID_);
	bool SetChillState(const ADLDisplayID displayID_, bool OnOff);
    void GetColorInfo(const int mainControlID, int option_, int EDITID, int sliderID, CSliderCtrl& sliderCtrl, DisplayColorBasicStruct& DisplayFeature);
    void SetColorTempInit();


	CComboBox m_cbScalingMode;
	CComboBox m_cbColorDepth;
	CComboBox m_cbVerticalRefresh;
	CComboBox m_cbPixelFormat;
	CSliderCtrl m_cbColorTemperature;
	CSliderCtrl m_cbColorBrightness;
	CSliderCtrl m_cbColorHue;
	CSliderCtrl m_cbColorSaturation;
	CSliderCtrl m_cbColorContrast;
	int TemperatureSliderpos;
	DisplayColorBasicStruct pColorBasicTemperature,
		pColorBasicBright, pColorBasicHue,
		pColorBasicContrast, pColorBasicSaturation;
public:
	afx_msg void OnBnClickedCheckResolution();
	afx_msg void OnBnClickedCheckFreesync();
	afx_msg void OnBnClickedCheckGpuScaling();
	afx_msg void OnCbnSelchangeComboScalingMode();
	afx_msg void OnCbnSelchangeComboScolorDepth();
	afx_msg void OnCbnSelchangeComboPixelFormat();
	afx_msg void OnBnClickedCheckColorTempeature();
	afx_msg void OnNMCustomdrawSliderColortemperature(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderColorbrightness(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderColorhue(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderColorcontrast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderColorsaturation(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSliderColortemperature(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnStnClickedStaticResolution();
	afx_msg void OnBnClickedCheckResolutionChill();
	afx_msg void OnStnClickedStaticColorTemperature();
	afx_msg void OnStnClickedStaticMin();
	afx_msg void OnStnDblclickStaticResolutionState2();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedButton1();
	afx_msg void OnCbnSelchangeComboVerticalRefresh();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

