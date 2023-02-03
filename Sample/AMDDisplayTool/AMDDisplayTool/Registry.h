/*******************************************************************************
 © 2015 Advanced Micro Devices, Inc.

 The software, documentation, and related materials (“Licensed Materials”) are 
 subject to the terms and conditions of a software development kit license 
 agreement executed between AMD and licensee.  THE LICENSED MATERIALS ARE 
 PROVIDED “AS IS,” WITH ALL FAULTS, AND WITHOUT WARRANTY OF ANY KIND.  AMD 
 DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 PARTICULAR PURPOSE, TITLE, AND NON-INFRINGEMENT.  AMD AND ITS LICENSORS DO NOT 
 WARRANT THAT THE LICENSED MATERIALS ARE ERROR FREE OR THAT THE LICENSED 
 MATERIALS WILL RUN CONTINUOUSLY.
 *******************************************************************************/

/*
 * #file Registry.h
 */
#pragma once

#include<string>
#include<sstream>

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

#define __WFILE__ WIDEN(__FILE__)

#if defined _UNICODE || defined UNICODE
#define _tmemset		wmemset
#define _tstring		std::wstring
#define	_tifstream		std::wifstream
#define _tofstream		std::wofstream
#define _tostringstream std::wostringstream
#define _tstringstream	std::wstringstream
#define _tcout			std::wcout
#define _tostream		std::wostream
#define _tto_string		std::to_wstring
#define _tcslen_s		wcsnlen_s
#define __TFILE__		__WFILE__
#define	_tsplitpath_s	_wsplitpath_s
#else
#define _tmemset memset
#define _tstring		std::sstring
#define	_tifstream		std::ifstream
#define _tofstream		std::ofstream
#define _tostringstream std::ostringstream
#define _tstringstream	std::stringstream
#define _tcout			std::cout
#define _tostream		std::ostream
#define _tto_string		std::to_string
#define _tcslen_s		strnlen_s
#define __TFILE__		__FILE__
#define	_tsplitpath_s	_splitpath_s
#endif

