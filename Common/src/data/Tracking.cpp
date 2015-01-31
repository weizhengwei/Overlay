#include <time.h>
#include <tchar.h>
#include "data/Tracking.h"
#include "crypt/md5.h"

#define  ENV_TEST _T("ArcInternalTest")
#define	 TRACKING_CONNECTION_TIMEOUT	5

CTracking g_theTracking ;

bool CTracking::Init()
{
	/*
	** initialize web service
	*/
	//m_sWebSvr.Init() ; remove calling of Init here to avoid multiple initialization of curl

	m_bStop = false ;
	return Thread::start();
}

void CTracking::Uninit(int code)
{
	m_bStop = true;
	Thread::stop(code);

	//m_sWebSvr.Uninit() ; remove calling of Uninit here to avoid multiple uninitialization of curl
}

void CTracking::GenerateRandString()
{
	/*
	** get current time elapsed since midnight, January 1, 1970. 
	** and generate rand number.
	*/
	t = (int )time(NULL);
	r = rand() % 10000;
	char tcode[256];
	int tcodeLen = _snprintf(tcode, sizeof(tcode) - 1, "%d%d", t, r);
	tcode[sizeof(tcode) - 1] = 0;

	/*
	** compute md5 for (t+r)
	*/
	unsigned char sum[16];
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (const unsigned char*)tcode, tcodeLen);	
	MD5Final(sum, &ctx);

	/*
	** retrieve vcode from the md5 result
	*/
	vcode[32];
	_snprintf(vcode, sizeof(vcode) - 1, "%02x%02x", sum[14], sum[15]);

}
void CTracking::GenerateNewRandString()
{
	/*
	** get current time elapsed since midnight, January 1, 1970. 
	** and generate rand number.
	*/
	t = (int )time(NULL);
	r = rand() % 10000;
	char tcode[256];
	int tcodeLen = _snprintf(tcode, sizeof(tcode) - 1, "(C!f$X*7Kg6#Pbn%d%d", r, t);	//Secret key: (C!f$X*7Kg6#Pbn
	tcode[sizeof(tcode) - 1] = 0;

	/*
	** compute md5 for (s-key+r+t)
	*/
	unsigned char sum[16];
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (const unsigned char*)tcode, tcodeLen);	
	MD5Final(sum, &ctx);

	/*
	** retrieve vcode from the md5 result
	*/
	vcode[32];
	_snprintf(vcode, sizeof(vcode) - 1, "%02x%02x", sum[14], sum[15]);
}
bool CTracking::UserLogin(_tstring sUrl, _tstring sUsername, _tstring sLanguage, bool bSync)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp;
	sUrlTemp.Format(6, _T("%sregion=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_USER_LOGIN ;
		pInfo->sUrl  = sUrlTemp ;

		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{
		CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
		m_sWebSvr.GetFieldService(sUrlTemp, _T(""), _T(""), &sSvrRes) ;
		if (sSvrRes.sResult[0] != 'o' || sSvrRes.sResult[1] != 'k')
		{
			return false ;
		}
		_tstring sTrackingId(&sSvrRes.sResult[3]) ;
		m_sTrackingId = sTrackingId ;
	}

	return true ;
}

bool CTracking::UserLoginNew(_tstring sUrl, _tstring sUsername, _tstring sLanguage, bool bSync /* = true */)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateNewRandString();
	_tstring sUrlTemp;
	sUrlTemp.Format(4, _T("%st=%d&r=%d&v=%s"),sUrl.c_str(), t, r, _tstring(vcode).c_str()) ;

	sUsername.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.
	sLanguage.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.
	_tstring sPostData = _T("account=") + sUsername + _T("&region=") + sLanguage;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_USER_LOGIN_NEW ;
		pInfo->sUrl  = sUrlTemp ;
		pInfo->sPostData = sPostData;

		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{
		return m_sWebSvr.PostFieldService(sUrlTemp, _T(""), sPostData, NULL) ;
	}

	return true ;

}

