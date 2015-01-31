#pragma once

#include <tchar.h>
#include <tstring/tstring.h>
using namespace String;

class CPluginData{

public:
	CPluginData(void) ;
	~CPluginData(void) ;

	CPluginData(const CPluginData& sPluginData) ;
	CPluginData& operator=(const CPluginData& sPluginData)  ;

	void SetStartType(int nStartType) ;
	int  GetStartType() ;

	void SetUserAccountName(TCHAR* pszUserName) ;
	_tstring GetUserAccountName() ;

	void SetGameAbbr(TCHAR* pszGameAbbr) ;
	_tstring GetGameAbbr() ;

	void SetLanguage(int nLang) ;
	int  GetLanguage() ;

	void ResetData() ;

private:
	int m_nStartType ;
	TCHAR m_sUserName[64] ;
	TCHAR m_sGameAbbr[16] ;
	int m_nLang ;
};

extern CPluginData g_thePluginData ;