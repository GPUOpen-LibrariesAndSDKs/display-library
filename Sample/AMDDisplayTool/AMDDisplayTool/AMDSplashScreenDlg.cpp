
// AMDSplashScreenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AMDSplashScreen.h"
#include "AMDSplashScreenDlg.h"
#include "afxdialogex.h"
#include "GPUScaling.h"

#include <iostream>
#include <string>
#include <codecvt>
#include <list>
#include <algorithm>
#include <Winnls.h>
#include <unordered_map>
#pragma comment(lib,"Kernel32.lib")

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UMD_REG     				"UMD"
#define R3D_VSYNC_NAME              "VSyncControl"
#define R3D_TURBOSYNC_NAME          "TurboSync"

#define ID_STR_MESSAGE 1
#define ID_STR_VIRTUAL 2
#define ID_STR_FREESYNC 3
#define ID_STR_SCALING 4
#define ID_STR_MODE 5
#define ID_STR_DEPTHL 6
#define ID_STR_FORMAT 7
#define ID_STR_CHILL 8
#define ID_STR_TEMPERATURE 9
#define ID_STR_BRIGHTNESS 10
#define ID_STR_HUE 11
#define ID_STR_SATURATION 12
#define ID_STR_CONTRAST 13
#define ID_STR_RESET 14
#define ID_STR_ON 15
#define ID_STR_OFF 16
#define ID_STR_NOTSUPPORT 17
#define ID_STR_RATIO 18
#define ID_STR_PANEL 19
#define ID_STR_CENTER 20
#define ID_STR_AUTO 21
#define ID_STR_ALWAYS_OFF 22
#define ID_STR_OFF_UNLESS_APPLICATION_SPECIFIES 23
#define ID_STR_ON_UNLESS_APPLICATION_SPECIFIES 24
#define ID_STR_ALWAYS_ON 25
#define ID_STR_ENHANCED_SYNC 26
#define ID_STR_VERTICAL_REFRESH 27

FILE* FileOperation::open(const TCHAR* file, const TCHAR* mode)
{
#if _WINDOWS
	FILE* f;
	_wfopen_s(&f, file, mode);
	return f;
#else
	return _wfopen(file, mode);
#endif
}

AString FileOperation::getDirPath(AString szDirPath)
{
	AString szCompDirPath;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = NULL;
	memset(FindFileData.cFileName, 0, sizeof(FindFileData.cFileName));
	hFind = FindFirstFile(szDirPath.data(), &FindFileData);
	AString szTmp = FindFileData.cFileName;
	FindClose(hFind);
	size_t nPos = szDirPath.find_last_of(L"/\\");
	szCompDirPath = szDirPath.substr(0, nPos + 1);
	szCompDirPath.append(szTmp);
	return szCompDirPath;
}

AString FileOperation::getCompleteDirPath(AString szDirName)
{
	AString szReturnPth(MAX_PATH + 1, 0);
	int nStartingPos = (int)szDirName.find_first_of(L"%");
	int nEndingPos = (int)szDirName.find_last_of(L"%");

	int CLSid;
	AString tmpStr = szDirName.substr(nStartingPos + 1, nEndingPos - 1);
	if (strcmp((char *)tmpStr.c_str(), (char *)_T("APPDATA")) == 0)
		CLSid = CSIDL_APPDATA;

	if (strcmp((char *)tmpStr.c_str(), (char *)_T("LOCALAPPDATA")) == 0)
		CLSid = CSIDL_LOCAL_APPDATA;

	SHGetFolderPath(NULL, CLSid, 0, NULL, &szReturnPth[0]);
	return szReturnPth;
}

AString FileOperation::getInfFilefromDir(AString szDirName)
{
	AString szInfFile;
	WIN32_FIND_DATA data;
	AString szTmpDirName = szDirName;
	szTmpDirName.append(L"\\*.*");
	HANDLE hFind = FindFirstFile(szTmpDirName.c_str(), &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			AString szTemp = data.cFileName;
			if (szTemp.find(_T(".inf")) != -1)
			{
				szInfFile = szDirName;
				szInfFile.append(_T("\\"));
				szInfFile.append(szTemp);
			}
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	return szInfFile;
}

bool FileOperation::isValidFileRepositoryDir(AString szDirtoDel)
{
	bool bretval = true;
	size_t nLen = szDirtoDel.length();
	size_t nStartPos = nLen - 16;
	AString szStr = szDirtoDel.substr(nStartPos, 15);

	if (szStr.compare(_T("FileRepository\\")) == 0)
		bretval = false;
	return bretval;
}

bool FileOperation::isFileExists(const TCHAR *fileName)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(fileName);
	if (0xFFFFFFFF == fileAttr)
		return false;
	return true;
}

bool FileOperation::isDirExists(const TCHAR *dirName)
{
	DWORD ftyp = GetFileAttributesW(dirName);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;// this is a directory!

	return false;// this is not a directory!
}

bool FileOperation::isDots(const TCHAR* str)
{
	if (_tcscmp(str, L".") && _tcscmp(str, L".."))
		return FALSE;
	return TRUE;
}

// CAMDSplashScreenDlg dialog

class StringOperation
{
public:
	StringOperation()
	{
	}

	~StringOperation()
	{
	}

	template<typename Type>
	static Type toLowerCase(const Type& srcString)
	{
		Type str(srcString);
		Type::iterator cIter;
		for (cIter = str.begin(); cIter != str.end(); cIter++)
		{
			*cIter = tolower(*cIter);
		}
		return str;
	}

	static std::string toStr(const AString& wstr);
	static std::string toStr(const wchar_t* wstr);
	static std::string toStr(const wchar_t* wstr, std::size_t strLen);
	static std::size_t toStr(char *mbstr, std::size_t sizeInWords, const wchar_t *wcstr, std::size_t count);

	static AString toWStr(const char* str);
	static AString toWStr(const char* str, const std::size_t strLen);
	static std::size_t toWStr(wchar_t *wcstr, size_t sizeInWords, const char *mbstr, std::size_t count);

	static AString intToStr(int number);
};

std::string StringOperation::toStr(const AString& wstr)
{
	return toStr(wstr.c_str());
}


std::string StringOperation::toStr(const wchar_t* wstr)
{
	std::size_t count = wcslen(wstr);
	return toStr(wstr, count);
}

std::string StringOperation::toStr(const wchar_t* wstr, std::size_t strLen)
{
	std::size_t count = strLen;
	count++;
	char *mbstr = new char[count];
	std::size_t i = toStr(mbstr, count, wstr, count);
	if (i == count)
	{
		mbstr[i - 1] = '\0';
	}
	else if (i < count && i > 0)
	{
		mbstr[i] = '\0';
	}
	else
	{
		mbstr[0] = '\0';
	}
	std::string s = mbstr;
	std::string eos = "\0";
	s += eos.c_str();
	delete[] mbstr;
	return s;
}

std::size_t StringOperation::toStr(char *mbstr, std::size_t sizeInWords, const wchar_t *wcstr, std::size_t count)
{
#if _WINDOWS
	size_t pReturnValue;
	wcstombs_s(&pReturnValue, mbstr, sizeInWords, wcstr, count);
	return pReturnValue;
#else
	return wcstombs(mbstr, wcstr, count);
#endif
}

AString StringOperation::toWStr(const char* str)
{
	size_t count = strlen(str);
	return toWStr(str, count);
}

AString StringOperation::toWStr(const char* str, const std::size_t strLen)
{
	size_t count = strLen;
	count++;
	wchar_t *wcstr = new wchar_t[count];
	size_t i = toWStr(wcstr, count, str, count);
	if (i == count)
	{
		wcstr[i - 1] = '\0';
	}
	else if (i < count && i > 0)
	{
		wcstr[i] = '\0';
	}
	else
	{
		wcstr[0] = '\0';
	}

	AString ws = wcstr;
	AString eos = L"\0";
	ws += eos.c_str();
	delete[] wcstr;
	return ws;
}

std::size_t StringOperation::toWStr(wchar_t *wcstr, size_t sizeInWords, const char *mbstr, std::size_t count)
{
#if _WINDOWS
	size_t pReturnValue;
	mbstowcs_s(&pReturnValue, wcstr, sizeInWords, mbstr, count);
	return pReturnValue;
#else
	return mbstowcs(wcstr, mbstr, count);
#endif
}

AString StringOperation::intToStr(int number)
{
	AString Result; //string which will contain the result
	AStringStream convert; // stringstream used for the conversion
	convert << number; //add the value of Number to the characters in the stream
	Result = convert.str();
	return Result;
}

// Provides access to System Services object through which other classes can access internal exposed functions of System Services.
// return - global copy for System Services object

CAMDSplashScreenDlg::CAMDSplashScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_AMDSPLASHSCREEN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);

	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	Localization();
}

CAMDSplashScreenDlg::~CAMDSplashScreenDlg()
{
	GdiplusShutdown(m_gdiplusToken);
}

void CAMDSplashScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SCALING_MODE, m_cbScalingMode);
	DDX_Control(pDX, IDC_COMBO_SCOLOR_DEPTH, m_cbColorDepth);
	DDX_Control(pDX, IDC_COMBO_VERTICAL_REFRESH, m_cbVerticalRefresh);
	DDX_Control(pDX, IDC_COMBO_PIXEL_FORMAT, m_cbPixelFormat);
	DDX_Control(pDX, IDC_SLIDER_COLORTEMPERATURE, m_cbColorTemperature);
	DDX_Control(pDX, IDC_SLIDER_COLORBRIGHTNESS, m_cbColorBrightness);
	DDX_Control(pDX, IDC_SLIDER_COLORHUE, m_cbColorHue);
	DDX_Control(pDX, IDC_SLIDER_COLORCONTRAST, m_cbColorContrast);
	DDX_Control(pDX, IDC_SLIDER_COLORSATURATION, m_cbColorSaturation);
}

