#pragma once
#ifndef _GET_USER_PROCESS_DATA_H__
#define _GET_USER_PROCESS_DATA_H__
#ifdef _DEBUG
#define ADL_PROCESS_PATH L"C:\\ProgramData\\ATI\\"
#else	
#define ADL_PROCESS_PATH L"C:\\ProgramData\\ATI\\"
#endif
#define ADL_PROCESS_NAME L"UserModeProcess.exe"
#include <stdio.h>
#include <Vector>
#include <stdlib.h>
#include "RunProcess.h"
#include "..\..\Include\GlobalDefines.h"

using namespace std;
DWORD WINAPI CreateUserProcess(LPVOID lpParam)
{
	if (lpParam == NULL)
		return FALSE;
	PCWCHAR lpFunctionName = (PCWCHAR)lpParam;
	WCHAR strPath[MAX_PATH] = { 0 };
	WCHAR strCommendLine[MAX_PATH] = { 0 };

	swprintf_s(strPath, L"%s%s", ADL_PROCESS_PATH, ADL_PROCESS_NAME);
	swprintf_s(strCommendLine, L"%s%s", ADL_FUNCTION_NAME, lpFunctionName);
	// Service create a user process
	RunProcess::RunProcessAsUser(strPath, strCommendLine);
}


template <class TData>
class GetUserProcessData
{
public:
	GetUserProcessData(PCWCHAR lpFunctionName, TData & requestServiceData);

	~GetUserProcessData();
	TData* GetProcessData() { return m_pData; }

private:
	HANDLE m_hThread;
	TData * m_pData;

};
// 
//  FUNCTION: GetUserProcessData
//
//  PURPOSE: Load user mode process to run ADL.
//
//  PARAMETERS:
//    lpFunctionName - ADL call function name
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//  
template <class TData>
GetUserProcessData<TData>::GetUserProcessData(PCWCHAR lpFunctionName, TData & requestServiceData) : m_hThread(0)
{

	if (lpFunctionName == NULL)
		return ;

	DWORD dwThreadId = 0;
	HANDLE m_cancelevent;
	WCHAR strEventPath[MAX_PATH] = { 0 };
	WCHAR strShareMemoryPath[MAX_PATH] = { 0 };
	DWORD nSize = sizeof(TData);

	swprintf_s(strEventPath, ADL_EVENT_NAME, lpFunctionName);
	swprintf_s(strShareMemoryPath, ADL_SHAREMENORY_NAME, lpFunctionName);

	// Create Event, Let Service and user process access share memory separately
	SECURITY_DESCRIPTOR sd;
	SECURITY_ATTRIBUTES sa;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
	m_cancelevent = CreateEventW(&sa, FALSE, FALSE, strEventPath);//create in service

	// Create a Share memory size = SHARE_MEMORY_MAX_SIZE to communicat two process  
	CShareMFile cShareMFile = CShareMFile();
	if (cShareMFile.CreateShareFile(SHARE_MEMORY_MAX_SIZE, strShareMemoryPath))
	{
		if (cShareMFile.WriteBuffer(strShareMemoryPath, (PVOID)&requestServiceData, nSize))
		{
			// Create user process to call ADL function
			m_hThread = CreateThread(NULL, 0, CreateUserProcess, (LPVOID)lpFunctionName, 0, &dwThreadId);
			// Wait user process Data writed to shared memory , if not wait please change INFINITE value
			ULONG ret = WaitForSingleObject(m_cancelevent, INFINITE);
			if (ret == WAIT_TIMEOUT)
			{
				cShareMFile.DeleteBuffer();
				return;
			}
			// Get template structure from shared memory
			if(cShareMFile.ReadBuffer(strShareMemoryPath, (PVOID)&requestServiceData, nSize))
			{
				// This ADL function had point buffer to read
				if (requestServiceData.szData > 0)
				{
					TData * responsePointADLdata =  (TData *)(new char[(nSize + requestServiceData.szData) / sizeof(char)]);
					ZeroMemory(responsePointADLdata, nSize + requestServiceData.szData );
					// Get ADL point buffer and template structure from shared memory
					cShareMFile.ReadBuffer(strShareMemoryPath, (PVOID)responsePointADLdata, requestServiceData.szData + nSize);
					m_pData = responsePointADLdata;
				}
			}
			cShareMFile.DeleteBuffer();
		}
	}
}
template <class TData>
GetUserProcessData<TData>::~GetUserProcessData()
{
	if (m_hThread)
		CloseHandle(m_hThread);
}
#endif _GET_USER_PROCESS_DATA_H__