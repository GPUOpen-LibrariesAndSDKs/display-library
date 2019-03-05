#pragma once
#ifndef _SHARE_M_FILE_H__
#define _SHARE_M_FILE_H__
#include <windows.h>
#include <wchar.h>

using namespace std;

class CShareMFile
{
public:

	CShareMFile()
	{
		m_pShareFileBuff = NULL;
		m_hShareFile = NULL;
	}

	~CShareMFile()
	{
	}

	bool DeleteBuffer()
	{

		if (m_pShareFileBuff)
		{
			UnmapViewOfFile(m_pShareFileBuff);
			m_pShareFileBuff = NULL;
		}
		if (m_hShareFile)
		{
			CloseHandle(m_hShareFile);
			m_hShareFile = NULL;
		}
		return true;
	}
	bool CreateShareFile(ULONG filesize, PCWCHAR filename)
	{
		DWORD dwErr = 0;
		SECURITY_DESCRIPTOR sd;
		SECURITY_ATTRIBUTES sa;
		m_mapsize = filesize;
		// After setting this permission, the general application does not have administrator privileges in order to start to openmappingfile Otherwise Access Dengy
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

#pragma warning(suppress: 6248)
		SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = &sd;

		//Create a memory-mapped file
		m_hShareFile = CreateFileMappingW(INVALID_HANDLE_VALUE,
			&sa,  
			PAGE_READWRITE,
			0,
			filesize,
			filename);
		if (m_hShareFile == NULL)
		{
			dwErr = GetLastError();
			return false;
		}

		// In the external function can be directly manipulate the variables to get the memory-mapped file.
		if (m_pShareFileBuff)
		{
			UnmapViewOfFile(m_pShareFileBuff);
			m_pShareFileBuff = NULL;
		}
		m_pShareFileBuff = MapViewOfFile(m_hShareFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			filesize);
		if (m_pShareFileBuff == NULL)
		{
			dwErr = GetLastError();
			return false;
		}


		return true;
	}

	bool WriteBuffer(PCWCHAR ShareFileName, PVOID buffer, ULONG size)
	{

		m_hShareFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, ShareFileName);
		m_pShareFileBuff = MapViewOfFile(m_hShareFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			size);
		if (m_pShareFileBuff)
		{
			memcpy_s(m_pShareFileBuff, size, buffer, size);
			return true;
		}
		return false;
	}
	bool WriteBuffer(PCWCHAR ShareFileName, PVOID buffer, ULONG size, PVOID *  pPointBuffer, ULONG uPointSize)
	{

		m_hShareFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, ShareFileName);
		m_pShareFileBuff = MapViewOfFile(m_hShareFile,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			size + uPointSize);
		if (m_pShareFileBuff)
		{
			// Get point buffer from shared memory 
			PVOID mPointShareFileBuff = (PVOID)((INT64)m_pShareFileBuff + size);
			memcpy_s(m_pShareFileBuff, size, buffer, size);
			memcpy_s(mPointShareFileBuff, uPointSize, *pPointBuffer, uPointSize);
			delete *pPointBuffer;
			pPointBuffer = &mPointShareFileBuff;
			return true;
		}
		return false;
	}
	bool ReadBuffer(PCWCHAR ShareFileName, PVOID buffer, ULONG size)
	{
		SECURITY_DESCRIPTOR sd;
		SECURITY_ATTRIBUTES sa;
		bool result = false;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

#pragma warning(suppress: 6248)
		SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = &sd;

		m_hShareFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, ShareFileName);

		if (m_hShareFile)
		{
			m_pShareFileBuff = MapViewOfFile(m_hShareFile,
				FILE_MAP_ALL_ACCESS,
				0,
				0,
				size);
			if (m_pShareFileBuff)
			{
				memcpy_s(buffer, size, m_pShareFileBuff, size);
				result = true;
			}
		}
		return result;
	}

private:
	//Memory-mapped file handle
	HANDLE  m_hShareFile;

	//The contents of the memory-mapped file buffer
	PVOID   m_pShareFileBuff;
	ULONG					m_mapsize;
};




#endif _SHARE_M_FILE_H__