#if defined WIN32 || defined _WIN32
#define	FOLDER_SEPARATOR TEXT( "\\" )
#else
#define FODLER_SEPARATOR "//"
#endif
//Include Windows.h on Windows platforms only
#if defined WIN32 || defined _WIN32 
	//#include "Exception.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>

	/*
	 * @namespace AMD
	 * @brief All functions and classes are in the AMD namespace
	 */
	//BEGIN_AMD_NAMESPACE

	/*
	 * @class Registry
	 *
	 * Wraps the Windows Registry APIs in a C++ class to make it simpler to use. 
	 */
	class Registry {
		public:
			/*
			 * @fn Registry
			 *
			 * @brief Default Constructor
			 * 
			 * This constructor opens the asked key in either Read or Read/Write mode. Throws @see RegistryException 
			 * in case it fails to open the registy key. The user of this class has to catch the exception
			 * 
			 * @param hKey HKEY to be opened. Could be a key opened earlier or one of the predefined keys:
			 *             HKEY_CLASSES_ROOT, HKEY_CURENT_CONFIG, HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE, HKEY_PERFORMANCE_DATA,
			 *			   HKEY_PERFORMANCE_NLSTEXT, HKEY_PERFORMMCE_TEXT or HKEY_USERS
			 * @param strSubKey Subkey under the hKey to be opened.
			 * @param bReadOnly boolean flag indicating whether to open the key in Readonly mode or not.
			 */
			Registry( __in_opt HKEY hKey = HKEY_LOCAL_MACHINE, __in_opt const _tstring& strSubKey = TEXT( "" ), __in_opt bool bReadOnly = false );

			/*
			 * @fn ~Registry
			 *
			 * @brief Destructor. Closes the key opened in the constructor.
			 */
			~Registry();

			/*
			 * @fn Read
			 *
			 * @brief Function To read a REG_SZ value from the registry under the key opened
			 *
			 * This function reads the data of a REG_SZ registry value under the key opened in this class
			 * @param strValueName Name of the REG_SZ value
			 * @param strValue data of the REG_SZ value specified in @strValueName
			 * @returns HRESULT. S_OK for success
			 */
			HRESULT	Read( __in const _tstring& strValueName, __out _tstring& strValue ) const;
			/*
		 	 * @fn Read
			 *
			 * @brief Overloaded Read Function To read a REG_SZ value from the registry under the key opened
			 *
			 * This function reads the data of a REG_SZ registry value under the key opened in this class
			 * and converts it to an int.
			 *
			 * @param strValueName Name of the REG_SZ value
			 * @param nValue integer data of the REG_SZ value specified in @strValueName. Note the caller can set this value to a default one
			 *				 as the function will not change this parameter if an error occurs during the reading.
			 * @returns HRESULT. S_OK for success
			 */
			HRESULT Read( __in const _tstring& strValueName, __inout int& nValue ) const;
			/*
			 * @fn Read
			 *
			 * @brief Overloaded Read Function To read a REG_SZ value from the registry under the key opened
			 *
			 * This function reads the data of a REG_SZ registry value under the key opened in this class
			 * and converts it to an int.
			 *
			 * @param strValueName Name of the REG_SZ value
			 * @param bValue boolean data of the REG_SZ value specified in @strValueName. Note the caller can set this value to a default one
			 *				 as the function will not change this parameter if an error occurs during the reading.
			 * @returns HRESULT. S_OK for success
			 */
			HRESULT Read( __in const _tstring& strValueName, __inout bool& bValue ) const;
		
			/*
			* @fn Read
			*
			* @brief Function To read a REG_DWORD value from the registry under the key opened
			*
			* This function reads the data of a REG_SZ registry value under the key opened in this class
			* @param strValueName Name of the REG_DWORD value
			* @param dwValue data of the REG_DWORD value specified in @strValueName
			* @returns HRESULT. S_OK for success
			*/
			HRESULT Read( __in const _tstring& strValueName, __out DWORD& dwValue ) const;

			/*
			* @fn Read
			*
			* @brief Function To read a REG_BIBARY value from the registry under the key opened
			*
			* This function reads the data of a REG_BINARY registry value under the key opened in this class
			* @param strValueName Name of the REG_BINARY value
			* @param ppByte Reference to a pointer, which will contain ghe data of the REG_BINARY value specified in @strValueName
			* @returns HRESULT. S_OK for success
			*/
			HRESULT Read( __in const _tstring& strValueName, __out BYTE*& ppByte ) const;

			/*
			* @fn Write
			*
			* @brief Function to write the date of a registry value
			*
			* This function writes the data of a either a REG_BINARY or a REG_SZ registry value under the key opened in this class
			* @param strValueName Name of the Regsitry value
			* @param strValue data of the Registry value specified in @see strValueName.
			* @param bBinary optional boolean flag telling the function whether to write the data as REG_SZ or REG_BINARY. If this flag is true
			*				     the data is written as REG_SZ otherwise as a REG_BINARY
			* @returns HRESULT. S_OK for success
			*/
			HRESULT Write( __in const _tstring& strValueName, __in const _tstring& strValue, __in_opt bool bBinary = false ) const;

			/*
			* @fn Write
			*
			* @brief Function to write the data of a registry value
			*
			* This function writes the data of a either a REG_BINARY or a REG_SZ registry value under the key opened in this class
			*
			* @param strValueName Name of the Regsitry value
			* @param nValue integer data of the Registry value specified in @see strValueName. This data is converted to string and then written
			*				since the registry expects a string representatio of the numbers in binary format.
			* @param bBinary optional boolean flag telling the function whether to write the data as REG_SZ or REG_BINARY. If this flag is true
			*				     the data is written as REG_SZ otherwise as a REG_BINARY
			* @returns HRESULT. S_OK for success
			*/
			HRESULT Write( __in const _tstring& strValueName, __in int nValue, __in_opt bool bBinary = false ) const;

            /*
            * @fn Write
            *
            * @brief Function to write the date of a registry value
            *
            * This function writes the data of a REG_DWORD registry value under the key opened in this class
            * @param strValueName Name of the Regsitry value
            * @param strValue data of the Registry value specified in @see strValueName.            
            * @returns HRESULT. S_OK for success
            */
            HRESULT WriteDWORD(__in const _tstring& strValueName, __in const DWORD& strValue) const;

		private:
			/*
			 * @fn Registry
			 *
			 * @brief hidden copy constuctor
			 * 
			 * Hidden so that object of this class are not copied
			 */
			Registry( const Registry& );
		
			/*
			* @fn operator = 
			*
			* @brief hidden assignment operator
			*
			* Hidden so that object of this class are assigned to other objects
			*/
			//Registry& operator = ( const Registry& );

			/*
			* @fn ReadData
			*
			* @brief Function to Read data of a given Registry value in a raw binary format.
			*
			* This function reads the data of a given registry value in a raw binary format
			* @param strValueName name of the Regsitry value
			* @param dwDataType type of registry.
			* @param pbyData reference to a pointer, pointing to the raw data. The User has to free this data by calling delete[].
			* @param dwSize The size of the data read in DWORD.
			* @returns HRESULT
			*/
			HRESULT ReadData( __in const _tstring& strValueName, __out DWORD& dwDataType, __out BYTE*& pbyData, __out DWORD& dwSize ) const;

			/*
			 * @fn ConvertToString
			 *
			 * @brief Converts a binary data to a std string.
			 * 
			 * This function converts a binary stream to a std string object.
			 * @param pbyBinary	array of binary data to be converted to string
			 * @param dwNumOfElements size of the binary data to be converted
			 * @param strValue std string containing the converted binary data.
			 * @returns HRESULT
			 */
			HRESULT ConvertToString( __in const LPBYTE pbBinary, __in DWORD dwNumOfElements, __out _tstring& strValue ) const;

			HKEY	m_hKey;	///< handle to the key opened for Read or Write operations.
	};//class Registry

	/*
	 * End AMD namepsace
	 */
	//END_AMD_NAMESPACE
#endif //defined WIN32 || defined _WIN32