bool CTracking::GameCCUNew(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, bool bSync /* = true */)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateNewRandString();
	_tstring sUrlTemp;
	sUrlTemp.Format(4, _T("%st=%d&r=%d&v=%s"),sUrl.c_str(), t, r, _tstring(vcode).c_str()) ;

	sUsername.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.
	sLanguage.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.
	_tstring sPostData = _T("account=") + sUsername + _T("&region=") + sLanguage + _T("&game=") + sGameAbbr;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_GAME_CCU_NEW ;
		pInfo->sUrl  = sUrlTemp ;
		pInfo->sPostData = sPostData;

		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{
		return m_sWebSvr.PostFieldService(sUrlTemp, _T(""), sPostData, NULL) ;
	}

	return true ;

}

bool CTracking::CounterTrackingNew(_tstring sUrl, _tstring sCounterName, _tstring sLanguage, bool bSync /* = true */)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateNewRandString();
	_tstring sUrlTemp;
	sUrlTemp.Format(4, _T("%st=%d&r=%d&v=%s"),sUrl.c_str(), t, r, _tstring(vcode).c_str()) ;

	sCounterName.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.
	sLanguage.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.
	_tstring sPostData = _T("name=") + sCounterName + _T("&region=") + sLanguage;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_COUNTER_NEW ;
		pInfo->sUrl  = sUrlTemp ;
		pInfo->sPostData = sPostData;

		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{
		return m_sWebSvr.PostFieldService(sUrlTemp, _T(""), sPostData, NULL) ;
	}

	return true ;
}

