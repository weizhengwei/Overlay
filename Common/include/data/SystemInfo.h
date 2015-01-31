#pragma once

#include <wbemcli.h>
#include <vector>
#include "tstring/tstring.h"

using namespace String ;
using namespace std ;

#define	IS_KEY_LEN	256
#define IS_DISPLAY	_T("DisplayName")

class CSystemInfo
{
public:

	_tstring SYSINFO_TYPE[9] ;

	CSystemInfo() ;
	virtual ~CSystemInfo() ;

	bool Init(DWORD dwFlag) ;
	bool Uninit() ;

	_tstring GetOSInfo() ;
	_tstring GetBOISInfo() ;
	_tstring GetProcessorInfo() ;
	_tstring GetMemoryInfo() ;
	_tstring GetMotherboardInfo() ;
	_tstring GetSoundDevInfo() ;
	_tstring GetVideoAdapterInfo() ;
	_tstring GetNetworkAdapterInfo() ;
	_tstring GetDriverInfo() ;
	DWORD GetVideoMemory() ;

	static unsigned __int64 GetCPUFrequency() ;
	static _tstring GetMacAddress() ;
	static void GetDiskSpaceInfo(_tstring& sTotalSpace, _tstring& sFreeSpace) ;
	static vector<_tstring> GetInstalledSoftware() ;
	static _tstring GetOSDetailedInfo() ;
	static SYSTEM_INFO GetCPUInfo() ;
    static _tstring GetPCName();

protected:
	_tstring GetInfo(_tstring sSysInfoType, int nTypeIndex) ;
	HRESULT GetOSInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	HRESULT GetBOISInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	HRESULT GetMemoryInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	HRESULT GetMotherBoardHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	HRESULT GetNetworkAdapterInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	HRESULT GetDriverInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	HRESULT GetWmiInfoHandler(IWbemClassObject* pInstance, _tstring& sInfo) ;
	static unsigned __int64 GetCycleCount() ;

private:
	IWbemLocator*  m_pLoc ;
	IWbemServices* m_pSvc ;
	bool m_bCoInit ;
};