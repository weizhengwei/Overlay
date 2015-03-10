#include <errno.h>
#include <shlwapi.h>
#include <AclAPI.h>
#include <time.h>
#include "constant.h"
#include "data/Utility.h"
#include "data/DataPool.h"
#include "data/UIString.h"
#include "data/WebData.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/elements.h"
#include "crypt/md5.h"
#include "download/zip.h"
#include "download//unzip.h"
#include "data/SystemInfo.h"
#include "log/local.h"
#include <atlbase.h>
#include <Sensapi.h>
#include <Wininet.h>


#pragma comment(lib, "Sensapi.lib")

static const char * _szZipPwd = "Arc211xx";
static const TCHAR * _szFileNameInZip = _T("gameinfo.xml");

BOOL CUtility::UnzipPacakge(LPCTSTR lpszBaseDir, LPCTSTR lpszZipFile, LPCTSTR lpszDstFolder)
{
	WCHAR wszZipFile[MAX_PATH] = {0} ;
	WCHAR wszDstFolder[MAX_PATH]= {0} ;

	typedef BOOL (*UNZIP)(LPCTSTR lpszSourceFile, LPCTSTR lpszDestFolder) ;

	_tstring sZlibPath = _tstring(lpszBaseDir) + _T("ZLibWrap.dll");

	HINSTANCE hInst = LoadLibrary(sZlibPath.c_str()) ;
	if (hInst == NULL)
	{
		return FALSE ;
	}

	UNZIP UnZip = (UNZIP)GetProcAddress(hInst, "ZWZipExtract") ;
	if (UnZip == NULL)
	{
		FreeLibrary(hInst) ;
		return FALSE ;
	}

	BOOL bRet = UnZip((LPCTSTR)lpszZipFile, (LPCTSTR)lpszDstFolder) ;

	FreeLibrary(hInst) ;
	return bRet ;
}

