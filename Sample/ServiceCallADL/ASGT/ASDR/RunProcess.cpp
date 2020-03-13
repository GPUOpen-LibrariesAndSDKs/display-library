#include <Windows.h>
#include <TlHelp32.h>
#include "RunProcess.h"
HANDLE g_hTokenToCheck = NULL;
HMODULE g_WtsApi32Lib = NULL;
typedef BOOL(WINAPI *LPWTSQUERYUSERTOKEN)(DWORD, PHANDLE);
LPWTSQUERYUSERTOKEN g_WTSQueryUserTokenPtr = NULL;


DWORD RunProcess::GetProcessIdOfWinLogon(DWORD dwActiveSessionId)
{
	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) return 0;

	PROCESSENTRY32 ProcEntry = { 0 };
	ProcEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!::Process32First(hSnap, &ProcEntry))
	{
		::CloseHandle(hSnap);
		return 0;
	}

	DWORD dwProcessId = 0;
	for (;;)
	{
		if (strcmp(ProcEntry.szExeFile,"winlogon.exe") == 0)
		{
			DWORD dwWinLogonSessionId = 0;
			if (!::ProcessIdToSessionId(ProcEntry.th32ProcessID, &dwWinLogonSessionId)) continue;

			if (dwWinLogonSessionId == dwActiveSessionId)
			{
				dwProcessId = ProcEntry.th32ProcessID;
				break;
			}
		}

		if (!Process32Next(hSnap, &ProcEntry)) break;
	}

	::CloseHandle(hSnap);
	return dwProcessId;
}

BOOL RunProcess::GetTokenByName(HANDLE &hToken)
{
	
	BOOL bRet = FALSE;
	do
	{
		DWORD dwSessionId = WTSGetActiveConsoleSessionId();
		if (dwSessionId == (DWORD)-1)
			break;

		if (g_WTSQueryUserTokenPtr == NULL)
		{
			if (g_WtsApi32Lib == NULL)
				g_WtsApi32Lib = LoadLibrary("WtsApi32.dll");
			if (g_WtsApi32Lib)
				g_WTSQueryUserTokenPtr = (LPWTSQUERYUSERTOKEN)GetProcAddress(g_WtsApi32Lib, "WTSQueryUserToken");
			if (!g_WTSQueryUserTokenPtr)
			{
				break;
			}
		}

		if (!g_WTSQueryUserTokenPtr(dwSessionId, &hToken))
		{
			//DBGPRINT("WTSQueryUserTokenPtr Failed\n");
			break;
		}


		HANDLE hTokenDup = NULL;
		DWORD dwSize = 0;

		DWORD dwWinLogonProcessId = GetProcessIdOfWinLogon(dwSessionId);

		HANDLE hWinlogon = ::OpenProcess(MAXIMUM_ALLOWED, FALSE, dwWinLogonProcessId);
		if (hWinlogon == NULL) break;

		BOOL bResult = ::OpenProcessToken(hWinlogon, TOKEN_ALL_ACCESS, &hToken);
		::CloseHandle(hWinlogon);
		if (!bResult) break;

		bResult = ::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
			SecurityIdentification, TokenPrimary, &hTokenDup);
		::CloseHandle(hToken);
		hToken = NULL;
		if (!bResult) break;

		bResult = ::SetTokenInformation(hTokenDup, TokenSessionId, (void*)&dwSessionId, sizeof(DWORD));
		if (!bResult) break;

		TOKEN_PRIVILEGES TokenPriv = { 0 };
		::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TokenPriv.Privileges[0].Luid);
		TokenPriv.PrivilegeCount = 0;
		TokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		::AdjustTokenPrivileges(hTokenDup, FALSE, &TokenPriv, sizeof(TokenPriv), NULL, NULL);
		g_hTokenToCheck = hTokenDup;
		hToken = hTokenDup;
		bRet = true;

	} while (false);
	return (bRet);
}

 BOOL  RunProcess::RunProcessAsUser(LPCWSTR lpImage,LPWSTR lpCommandLine)
{
	if (!lpImage)
	{
		return FALSE;
	}
	HANDLE hToken = NULL;
	if (!GetTokenByName(hToken))//  "SVCHOST.EXE"
	{
		return FALSE;
	}
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFOW));
	si.cb = sizeof(STARTUPINFOW);
	si.lpDesktop = L"winsta0\\default";
	if (hToken == NULL)
		return FALSE;
	BOOL bResult = CreateProcessAsUserW(hToken, lpImage,lpCommandLine, NULL, NULL,
		FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	if (bResult)
	{
		OutputDebugString("CreateProcessAsUser ok!\r\n");
	}
	else
	{
		OutputDebugString("CreateProcessAsUser false!\r\n");
	}
	return bResult;
}
