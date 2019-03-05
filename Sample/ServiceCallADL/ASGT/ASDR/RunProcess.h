#pragma once
extern HANDLE g_hTokenToCheck;

class RunProcess
{
public:
	RunProcess() {};
	~RunProcess() {};
	static BOOL GetTokenByName(HANDLE &hToken);
	static BOOL RunProcessAsUser(LPCWSTR lpImage, LPWSTR lpCommandLine);


private:
	static DWORD GetProcessIdOfWinLogon(DWORD dwActiveSessionId);

};