BEGIN_MESSAGE_MAP(CAMDSplashScreenDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_CHECK_RESOLUTION, &CAMDSplashScreenDlg::OnBnClickedCheckResolution)
	ON_BN_CLICKED(IDC_CHECK_FREESYNC, &CAMDSplashScreenDlg::OnBnClickedCheckFreesync)
	ON_BN_CLICKED(IDC_CHECK_GPU_SCALING, &CAMDSplashScreenDlg::OnBnClickedCheckGpuScaling)
	ON_BN_CLICKED(IDC_CHECK_COLOR_TEMPEATURE, &CAMDSplashScreenDlg::OnBnClickedCheckColorTempeature)
	ON_BN_CLICKED(IDC_CHECK_RESOLUTION_CHILL, &CAMDSplashScreenDlg::OnBnClickedCheckResolutionChill)
	ON_CBN_SELCHANGE(IDC_COMBO_SCALING_MODE, &CAMDSplashScreenDlg::OnCbnSelchangeComboScalingMode)
	ON_CBN_SELCHANGE(IDC_COMBO_SCOLOR_DEPTH, &CAMDSplashScreenDlg::OnCbnSelchangeComboScolorDepth)
	ON_CBN_SELCHANGE(IDC_COMBO_PIXEL_FORMAT, &CAMDSplashScreenDlg::OnCbnSelchangeComboPixelFormat)
	ON_CBN_SELCHANGE(IDC_COMBO_VERTICAL_REFRESH, &CAMDSplashScreenDlg::OnCbnSelchangeComboVerticalRefresh)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_COLORTEMPERATURE, &CAMDSplashScreenDlg::OnNMCustomdrawSliderColortemperature)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_COLORBRIGHTNESS, &CAMDSplashScreenDlg::OnNMCustomdrawSliderColorbrightness)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_COLORHUE, &CAMDSplashScreenDlg::OnNMCustomdrawSliderColorhue)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_COLORCONTRAST, &CAMDSplashScreenDlg::OnNMCustomdrawSliderColorcontrast)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_COLORSATURATION, &CAMDSplashScreenDlg::OnNMCustomdrawSliderColorsaturation)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_COLORTEMPERATURE, &CAMDSplashScreenDlg::OnNMReleasedcaptureSliderColortemperature)
	ON_STN_CLICKED(IDC_STATIC_RESOLUTION, &CAMDSplashScreenDlg::OnStnClickedStaticResolution)
	ON_STN_CLICKED(IDC_STATIC_COLOR_TEMPERATURE, &CAMDSplashScreenDlg::OnStnClickedStaticColorTemperature)
	ON_STN_CLICKED(IDC_STATIC_MIN, &CAMDSplashScreenDlg::OnStnClickedStaticMin)

	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

typedef enum tag_SCALING_MODE
{
	SCALING_MODE_PRESERVE_ASPECT_RATIO = 0,
	SCALING_MODE_FULL_PANEL,
	SCALING_MODE_CENTER
}SCALING_MODE;

typedef enum tag_VERTICAL_REFRESH
{
	VERTICAL_REFRESH_ALWAYS_OFF = 0,
	VERTICAL_REFRESH_OFF_UNLESS_APPLICATION_SPECIFIES,
	VERTICAL_REFRESH_ON_UNLESS_APPLICATION_SPECIFIES,
	VERTICAL_REFRESH_ALWAYS_ON,
	VERTICAL_REFRESH_ENHANCED_SYNC
}VERTICAL_REFRESH;

static const int & sc_margin_x = 10;
static const int & sc_margin_y = 30;
static const int & sc_logo_width = 50;
static const int & sc_logo_height = 12;
static const int & sc_min_width = 16;
static const int & sc_min_height = 16;
static const int & sc_max_width = 16;
static const int & sc_max_height = 16;
static const int & sc_exit_width = 16;
static const int & sc_exit_height = 16;
// CAMDSplashScreenDlg message handlers

BOOL CAMDSplashScreenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
									//
	CWnd::SetWindowPos(NULL, 0, 0, 1208, 658, SWP_NOZORDER | SWP_NOMOVE);
	//CWnd::SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);

	// TODO: Add extra initialization here
	AdjustTitlebarControls();
	InitTextControls();

	//
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) ^ 0x80000);
	/*m_cbBitmap.LoadBitmaps(IDB_PNG15, IDB_PNG15);*/
	HINSTANCE  hInst = LoadLibrary(_T("User32.DLL"));
	if (hInst != NULL)
	{
		typedef BOOL(WINAPI *MYFUNC)(HWND, COLORREF, BYTE, DWORD);

		MYFUNC pFunc = (MYFUNC)GetProcAddress(hInst, "SetLayeredWindowAttributes");
		if (pFunc != NULL)
		{
			pFunc(m_hWnd, 0, 240, LWA_ALPHA);
		}
		FreeLibrary(hInst);
		hInst = NULL;
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAMDSplashScreenDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
        CDialogEx::OnPaint();
        DrawPictureControls();

	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAMDSplashScreenDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CAMDSplashScreenDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	UINT tmp = pWnd->GetDlgCtrlID();
	// TODO:  Change any attributes of the DC here
	if (pWnd->GetDlgCtrlID() == (IDC_STATIC1) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC2) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC3) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC4) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC5) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC6) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC7) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC8) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC9) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC10) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC11) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC12) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC13)
		)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 0));

		return HBRUSH(GetStockObject(NULL_BRUSH));
	}

	if (pWnd->GetDlgCtrlID() == (IDC_STATIC_RESOLUTION_STATE) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC_FREESYNC_STATE) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC_GPU_SCALING_STATE) ||
		pWnd->GetDlgCtrlID() == (IDC_STATIC_CHILL_STATE)
		)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(96, 96, 96));

		return HBRUSH(GetStockObject(NULL_BRUSH));
	}

	if ((pWnd->GetDlgCtrlID() == IDC_CHECK_RESOLUTION) ||
		(pWnd->GetDlgCtrlID() == IDC_CHECK_FREESYNC) ||
		(pWnd->GetDlgCtrlID() == IDC_CHECK_GPU_SCALING) ||
		(pWnd->GetDlgCtrlID() == IDC_CHECK_COLOR_TEMPEATURE) ||
		(pWnd->GetDlgCtrlID() == IDC_CHECK_RESOLUTION_CHILL)
		)
	{
		pDC->SetBkMode(TRANSPARENT);
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);

		CDC* dc = GetDC();
		pDC->BitBlt(0, 0, rc.Width(), rc.Height(), dc, rc.left, rc.top, SRCCOPY);
		ReleaseDC(dc);
		return (HBRUSH) ::GetStockObject(NULL_BRUSH);
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CAMDSplashScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);

	// TODO: Add your message handler code here and/or call default
	if ((point.y > 0) && (point.y < rect.Height()))
	{
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}

	if ((point.x >= rect.Width() - 65 && point.x <= rect.Width() - 40) && (point.y >= 0 && point.y <= 50))
	{
		ShowWindow(SW_SHOWMINIMIZED);
	}

	if ((point.x >= rect.Width() - 30 && point.x <= rect.Width() - 5) && (point.y >= 0 && point.y <= 50))
	{
		exit(0);
	}

	/*if ((point.x >= 850 && point.x <= 870) && (point.y >= 0 && point.y <= 50))
	{
		ShowWindow(SW_SHOWMAXIMIZED);
	}*/

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CAMDSplashScreenDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);

	// TODO: Add your message handler code here and/or call default
	if ((point.x >= rect.Width() - 65 && point.x <= rect.Width() - 40) && (point.y >= 0 && point.y <= 50))
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}

	if ((point.x >= rect.Width() - 30 && point.x <= rect.Width() - 5) && (point.y >= 0 && point.y <= 50))
	{
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

///////////////////////////////////////////////////////////////////////////////
//
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////
BOOL CAMDSplashScreenDlg::ImageFromIDResource(UINT nID, LPCTSTR sTR, Image *&pImg)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nID), sTR); // type  
	if (!hRsrc)
		return FALSE;

	// load resource into memory  
	DWORD len = SizeofResource(hInst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
	if (!lpRsrc)
		return FALSE;

	// Allocate global memory on which to create stream  
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);
	GlobalUnlock(m_hMem);

	IStream* pstm = nullptr;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
	pImg = Gdiplus::Image::FromStream(pstm);// load from stream  

	// free/release stuff  
	pstm->Release();
	FreeResource(lpRsrc);
	GlobalFree(m_hMem);
	return TRUE;
}

void CAMDSplashScreenDlg::DrawPictureImage(UINT nItemID, UINT nID, int nWidth, int nHeight)
{
	CClientDC *pDC = new CClientDC(GetDlgItem(nItemID));
	CRect rect;
	GetDlgItem(nItemID)->GetWindowRect(&rect);

	Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
	Image *pimage; // Construct an image
	ImageFromIDResource(nID, _T("PNG"), pimage);

	graphics.DrawImage(pimage, 0, 0, (nWidth > 0) ? nWidth : rect.Width(), (nHeight > 0) ? nHeight : rect.Height());

	delete pDC, pDC = nullptr;
}

