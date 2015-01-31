#include <tchar.h>
#include <atlbase.h>
#include <iphlpapi.h>
#include <algorithm>
#include <WbemProv.h>
#include "data/SystemInfo.h"
#include "data/ddraw.h"
#include "data/UIString.h"
#include "data/Registry.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "iphlpapi.lib")

CSystemInfo::CSystemInfo()
	:m_pLoc(NULL),m_pSvc(NULL),m_bCoInit(false)
{
	SYSINFO_TYPE[0] = _T("Win32_OperatingSystem") ;
	SYSINFO_TYPE[1] = _T("Win32_BIOS") ;
	SYSINFO_TYPE[2] = _T("Win32_Processor") ;
	SYSINFO_TYPE[3] = _T("Win32_PhysicalMemory") ;
	SYSINFO_TYPE[4] = _T("Win32_BaseBoard") ;
	SYSINFO_TYPE[5] = _T("Win32_SoundDevice") ;
	SYSINFO_TYPE[6] = _T("Win32_VideoController") ;
	SYSINFO_TYPE[7] = _T("Win32_NetworkAdapter") ;
	SYSINFO_TYPE[8] = _T("Win32_PnPSignedDriver") ;
}

CSystemInfo::~CSystemInfo()
{
}

bool CSystemInfo::Init(DWORD dwFlag)
{
	bool bRet = false ;

	do {
		__try{

			HRESULT hRes = CoInitializeEx(NULL, dwFlag) ;
			if(S_OK == hRes)
			{//this will fail if CoInitializeEx is called in the context of 
			 //main looping thread, so do not care the result of hRes under
			 //this condition, just pass and do not call CoUninitialize in the end.
				m_bCoInit = true ;
			}

			hRes = CoInitializeSecurity(NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE, NULL) ; 
			if((S_OK != hRes) && (RPC_E_TOO_LATE != hRes))
			{
				break ;
			}

			hRes = CoCreateInstance(CLSID_WbemAdministrativeLocator, 0,
				CLSCTX_INPROC_SERVER, 
				IID_IWbemLocator,(LPVOID*)&m_pLoc);
			if(FAILED(hRes)) 
			{
				break ;
			}

			hRes = m_pLoc->ConnectServer(L"ROOT\\CIMV2",NULL, NULL, 0, NULL, 0, 0, &m_pSvc);
			if(S_OK!=hRes)
			{
				break ;
			}

			hRes = CoSetProxyBlanket(m_pSvc,
				RPC_C_AUTHN_WINNT,
				RPC_C_AUTHZ_NONE,
				NULL,
				RPC_C_AUTHN_LEVEL_CALL,
				RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL,
				EOAC_NONE);
			if (FAILED(hRes))
			{
				break ;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER){
			bRet = false ;
			break ;
		}
		bRet = true ;
	}while(false) ;

	if (!bRet)
	{
		if (m_pSvc != NULL)
		{
			m_pSvc->Release() ;
			m_pSvc = NULL ;
		}

		if (m_pLoc != NULL)
		{
			m_pLoc->Release() ;
			m_pLoc = NULL ;
		}

		if (m_bCoInit)
		{
			CoUninitialize();
			m_bCoInit = false ;
		}
	}

	return true ;
}

bool CSystemInfo::Uninit()
{
	if (m_pSvc != NULL && m_pLoc != NULL)
	{
		m_pSvc->Release();
		m_pLoc->Release();
	}

	if (m_bCoInit)
	{
		CoUninitialize() ;
	}

	return true ;
}

_tstring CSystemInfo::GetOSInfo()
{
	return GetInfo(SYSINFO_TYPE[0], 0) ;
}

_tstring CSystemInfo::GetProcessorInfo()
{
    _tstring sInfo = _T("");
    CRegistry reg ;
    sInfo = reg.GetProcessorInfo();//fixed bug: in xp os, the info is not updated in WMI database.
    if (sInfo.empty())
    {
       sInfo = GetInfo(SYSINFO_TYPE[2], 2);
    }
	return sInfo;
}

_tstring CSystemInfo::GetBOISInfo()
{
	return GetInfo(SYSINFO_TYPE[1], 1) ;
}

_tstring CSystemInfo::GetMemoryInfo()
{
	return GetInfo(SYSINFO_TYPE[3], 3) ;
}

_tstring CSystemInfo::GetMotherboardInfo()
{
	return GetInfo(SYSINFO_TYPE[4], 4) ;
}

_tstring CSystemInfo::GetSoundDevInfo()
{
	return GetInfo(SYSINFO_TYPE[5], 5) ;
}

_tstring CSystemInfo::GetVideoAdapterInfo()
{
	return GetInfo(SYSINFO_TYPE[6], 6) ;
}

_tstring CSystemInfo::GetNetworkAdapterInfo()
{
	return GetInfo(SYSINFO_TYPE[7], 7) ;
}

_tstring CSystemInfo::GetDriverInfo()
{
	return GetInfo(SYSINFO_TYPE[8], 8) ;
}

DWORD CSystemInfo::GetVideoMemory()
{
	LPDIRECTDRAW2 lpdd=NULL ; 
	HRESULT   ddrval=S_FALSE; 
	DWORD dwMem =0;
	__try
	{
		ddrval = CoCreateInstance(CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw2, (void**)&lpdd) ;
		if (FAILED(ddrval) || !lpdd)
		{
			return 0;
		}

		ddrval = IDirectDraw2_Initialize(lpdd, NULL) ;
		DDCAPS ddcaps={0} ;
		ddcaps.dwSize =sizeof(DDCAPS) ; 
		lpdd->GetCaps(&ddcaps,NULL); 
		dwMem = ddcaps.dwVidMemTotal ;
		lpdd->Release() ; lpdd=NULL;
		
	}
    __except(EXCEPTION_EXECUTE_HANDLER)
	{
         return 0;
	}
	return (dwMem/1024/1024);
}

/* get approximate frequency of cpu */
unsigned __int64 CSystemInfo::GetCPUFrequency()
{
	unsigned  __int64 start=GetCycleCount();
	Sleep(1000);
	unsigned  __int64 end=GetCycleCount();

	return (end - start)/1024/1024 ; // Hz
}

_tstring CSystemInfo::GetMacAddress()
{
	DWORD AdapterInfoSize = 0 ;  
	DWORD Err = GetAdaptersInfo(NULL, &AdapterInfoSize) ; 
	if((Err != 0) && (Err != ERROR_BUFFER_OVERFLOW))
	{  
		return _T("") ;  
	}  

	PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, AdapterInfoSize);
	if(pAdapterInfo == NULL)
	{
		return _T("") ;  
	}

	if(GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize) != 0)
	{  
		GlobalFree(pAdapterInfo) ;  
		return _T("") ;  
	}

	_tstring strMac ;
	strMac.Format(6, _T("%02X%02X%02X%02X%02X%02X"),
		pAdapterInfo->Address[0],  
		pAdapterInfo->Address[1],  
		pAdapterInfo->Address[2],  
		pAdapterInfo->Address[3],  
		pAdapterInfo->Address[4],  
		pAdapterInfo->Address[5]);  

	GlobalFree(pAdapterInfo);  

	return strMac ;  
}