_tstring CUtility::GetSysInfo()
{
	_tstring sSysInfo = _T("") ;
	CSystemInfo sSysString ;
	if (sSysString.Init(COINIT_MULTITHREADED))
	{
		sSysInfo = theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_OS")).c_str() ;
		sSysInfo.append(sSysString.GetOSInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_SYSMANU")).c_str()) ;
		sSysInfo.append(sSysString.GetBOISInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_PROCESSOR")).c_str());
		sSysInfo.append(sSysString.GetProcessorInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_RAM")).c_str());
		sSysInfo.append(sSysString.GetMemoryInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_MOTHERBOARD")).c_str());
		sSysInfo.append(sSysString.GetMotherboardInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_SOUND")).c_str());
		sSysInfo.append(sSysString.GetSoundDevInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_VIDEO")).c_str());
		sSysInfo.append(sSysString.GetVideoAdapterInfo()) ;
		sSysInfo.append(_T("$*$")) ;
		sSysInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_NETWORK")).c_str()) ;
		sSysInfo.append(sSysString.GetNetworkAdapterInfo()) ;
		sSysString.Uninit() ;
	}
	return sSysInfo ;
}

_tstring CUtility::GetKeyBoardTextFromVK(UINT nFunc, UINT nChar)
{
	_tstring sText = _T("") ;

	if (nFunc == VK_SHIFT)
	{
		sText += _T("Shift+") ;
	}
	else if (nFunc == VK_MENU)
	{
		sText += _T("Alt+") ;
	}
	else if (nFunc == VK_CONTROL)
	{
		sText += _T("Ctrl+") ;
	}

	switch(nChar)
	{
	case VK_ESCAPE:
		sText += _T("Escape") ;
		break ;
	case VK_F1:
		sText += _T("F1") ;
		break ;
	case VK_F2:
		sText += _T("F2") ;
		break ;
	case VK_F3:
		sText += _T("F3") ;
		break ;
	case VK_F4:
		sText += _T("F4");
		break ;
	case VK_F5:
		sText += _T("F5") ;
		break ;
	case VK_F6:
		sText += _T("F6") ;
		break ;
	case VK_F7:
		sText += _T("F7") ;
		break ;
	case VK_F8:
		sText += _T("F8") ;
		break ;
	case VK_F9:
		sText += _T("F9") ;
		break ;
	case VK_F10:
		sText += _T("F10") ;
		break ;
	case VK_F11:
		sText += _T("F11") ;
		break ;
	case VK_F12:
		sText += _T("F12") ;
		break ;
	case VK_F13:
		sText += _T("F13") ;
		break ;
	case VK_F14:
		sText += _T("F14") ;
		break ;
	case VK_F15:
		sText += _T("F15") ;
		break ;
	case VK_F16:
		sText += _T("F16");
		break ;
	case VK_F17:
		sText += _T("F17") ;
		break ;
	case VK_F18:
		sText += _T("F18") ;
		break ;
	case VK_F19:
		sText += _T("F19") ;
		break ;
	case VK_F20:
		sText += _T("F20") ;
		break ;
	case VK_F21:
		sText += _T("F21") ;
		break ;
	case VK_F22:
		sText += _T("F22") ;
		break ;
	case VK_F23:
		sText += _T("F23") ;
		break ;
	case VK_F24:
		sText += _T("F24") ;
		break ;
	case VK_SCROLL :
		sText += _T("Scroll Lock") ;
		break ;
	case VK_PAUSE:
		sText += _T("Break") ;
		break ;
	case VK_NUMLOCK:
		sText += _T("Num lock") ;
		break ;
	case VK_DIVIDE:
		sText += _T("Key pad /") ;
		break ;
	case VK_MULTIPLY:
		sText += _T("Key pad *") ;
		break ;
	case VK_ADD :
		sText += _T("Key pad +") ;
		break ;
	case VK_SUBTRACT :
		sText += _T("Key pad -") ;
		break ;
	case VK_DECIMAL:
		sText += _T("Key pad .") ;
		break ;
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
		{
			_tstring sChar ;
			sChar.Format(1, _T("NumPad%c"), nChar-48) ;
			sText += sChar ;
		}
		break ;
	case VK_OEM_3:
		sText += _T("`") ;
		break ;
	case VK_OEM_PLUS:
		sText += _T("=") ;
		break ;
	case VK_OEM_MINUS:
		sText += _T("-") ;
		break ;
	case VK_BACK:
		sText += _T("Back") ;
		break ;
	case VK_INSERT:
		sText += _T("Insert") ;
		break ;
	case VK_HOME:
		sText += _T("Home") ;
		break ;
	case VK_PRIOR:
		sText += _T("PageUp") ;
		break ;
	case VK_NEXT:
		sText += _T("PageDown") ;
		break ;
	case VK_END:
		sText += _T("End") ;
		break ;
	case VK_DELETE:
		sText += _T("Delete") ;
		break ;
	case VK_LEFT:
		sText += _T("Left") ;
		break ;
	case VK_UP:
		sText += _T("Up") ;
		break ;
	case VK_RIGHT:
		sText += _T("Right") ;
		break ;
	case VK_DOWN:
		sText += _T("Down") ;
		break ;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		{
			_tstring sChar ;
			sChar.Format(1, _T("%c"), nChar) ;
			sText += sChar ;
		}
		break ;
	case VK_TAB:
		sText += _T("Tab") ;
		break ;
	case VK_CAPITAL:
		sText += _T("Caps Lock") ;
		break ;
	case VK_RETURN:
		sText += _T("Enter") ;
		break ;
	case VK_SPACE:
		sText += _T("Space") ;
		break ;
	case VK_APPS:
		sText += _T("App") ;
		break ;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
		{
			_tstring sChar ;
			sChar.Format(1, _T("%c"), nChar) ;
			sText += sChar ;
		}
		break ;
	case VK_OEM_2:
		sText += _T("/") ;
		break ;
	case VK_OEM_PERIOD:
		sText += _T(".") ;
		break ;
	case VK_OEM_COMMA:
		sText += _T(",") ;
		break ;
	case VK_OEM_5:
		sText += _T("\\") ;
		break ;
	case VK_OEM_7:
		sText += _T("'") ;
		break ;
	case VK_OEM_1:
		sText += _T(";") ;
		break ;
	case VK_OEM_6:
		sText += _T("]") ;
		break ;
	case VK_OEM_4:
		sText += _T("[") ;
		break ;
	default:
		sText = _T("") ;
		break ;
	}

	return sText ;
}

void CUtility::GetVKFromKeyBoardText(_tstring sText, UINT& nFunc, UINT& nChar)
{
	int nPos = sText.find_first_of(_T('+')) ;

	_tstring sChar = sText  ;
	_tstring sFunc = _T("") ;

	if (nPos != -1)
	{
		sChar       = &sText[nPos+1] ;
		sText[nPos] = 0 ;
		sFunc       = sText ;
	}

	if (0 == sFunc.CompareNoCase(_T("Shift")))
	{
		nFunc = VK_SHIFT ;
	}
	else if (0 == sFunc.CompareNoCase(_T("Alt")))
	{
		nFunc = VK_MENU ;
	}
	else if (0 == sFunc.CompareNoCase(_T("Ctrl")))
	{
		nFunc = VK_CONTROL ;
	}
	else
	{
		nFunc = 0 ;
	}

	if (0 == sChar.CompareNoCase(_T("Escape")))
	{
		nChar = VK_ESCAPE ;
		return ;
	}

	int nIndex = 0 ;
	for (; nIndex < 24; ++ nIndex)
	{//VK_F1 ~ VK_F24
		_tstring sF ;
		sF.Format(1, _T("F%d"), nIndex+1) ;

		if (0 == sChar.CompareNoCase(sF))
		{
			nChar = VK_F1 + nIndex ;
			return ;
		}
	}

	for (nIndex = 0 ; nIndex < 10; ++ nIndex)
	{//NumPad0 ~ NumPad9
		_tstring sNum ;
		sNum.Format(1, _T("NumPad%d"), nIndex) ;

		if (0 == sChar.CompareNoCase(sNum))
		{
			nChar = VK_NUMPAD0 + nIndex ;
			return ;
		}
	}

	for (nIndex = 0 ; nIndex < 10; ++ nIndex)
	{//0 ~ 9
		_tstring sNum ;
		sNum.Format(1, _T("%d"), nIndex) ;

		if (0 == sChar.CompareNoCase(sNum))
		{
			nChar = '0' + nIndex ;
			return ;
		}
	}

	for (nIndex = 0 ; nIndex < 26; ++ nIndex)
	{//0 ~ 9
		_tstring sNum ;
		sNum.Format(1, _T("%c"), 'A' + nIndex) ;

		if (0 == sChar.CompareNoCase(sNum))
		{
			nChar = 'A' + nIndex ;
			return ;
		}
	}

	if (0 == sChar.CompareNoCase(_T("Scroll Lock")))
	{
		nChar = VK_SCROLL ;
	}
	else if (0 == sChar.CompareNoCase(_T("Break")))
	{
		nChar = VK_PAUSE ;
	}
	else if (0 == sChar.CompareNoCase(_T("Num lock")))
	{
		nChar = VK_NUMLOCK ;
	}
	else if (0 == sChar.CompareNoCase(_T("Key pad /")))
	{
		nChar = VK_DIVIDE ;
	}
	else if (0 == sChar.CompareNoCase(_T("Key pad *")))
	{
		nChar = VK_MULTIPLY ;
	}
	else if (0 == sChar.CompareNoCase(_T("Key pad +")))
	{
		nChar = VK_ADD ;
	}
	else if (0 == sChar.CompareNoCase(_T("Key pad -")))
	{
		nChar = VK_SUBTRACT ;
	}
	else if (0 == sChar.CompareNoCase(_T("Key pad .")))
	{
		nChar = VK_DECIMAL ;
	}
	else if (0 == sChar.CompareNoCase(_T("`")))
	{
		nChar = VK_OEM_3 ;
	}
	else if (0 == sChar.CompareNoCase(_T("=")))
	{
		nChar = VK_OEM_PLUS ;
	}
	else if (0 == sChar.CompareNoCase(_T("-")))
	{
		nChar = VK_OEM_MINUS ;
	}
	else if (0 == sChar.CompareNoCase(_T("Back")))
	{
		nChar = VK_BACK ;
	}
	else if (0 == sChar.CompareNoCase(_T("Insert")))
	{
		nChar = VK_INSERT ;
	}
	else if (0 == sChar.CompareNoCase(_T("Home")))
	{
		nChar = VK_HOME ;
	}
	else if (0 == sChar.CompareNoCase(_T("PageUp")))
	{
		nChar = VK_PRIOR ;
	}
	else if (0 == sChar.CompareNoCase(_T("PageDown")))
	{
		nChar = VK_NEXT ;
	}
	else if (0 == sChar.CompareNoCase(_T("End")))
	{
		nChar = VK_END ;
	}
	else if (0 == sChar.CompareNoCase(_T("Delete")))
	{
		nChar = VK_DELETE ;
	}
	else if (0 == sChar.CompareNoCase(_T("Left")))
	{
		nChar = VK_LEFT ;
	}
	else if (0 == sChar.CompareNoCase(_T("Up")))
	{
		nChar = VK_UP ;
	}
	else if (0 == sChar.CompareNoCase(_T("Right")))
	{
		nChar = VK_RIGHT ;
	}
	else if (0 == sChar.CompareNoCase(_T("Down")))
	{
		nChar = VK_DOWN ;
	}
	else if (0 == sChar.CompareNoCase(_T("Tab")))
	{
		nChar = VK_TAB ;
	}
	else if (0 == sChar.CompareNoCase(_T("Caps Lock")))
	{
		nChar = VK_CAPITAL ;
	}
	else if (0 == sChar.CompareNoCase(_T("Enter")))
	{
		nChar = VK_RETURN ;
	}
	else if (0 == sChar.CompareNoCase(_T("Space")))
	{
		nChar = VK_SPACE ;
	}
	else if (0 == sChar.CompareNoCase(_T("App")))
	{
		nChar = VK_APPS ;
	}
	else if (0 == sChar.CompareNoCase(_T("/")))
	{
		nChar = VK_OEM_2 ;
	}
	else if (0 == sChar.CompareNoCase(_T(".")))
	{
		nChar = VK_OEM_PERIOD ;
	}
	else if (0 == sChar.CompareNoCase(_T(",")))
	{
		nChar = VK_OEM_COMMA ;
	}
	else if (0 == sChar.CompareNoCase(_T("\\")))
	{
		nChar = VK_OEM_5 ;
	}
	else if (0 == sChar.CompareNoCase(_T("'")))
	{
		nChar = VK_OEM_7 ;
	}
	else if (0 == sChar.CompareNoCase(_T(";")))
	{
		nChar = VK_OEM_1 ;
	}
	else if (0 == sChar.CompareNoCase(_T("]")))
	{
		nChar = VK_OEM_6 ;
	}
	else if (0 == sChar.CompareNoCase(_T("[")))
	{
		nChar = VK_OEM_4 ;
	}
	else
	{
		nChar = nFunc = 0 ;
	}
}

bool CUtility::CopyFolder(const _tstring desc,const _tstring source)
{        
	_tstring tmpFrom(source);
	_tstring tmpTo(desc);    

	if (!PathFileExists(tmpTo.c_str()))
	{//if dst path not exist, create a new one.
		CreateDirectory(tmpTo.c_str(),NULL);
	}

	if (PathFileExists(tmpFrom.c_str()))
	{            
		WIN32_FIND_DATA FindFileData;
		_tstring tmpFind = tmpFrom ;
		tmpFind.append(_T("\\*.*")) ;

		HANDLE hFind=::FindFirstFile(tmpFind.c_str(),&FindFileData);
		if(INVALID_HANDLE_VALUE == hFind)
		{
			return false;
		}

		while(TRUE)
		{
			_tstring tmpFile = tmpTo ;
			tmpFile.append(_T("\\")) ;
			tmpFile.append(FindFileData.cFileName);

			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{//copy sub folder
				if(FindFileData.cFileName[0]!='.')
				{
					if (!PathFileExists(tmpFile.c_str()))
					{
						CreateDirectory(tmpFile.c_str(),NULL);    
					}

					/*
					** copy sub folder recursively.
					*/
					_tstring tmpSrcFilePath = tmpFrom + _T("\\") + FindFileData.cFileName ;
					CopyFolder(tmpFile.c_str(),tmpSrcFilePath.c_str());                
				}                
			}
			else                  
			{//copy single file.            
				//if (!PathFileExists(tmpFile))    
				{
					_tstring tmpSrcFilePath = tmpFrom + _T("\\") + FindFileData.cFileName ;
					CopyFile(tmpSrcFilePath.c_str(),tmpFile.c_str(),FALSE);
				}
			}

			if(!FindNextFile(hFind,&FindFileData))    
			{
				break;
			}
		}
		FindClose(hFind);
	}    
	return true;
}

bool CUtility::MakeDir(LPCTSTR lpszStr)
{
	_tstring strFolder(lpszStr) ;
	return MakeDir(strFolder) ;
}

bool CUtility::MakeDir(_tstring strFolder)
{
	if (strFolder.empty())
	{
		return false;
	}

	strFolder.Replace(_T("/"), _T("\\"));
	if (strFolder[strFolder.length()-1] != _T('\\'))
	{
		strFolder += _T("\\");
	}
	size_t nLength = strFolder.length();
	for (size_t i = 0; i<nLength; i++)
	{
		if (strFolder[i] == _T('\\'))
		{
			_tstring strTmpFolder = strFolder.Mid(0, i);
			if (_tmkdir(strTmpFolder.c_str()) == ENOENT)
			{
				return false;
			}
		}
	}
	return true;
}

_tstring CUtility::GetSystemSurveyInfo()
{
	_tstring sInfo = _T("") ;

	CSystemInfo sSysString ;
	sSysString.Init(COINIT_APARTMENTTHREADED) ;

	//get operating system info
	_tstring sOsInfo = CSystemInfo::GetOSDetailedInfo() ;
	
	//get system ram capacity
	_tstring sSystemRam = sSysString.GetMemoryInfo() ;

	//get cpu info
	_tstring sCPUInfo = sSysString.GetProcessorInfo() ;
	_tstring sCPUCount(CSystemInfo::GetCPUInfo().dwNumberOfProcessors) ;
	_tstring sCPUFeq  = CSystemInfo::GetCPUFrequency() ;

	//get sound card info
	_tstring sSoundInfo = sSysString.GetSoundDevInfo() ;

	//get video card info
	_tstring sVideoInfo   = sSysString.GetVideoAdapterInfo() ;
	_tstring sVideoMemory = sSysString.GetVideoMemory() ;

	//get screen resolution
	_tstring sResolution ;
	sResolution.Format(2, _T("%d*%d"), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)) ;

	//get disk space info
	_tstring sTotalDiskSpace, sFreeDiskSpace ;
	sSysString.GetDiskSpaceInfo(sTotalDiskSpace, sFreeDiskSpace) ;

	//get installed softwares list
	vector<_tstring> vecSoftware = CSystemInfo::GetInstalledSoftware() ;
	_tstring sSoftwareList = _T("") ;
	for (int i = 0; i < vecSoftware.size(); i++)
	{
		_tstring sItem = vecSoftware[i] ;
		if (sItem.Find(_T("(KB")) == string::npos)
		{
			sSoftwareList.append(vecSoftware[i]);

			if (!sSoftwareList.empty())
			{
				sSoftwareList.append(_T(";")) ;
			}
		}
	}
	sSysString.Uninit() ;

	/*
	** make json text package
	*/
	json::Object SystemInfo ;
	SystemInfo["OS Version"]        = json::String(sOsInfo.toNarrowString().c_str()) ;
	SystemInfo["System RAM"]        = json::String(sSystemRam.toNarrowString().c_str()) ; 
	SystemInfo["CPU Info"]          = json::String(sCPUInfo.toNarrowString().c_str()) ; 
	SystemInfo["CPU Speed"]         = json::String(sCPUFeq.toNarrowString().c_str()) ;
	SystemInfo["Physical CPUs"]     = json::String(sCPUCount.toNarrowString().c_str()) ;
	SystemInfo["Sound Card Info"]   = json::String(sSoundInfo.toNarrowString().c_str()) ;
	SystemInfo["Video Card Info"]   = json::String(sVideoInfo.toNarrowString().c_str()) ;
	SystemInfo["Video RAM"]         = json::String(sVideoMemory.toNarrowString().c_str()) ;
	SystemInfo["Primary Display Resolution"] = json::String(sResolution.toNarrowString().c_str()) ;
	SystemInfo["Total Hard Drive Space"]     = json::String(sTotalDiskSpace.toNarrowString().c_str()) ;
	SystemInfo["Free Hard Drive Space"]      = json::String(sFreeDiskSpace.toNarrowString().c_str()) ;
	SystemInfo["Installed Software List"]    = json::String(sSoftwareList.toNarrowString().c_str()) ;
	SystemInfo["Sound Card Info"]   = json::String(sSoundInfo.toNarrowString().c_str()) ;

	std::stringstream stream ;
	json::Writer::Write(SystemInfo, stream) ;

	return _tstring(stream.str()) ;
}

BOOL CUtility::GenerateMD5ForFile(_tstring sFilePath, unsigned char szMD5[16])
{
	const int nBlockSize   = 1024*1024 ;
	HANDLE hFile = CreateFile(sFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER lFileSize = {0} ;
		DWORD dwSizeHigh = 0 ;
		lFileSize.LowPart = GetFileSize(hFile, &dwSizeHigh) ;
		lFileSize.HighPart= dwSizeHigh ;
		if (lFileSize.QuadPart != 0)
		{
			char* pBlockBuffer = new char[nBlockSize] ;
			memset(pBlockBuffer, 0, nBlockSize) ;

			MD5_CTX ctx;
			MD5Init(&ctx);
			while (lFileSize.QuadPart > 0)
			{
				DWORD dwRead = 0 ;
				if (lFileSize.QuadPart >= nBlockSize)
				{
					ReadFile(hFile, pBlockBuffer, nBlockSize, &dwRead, NULL) ;
				}
				else
				{
					ReadFile(hFile, pBlockBuffer, lFileSize.LowPart, &dwRead, NULL) ;
				}
				lFileSize.QuadPart -= nBlockSize ;
				MD5Update(&ctx, (const unsigned char*)pBlockBuffer, dwRead);	
			}
			MD5Final(szMD5, &ctx);

			delete [] pBlockBuffer ;
			pBlockBuffer = NULL ;
		}

		CloseHandle(hFile) ;
	}
	else
	{
		DWORD dwErr = GetLastError() ;
		return FALSE ;
	}

	return TRUE ;
}

_tstring CUtility::ConvertHexToString(unsigned char* szBuf, int nLen)
{
	if (NULL == szBuf)
	{
		return _T("") ;
	}

	char* pResult = new char[nLen*2+1] ;
	if (pResult == NULL)
	{
		return _T("") ;
	}
	memset(pResult, 0, nLen*2 + 1) ;

	for (int i=0; i < nLen; ++ i)
	{
		pResult[i*2]   = ConvertHexToChar(szBuf[i] >> 4) ;
		pResult[i*2+1] = ConvertHexToChar(szBuf[i] & 0x0f) ;
	}

	_tstring sResult(pResult) ;

	delete pResult ;

	return sResult ;
}

unsigned char CUtility::ConvertHexToChar(unsigned char cHex)
{
	char cResult = 0 ;

	if ((cHex >= 0) && (cHex <= 9))
	{
		cResult = '0' + cHex ;
	}
	else if ((cHex > 9) && (cHex <= 15))
	{
		cResult = 'A' + cHex - 10 ;
	}

	return cResult ;
}

int CUtility::CompareSysTime(const SYSTEMTIME sysTime1,const SYSTEMTIME sysTime2)
{
	int nRet = 1;// //if systime:-1,sysTime1<sysTime2;0,sysTime1=sysTime2;1,sysTime1>sysTime2
	do 
	{
		if (sysTime1.wYear > sysTime2.wYear)
		{
			break;
		}
		else if (sysTime1.wYear == sysTime2.wYear)
		{
			if (sysTime1.wMonth > sysTime2.wMonth)
			{
				break;
			}
			else if (sysTime1.wMonth == sysTime2.wMonth)
			{
				if (sysTime1.wDay > sysTime2.wDay)
				{
					break;
				}
				else if (sysTime1.wDay == sysTime2.wDay)
				{
					if (sysTime1.wHour > sysTime2.wHour)
					{
						break;
					}
					else if ( sysTime1.wHour == sysTime2.wHour)
					{
						if (sysTime1.wMinute > sysTime2.wMinute)
						{
							break;
						}
						else if (sysTime1.wMinute == sysTime2.wMinute)
						{
							if (sysTime1.wSecond > sysTime2.wSecond)
							{
								break;
							}
							else if (sysTime1.wSecond == sysTime2.wSecond)
							{
								nRet = 0;
								break;
							}
						}
					}
				}
			}
		}
		nRet = -1;
	} while (FALSE);
	return nRet;	
}

BOOL CUtility::IsPathValid(_tstring sPath)
{
	if (sPath.empty() || (sPath.length() <= 1))
	{
		return FALSE ;
	}

	if (!((sPath[0] > _T('a') && sPath[0] < _T('z')) || (sPath[0] > _T('A') && sPath[0] < _T('Z'))))
	{//first character must be english letter
		return FALSE ;
	}

	if (sPath[1] != _T(':'))
	{//: need to be put in second position
		return FALSE ;
	}

	if ((sPath.length() >= 3) && (sPath[2] != _T('\\')))
	{
		return FALSE ;
	}

	if ((sPath.find(_T('/')) != string::npos) || (sPath.find(_T('*')) != string::npos) || \
		(sPath.find(_T('"')) != string::npos) || (sPath.find(_T('?')) != string::npos) || \
		(sPath.find(_T('<')) != string::npos) || (sPath.find(_T('>')) != string::npos) || \
		(sPath.find(_T('|')) != string::npos) || (_tstring(&sPath[2]).find(_T(':')) != string::npos) \
		)
	{//these characters should not be included in a valid path.
		return FALSE ;
	}

	_tstring sTempPath = sPath ;
	while (!sTempPath.empty())
	{//continuous \ is invalid.
		int nPos = sTempPath.find(_T('\\')) ;
		if (nPos == -1)
		{
			break ;
		}

		if (sTempPath.length() <= (nPos+1))
		{//reach end of path
			break ;
		}

		sTempPath = &sTempPath[nPos+1] ;
		if (sTempPath[0] == _T('\\'))
		{
			return FALSE ;
		}
	}

	return TRUE ;
}

_tstring CUtility::ZipFile(_tstring sFilePath)
{
	_tstring sZipPath = sFilePath + _T(".zip");
	_tstring sFileName = sFilePath ;
	int nPos = sFileName.find_last_of(_T('\\')) ;
	if (nPos != -1)
	{
		sFileName = &sFileName[nPos+1] ;
	}
	HZIP hZip = CreateZip(sZipPath.c_str(), NULL);
	ZRESULT zRes = ZipAdd(hZip, sFileName.c_str(), sFilePath.c_str());
	zRes = CloseZip(hZip);
	return sZipPath;
}

BOOL CUtility::ZipMultiFile(_tstring sDisZip,std::vector<_tstring>& vecFileList)
{
	int nPos = sDisZip.find_last_of(_T("."));
	if (nPos != -1)
	{
		_tstring sTmp(&sDisZip[nPos+1]);
		if (sTmp.CompareNoCase(_T("zip")) != 0)
		{
           sDisZip.append(_T(".zip"));
		}
	}else
	{
        sDisZip.append(_T(".zip"));
	}
	HZIP hZip = CreateZip(sDisZip.c_str(), NULL);
	std::vector<_tstring>::iterator iter = vecFileList.begin();
	for (;iter != vecFileList.end();++iter)
	{
		_tstring sFileName(*iter);
		int nPos = sFileName.find_last_of(_T('\\')) ;
		if (nPos != -1)
		{
			sFileName = &sFileName[nPos+1] ;
		}
		ZRESULT zRes = ZipAdd(hZip, sFileName.c_str(), _tstring(*iter).c_str());
		DeleteFile(_tstring(*iter).c_str());
	}
	ZRESULT zRes = CloseZip(hZip);
	
	return zRes == ZR_OK;
}

bool CUtility::IsFileOccupied(LPCTSTR lpszFilePath)
{
	if (lpszFilePath == NULL)
	{
		return false ;
	}

	if (_taccess(lpszFilePath, 0))
	{
		return false ;
	}

	HANDLE hFile = CreateFile(lpszFilePath, GENERIC_ALL,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL); 
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return true ;
	}
	else
	{
		CloseHandle(hFile) ;
	}

	return false ;
}

BOOL CUtility::IsPathAuthorized(_tstring sPath)
{
	if(!CUtility::IsPathValid(sPath))
	{
		return FALSE;
	}

	PSECURITY_DESCRIPTOR pSD = NULL;
	PTRUSTEE pOwner = NULL;
	PTRUSTEE pGroup = NULL;
	ULONG lCount;
	PEXPLICIT_ACCESS pList = NULL;

	BOOL bResult = TRUE;
	do
	{
		DWORD dwRet = GetNamedSecurityInfo(
			sPath.c_str(),
			SE_FILE_OBJECT,
			DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
			NULL,
			NULL,
			NULL,
			NULL,
			&pSD);

		if (dwRet != ERROR_SUCCESS)
		{
			bResult = FALSE;
			break;
		}

		dwRet = LookupSecurityDescriptorParts(
			&pOwner,
			&pGroup,
			&lCount,
			&pList,
			NULL,
			NULL,
			pSD
			);

		if (dwRet != ERROR_SUCCESS)
		{
			bResult = FALSE;
			break;
		}

		if (lCount>0 && pList != NULL)
		{
			_tstring strAdmin = _T("BUILTIN\\Administrators");
			_tstring strTInstaller = _T("NT SERVICE\\TrustedInstaller");

			if(!strAdmin.CompareNoCase(pGroup->ptstrName)
				|| !strTInstaller.CompareNoCase(pGroup->ptstrName)
				)
			{
				bResult = FALSE;
				break;
			}
		}
	}
	while(FALSE);

	if (pSD)
	{
		LocalFree((HLOCAL)pSD);
	}
	if (pList)
	{
		LocalFree((HLOCAL)pList);
	}
	if (pOwner)
	{
		LocalFree((HLOCAL)pOwner);
	}
	if (pGroup)
	{
		LocalFree((HLOCAL)pGroup);
	}

	return bResult;
}

BOOL CUtility::DeleteFileEx(LPCTSTR lpFileName, LPCTSTR szDelExt)
{
	if(!PathFileExists(lpFileName) || PathIsDirectory(lpFileName))
	{
		return TRUE;
	}
	SetFileAttributes(lpFileName, 0);
	if(!DeleteFile(lpFileName))
	{
		TCHAR szName[MAX_PATH];
		if(szDelExt && szDelExt[0])
		{
			_sntprintf(szName, MAX_PATH, _T("%s.%s"), lpFileName, szDelExt);
		}
		else
		{
			srand((unsigned int)time(NULL));
			_sntprintf(szName, MAX_PATH, _T("%s.%d"), lpFileName, rand()%10000);
		}
		if(!MoveFileEx(lpFileName, szName, MOVEFILE_REPLACE_EXISTING))
		{
			return FALSE;
		}
		MoveFileEx(szName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		return TRUE;
	}
	return TRUE;
}

BOOL CUtility::NavURLWithDefaultBrowser(_tstring sURL)
{
	HINSTANCE hRes = ShellExecute(NULL,_T("open"),sURL.c_str(), NULL,NULL,SW_SHOWNORMAL);
	if ((int)hRes >=32)
	{
		return TRUE;
	}

	_tstring sApp(_T(""));
	BOOL bRet= FALSE;
	CRegKey regKey;
	do 
	{
		TCHAR keyValue[1024] = {0};
		ULONG nSize = 1024;
		if(regKey.Open(HKEY_CLASSES_ROOT, _T("HTTP\\shell\\open\\command")) != ERROR_SUCCESS)
		{
			break;
		}
		if(regKey.QueryStringValue(NULL, keyValue, &nSize) != ERROR_SUCCESS || _tcslen(keyValue) == 0)
		{
			break;
		}
		_tstring str(keyValue);
		_tstring strlower = str;
		strlower.MakeLower();
		int nPos = strlower.Find(_T(".exe"));
		if (nPos ==-1)
		{
			break;
		}
		sApp= str.Left(nPos+4);
		if (sApp.Left(1) == _T("\""))
		{
			sApp.append(_T("\""));
		}
		bRet =TRUE;
	} while (FALSE);

	regKey.Close();
	if (bRet)
	{
		ShellExecute(NULL,_T("open"),sApp.c_str(),sURL.c_str(), NULL, SW_SHOWNORMAL);
	}
	return bRet;
}

BOOL CUtility::LoadEncryptedXML( TiXmlDocument * pDoc, LPCTSTR szFilePath )
{
	if(!pDoc || !PathFileExists(szFilePath))
	{
		return FALSE;
	}
	
	BOOL bRet = FALSE;

	char stmpPath[MAX_PATH] = {0};
	int nConvLength = WideCharToMultiByte(CP_ACP, 0, szFilePath, -1, stmpPath, MAX_PATH, NULL, NULL);

	//avoid crash
	__try
	{
		bRet = pDoc->LoadFile(stmpPath);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bRet = FALSE;
	}
	if(bRet)
	{
		return TRUE;
	}
	pDoc->Clear();
	HZIP hz = OpenZip(szFilePath, _szZipPwd);
	if(hz)
	{
		ZIPENTRY ze;
		if(GetZipItem(hz, 0, &ze) == ZR_OK)
		{
			int nUncSize = ze.unc_size > 0 ? ze.unc_size : 1024*100;
			char * pBuff = new char[nUncSize];
			if(UnzipItem(hz, 0, pBuff, (UINT)nUncSize) == ZR_OK)
			{
				//avoid crash
				__try
				{
					bRet = !!pDoc->Parse(pBuff);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					bRet = FALSE;
				}
			}
			delete []pBuff;
		}
		CloseZip(hz);
	}	
	return bRet;
}

BOOL CUtility::SaveEncryptedXML( TiXmlDocument * pDoc, LPCTSTR szFilePath )
{
	if(!pDoc || !szFilePath)
	{
		return FALSE;
	}
	TiXmlPrinter printer;
	if(!pDoc->Accept(&printer))
	{
		return FALSE;
	}
	BOOL bRet = FALSE;
	HZIP hz = CreateZip(szFilePath, _szZipPwd);
	if(hz)
	{
		if(ZipAdd(hz, _szFileNameInZip, (LPVOID)printer.CStr(), printer.Size()) == ZR_OK)
		{
			bRet = TRUE;
		}
		CloseZip(hz);
	}
	return bRet;
}

BOOL CUtility::IsNetworkAlive()
{
	DWORD dwFlag0 = 0;
	if (InternetGetConnectedState(&dwFlag0, 0))
	{
		return TRUE;
	}

	DWORD dwFlag1 = 0;
	if (::IsNetworkAlive(&dwFlag1))
	{	
		return TRUE;
	}

	return FALSE;
}

WORD CUtility::FileMachineType( LPCSTR lpExecuteFile )
{
    FILE *pfile = fopen(lpExecuteFile, "rb");
    if (NULL == pfile)
        return (WORD)(0);

    IMAGE_DOS_HEADER idh;
    fread(&idh, sizeof(idh), 1, pfile);

    IMAGE_FILE_HEADER ifh;
    fseek(pfile, idh.e_lfanew + 4, SEEK_SET);
    fread(&ifh, sizeof(ifh), 1, pfile);
    fclose(pfile);

    return ifh.Machine;
}