void CAMDSplashScreenDlg::DrawPictureControls()
{
    Invalidate(TRUE);
    //Background
    {
        CPaintDC dc(this);
        CRect rect;
        GetClientRect(&rect);

        CDC dcMem;
        dcMem.CreateCompatibleDC(&dc);

        CBitmap bmpBackground;
        bmpBackground.LoadBitmap(IDB_BITMAP2);

        BITMAP bitmap;
        bmpBackground.GetBitmap(&bitmap);
        CBitmap* pbmpOld = dcMem.SelectObject(&bmpBackground);

        //dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
        dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

        dcMem.SelectObject(pbmpOld);
        dcMem.DeleteDC();
    }

    //AMD Logo
    DrawPictureImage(IDC_STATIC_LOGO, IDB_PNG1, sc_logo_width, sc_logo_height);

	CRect clientRect;
	GetClientRect(&clientRect);
    //Minimize
    DrawPictureImage(IDC_STATIC_MIN, IDB_PNG4, sc_min_width, sc_min_height);
	//{
	//	CClientDC* pDC = new CClientDC(GetDlgItem(IDC_STATIC_MIN));
	//	CRect rect;
	//	GetDlgItem(IDC_STATIC_MIN)->GetWindowRect(&rect);

	//	Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
	//	Image* pimage; // Construct an image
	//	ImageFromIDResource(IDB_PNG4, _T("PNG"), pimage);

	//	graphics.DrawImage(pimage,
	//		clientRect.left + clientRect.Width() - 40 - sc_min_width - sc_exit_width,
	//		//clientRect.right - sc_margin_x*2 - sc_exit_width - sc_min_width,
	//		clientRect.top + 15,
	//		sc_min_width, 
	//		sc_min_height);

	//	delete pDC, pDC = nullptr;
	//}
    //Exit
    DrawPictureImage(IDC_STATIC_EXIT, IDB_PNG3, sc_exit_width, sc_exit_height);
	//{
	//	CClientDC* pDC = new CClientDC(GetDlgItem(IDC_STATIC_EXIT));
	//	CRect rect;
	//	GetDlgItem(IDC_STATIC_EXIT)->GetWindowRect(&rect);

	//	Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
	//	Image* pimage; // Construct an image
	//	ImageFromIDResource(IDB_PNG3, _T("PNG"), pimage);

	//	graphics.DrawImage(pimage,
	//		clientRect.left + clientRect.Width() - sc_margin_x - sc_exit_width,
	//		//clientRect.right - sc_margin_x - sc_exit_width,
	//		clientRect.top + 15,
	//		sc_exit_width,
	//		sc_exit_height);

	//	delete pDC, pDC = nullptr;
	//}


    //VIRTUAL SUPER RESOLUTION
    {

        {
            CClientDC *pDC = new CClientDC(GetDlgItem(IDC_STATIC_SELECTED));
            CRect rect;
            GetDlgItem(IDC_STATIC_SELECTED)->GetWindowRect(&rect);

            Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
            Image *pimage; // Construct an image

            bool bState = true;
            std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &bState](const std::pair<int, ADLDisplayID*>& kv) {
                if (GetVirtualResolutionState(*(kv.second)) == 0)
                {
                    bState = false;
                }
            });

            if (bState)
            {
                ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
                AString szMessage;
                szMessage = GetStringFromFile(ID_STR_ON, szMessage);
                GetDlgItem(IDC_STATIC_RESOLUTION_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_RESOLUTION))->SetCheck(true);
            }
            else
            {
                ImageFromIDResource(IDB_PNG9, _T("PNG"), pimage);
                AString szMessage;
                szMessage = GetStringFromFile(ID_STR_OFF, szMessage);
                GetDlgItem(IDC_STATIC_RESOLUTION_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_RESOLUTION))->SetCheck(false);
            }

            graphics.DrawImage(pimage, 0, 0, rect.Width(), rect.Height());
            delete pDC, pDC = nullptr;
        }

        DrawPictureImage(IDC_STATIC_RESOLUTION, IDB_PNG7);
    }

    {//FreeSync
        {
            CClientDC *pDC = new CClientDC(GetDlgItem(IDC_STATIC_FREESYNC_SELECTED));
            CRect rect;
            GetDlgItem(IDC_STATIC_FREESYNC_SELECTED)->GetWindowRect(&rect);

            Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
            Image *pimage; // Construct an image

            int state = GetFreeSyncState();
            AString szMessage;
            switch (state)
            {
            case 1: // Enabled
                ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
                szMessage = GetStringFromFile(ID_STR_ON, szMessage);
                GetDlgItem(IDC_STATIC_FREESYNC_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_FREESYNC))->SetCheck(true);
                break;
            case 0: // Disabled
                ImageFromIDResource(IDB_PNG9, _T("PNG"), pimage);
                szMessage = GetStringFromFile(ID_STR_OFF, szMessage);
                GetDlgItem(IDC_STATIC_FREESYNC_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_FREESYNC))->SetCheck(false);
                break;
            default:// Not Supported
                ImageFromIDResource(IDB_PNG9, _T("PNG"), pimage);
                szMessage = GetStringFromFile(ID_STR_NOTSUPPORT, szMessage);
                GetDlgItem(IDC_STATIC_FREESYNC_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_FREESYNC))->EnableWindow(false);
                break;
            }

            graphics.DrawImage(pimage, 0, 0, rect.Width(), rect.Height());

            delete pDC, pDC = nullptr;
        }

        DrawPictureImage(IDC_STATIC_FREESYNC, IDB_PNG7);
    }

    {//GPU SCALING
        GPUScaling GPUScale(ADLApp_context.adlContext);

        bool state = true;
        std::for_each(std::begin(DisplayID), std::end(DisplayID), [&GPUScale, &state](const std::pair<int, ADLDisplayID*>& kv) {
            GPUScale.GetGPUScalingEnable(*(kv.second), state);
        });

        {
            CClientDC *pDC = new CClientDC(GetDlgItem(IDC_STATIC_GPU_SCALING_SELECTED));
            CRect rect;
            GetDlgItem(IDC_STATIC_GPU_SCALING_SELECTED)->GetWindowRect(&rect);

            Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
            Image *pimage; // Construct an image
            if (state)
            {
                ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
                AString szMessage;
                szMessage = GetStringFromFile(ID_STR_ON, szMessage);
                GetDlgItem(IDC_STATIC_GPU_SCALING_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_GPU_SCALING))->SetCheck(true);
            }
            else
            {
                ImageFromIDResource(IDB_PNG9, _T("PNG"), pimage);
                AString szMessage;
                szMessage = GetStringFromFile(ID_STR_OFF, szMessage);
                GetDlgItem(IDC_STATIC_GPU_SCALING_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_GPU_SCALING))->SetCheck(false);
            }

            graphics.DrawImage(pimage, 0, 0, rect.Width(), rect.Height());

            delete pDC, pDC = nullptr;
        }
        DrawPictureImage(IDC_STATIC_GPU_SCALING, IDB_PNG7);
    }

    { //SCALING MODE
        DrawPictureImage(IDC_STATIC_SCALING_MODE, IDB_PNG7);
    }

    { //Color Pixel
        DrawPictureImage(IDC_STATIC_COLOR_DEPTH, IDB_PNG7);
    }

    { //Pixel Format
        DrawPictureImage(IDC_STATIC_PIXEL_FORMAT, IDB_PNG7);
    }

    //Color Tempeature
    {
        {
            SetColorTempInit();
        }

        DrawPictureImage(IDC_STATIC_COLOR_TEMPERATURE, IDB_PNG7);
    }

    //Color Brightness
    {
        {

            GetColorInfo(IDC_STATIC_COLOR_BRIGHTNESS2, ADL_DISPLAY_COLOR_BRIGHTNESS, IDC_EDIT3, IDC_SLIDER_COLORBRIGHTNESS,
                m_cbColorBrightness, pColorBasicBright);
        }

        DrawPictureImage(IDC_STATIC_COLOR_BRIGHTNESS2, IDB_PNG7);
    }

    //Color Hue
    {
        {
            GetColorInfo(IDC_STATIC_COLOR_HUE, ADL_DISPLAY_COLOR_HUE, IDC_EDIT4, IDC_SLIDER_COLORHUE,
                m_cbColorHue, pColorBasicHue);
        }
        DrawPictureImage(IDC_STATIC_COLOR_HUE, IDB_PNG7);
    }

    //Color Contrast
    {
        {
            GetColorInfo(IDC_STATIC_COLOR_CONTRAST, ADL_DISPLAY_COLOR_CONTRAST, IDC_EDIT6, IDC_SLIDER_COLORCONTRAST,
                m_cbColorContrast, pColorBasicContrast);
        }

        DrawPictureImage(IDC_STATIC_COLOR_CONTRAST, IDB_PNG7);
    }

    //Color Saturation
    {
        {
            GetColorInfo(IDC_STATIC_COLOR_SATURATION, ADL_DISPLAY_COLOR_SATURATION, IDC_EDIT5, IDC_SLIDER_COLORSATURATION,
                m_cbColorSaturation, pColorBasicSaturation);
        }
        DrawPictureImage(IDC_STATIC_COLOR_SATURATION, IDB_PNG7);
    }

    //CHILL
    {
        {
            CClientDC *pDC = new CClientDC(GetDlgItem(IDC_STATIC_SELECTED2));
            CRect rect;
            GetDlgItem(IDC_STATIC_SELECTED2)->GetWindowRect(&rect);

            Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
            Image *pimage; // Construct an image

            bool bState = true;
            std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &bState](const std::pair<int, ADLDisplayID*>& kv) {
                if (!GetChillState(*(kv.second)))
                {
                    bState = false;
                }
            });

            if (bState)
            {
                ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
                AString szMessage;
                szMessage = GetStringFromFile(ID_STR_ON, szMessage);
                GetDlgItem(IDC_STATIC_CHILL_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_RESOLUTION_CHILL))->SetCheck(true);
            }
            else
            {
                ImageFromIDResource(IDB_PNG9, _T("PNG"), pimage);
                AString szMessage;
                szMessage = GetStringFromFile(ID_STR_OFF, szMessage);
                GetDlgItem(IDC_STATIC_CHILL_STATE)->SetWindowText(szMessage.c_str());
                ((CButton *)GetDlgItem(IDC_CHECK_RESOLUTION_CHILL))->SetCheck(false);
            }

            graphics.DrawImage(pimage, 0, 0, rect.Width(), rect.Height());

            delete pDC, pDC = nullptr;
        }

        DrawPictureImage(IDC_STATIC_RESOLUTION2, IDB_PNG7);
    }

    { //VERTICAL REFRESH
        DrawPictureImage(IDC_STATIC_VERTICAL_REFRESH, IDB_PNG7);
    }

    //RefreshControls();
}
void CAMDSplashScreenDlg::SetColorTempInit()
{
    CClientDC *pDC = new CClientDC(GetDlgItem(IDC_STATIC_COLOR_TEMPERATURE_SELECTED));
    CRect rect;
    GetDlgItem(IDC_STATIC_COLOR_TEMPERATURE_SELECTED)->GetWindowRect(&rect);

    Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
    Image *pimage; // Construct an image
    DisplayColorBasicStruct pColorBasic;
    bool bState = false;
    std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &bState, &pColorBasic](const std::pair<int, ADLDisplayID*>& kv) {
        pColorBasic = GetColorTemperatureMode(*(kv.second));
        if (pColorBasic.SourceManual)
        {
            bState = true;
        }
    });

    pColorBasicTemperature = pColorBasic;
    if (bState)
    {
        ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
        std::wstringstream tempVal;
        tempVal << pColorBasic.Current;
        std::wstring str;
        tempVal >> str;
        ((CEdit *)GetDlgItem(IDC_EDIT2))->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT2)->SetWindowText(str.c_str());
        ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->SetCheck(true);
        //slider
        m_cbColorTemperature.SetRange(pColorBasic.Min, pColorBasic.Max);
        m_cbColorTemperature.SetTicFreq(pColorBasic.Step);
        m_cbColorTemperature.SetPos(pColorBasic.Current);
        TemperatureSliderpos = pColorBasic.Current;
        ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_COLORTEMPERATURE))->EnableWindow(TRUE);
    }
    else
    {
        ImageFromIDResource(IDB_PNG9, _T("PNG"), pimage);
        AString szMessage;
        szMessage = GetStringFromFile(ID_STR_AUTO, szMessage);
        GetDlgItem(IDC_EDIT2)->SetWindowText(szMessage.c_str());
        ((CEdit *)GetDlgItem(IDC_EDIT2))->EnableWindow(FALSE);
        ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->SetCheck(false);

        ((CSliderCtrl*)GetDlgItem(IDC_SLIDER_COLORTEMPERATURE))->EnableWindow(FALSE);
    }

    graphics.DrawImage(pimage, 0, 0, rect.Width(), rect.Height());

    delete pDC, pDC = nullptr;
}