bool CTracking::UserLoginKeepLive(_tstring sUrl, _tstring sUsername, _tstring sLanguage, bool bSync)
{
	if (IsInternalTest())
	{
		return true;
	}
    if (m_sTrackingId.empty())
    {
        return false;//means UserLogin has not responsed at present.
    }
	GenerateRandString();
	_tstring sUrlTemp ;
	sUrlTemp.Format(7, _T("%sregion=%s&sid=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), m_sTrackingId.c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_KEEPLIVE_LOGIN ;
		pInfo->sUrl  = sUrlTemp ;
		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{//execute the service synchronize
		return m_sWebSvr.GetFieldService(sUrlTemp, _T(""), _T(""), NULL) ;
	}

	return true ;
}

bool CTracking::UseOverlayIngame(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	_tstring sGameAbbrTemp = sGameAbbr ;
	sUrlTemp.Format(7, _T("%sregion=%s&game=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sGameAbbrTemp.MakeLower().c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_USE_OVERLAY ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CTracking::GameDownloaded(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nDownloadTool)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	_tstring sGameAbbrTemp = sGameAbbr ;
	sUrlTemp.Format(8, _T("%sregion=%s&game=%s&account=%s&downloadtype=%d&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sGameAbbrTemp.MakeLower().c_str(), sUsername.c_str(), nDownloadTool,t, r, _tstring(vcode).c_str()) ;

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_DOWNLOAD_GAME ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CTracking::GameInstalled(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	_tstring sGameAbbrTemp = sGameAbbr ;
	sUrlTemp.Format(7, _T("%sregion=%s&game=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sGameAbbrTemp.MakeLower().c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_INSTALL_GAME ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CTracking::GameInstallError(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nErrorCode, int nDownloadType, _tstring sComment, bool bSync)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	_tstring sGameAbbrTemp = sGameAbbr ;
	sUrlTemp.Format(12, _T("%sregion=%s&game=%s&account=%s&errcode=%d&pid=%d&time=%s&downloadtype=%d&comment=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sGameAbbrTemp.MakeLower().c_str(), sUsername.c_str(), nErrorCode, GetCurrentProcessId(), _tstring(time(NULL)).c_str(), \
		nDownloadType, sComment.c_str(), t, r, _tstring(vcode).c_str()) ;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_INSTALL_GAME_ERROR ;
		pInfo->sUrl  = sUrlTemp ;

		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{
		return m_sWebSvr.GetFieldService(sUrlTemp, _T(""), _T(""), NULL) ;
	}

	return true ;
}

bool CTracking::GameLaunched(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr, int nDownloadTool)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	_tstring sGameAbbrTemp = sGameAbbr ;
	sUrlTemp.Format(8, _T("%sregion=%s&game=%s&account=%s&downloadtype=%d&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sGameAbbrTemp.MakeLower().c_str(), sUsername.c_str(), nDownloadTool, t, r, _tstring(vcode).c_str()) ;

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_LAUNCH_GAME ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CTracking::GameUninstalled(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sGameAbbr,int nDownloadTool, bool bSync)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	_tstring sGameAbbrTemp = sGameAbbr ;
	sUrlTemp.Format(8, _T("%sregion=%s&game=%s&account=%s&downloadtype=%d&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sGameAbbrTemp.MakeLower().c_str(), sUsername.c_str(), nDownloadTool,t, r, _tstring(vcode).c_str()) ;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_UNINSTALL_GAME ;
		pInfo->sUrl  = sUrlTemp ;
		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{
		CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
		m_sWebSvr.GetFieldService(sUrlTemp, _T(""), _T(""), &sSvrRes) ;
		if (sSvrRes.sResult[0] != 'o' || sSvrRes.sResult[1] != 'k')
		{
			return false ;
		}
	}

	return true ;
}

bool CTracking::NotificationsDisplayed(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sNotifyIdList, bool bSync)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	sUrlTemp.Format(6, _T("%sregion=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	if (!bSync)
	{
		PTRACK_INFO pInfo = new TRACK_INFO ;
		//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
		pInfo->nType = TRACK_NOTIFICATION_DISPLAY ;
		pInfo->sUrl  = sUrlTemp ;
		pInfo->sPostData = _T("notifid_list=") + sNotifyIdList ;
		pInfo->sPostData.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.

		m_wTaskQueue.enter(pInfo) ;
	}
	else
	{//execute the service synchronize
		_tstring sPostData = _T("notifid_list=") + sNotifyIdList ;
		return m_sWebSvr.PostFieldService(sUrlTemp, _T(""), sPostData, NULL) ;
	}

	return true ;
}

bool CTracking::NotificationClicked(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sNotifyId, _tstring sClickFrom)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	sUrlTemp.Format(8, _T("%snotifid=%s&clickfrom=%s&region=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sNotifyId.c_str(), sClickFrom.c_str(), sLanguage.c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_NOTIFICATION_CLICK ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CTracking::SystemInfoSurvey(_tstring sUrl, _tstring sUsername, _tstring sLanguage, _tstring sMacAddress, _tstring sSysInfo)
{	
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	sUrlTemp.Format(7, _T("%sregion=%s&cid=%s&account=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), \
		sLanguage.c_str(), sMacAddress.c_str(), sUsername.c_str(), t, r, _tstring(vcode).c_str()) ;

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_SYSTEM_INFO_SURVEY ;
	pInfo->sUrl  = sUrlTemp ;
	pInfo->sPostData = _T("data=") + sSysInfo ;
	pInfo->sPostData.Replace(_T("&"), _T("%26")) ;//ESC this is important to avoid truncation of string.

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CTracking::InstallUninstallSoftware(_tstring sUrl, _tstring sLang)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	sUrlTemp.Format(5, _T("%sregion=%s&t=%d&r=%d&v=%s"), sUrl.c_str(), sLang.c_str(), t, r, _tstring(vcode).c_str()) ;

	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	m_sWebSvr.GetFieldService(sUrlTemp, _T(""), _T(""), &sSvrRes) ;
	if (sSvrRes.sResult[0] != 'o' || sSvrRes.sResult[1] != 'k')
	{
		return false ;
	}

	return true ;
}

bool CTracking::UsageStatistics(_tstring sUrl, _tstring sUsername, _tstring sType, _tstring sSubType, UINT nValue)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	sUrlTemp.Format(8, _T("%saccount=%s&usagetype=%s&usagesubtype=%s&value=%u&t=%d&r=%d&v=%s"), \
		sUrl.c_str(), sUsername.c_str(), sType.c_str(), sSubType.c_str(), nValue,t, r, _tstring(vcode).c_str());

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = TRACK_USAGE_STATISTICS ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;
	return true;
}
 bool CTracking::FocusTimeStatistics(_tstring sUrl, _tstring sUsername,bool bEnd,int nMinSeconds)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	static DWORD nArcFocusBegin = 0;
	if (!bEnd)
	{
		nArcFocusBegin = ::GetTickCount();//start 
	}
	else if (nArcFocusBegin !=0)
	{
		DWORD dwSpentTime = (::GetTickCount()-nArcFocusBegin)/1000;
        nArcFocusBegin =0;
		if (dwSpentTime < nMinSeconds)
		{
           return true; //ignore it.
		}
        UsageStatistics(sUrl,sUsername,_T("clientusage"),_T("focus-time"),dwSpentTime);
	}
	else
	{//nArcFocusBegin==0 ,means no start.
		return false;
	}

	return true;
}

bool CTracking::GameCCUStatistics(_tstring sUrl,_tstring sUserName,_tstring slanguage,_tstring sGameAbbr,bool bBegin)
{
	if (IsInternalTest())
	{
		return true;
	}
	GenerateRandString();
	_tstring sUrlTemp ;
	int nType = TRACK_GAME_CCU_BEGIN;
	if (bBegin)
	{
		sUrlTemp.Format(7, _T("%sregion=%s&game=%s&account=%s&t=%d&r=%d&v=%s"), \
			sUrl.c_str(), slanguage.c_str(), sGameAbbr.MakeLower().c_str(), sUserName.c_str() ,t, r, _tstring(vcode).c_str());
	}
	else
	{
        _tstring sAbbr = sGameAbbr.MakeLower();
        if (m_mapSessionId.find(sAbbr) == m_mapSessionId.end())
        {
           return false;
        }
		nType=TRACK_GAME_CCU_UPDATE;
		_tstring sSessionId = m_mapSessionId[sAbbr];
		sUrlTemp.Format(7, _T("%ssid=%s&game=%s&account=%s&t=%d&r=%d&v=%s"), \
			sUrl.c_str(), sSessionId.c_str(), sAbbr.c_str(), sUserName.c_str() ,t, r, _tstring(vcode).c_str());
	}

	PTRACK_INFO pInfo = new TRACK_INFO ;
	//memset(pInfo, 0, sizeof(TRACK_INFO)) ;
	pInfo->nType = nType ;
	pInfo->sUrl  = sUrlTemp ;

	m_wTaskQueue.enter(pInfo) ;
	return true;
}
bool CTracking::PostPlayGameInfo(_tstring sUrl, _tstring sUsername, _tstring sGameAbbr, _tstring sGameName, _tstring sGameLanguage)
{
	int nType = POST_PLAYGAME_INFO;
	_tstring sUrlTemp;
	sUrlTemp.Format(5, _T("%saccount=%s&gameabbr=%s&gamename=%s&lang=%s"), sUrl.c_str(), sUsername.c_str(), sGameAbbr.c_str(), sGameName.c_str(), sGameLanguage.c_str());
	PTRACK_INFO pInfo = new TRACK_INFO;
	if (pInfo == NULL)
	{
		return false;
	}
	//memset(pInfo, 0, sizeof(TRACK_INFO));
	pInfo->nType = nType;
	pInfo->sUrl = sUrlTemp;

	m_wTaskQueue.enter(pInfo);

	return true;
}
long CTracking::run()
{
	while (!m_bStop)
	{
		TRACK_INFO* pInfo = m_wTaskQueue.leave();

		if (pInfo != NULL)
		{
			bool bRet = false ;
			CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
			switch(pInfo->nType)
			{
			case TRACK_NOTIFICATION_DISPLAY:
			case TRACK_SYSTEM_INFO_SURVEY:
			case TRACK_USER_LOGIN_NEW:
			case TRACK_GAME_CCU_NEW:
			case TRACK_COUNTER_NEW:
				bRet = m_sWebSvr.PostFieldService(pInfo->sUrl, _T(""), pInfo->sPostData, NULL, TRACKING_CONNECTION_TIMEOUT) ;
				break ;
			case POST_PLAYGAME_INFO:
				bRet = m_sWebSvr.GetFieldService(pInfo->sUrl, _T(""), pInfo->sPostData, &sSvrRes, TRACKING_CONNECTION_TIMEOUT);
				break;
			default:
				bRet = m_sWebSvr.GetFieldService(pInfo->sUrl, _T(""), _T(""), &sSvrRes, TRACKING_CONNECTION_TIMEOUT) ;
				if (sSvrRes.sResult[0] != 'o' || sSvrRes.sResult[1] != 'k')
				{
					bRet = false ;
				}
				break ;
			}
		
			if (bRet && (TRACK_USER_LOGIN == pInfo->nType))
			{
				_tstring sTrackingId(&sSvrRes.sResult[3]) ;
				m_sTrackingId = sTrackingId ;
			}
			else if (bRet && (TRACK_GAME_CCU_BEGIN == pInfo->nType))
			{
				_tstring sId(sSvrRes.sResult);
                _tstring::SplitList sList = sId.Split(_T(","));
				if (sList.empty())
				{
					bRet =false;
				}
                sList.pop_front();
				if (sList.empty())
				{
					bRet =false;
				}
                _tstring sSession = sList.front();

                sList.pop_front();
				if (sList.empty())
				{
					bRet =false;
				}
                _tstring sAbbr= sList.front();
                m_mapSessionId[sAbbr.MakeLower()]=sSession;
			}

			if (!bRet && (pInfo->nRetry < CTracking::RESEND_TIMES))
			{//re-queue the task if failed.
				++ pInfo->nRetry ;
				m_wTaskQueue.enter(pInfo) ;
			}
			else
			{
				delete pInfo ;
				pInfo = NULL ;
			}
		}
	}

	return 0;
}

_tstring CTracking::GetNotifyIdList()
{
	m_lockNotifyIdList.lock() ;
	_tstring sNotifyIdList = m_sNotifyIdList ;
	m_lockNotifyIdList.unlock() ;

	return sNotifyIdList ;
}

void CTracking::ClearNotifyIdList()
{
	m_lockNotifyIdList.lock() ;
	m_sNotifyIdList.clear() ;
	m_lockNotifyIdList.unlock() ;
}

void CTracking::PushNotifyIdList(_tstring sId, int nShowCmd, bool bGamePlaying)
{
	m_lockNotifyIdList.lock() ;

	if (!m_sNotifyIdList.empty())
	{
		m_sNotifyIdList.append(_T(";")) ;
	}

	if (nShowCmd == 1 || nShowCmd == 3)
	{
		m_sNotifyIdList.append(sId) ;
		m_sNotifyIdList.append(_T(",client")) ;
	}

	if ((nShowCmd == 2 || nShowCmd == 3) && bGamePlaying)
	{
		if (nShowCmd == 3)
		{
			m_sNotifyIdList.append(_T(";")) ;
		}
		m_sNotifyIdList.append(sId) ;
		m_sNotifyIdList.append(_T(",game")) ;
	}

	m_lockNotifyIdList.unlock() ;
}

bool CTracking::IsInternalTest()
{
	bool bRet = true;
	TCHAR cBuffer[256] ={0};
	DWORD dwRet = GetEnvironmentVariable(ENV_TEST, cBuffer, 256);
	if (!dwRet)
	{
		if (ERROR_ENVVAR_NOT_FOUND == GetLastError())
		{
			bRet = false;
		}
	}
    return bRet;
}