#include "stdafx.h"
#include <wincrypt.h>

#include "crypt/des.h"


BOOL DES(__in const BYTE* pbKey,
		 __in DWORD cbKeyLen,
		 __in const BYTE* pbData,
		 __in DWORD cbDataLen,
		 __out BYTE* pbBuf,
		 __inout DWORD* pcbBufLen,
		 __in BOOL bIsDecrypt  
		 )
{
	struct
	{
		BLOBHEADER hdr;
		DWORD cbKeySize;
		BYTE rgbKeyData[8];
	} keyBlob;                      //½á¹¹²Î¿¼MSDN - CryptImportKey

	keyBlob.hdr.bType = PLAINTEXTKEYBLOB;
	keyBlob.hdr.bVersion = CUR_BLOB_VERSION;
	keyBlob.hdr.reserved = 0;
	keyBlob.hdr.aiKeyAlg = CALG_DES;
	keyBlob.cbKeySize = 8;
	ZeroMemory(keyBlob.rgbKeyData, 8);
	CopyMemory(keyBlob.rgbKeyData, pbKey, cbKeyLen > 8 ? 8 : cbKeyLen);

	HCRYPTPROV hProv;
	if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0))
		return FALSE;

	HCRYPTKEY hKey;
	if (!CryptImportKey(hProv, (BYTE*)&keyBlob, sizeof(keyBlob), 0, 0, &hKey))
	{
		CryptReleaseContext(hProv, 0);
		return FALSE;
	}

	BOOL bRet;
	BYTE pbBlock[16];
	DWORD dwBlock, dwOut = 0;
	BOOL bEOF;
	for (DWORD i = 0; i < cbDataLen; i += 8)
	{
		bEOF = cbDataLen - i <= 8;    
		dwBlock = bEOF ? cbDataLen - i : 8;

		CopyMemory(pbBlock, pbData + i, dwBlock);    
		if (bIsDecrypt)
			bRet = CryptDecrypt(hKey, NULL, bEOF, 0, pbBlock, &dwBlock);
		else
			bRet = CryptEncrypt(hKey, NULL, bEOF, 0, pbBlock, &dwBlock, sizeof(pbBlock));

		if (!bRet)
			break;

		if (pbBuf)
		{
			if (dwOut + dwBlock > *pcbBufLen)
			{
				bRet = FALSE;
				break;
			} 
			else
			{
				CopyMemory(pbBuf + dwOut, pbBlock, dwBlock);
				dwOut += dwBlock;
			}
		} 
		else   
			dwOut += dwBlock;
	}

	*pcbBufLen = dwOut;
	CryptDestroyKey(hKey);
	CryptReleaseContext(hProv, 0);
	return bRet;
}