void CAMDSplashScreenDlg::GetColorInfo(const int mainControlID, int option_, int EDITID, int sliderID, CSliderCtrl& sliderCtrl, DisplayColorBasicStruct& DisplayFeature)
{
    CClientDC *pDC = new CClientDC(GetDlgItem(mainControlID));
    CRect rect;
    GetDlgItem(mainControlID)->GetWindowRect(&rect);

    Graphics graphics(pDC->m_hDC); // Create a GDI+ graphics object  
    Image *pimage; // Construct an image
    DisplayColorBasicStruct pColorBasic;
    std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &pColorBasic, &option_](const std::pair<int, ADLDisplayID*>& kv) {
        pColorBasic = GetColorVal(*(kv.second), option_);
    });
    DisplayFeature = pColorBasic;
    ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
    std::wstringstream tempVal;
    tempVal << pColorBasic.Current;
    std::wstring str;
    tempVal >> str;
    GetDlgItem(IDC_EDIT3)->SetWindowText(str.c_str());
    //slider
    ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
    sliderCtrl.SetRange(pColorBasic.Min, pColorBasic.Max);
    sliderCtrl.SetTicFreq(pColorBasic.Step);
    sliderCtrl.SetPos(pColorBasic.Current);
    ((CSliderCtrl*)GetDlgItem(EDITID))->EnableWindow(TRUE);
    graphics.DrawImage(pimage, 0, 0, rect.Width(), rect.Height());

    delete pDC, pDC = nullptr;
}


DisplayColorBasicStruct CAMDSplashScreenDlg::GetColorTemperatureMode(const ADLDisplayID displayID_)
{
	int temperatureSource;
	//bool SourceManual = false;
	DisplayColorBasicStruct *pColorBasic = new DisplayColorBasicStruct();
	if (ADL_OK == ADL2_Display_ColorTemperatureSource_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &temperatureSource))
	{
		//If the color temperature source is manual, get the supported value range.
		if (temperatureSource == ADL_DISPLAY_COLOR_TEMPERATURE_SOURCE_USER)
			pColorBasic->SourceManual = true;
		else
			pColorBasic->SourceManual = false;

		if (ADL_OK != ADL2_Display_Color_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, ADL_DISPLAY_COLOR_TEMPERATURE,
			&(pColorBasic->Current), &(pColorBasic->Default), &(pColorBasic->Min), &(pColorBasic->Max), &(pColorBasic->Step)))
		{
			std::cout << L"Failed to get color temperature values";
		}
	}
	return *pColorBasic;
}

void CAMDSplashScreenDlg::AdjustTitlebarControls()
{
	CRect clientRect, editRect;
	GetClientRect(clientRect);

	//AMD Logo
	GetDlgItem(IDC_STATIC_LOGO)->GetWindowRect(editRect);
	GetDlgItem(IDC_STATIC_LOGO)->SetWindowPos(NULL,
		clientRect.left + 15,
		clientRect.top + 15,
		sc_logo_width,
		sc_logo_height,
		SWP_NOZORDER | SWP_NOSIZE);

	GetDlgItem(IDC_STATIC_MIN)->GetWindowRect(editRect);
	GetDlgItem(IDC_STATIC_MIN)->SetWindowPos(NULL,
		clientRect.left + clientRect.Width() - 40 - sc_min_width - sc_exit_width,
		//clientRect.right - sc_margin_x*2 - sc_exit_width - sc_min_width,
		clientRect.top + 15,
		sc_min_width,
		sc_min_height,
		SWP_NOZORDER | SWP_NOSIZE);


	/*GetDlgItem(IDC_STATIC_MAX)->GetWindowRect(editRect);
	GetDlgItem(IDC_STATIC_MAX)->SetWindowPos(NULL,
		clientRect.left + clientRect.Width() - 3 - sc_margin_x - sc_max_width - sc_exit_width ,
		clientRect.top + 15,
		sc_max_width,
		sc_max_height,
		SWP_NOZORDER | SWP_NOSIZE);*/

	GetDlgItem(IDC_STATIC_EXIT)->GetWindowRect(editRect);
	GetDlgItem(IDC_STATIC_EXIT)->SetWindowPos(NULL,
		clientRect.left + clientRect.Width() - sc_margin_x - sc_exit_width,
		//clientRect.right - sc_margin_x - sc_exit_width,
		clientRect.top + 15,
		sc_exit_width,
		sc_exit_height,
		SWP_NOZORDER | SWP_NOSIZE);
}

void CAMDSplashScreenDlg::InitTextControls()
{
	AString szMessage;
	szMessage = GetStringFromFile(ID_STR_VIRTUAL, szMessage);
	GetDlgItem(IDC_STATIC1)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_FREESYNC, szMessage);
	GetDlgItem(IDC_STATIC2)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_SCALING, szMessage);
	GetDlgItem(IDC_STATIC3)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_MODE, szMessage);
	GetDlgItem(IDC_STATIC4)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_DEPTHL, szMessage);
	GetDlgItem(IDC_STATIC5)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_FORMAT, szMessage);
	GetDlgItem(IDC_STATIC6)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_CHILL, szMessage);
	GetDlgItem(IDC_STATIC7)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_TEMPERATURE, szMessage);
	GetDlgItem(IDC_STATIC8)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_BRIGHTNESS, szMessage);
	GetDlgItem(IDC_STATIC9)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_HUE, szMessage);
	GetDlgItem(IDC_STATIC10)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_SATURATION, szMessage);
	GetDlgItem(IDC_STATIC11)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_CONTRAST, szMessage);
	GetDlgItem(IDC_STATIC12)->SetWindowText(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_VERTICAL_REFRESH, szMessage);
	GetDlgItem(IDC_STATIC13)->SetWindowText(szMessage.c_str());
	//szMessage = GetStringFromFile(ID_STR_RESET, szMessage);
	//GetDlgItem(IDC_BUTTON1)->SetWindowText(szMessage.c_str());

	//
	szMessage = GetStringFromFile(ID_STR_RATIO, szMessage);
	m_cbScalingMode.AddString(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_PANEL, szMessage);
	m_cbScalingMode.AddString(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_CENTER, szMessage);
	m_cbScalingMode.AddString(szMessage.c_str());

	GPUScaling GPUScale(ADLApp_context.adlContext);
	GPUSCALINGMODES GPUScalingModes = SCALING_FullPanel;
	int color_depth = 0, pixel_format= 0;
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &GPUScale, &GPUScalingModes, &color_depth, &pixel_format](const std::pair<int, ADLDisplayID*>& kv) {
		GPUScalingModes = GPUScale.GetDisplayScalingMode(*(kv.second));
		color_depth = GetColorDepth();
		pixel_format = GetPixelFormat();
		});

	m_cbScalingMode.SetCurSel(GPUScalingModes);

	m_cbColorDepth.AddString(L"6 dpc");
	m_cbColorDepth.AddString(L"8 dpc");
	m_cbColorDepth.SetCurSel(color_depth);

	m_cbPixelFormat.AddString(L"RGB 4:4:4 Pixel Format PC Standard(Full RGB)");
	m_cbPixelFormat.AddString(L"YCbCr 4:4:4 Pixel Format");
	m_cbPixelFormat.AddString(L"YCbCr 4:2:2 Pixel Format");
	m_cbPixelFormat.AddString(L"RGB 4:4:4 Pixel Format Studio(Limited RGB)");
	m_cbPixelFormat.SetCurSel(pixel_format);

	szMessage = GetStringFromFile(ID_STR_ALWAYS_OFF, szMessage);
	m_cbVerticalRefresh.AddString(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_OFF_UNLESS_APPLICATION_SPECIFIES, szMessage);
	m_cbVerticalRefresh.AddString(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_ON_UNLESS_APPLICATION_SPECIFIES, szMessage);
	m_cbVerticalRefresh.AddString(szMessage.c_str());
	szMessage = GetStringFromFile(ID_STR_ALWAYS_ON, szMessage);
	m_cbVerticalRefresh.AddString(szMessage.c_str());

	bool bFlag = false;
	for (auto iter = DisplayID.begin(); iter != DisplayID.end(); ++iter) {
		if (GetTurboSyncSupport(*((*iter).second)) && !bFlag) {
			szMessage = GetStringFromFile(ID_STR_ENHANCED_SYNC, szMessage);
			m_cbVerticalRefresh.AddString(szMessage.c_str());
			bFlag = true;
		}

		string value;
		if (ADL_OK == GetRegistrySetting(*((*iter).second), UMD_REG, R3D_TURBOSYNC_NAME, value))
		{
			std::string::size_type sz;   // alias of size_t

			try
			{
				int sel = std::stoi(value, &sz);
				if (sel == 0)
				{
					if (ADL_OK == GetRegistrySetting(*((*iter).second), UMD_REG, R3D_VSYNC_NAME, value))
					{
						sel = std::stoi(value, &sz);
						m_cbVerticalRefresh.SetCurSel(sel);
					}
				}
				else
				{
					m_cbVerticalRefresh.SetCurSel(VERTICAL_REFRESH_ENHANCED_SYNC);
				}
			}
			catch (std::invalid_argument&)
			{
				// if no conversion could be performed, handle as same as R3D_TURBOSYNC_NAME is not existed
			}
		}
	}
}

void CAMDSplashScreenDlg::RedrawControls(UINT nItemID)
{
	CWnd* pWnd = GetDlgItem(nItemID);
	pWnd->ShowWindow(SW_HIDE);
	pWnd->ShowWindow(SW_SHOW);
	//CClientDC *pDC = new CClientDC(pWnd);
	//pDC->SetBkMode(TRANSPARENT);
	//pDC->SetTextColor(RGB(0, 0, 0));

	//CRect rc;
	//pWnd->GetWindowRect(&rc);
	//ScreenToClient(&rc);
	//InvalidateRect(rc, TRUE);
}

void CAMDSplashScreenDlg::RefreshControls()
{
	RedrawControls(IDC_STATIC1);
	RedrawControls(IDC_STATIC2);
	RedrawControls(IDC_STATIC3);
	RedrawControls(IDC_STATIC4);
	RedrawControls(IDC_STATIC5);
	RedrawControls(IDC_STATIC6);
	RedrawControls(IDC_STATIC7);
	RedrawControls(IDC_STATIC8);
	RedrawControls(IDC_STATIC9);
	RedrawControls(IDC_STATIC10);
	RedrawControls(IDC_STATIC11);
	RedrawControls(IDC_STATIC12);
	RedrawControls(IDC_STATIC13);

	RedrawControls(IDC_STATIC_RESOLUTION_STATE);
	RedrawControls(IDC_STATIC_FREESYNC_STATE);
	RedrawControls(IDC_STATIC_GPU_SCALING_STATE);
	RedrawControls(IDC_STATIC_CHILL_STATE);

	RedrawControls(IDC_CHECK_RESOLUTION);
	RedrawControls(IDC_CHECK_FREESYNC);
	RedrawControls(IDC_CHECK_GPU_SCALING);
	RedrawControls(IDC_CHECK_COLOR_TEMPEATURE);
	RedrawControls(IDC_CHECK_RESOLUTION_CHILL);

	RedrawControls(IDC_COMBO_SCALING_MODE);
	RedrawControls(IDC_COMBO_SCOLOR_DEPTH);
	RedrawControls(IDC_COMBO_PIXEL_FORMAT);
	RedrawControls(IDC_COMBO_VERTICAL_REFRESH);

}

