#ifndef _KAES_H
#define _KAES_H

#include <windows.h>

// 
// AES²Ù×÷¾ä±ú
// 

#ifndef AES_HANDLE
#define AES_HANDLE PVOID
#endif // !AES_HANDLE


#ifndef BYTE
#define BYTE UCHAR
#endif // !BYTE

#ifndef LPBYTE
#define LPBYTE BYTE*
#endif // LPBYTE


typedef enum
{
	BIT128 = 0,
	BIT192,
	BIT256

} ENUM_KEYSIZE;


///extern "C"
AES_HANDLE AESInitialize(ENUM_KEYSIZE keysize, BYTE *AesKey);

///extern "C"
void AESUnInitialize(AES_HANDLE AesHandle);

///extern "C"
void AESCipher(AES_HANDLE AesHandle, BYTE *input, BYTE *output, ULONG length);

///extern "C"
void AESInvCipher(AES_HANDLE AesHandle, BYTE *input, BYTE *output, ULONG length);


#endif // !_KAES_H