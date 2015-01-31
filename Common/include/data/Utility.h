#pragma once

#include "datatype.h"
#include <vector>
#include "tinyxml/tinyxml.h"

class CUtility
{
public:

	static _tstring GetSysInfo() ;
	static _tstring GetSystemSurveyInfo() ;

	static _tstring GetKeyBoardTextFromVK(UINT nFunc, UINT nChar) ;
	static void GetVKFromKeyBoardText(_tstring sText, UINT& nFunc, UINT& nChar) ;
	
	static _tstring ConvertHexToString(unsigned char* szBuf, int nLen) ;
	static unsigned char ConvertHexToChar(unsigned char cHex) ;
	
	//compare two systime:-1,sysTime1<sysTime2;0,sysTime1=sysTime2;1,sysTime1>sysTime2
	static int CompareSysTime(const SYSTEMTIME sysTime1,const SYSTEMTIME sysTime2);
	
	static bool MakeDir(_tstring strFolder) ;
	static bool MakeDir(LPCTSTR lpszStr);
	static BOOL IsPathValid(_tstring sPath) ;
	static BOOL IsPathAuthorized(_tstring sPath) ;
	static bool CopyFolder(const _tstring desc,const _tstring source) ;

	static BOOL UnzipPacakge(LPCTSTR lpszBaseDir, LPCTSTR lpszZipFile, LPCTSTR lpszDstFolder) ;
	static BOOL GenerateMD5ForFile(_tstring sFilePath, unsigned char szMD5[16]) ;
	static _tstring ZipFile(_tstring sFilePath);
	static BOOL ZipMultiFile(_tstring sDisZip,std::vector<_tstring>& vecFileList);

	static bool IsFileOccupied(LPCTSTR lpszFilePath) ;
	static BOOL DeleteFileEx(LPCTSTR lpFileName, LPCTSTR szDelExt = NULL) ;
	static BOOL NavURLWithDefaultBrowser(_tstring sURL);

	static BOOL LoadEncryptedXML(TiXmlDocument * pDoc, LPCTSTR szFilePath);
	static BOOL SaveEncryptedXML(TiXmlDocument * pDoc, LPCTSTR szFilePath);

	static BOOL IsNetworkAlive();
    static WORD FileMachineType(LPCSTR lpExecuteFile);
};