vector<_tstring> CSystemInfo::GetInstalledSoftware()
{
	HKEY hKey;
	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),\
		0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return vector<_tstring>() ;
	}

	vector<_tstring> vecSoftware ;

	DWORD dwIndex = 0;
	LONG lRet;
	DWORD cbName = IS_KEY_LEN;
	TCHAR szSubKeyName[IS_KEY_LEN];

	while ((lRet = ::RegEnumKeyEx(hKey, dwIndex, szSubKeyName, &cbName, NULL, \
		NULL, NULL, NULL)) != ERROR_NO_MORE_ITEMS)
	{
		// Do we have a key to open?
		if (lRet == ERROR_SUCCESS)
		{
			// Open the key and get the value
			HKEY hItem;
			if (::RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_READ, &hItem) != ERROR_SUCCESS)
				continue;
			// Opened - look for "DisplayName"
			TCHAR szDisplayName[IS_KEY_LEN];
			DWORD dwSize = sizeof(szDisplayName);
			DWORD dwType;

			if (::RegQueryValueEx(hItem, IS_DISPLAY, NULL, &dwType,
				(LPBYTE)&szDisplayName, &dwSize) == ERROR_SUCCESS)
			{
				// Add to the main array
				if (vecSoftware.end() == find(vecSoftware.begin(), vecSoftware.end(), _tstring(szDisplayName)))
				{
					vecSoftware.push_back(_tstring(szDisplayName)) ;
				}
				else
				{
					szDisplayName[0] = 0 ;
				}
			}
			::RegCloseKey(hItem);
		}
		dwIndex++;
		cbName = IS_KEY_LEN;
	}
	::RegCloseKey(hKey);

	return vecSoftware ;
}