void CAMDSplashScreenDlg::Localization()
{
	m_hInstance = AfxGetResourceHandle();
	TCHAR szTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, szTempDir);
	//GetModuleFileName(NULL,szTempDir, sizeof(szTempDir));
	std::wstring m_szCIMXMLPath;
	if (sizeof(szTempDir) > 0)
	{
		std::basic_string<TCHAR> szStrtemp(szTempDir);
		m_szCIMXMLPath = szStrtemp;
	}

	ExtractResource(m_hInstance, IDR_BINARY16, (m_szCIMXMLPath + L"CHS.xml").c_str());
	ExtractResource(m_hInstance, IDR_CHT_STRS, (m_szCIMXMLPath + L"CHT.xml").c_str());
	ExtractResource(m_hInstance, IDR_CZE_STRS, (m_szCIMXMLPath + L"CZE.xml").c_str());
	ExtractResource(m_hInstance, IDR_DAN_STRS, (m_szCIMXMLPath + L"DAN.xml").c_str());
	ExtractResource(m_hInstance, IDR_DEU_STRS, (m_szCIMXMLPath + L"DEU.xml").c_str());
	ExtractResource(m_hInstance, IDR_ELL_STRS, (m_szCIMXMLPath + L"ELL.xml").c_str());
	ExtractResource(m_hInstance, IDR_ELSA_STRS, (m_szCIMXMLPath + L"ELSA.xml").c_str());
	ExtractResource(m_hInstance, IDR_FIN_STRS, (m_szCIMXMLPath + L"FIN.xml").c_str());
	ExtractResource(m_hInstance, IDR_FRA_STRS, (m_szCIMXMLPath + L"FRA.xml").c_str());
	ExtractResource(m_hInstance, IDR_HUN_STRS, (m_szCIMXMLPath + L"HUN.xml").c_str());
	ExtractResource(m_hInstance, IDR_ITA_STRS, (m_szCIMXMLPath + L"ITA.xml").c_str());
	ExtractResource(m_hInstance, IDR_JPN_STRS, (m_szCIMXMLPath + L"JPN.xml").c_str());
	ExtractResource(m_hInstance, IDR_KOR_STRS, (m_szCIMXMLPath + L"KOR.xml").c_str());
	ExtractResource(m_hInstance, IDR_NLD_STRS, (m_szCIMXMLPath + L"NLD.xml").c_str());
	ExtractResource(m_hInstance, IDR_NOR_STRS, (m_szCIMXMLPath + L"NOR.xml").c_str());
	ExtractResource(m_hInstance, IDR_POL_STRS, (m_szCIMXMLPath + L"POL.xml").c_str());
	ExtractResource(m_hInstance, IDR_POR_STRS, (m_szCIMXMLPath + L"POR.xml").c_str());
	ExtractResource(m_hInstance, IDR_RUS_STRS, (m_szCIMXMLPath + L"RUS.xml").c_str());
	ExtractResource(m_hInstance, IDR_SWE_STRS, (m_szCIMXMLPath + L"SWE.xml").c_str());
	ExtractResource(m_hInstance, IDR_THA_STRS, (m_szCIMXMLPath + L"THA.xml").c_str());
	ExtractResource(m_hInstance, IDR_TUR_STRS, (m_szCIMXMLPath + L"TUR.xml").c_str());
	ExtractResource(m_hInstance, IDR_BINARY17, (m_szCIMXMLPath + L"ENU.xml").c_str());

	AString g_sLocalefile = GetLanguageFileName();
}

AString CAMDSplashScreenDlg::GetLanguageFileName()
{
	AStringStream filepath;
	TCHAR szTempDirPath[MAX_PATH];
	GetTempPath(MAX_PATH, szTempDirPath);
	filepath << szTempDirPath;
	filepath << AppendBackSlash(filepath.str().c_str());
	filepath << GetLanguageFileExt();

	AString file = filepath.str();
	replaceNoCaseW(file, _T("/"), _T("\\"));
	return file;
}

const wchar_t * CAMDSplashScreenDlg::AppendBackSlash(const wchar_t * file)
{
	size_t len = wcslen(file);
	if (file[len - 1] == '/' || file[len - 1] == '\\')
		return L"";
#ifdef _WINDOWS
	return L"\\";
#else
	return L"/";
#endif

}

void CAMDSplashScreenDlg::replaceNoCaseW(std::wstring & str, const wchar_t * oldStr, const wchar_t * newStr)
{
	if (oldStr == 0 || wcscmp(oldStr, L"") == 0)
		return; //no good, who the hell wants to repace ""?
	if (wcscmp(oldStr, newStr) == 0)
		return; //no need to replace
	unsigned int start = 0;
	while (start != -1)
	{
		start = replaceOnce(str, oldStr, newStr, start);
	}
}

int CAMDSplashScreenDlg::replaceOnce(std::wstring & str, const wchar_t * oldStr, const wchar_t * newStr, unsigned int start)
{
	unsigned int pos = 0;
	size_t len = wcslen(oldStr);
	std::wstring tmp = str;
	unsigned int j = 0;
	int end = -1;
	size_t newLen = wcslen(newStr);
	for (unsigned int i = start; i < tmp.length(); i++)
	{
		if (towlower(tmp[i]) == towlower(oldStr[j]))
		{
			j++;
			if (j == len)
			{
				str.replace(i - j + 1, len, newStr);
				if (newLen == 0)
					end = i;
				else
					end = i + 1;
				break;
			}
		}
		else
			j = 0; //reset
	}
	return end;
}

const TCHAR* CAMDSplashScreenDlg::GetLanguageFileExt()
{
	/////////////////////////////
	// Hardcode for simplicity //
	/////////////////////////////
	LANGID m_nLangId = GetUserDefaultUILanguage();
	//Set the language to chinese simplified
	//m_nLangId = 2052;
	//LCID m_nLangId = GetSystemDefaultLCID();
	//TODO - Append language codes
	switch (m_nLangId)
	{
	case 1028:
		return _T("CHT.xml");
	case 1029:
		return _T("CZE.xml");
	case 1030:
		return _T("DAN.xml");
	case 1031:
		return _T("DEU.xml");
	case 1032:
		return _T("ELL.xml");
	case 1034:
		return _T("ESLA.xml");
	case 1035:
		return _T("FIN.xml");
	case 1036:
		return _T("FRA.xml");
	case 1038:
		return _T("HUN.xml");
	case 1040:
		return _T("ITA.xml");
	case 1041:
		return _T("JPN.xml");
	case 1042:
		return _T("KOR.xml");
	case 1043:
		return _T("NLD.xml");
	case 1044:
		return _T("NOR.xml");
	case 1045:
		return _T("POL.xml");
	case 1046:
		return _T("POR.xml");
	case 1049:
		return _T("RUS.xml");
	case 1053:
		return _T("SWE.xml");
	case 1054:
		return _T("THA.xml");
	case 1055:
		return _T("TUR.xml");
	case 2052:
		return _T("CHS.xml");
	default:
		return _T("ENU.xml");
	}
}


bool CAMDSplashScreenDlg::ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCTSTR szOutputFilename)
{
	bool bSuccess = false;
	try
	{
		// First find and load the required resource
		HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), _T("BINARY"));
		HGLOBAL hFileResource = LoadResource(hInstance, hResource);

		// Now open and map this to a disk file
		LPVOID lpFile = LockResource(hFileResource);
		DWORD dwSize = SizeofResource(hInstance, hResource);

		// Open the file and filemap
		HANDLE hFile = CreateFile(szOutputFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		HANDLE hFilemap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, dwSize, NULL);
		LPVOID lpBaseAddress = MapViewOfFile(hFilemap, FILE_MAP_WRITE, 0, 0, 0);

		// Write the file

		CopyMemory(lpBaseAddress, lpFile, dwSize);

		// Unmap the file and close the handles

		UnmapViewOfFile(lpBaseAddress);
		CloseHandle(hFilemap);
		CloseHandle(hFile);
	}
	catch (...)
	{
		// Ignore all type of errors
	}
	return bSuccess;
}

AString CAMDSplashScreenDlg::GetStringFromFile(UINT id, const AString& defaultString)
{
	AString buffer;
	AString g_sLocalefile = GetLanguageFileName();
	if (!LoadFileToBuffer(g_sLocalefile, buffer))
		return defaultString;

	//Parse buffer!
	size_t posStart = buffer.find(_T("<Language "));
	if (posStart == AString::npos)
		return defaultString;

	size_t posEnd = buffer.find(_T("</Language>"), posStart + 1);
	AString tranString = defaultString;
	while (posEnd != AString::npos)
	{
		AString product = buffer.substr(posStart, posEnd - posStart);
		posStart = posEnd;
		if (GetXMLIDTransString(product, id, tranString))
			return tranString;
		break; //? what's the reason "break" appear here

			   //posEnd = buffer.find(_T("</Language>"), posStart+1);
	}
	return defaultString;
}

bool CAMDSplashScreenDlg::GetXMLIDTransString(const AString& buffer, UINT id, AString& tranString)
{
	AString buf = StringOperation::toLowerCase<AString>(buffer);
	replaceNoCaseW(buf, _T("'"), _T("\""));
	AStringStream strID;
	strID << _T("value=\"") << id << _T("\"");
	size_t posStart = buf.find(strID.str());
	if (posStart == AString::npos)
		return false;

	posStart = buf.find(_T(">"), posStart);
	size_t posEnd = buf.find(_T("<"), posStart);
	if (posEnd == AString::npos)
		return false;

	posStart++;
	tranString = buffer.substr(posStart, posEnd - posStart);
	return true;
}

