
// AMDSplashScreen.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "AMDSplashScreen.h"
#include "AMDSplashScreenDlg.h"
#include <shlwapi.h>
#include <srrestoreptapi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAMDSplashScreenApp

BEGIN_MESSAGE_MAP(CAMDSplashScreenApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAMDSplashScreenApp construction

CAMDSplashScreenApp::CAMDSplashScreenApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,

	//Make sure that you use a name that is unique for this application otherwise
	//two apps may think they are the same if they are using same name for
	//3rd parm to CreateMutex
	m_hMutex = CreateMutex(NULL, FALSE, _T("AMDDisplayTool Instance")); //do early
	m_dwLastError = GetLastError(); //save for use later...

	// Place all significant initialization in InitInstance

	if (ADL_OK == InitADL()) //Initialization of ADL
	{
		PrepareAPI();
	}

}

CAMDSplashScreenApp::~CAMDSplashScreenApp()
{
	if (m_hMutex)  //Do not forget to close handles.
	{
		CloseHandle(m_hMutex); //Do as late as possible.
		m_hMutex = NULL; //Good habit to be in.
	}
	DestroyADL();
}

// The one and only CAMDSplashScreenApp object

CAMDSplashScreenApp theApp;

typedef BOOL(WINAPI *PFN_SETRESTOREPTW) (PRESTOREPOINTINFOW, PSTATEMGRSTATUS);
typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

// CAMDSplashScreenApp initialization
bool CAMDSplashScreenApp::IsWindows64bit()
{
#if _WIN64
	return true;

#elif _WIN32
	BOOL bIsWow64 = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process =
		(LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
		}
	}
	return bIsWow64;

#else
	assert(0);
	return false;
#endif
}

DWORD CAMDSplashScreenApp::GetProcessIDByName(const wchar_t* pName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return NULL;
	}

	PROCESSENTRY32 pe = { sizeof(pe) };
	for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe)) {
		if (wcscmp(pe.szExeFile, pName) == 0) {
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
		//printf("%-6d %s\n", pe.th32ProcessID, pe.szExeFile);
	}
	CloseHandle(hSnapshot);
	return 0;
}

BOOL CAMDSplashScreenApp::InitInstance()
{
	if (ERROR_ALREADY_EXISTS == m_dwLastError)
	{
		return FALSE;
	}

	//Detect whether process "RadeonInstaller.exe" is exist and in running state.
	/*DWORD pID = GetProcessIDByName(L"RadeonInstaller.exe");
	if (pID != 0)
	{
		HANDLE hProcess;
		DWORD dwPriorityClass;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
		if (hProcess != NULL)
		{
			dwPriorityClass = GetPriorityClass(hProcess);
			if (dwPriorityClass == NORMAL_PRIORITY_CLASS)
			{
				CloseHandle(hProcess);
				return FALSE;
			}

			CloseHandle(hProcess);
		}
	}*/

	if (IsWindows64bit())
	{
#if _WIN64
#else
		MessageBox(NULL, _T("Current application is not support x64."), _T("AMD SOFTWARE"), MB_OK | MB_ICONINFORMATION);
		return FALSE;
#endif
	}
	else
	{
#if _WIN64
		MessageBox(NULL, _T("Current system is not x64."), _T("AMD SOFTWARE"), MB_OK | MB_ICONINFORMATION);
		return FALSE;
#endif
	}
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CAMDSplashScreenDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

BOOL CAMDSplashScreenApp::ExitInstance()
{
	return 0;
}