void CSystemInfo::GetDiskSpaceInfo(_tstring& sTotalSpace, _tstring& sFreeSpace)
{
	ULARGE_INTEGER sTotalBytes = {0}, sFreeBytes = {0};
	for (int i=0; i < 24 ; ++i)
	{
		TCHAR szDrive[3] = _T("C:") ;
		szDrive[0] = szDrive[0] + i ;
		ULARGE_INTEGER sVolTotalBytes, sVolFreeBytes ;
		if (GetDriveType(szDrive) == DRIVE_FIXED)
		{
			GetDiskFreeSpaceEx(szDrive, NULL, &sVolTotalBytes, &sVolFreeBytes) ;
			sTotalBytes.QuadPart += sVolTotalBytes.QuadPart ;
			sFreeBytes.QuadPart  += sVolFreeBytes.QuadPart  ;
		}
	}
	
	sTotalSpace = _tstring(sTotalBytes.QuadPart/1024/1024/1024) ;
	sFreeSpace  = _tstring(sFreeBytes.QuadPart/1024/1024/1024) ;
}

_tstring CSystemInfo::GetOSDetailedInfo()
{
	OSVERSIONINFOEX sOsVersion = {0} ;
	sOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX) ; 
	GetVersionEx((OSVERSIONINFO*)&sOsVersion) ;

	SYSTEM_INFO sSysInfo = {0} ;
	GetSystemInfo(&sSysInfo) ;

	BOOL b64BitOpSys ;
#ifdef _WIN64
	b64BitOpSys = TRUE
#else
	IsWow64Process(GetCurrentProcess(), &b64BitOpSys) ;