bool CAMDSplashScreenDlg::LoadFileToBuffer(const AString& file, AString& buffer)
{
	try
	{
		STRING_ENCODING ftype = GetFileEnconding(file);
		if (ftype == STR_ASCII)
		{
			AString afile = file;

			struct _stat s;
			memset(&s, 0, sizeof(s));
			_wstat(afile.c_str(), &s);

			if (0 == s.st_size)
				return false;

			FILE* fp;
			_wfopen_s(&fp, afile.c_str(), _T("rb"));
			if (NULL == fp) return false;

			char* pBuf = new char[s.st_size + 1];
			if (NULL == pBuf)
				return false;
			memset(pBuf, 0, sizeof(char)*(s.st_size + 1));

			fread(pBuf, sizeof(char)*s.st_size, 1, fp);
			pBuf[s.st_size] = '\0';
			wchar_t* pBufUnicode = new wchar_t[s.st_size + 1];
			if (NULL == pBufUnicode)
			{
				if (NULL != pBuf)
				{
					delete[] pBuf;
					pBuf = NULL;
				}
				return false;
			}
			memset(pBufUnicode, 0, sizeof(wchar_t)*(s.st_size + 1));

			::MultiByteToWideChar(CP_ACP, 0, pBuf, (int)strlen(pBuf), pBufUnicode, s.st_size);
			pBufUnicode[s.st_size] = '\0';
			AStringStream tmp;
			tmp << pBufUnicode << _T("\0");
			buffer = tmp.str();

			if (NULL != fp)
			{
				fclose(fp);
				fp = NULL;
			}
			if (NULL != pBufUnicode)
			{
				delete[] pBufUnicode;
				pBufUnicode = NULL;
			}
			if (NULL != pBuf)
			{
				delete[] pBuf;
				pBuf = NULL;
			}
			return true;
		}
		else if (ftype == STR_UTF8)
		{
			std::wifstream in(StringOperation::toStr(file).c_str());
			if (!in)
				return false;
			std::locale loc("");
			std::ios::sync_with_stdio(false);
			in.imbue(loc);
			std::wstringstream tmp;
			tmp << in.rdbuf();
			buffer = tmp.str();
			return true;
		}
		//loading unicode file!

		void* buf = 0;
		long bufSize = 0;
		if (!LoadBinaryFileW(file, &buf, bufSize))
			return false;
		//Note: we have to watch out for this type of characters!
		//EF BB BF		 UTF-8
		//FE FF		 UTF-16/UCS-2, little endian
		//FF FE		 UTF-16/UCS-2, big endian
		//FF FE 00 00	 UTF-32/UCS-4, little endian.
		//00 00 FE FF	 UTF-32/UCS-4, big-endian.
		bool remove = false;
		int num = 0;
		const wchar_t* tmpChar = (const wchar_t*)buf;
		if (bufSize > 4)
		{
			if (tmpChar[0] == 0xEFFE && tmpChar[1] == 0x0000)
			{
				remove = true;
				num = 2; //remove first 4 bytes
			}
			else if (tmpChar[0] == 0x0000 && tmpChar[1] == 0xFEFF)
			{
				remove = true;
				num = 2; //remove first 4 bytes
			}
			else if (tmpChar[0] == 0xEFBB && (tmpChar[1] | 0xFF00) == 0xBF)
			{
				remove = true;
				num = 3; //remove first 3 bytes
			}
			else if (tmpChar[0] == 0xFEFF)
			{
				remove = true;
				num = 1; //remove first 2 bytes
			}
			else if (tmpChar[0] == 0xFFFE)
			{
				remove = true;
				num = 1; //remove first 2 bytes
			}
		}
		else if (bufSize > 3)
		{
			if (tmpChar[0] == 0xEFBB && tmpChar[1] == 0xBF)
			{
				remove = true;
				num = 3; //remove first 2 bytes
			}
			else if (tmpChar[0] == 0xFE && tmpChar[1] == 0xFF)
			{
				remove = true;
				num = 2; //remove first 2 bytes
			}
			else if (tmpChar[0] == 0xFF && tmpChar[1] == 0xFE)
			{
				remove = true;
				num = 2; //remove first 2 bytes
			}
		}
		else if (bufSize > 2)
		{
			if (tmpChar[0] == 0xFE && tmpChar[1] == 0xFF)
			{
				remove = true;
				num = 2; //remove first 2 bytes
			}
			else if (tmpChar[0] == 0xFF && tmpChar[1] == 0xFE)
			{
				remove = true;
				num = 2; //remove first 2 bytes
			}
		}
		if (remove)
		{
			tmpChar += num;
		}

		buffer = tmpChar;
		buffer += L"\0";
		free(buf);
	}
	catch (...)
	{
		OutputDebugString(_T("*** Error Unknown exception on LoadBuffer\n"));
		return false;
	}
	return true;
}

STRING_ENCODING CAMDSplashScreenDlg::GetFileEnconding(const AString& file)
{
	STRING_ENCODING ret = STR_ASCII;
	FILE *source = NULL;
	source = FileOperation::open(file.c_str(), _T("rb"));

	if (source == 0)
		return ret;

	void* buffer = malloc(4);
	if (buffer == 0)
		return ret;
	memset(buffer, 0, 4);
	// copy the file into the buffer.
	fread(buffer, 1, 4, source);
	fclose(source);

	//EF BB BF		 UTF-8
	//FE FF		 UTF-16/UCS-2, little endian
	//FF FE		 UTF-16/UCS-2, big endian
	//FF FE 00 00	 UTF-32/UCS-4, little endian.
	//00 00 FE FF	 UTF-32/UCS-4, big-endian.
	const wchar_t* tmpChar = (const wchar_t*)buffer;
	if (tmpChar[0] == 0xEFFE && tmpChar[1] == 0x0000)
		ret = STR_UTF32L;
	else if (tmpChar[0] == 0x0000 && tmpChar[1] == 0xFEFF)
		ret = STR_UTF32B;
	else if (tmpChar[0] == 0xEFBB && (tmpChar[1] | 0xFF00) == 0xBF)
		ret = STR_UTF8;
	else if (tmpChar[0] == 0xFEFF)
		ret = STR_UTF16L;
	else if (tmpChar[0] == 0xFFFE)
		ret = STR_UTF16B;
	free(buffer);
	return ret;
}

bool CAMDSplashScreenDlg::LoadBinaryFileW(const std::wstring& file, void** buffer, long& size)
{
	FILE * source = FileOperation::open(file.c_str(), L"rb");
	if (source == 0)
		return false;

	fseek(source, 0, SEEK_END);
	// obtain file size.
	size = ftell(source);
	rewind(source);

	*buffer = malloc(size + 2);
	if (*buffer == 0)
		return false;

	memset(*buffer, 0, size + 2);
	// copy the file into the buffer.
	fread(*buffer, 1, size, source);
	fclose(source);
	return true;
}

int CAMDSplashScreenDlg::GetVirtualResolutionState(const ADLDisplayID displayID_)
{
	ADLDisplayProperty DisplayProperty;
	memset(&DisplayProperty, 0, sizeof(DisplayProperty));
	DisplayProperty.iSize = sizeof(DisplayProperty);
	DisplayProperty.iPropertyType = ADL_DL_DISPLAYPROPERTY_TYPE_DOWNSCALE;
	DisplayProperty.iDefault = -1;
	DisplayProperty.iCurrent = -1;

	if (ADL_OK == ADL2_Display_Property_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &DisplayProperty))
	{
		return (DisplayProperty.iCurrent == 1) ? 1 : 0;
	}
	return -1;
}

bool CAMDSplashScreenDlg::SetVirtualResolutionState(const ADLDisplayID displayID_, bool OnOff)
{
	ADLDisplayProperty DisplayProperty;
	memset(&DisplayProperty, 0, sizeof(DisplayProperty));
	DisplayProperty.iSize = sizeof(DisplayProperty);
	DisplayProperty.iPropertyType = ADL_DL_DISPLAYPROPERTY_TYPE_DOWNSCALE;
	DisplayProperty.iDefault = -1;
	DisplayProperty.iCurrent = -1;
	
	if ((GetVirtualResolutionState(displayID_) == 1 ? true : false) == OnOff)
		return true;

	DisplayProperty.iCurrent = OnOff;
	if (ADL_OK == ADL2_Display_Property_Set(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, &DisplayProperty))
	{
		if ((GetVirtualResolutionState(displayID_) == 1 ? true : false) == OnOff)
			return true;
	}

	return false;
}

bool CAMDSplashScreenDlg::GetTurboSyncSupport(const ADLDisplayID displayID_)
{
	int turboSyncSupport = 0;

	if (NULL != ADL2_TurboSyncSupport_Get)
	{
		ADL2_TurboSyncSupport_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, &turboSyncSupport);
	}
	return (turboSyncSupport == 1);
}

int CAMDSplashScreenDlg::SetRegistrySetting(const ADLDisplayID displayID_, const string &subkey_, const string &key_, const string &value_)
{
	int result = ADL2_Adapter_RegValueString_Set(ADLApp_context.adlContext,
		displayID_.iDisplayLogicalAdapterIndex,
		ADL_REG_DEVICE_FUNCTION_1,
		subkey_.c_str(),
		key_.c_str(),
		(int)(value_.length()) * sizeof(wchar_t),
		(char*)value_.c_str());


	if (ADL_OK == result)
	{
		value_;
	}
	return result;
}

int CAMDSplashScreenDlg::GetRegistrySetting(const ADLDisplayID displayID_, const string &subkey_, const string &key_, string &value_)
{
	char regValue[1024] = { 0 };

	int ret = ADL2_Adapter_RegValueString_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, ADL_REG_DEVICE_FUNCTION_1, subkey_.c_str(), key_.c_str(), 1024, regValue);

	if (ADL_OK == ret)
	{
		//Remove Slash Zero from String:
		//If the registry key value type is REG_SZ, when do CI call to read registry value, driver always add '\0'( 00 00) behind each character
		//For example: "AB" in registry, driver will returns A\0B\0, so the '\0' should be removed.
		//If the registry key value type is not REG_SZ, the following code also works.
		int pos = 0;
		unsigned int ii = 0;

		for (ii = 0; ii < 1024; ii++)
		{
			if (regValue[ii] != '\0')
			{
				regValue[pos++] = regValue[ii];
			}
		}

		if (pos < 1024)
		{
			regValue[pos] = '\0';
		}


		value_ = string(regValue);
	}

	return ret;
}

//-1: Not support
// 1: Enabled
// 0: Disabled
int CAMDSplashScreenDlg::GetFreeSyncState()
{
	int state = 0;
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &state](const std::pair<int, ADLDisplayID*>& kv) {
		int default = 0;
		int current = 0;
		int minRefreshRateInMicroHz = 0;
		int maxRefreshRateInMicroHz = 0;
		ADLDisplayID *displayID = kv.second;

		if (ADL_OK == ADL2_Display_FreeSyncState_Get(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex,
			&current, &default, &minRefreshRateInMicroHz, &maxRefreshRateInMicroHz))
		{
			std::cout << "    Current Enable: " << (current ? "True" : "False") << std::endl;
			std::cout << "    Default Enable: " << (ADL2_Display_FreeSyncState_Get ? "True" : "False") << std::endl;
			std::cout << "    Min Refresh Rate(Hz): " << minRefreshRateInMicroHz << std::endl;
			std::cout << "    Min Refresh Rate(Hz): " << maxRefreshRateInMicroHz << std::endl;
			state = current ? 1 : 0;
		}
		else
		{
			std::cout << "    Not support FreeSync!" << std::endl;
			state = -1;
		}
		});

	return state;
}

