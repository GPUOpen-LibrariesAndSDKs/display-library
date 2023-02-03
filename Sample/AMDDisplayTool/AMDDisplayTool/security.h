#ifndef SECURITY_H
#define SECURITY_H

#include <string>

/******************************************************************************
  Copyright(c) 2014-2022 Advanced Micro Devices, Inc. All Rights Reserved.

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

  File:         security.h

  Description:  Contains security helper functions
*******************************************************************************/

namespace Utilities
{
    class Security
    {
    public:
        // Encrypt a string
        // plainText_   - the string to be encrypted.
        // output_      - the encrypted output byte array. The caller is responsible to free it.
        // size_        - the size of output_, in bytes.
        // Returns TRUE if successful, FALSE otherwise
        static bool EncryptString(std::wstring plainText_, unsigned char **output_, int *size_);

        // Decrypt a string
        // data_        - the binary data to be decrypted.
        // size_        - the size of data_, in bytes.
        // output_      - the decrypted string. The caller is responsible to free it.
        // outputSize_  - the size of decrypted string, in bytes.
        // Returns TRUE if successful, FALSE otherwise
        static bool DecryptString(unsigned char *data_, int size_, wchar_t **output_, int *outputSize_);
    };
}
#endif SECURITY_H