#endif

	/*
	** get operating system version
	*/ 
	_tstring strOsVersion = _T("") ;
	if (sOsVersion.dwMajorVersion == 5)
	{
		if (sOsVersion.dwMinorVersion == 1)
		{//Windows XP
			strOsVersion = _T("Windows XP ") ;
		}
		else if (sOsVersion.dwMinorVersion == 0)
		{//Windows 2000
			strOsVersion = _T("Windows 2000 ") ;
		}
		else if (sOsVersion.dwMinorVersion == 2)
		{
			if ((sOsVersion.wProductType == VER_NT_WORKSTATION) && (sSysInfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64))
			{//Windows XP Professional x64 Edition
				strOsVersion = _T("Windows XP Professional x64 Edition ") ;
			}
			else if (GetSystemMetrics(SM_SERVERR2) == 0)
			{//Windows Server 2003
				strOsVersion = _T("Windows Server 2003 ") ;
			}
			else if (sOsVersion.wSuiteMask & VER_SUITE_WH_SERVER)
			{//Windows Home Server
				strOsVersion = _T("Windows Home Server ") ;
			}
			else if (GetSystemMetrics(SM_SERVERR2) != 0)
			{//Windows Server 2003 R2
				strOsVersion = _T("Windows Server 2003 R2 ") ;
			}
		}
	}
	else if (sOsVersion.dwMajorVersion == 6)
	{
		if (sOsVersion.dwMinorVersion == 1)
		{
			if (sOsVersion.wProductType == VER_NT_WORKSTATION)
			{//Windows 7
				strOsVersion = _T("Windows 7 ") ;
			}
			else
			{//Windows Server 2008 R2
				strOsVersion = _T("Windows Server 2008 R2 ") ;
			}
		}
		else if (sOsVersion.dwMinorVersion == 0)
		{
			if (sOsVersion.wProductType == VER_NT_WORKSTATION)
			{//Windows Vista
				strOsVersion = _T("Windows Vista") ;
			}
			else
			{//Windows Server 2008
				strOsVersion = _T("Windows Server 2008 ") ;
			}
		}
		else if (sOsVersion.dwMinorVersion == 2)
		{
			if (sOsVersion.wProductType == VER_NT_WORKSTATION)
			{//Windows Vista
				strOsVersion = _T("Windows 8 ") ;
			}
			else
			{//Windows Server 2008
				strOsVersion = _T("Windows Server 2012 ") ;
			}
		}
	}

	if (!strOsVersion.empty())
	{
		strOsVersion.append(_tstring(sOsVersion.szCSDVersion).c_str()) ;
	}

	if (b64BitOpSys)
	{
		strOsVersion.append(_T(" 64bit")) ;
	}
	else 
	{
		strOsVersion.append(_T(" 32bit")) ;
	}

	return strOsVersion ;
}

SYSTEM_INFO CSystemInfo::GetCPUInfo()
{
	SYSTEM_INFO sSysInfo = {0} ;
	GetSystemInfo(&sSysInfo) ;

	return sSysInfo ;
}

/* get tick counts from CPU running */
unsigned __int64 CSystemInfo::GetCycleCount()
{
	__asm   _emit   0x0F ;  
	__asm   _emit   0x31 ; 
}

_tstring CSystemInfo::GetInfo(_tstring sSysInfoType, int nTypeIndex)
{
	_tstring sSysInfo(_T("")) ;
	HRESULT hRes = S_FALSE ;
	IEnumWbemClassObject* pEnumerator = NULL ;
	IWbemClassObject* pInstance = NULL ;

	do{

		if (!m_pLoc || !m_pSvc)
		{
			break ;
		}

		CComBSTR  bstrQuery(L"SELECT * from  ") ;
		bstrQuery += T2OLE((LPTSTR)sSysInfoType.c_str( )) ;

		hRes = m_pSvc->ExecQuery(L"WQL",bstrQuery, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
		if(S_OK != hRes)
		{
			break;
		}
		pEnumerator->Reset() ;

		while(pEnumerator)
		{
			ULONG dwCount = 0 ;
			hRes = pEnumerator->Next(WBEM_INFINITE, 1, &pInstance, &dwCount) ;
			if(0 == dwCount)
			{
				break ;
			}

			LRESULT lRes ;
			switch(nTypeIndex)
			{
			case 0:
				lRes = GetOSInfoHandler(pInstance, sSysInfo) ;
				break ;
			case 1:
				lRes = GetBOISInfoHandler(pInstance, sSysInfo) ;
				break ;
			case 3:
				lRes = GetMemoryInfoHandler(pInstance, sSysInfo) ;
				break ;
			case 4:
				lRes = GetMotherBoardHandler(pInstance, sSysInfo) ;
				break ;
			case 7:
				lRes = GetNetworkAdapterInfoHandler(pInstance, sSysInfo) ;
				break ;
			case 8:
				lRes = GetDriverInfoHandler(pInstance, sSysInfo) ;
				break ;
			default:
				lRes = GetWmiInfoHandler(pInstance, sSysInfo) ;
				break ;
			}

			if(ERROR_SUCCESS != lRes)
			{
				break ;
			}
		}

	}while(FALSE) ;

	if (pInstance != NULL)
	{
		pInstance->Release() ;
		pInstance = NULL ;
	}

	if (pEnumerator != NULL)
	{
		pEnumerator->Release() ;
		pEnumerator = NULL ;
	}

	return sSysInfo ;
}

HRESULT CSystemInfo::GetOSInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	CComVariant varValue ;
	if(S_OK == pInstance->Get(L"Name", 0, &varValue, NULL, 0))
	{
		_tstring  sStr ;
		if( varValue.vt == VT_BSTR )
		{
			sStr = varValue.bstrVal ;
		}
		size_t nIndex = sStr.Find("|");
		if(nIndex>=0)
			sStr = sStr.Left(nIndex) ;
		sInfo.append(sStr.c_str()) ;
		sInfo.append(_T("\r\n")) ;
		varValue.Clear() ;
	}

	return ERROR_SUCCESS ;
}

