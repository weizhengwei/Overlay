#include "data/PluginData.h"

CPluginData g_thePluginData ;

CPluginData::CPluginData(void)
	:m_nStartType(0), m_nLang(-1)
{
	memset(m_sUserName, 0, sizeof(m_sUserName)) ;
	memset(m_sGameAbbr, 0, sizeof(m_sGameAbbr)) ;
}

CPluginData::~CPluginData(void)
{
}

CPluginData::CPluginData(const CPluginData& sPluginData)
{
	m_nStartType = sPluginData.m_nStartType ;
	memcpy(m_sUserName, sPluginData.m_sUserName, sizeof(m_sUserName)) ;
	memcpy(m_sGameAbbr, sPluginData.m_sGameAbbr, sizeof(m_sGameAbbr)) ;
	m_nLang = sPluginData.m_nLang ;

}

CPluginData& CPluginData::operator=(const CPluginData& sPluginData)
{
	m_nStartType = sPluginData.m_nStartType ;
	memcpy(m_sUserName, sPluginData.m_sUserName, sizeof(m_sUserName)) ;
	memcpy(m_sGameAbbr, sPluginData.m_sGameAbbr, sizeof(m_sGameAbbr)) ;
	m_nLang = sPluginData.m_nLang ;

	return *this ;
}

void CPluginData::SetStartType(int nStartType)
{
	m_nStartType = nStartType ;
}

int  CPluginData::GetStartType()
{
	return m_nStartType ;
}

void CPluginData::SetUserAccountName(TCHAR* pszUserName)
{
	if (pszUserName == NULL)
	{
		return ;
	}
	lstrcpyn(m_sUserName, pszUserName,64) ;
}

String::_tstring CPluginData::GetUserAccountName()
{
	return m_sUserName ;
}

void CPluginData::SetGameAbbr(TCHAR* pszGameAbbr)
{
	if (pszGameAbbr == NULL)
	{
		return ;
	}
	lstrcpyn(m_sGameAbbr, pszGameAbbr,16) ;
}

String::_tstring CPluginData::GetGameAbbr()
{
	return m_sGameAbbr ;
}

void CPluginData::SetLanguage(int nLang)
{
	m_nLang = nLang ;
}

int  CPluginData::GetLanguage()
{
	return m_nLang ;
}

void CPluginData::ResetData()
{
	m_nStartType = 0 ;
	memset(m_sUserName, 0, sizeof(m_sUserName)) ;
	memset(m_sGameAbbr, 0, sizeof(m_sGameAbbr)) ;
	m_nLang      = -1 ;
}