bool CAMDSplashScreenDlg::SetFreeSyncState(bool onOff_)
{
	bool ret = true;
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, onOff_, &ret](const std::pair<int, ADLDisplayID*>& kv) {
		int default = 0;
		int current = 0;
		int minRefreshRateInMicroHz = 0;
		int maxRefreshRateInMicroHz = 0;
		ADLDisplayID *displayID = kv.second;

		if (ADL_OK == ADL2_Display_FreeSyncState_Get(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex,
			&current, &default, &minRefreshRateInMicroHz, &maxRefreshRateInMicroHz))
		{
			// turn on/off free sync
			int freeSynStatus = 0;

			if (onOff_)
			{
				freeSynStatus = (ADL_FREESYNC_USECASE_STATIC | ADL_FREESYNC_USECASE_VIDEO | ADL_FREESYNC_USECASE_GAMING);
			}
			//apply the new state
			if (ADL_OK != ADL2_Display_FreeSyncState_Set(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, freeSynStatus, 0))
			{
				ret = false;
			}
		}
		});
	return ret;
}

void CAMDSplashScreenDlg::OnBnClickedCheckResolution()
{
	int state = ((CButton *)GetDlgItem(IDC_CHECK_RESOLUTION))->GetCheck();
	bool OnOff = (state == 1 ? true : false);

	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, OnOff](const std::pair<int, ADLDisplayID*>& kv) {
		SetVirtualResolutionState(*(kv.second), OnOff);
		});

	RedrawWindow();
}

void CAMDSplashScreenDlg::OnBnClickedCheckFreesync()
{
	int state = ((CButton *)GetDlgItem(IDC_CHECK_FREESYNC))->GetCheck();
	bool OnOff = (state == 1 ? true : false);
	SetFreeSyncState(OnOff);

	RedrawWindow();
}

void CAMDSplashScreenDlg::OnBnClickedCheckGpuScaling()
{
	int state = ((CButton *)GetDlgItem(IDC_CHECK_GPU_SCALING))->GetCheck();
	bool OnOff = (state == 1 ? true : false);

	GPUScaling GPUScale(ADLApp_context.adlContext);

    int sel = m_cbScalingMode.GetCurSel();
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [sel, &GPUScale, OnOff](const std::pair<int, ADLDisplayID*>& kv) {
		GPUScale.SetGPUScalingEnable(*(kv.second), OnOff);
		GPUScale.SetDisplayScalingMode(*(kv.second), (GPUSCALINGMODES)sel);
		});

	RedrawWindow();
}

void CAMDSplashScreenDlg::OnCbnSelchangeComboScalingMode()
{
	GPUScaling GPUScale(ADLApp_context.adlContext);

	int sel = m_cbScalingMode.GetCurSel();
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [sel, &GPUScale](const std::pair<int, ADLDisplayID*>& kv) {
		GPUScale.SetDisplayScalingMode(*(kv.second), (GPUSCALINGMODES)sel);
		});
}


bool CAMDSplashScreenDlg::SetColorDepth(int index)
{
	bool ret = false;
	// 6 dpc is 1, 8 dpc is 2
	index = index + 1;
	if (index > 2)
	{
		return false;
	}

	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, index, &ret](const std::pair<int, ADLDisplayID*>& kv) {
		ADLDisplayID *displayID = kv.second;
		int currentColorDepth = 0;
		if (ADL_OK == ADL2_Display_ColorDepth_Get(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, &currentColorDepth))
		{
			if (ADL_OK == ADL2_Display_ColorDepth_Set(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, index))
			{
				ret = true;
			}
		}
	});

	return ret;
}

int CAMDSplashScreenDlg::GetColorDepth()
{
	// 6 dpc is 1, 8 dpc is 2
	int index = 0;

	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &index](const std::pair<int, ADLDisplayID*>& kv) {
		ADLDisplayID *displayID = kv.second;
		int currentColorDepth = 0;
		if (ADL_OK == ADL2_Display_ColorDepth_Get(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, &currentColorDepth))
		{
			index = currentColorDepth;
		}
	});
	return index-1;
}

bool CAMDSplashScreenDlg::SetPixelFormat(int index)
{
	bool ret = false;
	// RGB 4:4:4  is 1
    if (index > 3)
    {
        return ret;
    }
    index = 1 << index;

	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, index, &ret](const std::pair<int, ADLDisplayID*>& kv) {
		ADLDisplayID *displayID = kv.second;
		int currentPixelFormat = 0;
		if (ADL_OK == ADL2_Display_PixelFormat_Get(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, &currentPixelFormat))
		{
			if (ADL_OK == ADL2_Display_PixelFormat_Set(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, index))
			{
				ret = true;
			}
		}
	});

	return ret;
}

int CAMDSplashScreenDlg::GetPixelFormat()
{
	int index = 0;

	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &index](const std::pair<int, ADLDisplayID*>& kv) {
		ADLDisplayID *displayID = kv.second;
		int currentPixelFormat = 0;
		if (ADL_OK == ADL2_Display_PixelFormat_Get(ADLApp_context.adlContext, displayID->iDisplayLogicalAdapterIndex, displayID->iDisplayLogicalIndex, &currentPixelFormat))
		{
			index = currentPixelFormat;
		}
	});

	int count = 0;
	for (; index; index >>= 1) {
		count++;
	}
	return count-1;
}

void CAMDSplashScreenDlg::OnCbnSelchangeComboScolorDepth()
{
	int sel = m_cbColorDepth.GetCurSel();
	SetColorDepth(sel);
	RedrawWindow();
}

void CAMDSplashScreenDlg::OnCbnSelchangeComboPixelFormat()
{
	int sel = m_cbPixelFormat.GetCurSel();
	SetPixelFormat(sel);
	RedrawWindow();
}

void CAMDSplashScreenDlg::OnCbnSelchangeComboVerticalRefresh()
{
	int sel = m_cbVerticalRefresh.GetCurSel();
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [sel, this](const std::pair<int, ADLDisplayID*>& kv) {
		if (sel >= VERTICAL_REFRESH_ENHANCED_SYNC)
		{
			SetRegistrySetting(*kv.second, UMD_REG, R3D_TURBOSYNC_NAME, "1");
		}
		else
		{
			SetRegistrySetting(*kv.second, UMD_REG, R3D_VSYNC_NAME, std::to_string(sel));
			SetRegistrySetting(*kv.second, UMD_REG, R3D_TURBOSYNC_NAME, "0");
		}
		});
}

void CAMDSplashScreenDlg::OnBnClickedCheckColorTempeature()
{
	// TODO: Add your control notification handler code here
	int state = ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->GetCheck();
	bool OnOff = (state == 1 ? true : false);
	DisplayColorBasicStruct pColorBasic;
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &OnOff, &pColorBasic](const std::pair<int, ADLDisplayID*>& kv) {
		if (!OnOff)
		{
			SetColorTemperatureMode(*(kv.second), OnOff);
		}
		else
		{
			SetColorTemperatureMode(*(kv.second), OnOff);
			pColorBasic = GetColorTemperatureMode(*(kv.second));
		}

		});

	RedrawWindow();
}

bool CAMDSplashScreenDlg::SetColorTemperatureMode(const ADLDisplayID displayID_, bool OnOff)
{
	if (ADL_OK == ADL2_Display_ColorTemperatureSource_Set(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, (OnOff == false) ? ADL_DISPLAY_COLOR_TEMPERATURE_SOURCE_EDID : ADL_DISPLAY_COLOR_TEMPERATURE_SOURCE_USER))
		return true;
	else
		return false;
}

DisplayColorBasicStruct CAMDSplashScreenDlg::GetColorVal(const ADLDisplayID displayID_, int option)
{
	DisplayColorBasicStruct *pColorBasic = new DisplayColorBasicStruct();
	//If the color temperature source is manual, get the supported value range.
	if (ADL_OK != ADL2_Display_Color_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, displayID_.iDisplayLogicalIndex, option,
		&(pColorBasic->Current), &(pColorBasic->Default), &(pColorBasic->Min), &(pColorBasic->Max), &(pColorBasic->Step)))
	{
		std::cout << L"Failed to get color temperature values";
	}
	return *pColorBasic;
}

void CAMDSplashScreenDlg::SetColor(int CurrentVal, bool OnOff, int option)
{
	DisplayColorBasicStruct pColorBasic;
	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, &OnOff, &pColorBasic, &CurrentVal, &option](const std::pair<int, ADLDisplayID*>& kv) {
		if (OnOff)
		{
			pColorBasic = GetColorVal(*(kv.second), option);
			if (CurrentVal <= pColorBasic.Max && CurrentVal >= pColorBasic.Min)
			{
				ADLDisplayID *displayID_ = kv.second;
				if (ADL_OK == ADL2_Display_Color_Set(ADLApp_context.adlContext, displayID_->iDisplayLogicalAdapterIndex, displayID_->iDisplayLogicalIndex, option, CurrentVal))
				{
					std::cout << "Success set color options\n";
					/*Image *pimage;
					ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
					std::wstringstream tempVal;
					tempVal << pColorBasic.Current;
					std::wstring str;
					tempVal >> str;
					GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->SetWindowText(str.c_str());*/
				}
				else
					std::cout << "Failed set color options";
			}
		}

		});
}

void CAMDSplashScreenDlg::OnNMCustomdrawSliderColortemperature(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	int CurrentVal = m_cbColorTemperature.GetPos();
	int state = ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->GetCheck();
	bool OnOff = (state == 1 ? true : false);

	if (OnOff && (TemperatureSliderpos != CurrentVal))
	{
		//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_HIDE);
		Image *pimage;
		ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
		std::wstringstream tempVal;
		tempVal << CurrentVal;
		std::wstring str;
		tempVal >> str;
		GetDlgItem(IDC_EDIT2)->SetWindowText(str.c_str());
		SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_TEMPERATURE);
		TemperatureSliderpos = CurrentVal;
		//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_SHOW);
		//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->GetParent()->RedrawWindow();
		//RefreshControl(IDC_STATIC_RESOLUTION_STATE2);
	}
}


void CAMDSplashScreenDlg::OnNMCustomdrawSliderColorbrightness(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	int CurrentVal = m_cbColorBrightness.GetPos();
	bool OnOff = true;
	SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_BRIGHTNESS);

	Image *pimage;
	ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
	std::wstringstream tempVal;
	tempVal << CurrentVal;
	std::wstring str;
	tempVal >> str;
	GetDlgItem(IDC_EDIT3)->SetWindowText(str.c_str());
}

void CAMDSplashScreenDlg::OnNMCustomdrawSliderColorhue(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int CurrentVal = m_cbColorHue.GetPos();
	bool OnOff = true;
	SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_HUE);

	Image *pimage;
	ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
	std::wstringstream tempVal;
	tempVal << CurrentVal;
	std::wstring str;
	tempVal >> str;
	GetDlgItem(IDC_EDIT4)->SetWindowText(str.c_str());
}