HRESULT CSystemInfo::GetBOISInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	CComVariant varValue ;

	if(pInstance->Get(L"Manufacturer", 0, &varValue, NULL, 0) == S_OK)
	{
		_tstring sStr ;
		if(varValue.vt == VT_BSTR)
		{
			sStr = varValue.bstrVal ;
		}
		//size_t nIndex=szStr.Find("|");
		//if(nIndex>=0)
		//	szStr=szStr.Left(nIndex);
		sInfo.append(sStr.c_str());
		sInfo.append(_T("\r\n"));
		varValue.Clear();
	}

	if(S_OK == pInstance->Get(L"Name", 0, &varValue, NULL, 0))
	{
		_tstring sStr ;
		if(varValue.vt == VT_BSTR)
		{
			sStr = varValue.bstrVal ;
		}
		//size_t nIndex=szStr.Find("|");
		//if(nIndex>=0)
		//	szStr=szStr.Left(nIndex);
		sInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_BIOS")).c_str()) ;
		sInfo.append(sStr.c_str());
		sInfo.append(_T("\r\n"));
		varValue.Clear();
	}
	else
	{
		return E_FAIL;
	}	

	return ERROR_SUCCESS ;
}

HRESULT CSystemInfo::GetMemoryInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	MEMORYSTATUSEX status = {0} ;
	status.dwLength = sizeof(MEMORYSTATUSEX) ;
	if (GlobalMemoryStatusEx(&status))
	{
		DWORDLONG dwlMemory = status.ullTotalPhys/1024/1024 ;
		_tstring  sStr(dwlMemory) ;
		sStr.append(_T("MB")) ;

		//size_t nIndex=szStr.Find("|");
		//if(nIndex>=0)
		//	szStr=szStr.Left(nIndex);

		size_t nIndex = sInfo.Find(sStr.c_str()) ; 
		if(nIndex == _tstring::npos)
		{
			sInfo.append(sStr.c_str()) ;
			sInfo.append(_T("\r\n")) ;
		}
		return ERROR_SUCCESS ;
	}

	sInfo.append(_T("\r\n")) ;
	return ERROR_SUCCESS ;
}

