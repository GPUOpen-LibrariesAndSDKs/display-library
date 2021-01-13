#include "stdafx.h"
#include <wincrypt.h>

#if defined WIN32 || defined _WIN32
	#include "Registry.h"
#define REGOP_SUCCEEDED( x )			( (x) == ERROR_SUCCESS ? true : false )

	Registry::Registry( __in HKEY hKey /*= HKEY_LOCAL_MACHINE*/, __in const _tstring& strSubKey /*= TEXT( "" )*/ , __in_opt bool bReadOnly /*= true*/ )
	{
		//Opens the passed strkey in the desired mode i.e. readonly or rw
		LONG	lRetval = ERROR_SUCCESS;

		//no catching exceptions. Passed on to the caller
		lRetval = ::RegOpenKeyEx( hKey,							//handle to an open registry key.
											  strSubKey.c_str(),			//registry subkey to open
											  0,							//reserved, must be zero
											  bReadOnly ? KEY_READ : KEY_READ | KEY_WRITE,
											  &m_hKey );
	}

	Registry::~Registry()
	{
		LONG	lRetval = ERROR_SUCCESS; 
		//destructor, so no throwing exception out.
		lRetval = ::RegCloseKey( m_hKey );
	}

	HRESULT Registry::Read( __in const _tstring& strValueName, __out DWORD& dwValue ) const
	{
		HRESULT hr		= E_FAIL;
		LONG	lRetval = ERROR_SUCCESS;
			DWORD dwDataType	= REG_NONE;
			DWORD dwSize		= sizeof( DWORD );

			lRetval = ::RegQueryValueEx( m_hKey, strValueName.c_str(), NULL, &dwDataType, (LPBYTE)&dwValue, &dwSize );

			if( REG_DWORD != dwDataType ) {
				//printf( "Registry Read failed\n" ); 
                hr = S_FALSE;
                return hr;
			}

			hr = S_OK;

		return hr;
	}

	HRESULT Registry::Read( __in const _tstring& strValueName, __out BYTE*& pByte ) const
	{
		HRESULT hr		= E_FAIL;
		LONG	lRetval = ERROR_SUCCESS;

			DWORD dwDataType	= 0;
			DWORD dwSize		= 0;	

			lRetval = ReadData( strValueName, dwDataType, pByte, dwSize );
		
			if( REG_BINARY != dwDataType ) {
                //printf("Registry read failed\n");
                hr = S_FALSE;
                return hr;
			}
			hr = S_OK;
		return hr;
	}

	HRESULT Registry::Read( __in const _tstring& strValueName, __inout int& nValue ) const
	{
		HRESULT hr = E_FAIL;
			_tstring strValue;
			 hr = Read( strValueName, strValue );
			if( SUCCEEDED( hr ) ) {
				//convert to integer
				_tstringstream ss( strValue) ;
				ss >> nValue;
				if( ss.fail() ) {
					//printf( "Failed to convert string to integer\n" );
                    return S_FALSE;
				}
				//printf( "REG_BINARY %s RegValue found: Data from Registry: %s. Converted to integer: %d", strValueName.c_str(), strValue.c_str( ), nValue );
			}
			else {
				//don't do anything let the default value be as is
				//printf( "REG_BINARY %s RegValue NOT found: Default Integer value: %d\n", strValueName.c_str( ), nValue );
				hr = S_FALSE;
			}

		return hr;
	}

	HRESULT Registry::Read( __in const _tstring& strValueName, __inout bool& bValue ) const
	{
		HRESULT hr = E_FAIL;
			_tstring strValue;
			hr = Read( strValueName, strValue );
			if( SUCCEEDED( hr ) ) {
				//convert to bool
				_tstringstream ss( strValue );
				ss >> bValue;
				if( ss.fail( ) ) {
					//printf( "Failed to convert string to bool\n" );
                    return S_FALSE;
				}
				//printf( "REG_BINARY %s RegValue found: Data from Registry: %s. Converted to bool: %d\n", strValueName.c_str( ), strValue.c_str( ), bValue );
			}
			else {
				//don't do anything let the default value be as is
				//printf( "REG_BINARY %s RegValue NOT found: Default bool value: %d\n", strValueName.c_str( ), bValue );
				hr = S_FALSE;
			}

		return hr;
	}

	HRESULT Registry::Read( __in const _tstring& strValueName, __out _tstring& strValue ) const
	{
		HRESULT hr		= E_FAIL;
			PBYTE pbyData		= NULL;
			DWORD dwDataType	= 0;
			DWORD dwSize		= 0;

			hr = ReadData( strValueName, dwDataType, pbyData, dwSize );

			switch( dwDataType ) {
				case REG_SZ:
					strValue = reinterpret_cast< TCHAR* >( pbyData );
					break;
				case REG_BINARY:
					//Convert the binary data to string
					hr = ConvertToString( pbyData, dwSize, strValue );
					break;
				default:
					if( pbyData ) {
						delete[] pbyData;
						pbyData = NULL;
					}
                    printf("Read registry failed\n");
                    return S_FALSE;
			}
		
			if( pbyData ) {
				delete[] pbyData;
				pbyData = NULL;
			}
			hr = S_OK;

		return hr;
	}

	HRESULT Registry::ReadData( __in const _tstring& strValueName, __out DWORD& dwDataType, __out BYTE*& pbyData, __out DWORD& dwSize ) const
	{
		HRESULT hr		= E_FAIL;
		LONG	lRetval = ERROR_SUCCESS;
			//Funtion will return ERROR_SUCCESS and dwSize will contain the size of the buffer needed to read the
			//value of this registy value under the given key

			//Don't throw an exception, the registry value may not be present
			//Let the caller decide what to do.
			lRetval = ::RegQueryValueEx( m_hKey, strValueName.c_str(), NULL, &dwDataType, NULL, &dwSize );
			if( REGOP_SUCCEEDED( lRetval ) ) {
				pbyData = new BYTE[ dwSize ];
				memset( pbyData, '\0', dwSize );

				lRetval = ::RegQueryValueEx( m_hKey, strValueName.c_str(), NULL, &dwDataType, pbyData, &dwSize );
				hr = REGOP_SUCCEEDED( lRetval ) ? S_OK : E_FAIL;
			}

		return hr;
	}

	HRESULT Registry::ConvertToString( __in const LPBYTE pbBinary, __in DWORD dwNumOfElements, __out _tstring& strValue ) const
	{
		HRESULT	hr		= E_FAIL;
		BOOL	bRetVal = FALSE;

			DWORD dwSize = 0;

			bRetVal = ::CryptBinaryToString( pbBinary, dwNumOfElements, CRYPT_STRING_BINARY | CRYPT_STRING_NOCRLF, NULL, &dwSize );
			TCHAR* pBuffer = new TCHAR[ dwSize ];
			ZeroMemory( pBuffer, sizeof( TCHAR ) * dwSize );
			bRetVal = ::CryptBinaryToString( pbBinary, dwNumOfElements, CRYPT_STRING_BINARY | CRYPT_STRING_NOCRLF, pBuffer, &dwSize );

			strValue = reinterpret_cast< TCHAR* >( pBuffer );
		
			delete[] pBuffer;
			pBuffer = NULL;

			hr = S_OK;

		return hr;
	}

	HRESULT Registry::Write( __in const _tstring& strValueName, __in const _tstring& strValue, __in_opt bool bBinary /*= false */ ) const
	{
		HRESULT hr		= E_FAIL;
		LONG	lRetval	= ERROR_SUCCESS;
			BYTE *pByData = reinterpret_cast<BYTE* >( const_cast< TCHAR* >( strValue.c_str() ) );;
			DWORD dwSize  = strValue.size();
			DWORD dwType  = REG_SZ;

			if( bBinary ) {
				//convert the string to hex and then write 
				dwType = REG_BINARY;
				dwSize *= 2;
			}

			lRetval = ::RegSetValueEx( m_hKey, strValueName.c_str(), NULL, dwType, pByData, dwSize );
			hr = S_OK;

		return hr;
	}

	HRESULT Registry::Write( __in const _tstring& strValueName, __in int nValue, __in_opt bool bBinary /*= false */ ) const
	{
		HRESULT hr	= E_FAIL;
			//NOTE:: the std::to_string is only available in C++11. Although std::to_wstring is availble in VS 2010, but not for int.
			_tstring strValue;			
#ifdef _MSC_VER
	#if _MSC_VER <= 1600
			TCHAR szBuff[2 * _MAX_INT_DIG];
			_tmemset( szBuff, TEXT( '\0' ), 2 * _MAX_INT_DIG );
			swprintf_s(szBuff, sizeof (szBuff) / sizeof (TCHAR), TEXT( "%d" ), nValue);
			strValue = szBuff;
	#else
			strValue = _tto_string( nValue );
	#endif
#else
	error "Not supported compiler version. Only supports Visual Studio 2010 and above"
#endif
			hr = Write( strValueName, strValue, bBinary );

		return hr;
	}

    HRESULT Registry::WriteDWORD(__in const _tstring& strValueName, __in const DWORD& dwValue) const
    {
        HRESULT hr = E_FAIL;
        LONG	lRetval = ERROR_SUCCESS;

        DWORD dwType = REG_DWORD;

        lRetval = ::RegSetValueEx(m_hKey, strValueName.c_str(), NULL, dwType, (LPBYTE)&dwValue, sizeof(DWORD));
        hr = S_OK;

        return hr;
    }
#endif //defined WIN32 || defined _WIN32