void CAMDSplashScreenDlg::OnNMCustomdrawSliderColorcontrast(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int CurrentVal = m_cbColorContrast.GetPos();
	bool OnOff = true;
	SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_CONTRAST);

	Image *pimage;
	ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
	std::wstringstream tempVal;
	tempVal << CurrentVal;
	std::wstring str;
	tempVal >> str;
	GetDlgItem(IDC_EDIT6)->SetWindowText(str.c_str());
}


void CAMDSplashScreenDlg::OnNMCustomdrawSliderColorsaturation(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int CurrentVal = m_cbColorSaturation.GetPos();
	bool OnOff = true;
	SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_SATURATION);

	Image *pimage;
	ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
	std::wstringstream tempVal;
	tempVal << CurrentVal;
	std::wstring str;
	tempVal >> str;
	GetDlgItem(IDC_EDIT5)->SetWindowText(str.c_str());
}

void CAMDSplashScreenDlg::OnNMReleasedcaptureSliderColortemperature(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	//int CurrentVal = m_cbColorTemperature.GetPos();
	//int state = ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->GetCheck();
	//bool OnOff = (state == 1 ? true : false);

	//if (OnOff)
	//{
	//    SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_TEMPERATURE);
	//    //GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_HIDE);
	//    Image *pimage;
	//    ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
	//    std::wstringstream tempVal;
	//    tempVal << CurrentVal;
	//    std::wstring str;
	//    tempVal >> str;

	//    GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->SetWindowText(str.c_str());
	//    //GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_SHOW);
	//    //GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->GetParent()->RedrawWindow();
	//    //RefreshControl(IDC_STATIC_RESOLUTION_STATE2);
	//}
}


void CAMDSplashScreenDlg::OnStnClickedStaticResolution()
{
	// TODO: Add your control notification handler code here
}

int CAMDSplashScreenDlg::GetChillState(const ADLDisplayID displayID_)
{
	int val = 0;
	if (ADL_OK == ADL2_Chill_Settings_Get(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, &val))
	{
		return val;
	}
	return ADL_ERR;
}

bool CAMDSplashScreenDlg::SetChillState(const ADLDisplayID displayID_, bool OnOff)
{
	if (ADL_OK == ADL2_Chill_Settings_Set(ADLApp_context.adlContext, displayID_.iDisplayLogicalAdapterIndex, OnOff))
	{
		return true;
	}
	return false;
}

void CAMDSplashScreenDlg::OnBnClickedCheckResolutionChill()
{
	// TODO: Add your control notification handler code here
	int state = ((CButton *)GetDlgItem(IDC_CHECK_RESOLUTION_CHILL))->GetCheck();
	bool OnOff = (state == 1 ? true : false);

	std::for_each(std::begin(DisplayID), std::end(DisplayID), [this, OnOff](const std::pair<int, ADLDisplayID*>& kv) {
		SetChillState(*(kv.second), OnOff);
		});

	RedrawWindow();
}


void CAMDSplashScreenDlg::OnStnClickedStaticColorTemperature()
{
	// TODO: Add your control notification handler code here
}


void CAMDSplashScreenDlg::OnStnClickedStaticMin()
{
	// TODO: Add your control notification handler code here
}


void CAMDSplashScreenDlg::OnStnDblclickStaticResolutionState2()
{
	// TODO: Add your control notification handler code here
}


BOOL CAMDSplashScreenDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		if (GetFocus()->GetDlgCtrlID() == IDC_EDIT2)
		{
			UpdateData(TRUE);
			CString tempInput;
			((CEdit*)GetDlgItem(IDC_EDIT2))->GetWindowText(tempInput);

			int CurrentVal = _ttoi(tempInput);
			if (CurrentVal >= pColorBasicTemperature.Min && CurrentVal <= pColorBasicTemperature.Max)
			{

				int state = ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->GetCheck();
				bool OnOff = (state == 1 ? true : false);

				if (OnOff && (TemperatureSliderpos != CurrentVal))
				{
					m_cbColorTemperature.SetPos(CurrentVal);
					//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_HIDE);
					Image *pimage;
					ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
					std::wstringstream tempVal;
					tempVal << CurrentVal;
					std::wstring str;
					tempVal >> str;
					GetDlgItem(IDC_EDIT2)->SetWindowText(str.c_str());
					SetColor(CurrentVal, OnOff, ADL_DISPLAY_COLOR_TEMPERATURE);
					TemperatureSliderpos = CurrentVal;
				}
				//RedrawWindow();
			}
		}
		else if (GetFocus()->GetDlgCtrlID() == IDC_EDIT3)
		{
			UpdateData(TRUE);
			CString tempInput;
			((CEdit*)GetDlgItem(IDC_EDIT3))->GetWindowText(tempInput);

			int CurrentVal = _ttoi(tempInput);
			if (CurrentVal >= pColorBasicBright.Min && CurrentVal <= pColorBasicBright.Max)
			{
				m_cbColorBrightness.SetPos(CurrentVal);
				Image *pimage;
				ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
				std::wstringstream tempVal;
				tempVal << CurrentVal;
				std::wstring str;
				tempVal >> str;
				GetDlgItem(IDC_EDIT3)->SetWindowText(str.c_str());
				SetColor(CurrentVal, true, ADL_DISPLAY_COLOR_BRIGHTNESS);
			}
		}
		else if (GetFocus()->GetDlgCtrlID() == IDC_EDIT4)
		{
			UpdateData(TRUE);
			CString tempInput;
			((CEdit*)GetDlgItem(IDC_EDIT4))->GetWindowText(tempInput);

			int CurrentVal = _ttoi(tempInput);
			if (CurrentVal >= pColorBasicHue.Min && CurrentVal <= pColorBasicHue.Max)
			{
				m_cbColorHue.SetPos(CurrentVal);

				Image *pimage;
				ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
				std::wstringstream tempVal;
				tempVal << CurrentVal;
				std::wstring str;
				tempVal >> str;
				GetDlgItem(IDC_EDIT4)->SetWindowText(str.c_str());
				SetColor(CurrentVal, true, ADL_DISPLAY_COLOR_HUE);
			}
		}
		else if (GetFocus()->GetDlgCtrlID() == IDC_EDIT5)
		{
			UpdateData(TRUE);
			CString tempInput;
			((CEdit*)GetDlgItem(IDC_EDIT5))->GetWindowText(tempInput);

			int CurrentVal = _ttoi(tempInput);
			if (CurrentVal >= pColorBasicSaturation.Min && CurrentVal <= pColorBasicSaturation.Max)
			{
				m_cbColorSaturation.SetPos(CurrentVal);

				Image *pimage;
				ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
				std::wstringstream tempVal;
				tempVal << CurrentVal;
				std::wstring str;
				tempVal >> str;
				GetDlgItem(IDC_EDIT5)->SetWindowText(str.c_str());
				SetColor(CurrentVal, true, ADL_DISPLAY_COLOR_SATURATION);
			}
		}
		else if (GetFocus()->GetDlgCtrlID() == IDC_EDIT6)
		{
			UpdateData(TRUE);
			CString tempInput;
			((CEdit*)GetDlgItem(IDC_EDIT6))->GetWindowText(tempInput);

			int CurrentVal = _ttoi(tempInput);
			if (CurrentVal >= pColorBasicContrast.Min && CurrentVal <= pColorBasicContrast.Max)
			{
				m_cbColorContrast.SetPos(CurrentVal);

				Image *pimage;
				ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
				std::wstringstream tempVal;
				tempVal << CurrentVal;
				std::wstring str;
				tempVal >> str;
				GetDlgItem(IDC_EDIT6)->SetWindowText(str.c_str());
				SetColor(CurrentVal, true, ADL_DISPLAY_COLOR_CONTRAST);
			}
		}
		return TRUE;
	}
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CAMDSplashScreenDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	//temperature
	{
		int state = ((CButton *)GetDlgItem(IDC_CHECK_COLOR_TEMPEATURE))->GetCheck();
		bool OnOff = (state == 1 ? true : false);

		if (OnOff)
		{
			m_cbColorTemperature.SetPos(pColorBasicTemperature.Current);
			//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_HIDE);
			Image *pimage;
			ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
			std::wstringstream tempVal;
			tempVal << pColorBasicTemperature.Current;
			std::wstring str;
			tempVal >> str;
			GetDlgItem(IDC_EDIT2)->SetWindowText(str.c_str());
			SetColor(pColorBasicTemperature.Current, OnOff, ADL_DISPLAY_COLOR_TEMPERATURE);
			TemperatureSliderpos = pColorBasicTemperature.Current;
			//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_STATIC_RESOLUTION_STATE2)->GetParent()->RedrawWindow();
			//RefreshControl(IDC_STATIC_RESOLUTION_STATE2);

		}
	}
	//brightness
	{
		m_cbColorBrightness.SetPos(pColorBasicBright.Current);
		Image *pimage;
		ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
		std::wstringstream tempVal;
		tempVal << pColorBasicBright.Current;
		std::wstring str;
		tempVal >> str;
		GetDlgItem(IDC_EDIT3)->SetWindowText(str.c_str());
		SetColor(pColorBasicBright.Current, true, ADL_DISPLAY_COLOR_BRIGHTNESS);
	}
	//hue
	{
		m_cbColorHue.SetPos(pColorBasicHue.Current);

		Image *pimage;
		ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
		std::wstringstream tempVal;
		tempVal << pColorBasicHue.Current;
		std::wstring str;
		tempVal >> str;
		GetDlgItem(IDC_EDIT4)->SetWindowText(str.c_str());
		SetColor(pColorBasicHue.Current, true, ADL_DISPLAY_COLOR_HUE);
	}
	//saturation
	{
		m_cbColorSaturation.SetPos(pColorBasicSaturation.Current);
		Image *pimage;
		ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
		std::wstringstream tempVal;
		tempVal << pColorBasicSaturation.Current;
		std::wstring str;
		tempVal >> str;
		GetDlgItem(IDC_EDIT5)->SetWindowText(str.c_str());
		SetColor(pColorBasicSaturation.Current, true, ADL_DISPLAY_COLOR_SATURATION);
	}
	//contrast
	{
		m_cbColorContrast.SetPos(pColorBasicContrast.Current);

		Image *pimage;
		ImageFromIDResource(IDB_PNG8, _T("PNG"), pimage);
		std::wstringstream tempVal;
		tempVal << pColorBasicContrast.Current;
		std::wstring str;
		tempVal >> str;
		GetDlgItem(IDC_EDIT6)->SetWindowText(str.c_str());
		SetColor(pColorBasicContrast.Current, true, ADL_DISPLAY_COLOR_CONTRAST);
	}
}


BOOL CAMDSplashScreenDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CDialogEx::OnEraseBkgnd(pDC);
}