HRESULT CSystemInfo::GetMotherBoardHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	CComVariant varValue1, varValue2 ;
	if(S_OK == pInstance->Get(L"Manufacturer", 0, &varValue1, NULL, 0))
	{
		if (S_OK == pInstance->Get(L"Product", 0, &varValue2, NULL, 0))
		{
			_tstring  szStr ;
			if(varValue1.vt == VT_BSTR && varValue2.vt == VT_BSTR)
			{
				szStr.assign(varValue1.bstrVal) ;
				szStr.append(_T(" ")) ;
				szStr.append(varValue2.bstrVal) ;
			}
			//size_t nIndex=szStr.Find("|");
			//if(nIndex>=0)
			//	szStr=szStr.Left(nIndex);

			size_t nIndex = sInfo.Find(szStr.c_str());
			if(nIndex == _tstring::npos)
			{
				sInfo.append(szStr.c_str()) ;
				sInfo.append(_T("\r\n"));
			}

			varValue1.Clear() ;
			varValue2.Clear() ;
			return ERROR_SUCCESS ;
		}
	}

	sInfo.append(_T("\r\n"));
	return E_FAIL ;
}

HRESULT CSystemInfo::GetNetworkAdapterInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	CComVariant varValue ;
	if( pInstance->Get(L"AdapterTypeID" , 0 , &varValue , NULL , 0 ) == S_OK )
	{
		int  uId=-1;
		if( varValue.vt == VT_I4)
		{
			uId = varValue.intVal;
		}
		if(0!=uId)
			return ERROR_SUCCESS;

	}
	if( pInstance->Get(L"Name" , 0 , &varValue , NULL , 0 ) == S_OK )
	{
		_tstring  szStr ;
		if( varValue.vt == VT_BSTR )
		{
			szStr = varValue.bstrVal;
		}

		size_t nIndex=sInfo.Find(szStr.c_str());
		if(nIndex == _tstring::npos)
		{
			sInfo.append(theUIString._GetStringFromId(_T("IDS_DIALOG_SYSINFO_NETWORK")).c_str());
			sInfo.append(szStr.c_str( ) );
			sInfo.append(_T("\r\n"));

		}
		varValue.Clear( );

		return ERROR_SUCCESS ;
	}
	else
	{
		return E_FAIL;
	}	
}

HRESULT CSystemInfo::GetDriverInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	CComVariant varValue ;
	if( pInstance->Get(L"DeviceName" , 0 , &varValue , NULL , 0 ) == S_OK )
	{
		_tstring  szStr ;
		if( varValue.vt == VT_BSTR )
		{
			szStr = varValue.bstrVal;
		}

		size_t nIndex=sInfo.Find(szStr.c_str());
		if(nIndex == _tstring::npos)
		{
			sInfo.append(_T("Hardware Driver:"));
			sInfo.append(szStr.c_str( ) );
			sInfo.append(_T("\r\n"));
		}

		varValue.Clear( );
		return ERROR_SUCCESS ;
	}
	else
	{
		return E_FAIL;
	}	
}

HRESULT CSystemInfo::GetWmiInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo)
{
	CComVariant varValue ;
	if(S_OK == pInstance->Get(L"Name", 0, &varValue, NULL, 0))
	{
		_tstring  szStr ;
		if(varValue.vt == VT_BSTR)
		{
			szStr = varValue.bstrVal ;
		}

		//size_t nIndex=szStr.Find("|");
		//if(nIndex>=0)
		//	szStr=szStr.Left(nIndex);

		size_t nIndex = sInfo.Find(szStr.c_str()) ;
		if(nIndex == _tstring::npos)
		{
			sInfo.append(szStr.c_str()) ;
			sInfo.append(_T("\r\n")) ;
		}

		varValue.Clear( );
		return ERROR_SUCCESS ;
	}
	else
	{
		sInfo.append(_T("\r\n"));
		return E_FAIL ;
	}
}

_tstring CSystemInfo::GetPCName()
{
	static _tstring szStaticDevName(_T(""));
	if (szStaticDevName.empty())
	{
		TCHAR  szBuf[MAX_PATH]= {0};
		DWORD  bufCharCount = MAX_PATH;

		// Get and display the name of the computer. 
		if( !GetComputerName(szBuf, &bufCharCount))
		{
			return szStaticDevName;
		}
		szStaticDevName = szBuf;
	}
	return szStaticDevName;
}