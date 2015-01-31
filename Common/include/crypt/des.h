#pragma  once

BOOL DES(__in const BYTE* pbKey,
		 __in DWORD cbKeyLen,
		 __in const BYTE* pbData,
		 __in DWORD cbDataLen,
		 __out BYTE* pbBuf,
		 __inout DWORD* pcbBufLen,
		 __in BOOL bIsDecrypt = FALSE  
		 );
