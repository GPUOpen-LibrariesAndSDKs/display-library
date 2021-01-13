
// AMDSplashScreen.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAMDSplashScreenApp:
// See AMDSplashScreen.cpp for the implementation of this class
//

class CAMDSplashScreenApp : public CWinApp
{
	HANDLE m_hMutex;
	DWORD  m_dwLastError;

	bool IsWindows64bit();
	DWORD GetProcessIDByName(const wchar_t* pName);

public:
	CAMDSplashScreenApp();
	~CAMDSplashScreenApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL ExitInstance();
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAMDSplashScreenApp theApp;