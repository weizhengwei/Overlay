#include <gloox/md5.h>
#include "data/DataPool.h"
#include "data/SNSManager.h"
#include "data/ThirdPartyAppManager.h"
#include "login/XmppLogin.h"
#include <tinyxml/tinystr.h>
#include <tinyxml/tinyxml.h>
#include "data/SystemInfo.h"
#include "version.h"

CLoginManager g_theLoginMgr;

CLoginManager::CLoginManager()
{
    m_bIsReconnect = false;
	m_bIMTokening=false;
	m_bCoreLogged = false;
	m_bInit = false;
	m_bStop = true;
	m_sUser = _T("");
	m_tokenConf.psrvList = NULL;
}

CLoginManager::~CLoginManager()
{
	if(m_tokenConf.psrvList)
	{
		delete m_tokenConf.psrvList;
		m_tokenConf.psrvList = NULL;
	}
}

bool CLoginManager::Init()
{
	m_bStop = false;
	return Thread::start();
}

void CLoginManager::UnInit(int code /* = 0 */)
{
	m_bStop = true;
	Thread::stop(code);
	ClearAllListener();
}

void CLoginManager::SetConfig(LPCSTR lpszServer, unsigned int uPort, int iLevel)
{
	if(m_tokenConf.psrvList)
	{
		delete m_tokenConf.psrvList;
		m_tokenConf.psrvList = NULL;
	}

	hostent* host_entry = gethostbyname(lpszServer) ;
	char sBuf [256] = {0} ;
	if (host_entry != NULL)
	{
		sprintf(sBuf , "%d.%d.%d.%d",
			( host_entry->h_addr_list [0][0]&0x00ff),
			( host_entry->h_addr_list [0][1]&0x00ff),
			( host_entry->h_addr_list [0][2]&0x00ff),
			( host_entry->h_addr_list [0][3]&0x00ff));
	}

	m_tokenConf.psrvList = new LibLoginSrv;
	m_tokenConf.psrvList->coreHost = sBuf;
	m_tokenConf.psrvList->corePort = uPort;
	m_tokenConf.srvNums = 1;
	m_tokenConf.language = theDataPool.GetLangAbbrString().toUTF8();
	m_tokenConf.logLevel = iLevel;
}

bool CLoginManager::ClearAllListener()
{
	m_lockListener.lock();
	m_setLoginListener.clear();
	m_lockListener.unlock();
	return true;
}

bool CLoginManager::RegisterListener(ILoginListener* pListener)
{
	m_lockListener.lock();
	if(pListener)
	{
		m_setLoginListener.insert(pListener);
	}
	m_lockListener.unlock();
	return true;
}

bool CLoginManager::UnRegisterListener(ILoginListener* pListener)
{
	m_lockListener.lock();
	if(m_setLoginListener.find(pListener) != m_setLoginListener.end())
	{
		m_setLoginListener.erase(pListener);
	}
	m_lockListener.unlock();
	return true;
}

void CLoginManager::onLoginResponse(int result, const ResponseData& data)
{
	if(result)
	{
		m_bCoreLogged = false;
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_ResponseFail(data.errorCode, result);
			++ iter;
		}
		m_lockListener.unlock();
	}
	else
	{  
		m_bCoreLogged = true;
		if (!m_bIsReconnect)
		{
            m_bIsReconnect =true;
			g_theSNSManager.SetSessionId(data.sessionID.c_str());
            
			_tstring sNick;
            sNick.fromUTF8(data.nickname.c_str());
			theDataPool.SetUserNick(sNick);

			_tstring sUser;
			sUser.fromUTF8(data.userName.c_str());
			theDataPool.SetUserName(sUser.c_str());

			theDataPool.SetUserEmail(_tstring(data.email).c_str());
			theDataPool.SetGuardState(data.devIDState);
			theDataPool.SetServerTimer(data.serverTime);
			//fix bug: bypass failed for PWE games when legacy user logged in with email address and the email address has not been changed before. 
			//reason: MD5(email+password) is not saved in AU database, we cannot get token based on email address.
			//temporary solution: use username to get token for bypass login for legacy user(new username starts with "#1#").
			theDataPool.SetLoginType((sUser.Find(_T("#1#"))!=_tstring::npos)?LOGIN_EMAIL:LOGIN_USERNAME) ;
            
			//to fix: http://mantis.perfectworld.com/view.php?id=154042
			if (theDataPool.GetLoginType() == LOGIN_USERNAME)
			{
				theDataPool.SetUserInput(sUser.c_str());
			}
			else
			{
				theDataPool.SetUserInput(_tstring(data.email).c_str());
			}

			if ( theDataPool.IsFacebookAccount())
			{
				theDataPool.SetUserInput(_tstring(data.email).c_str());
				theDataPool.SetByPassToken(data.password.c_str());
				theDataPool.SetUserAvatarUrl(data.photoUrl.c_str());
			}
			else
			{
				theDataPool.SetLoginCookie(data.password.c_str());
			}
		}
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_ResponseSuccess(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
}

void CLoginManager::onTokenResponse(const std::string& token, const std::string& tokenType)
{
	if (tokenType.compare(ARC_IM_TOKEN))
	{ //game token: the token type is game abbr.
		theDataPool.SetGameToken(tokenType,token.c_str());

		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_GameTokenSuccess(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
	else
	{  //IM token
		m_bIMTokening=false;

		char input_trunk[] = "PWRD7766CORE"; // the same as server side
		_tstringA tsToken = token.c_str();
		tsToken.append(input_trunk);

		gloox::MD5 md5;
		md5.feed(tsToken.c_str());
		md5.finalize();

		theDataPool.SetIMToken(md5.hex().c_str());

		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_IMTokenSuccess(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
}
void CLoginManager::onTokenTimeout(const std::string& tokenType)
{
	if (tokenType.compare(ARC_IM_TOKEN))
	{  //get game token failed.
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_GameTokenFail(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
	else
	{
       //get im token failed
		m_bIMTokening =false;
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_IMTokenFail(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
        
	}
}
void CLoginManager::onNotif(const std::string& title, const std::string& text, uint16 showCmd, uint16 target, uint16 serverCount, uint32 *pserverID)
{
 	if (!title.compare("Emergency Message"))
	{
		PostEmergentNotif(text.c_str());
	}
	if (!title.compare("Emergency Message New"))
	{
		PostEmsArrive(text);
	}
	else if (!title.compare("accessbeta"))
	{
		PostBetaAccessNotif(text.c_str());
	}
	else if (!title.compare("Early Access"))
	{
		PostEarlyBetaNofif(text.c_str());
	}
	else if (!title.compare("Beta Server Status"))
	{
		PostBetaOfflineNotif(text.c_str());
	}
	else if(!title.compare("New Game Info"))
	{
		PostGameinfoNotif(text.c_str());
	}
	else if(!title.compare("New Game Info 2"))
	{
		PostGameinfoNotif(text.c_str());
	}
	else if(!title.compare("Avatar Update"))
	{
		PostAvatarUpdate();
	}
	else if(!title.compare("Shopping Cart Update"))
	{
		PostShoppingCartNotif(text.c_str());
	}
	else if (!title.compare("Inbox Message"))
	{
		PostInboxMessage(text.c_str());
	}
	else if (!title.compare("Friend Request"))
	{
		PostFriendRequest(text.c_str());
	}
	else if (!title.compare("Friend Status"))
	{
		PostFriendStatus(text.c_str());
	}
	else if (!title.compare("Inbox Update"))
	{
		//update the inbox data
		PostWebDataUpdate(title.c_str());
	}
	else if (!title.compare("Request Update"))
	{
		//update the friend request data
		PostWebDataUpdate(title.c_str());
	}
	else if (!title.compare("Popup Message"))
	{
		PostPromotionCampaignNotif();
	}
	else if (!title.compare("autoinstall"))
	{
		PostAutoInstall();
	}
	else
	{
		//title is uselesss now, all of content has been put into text
		PostNotification(text.c_str(), showCmd, serverCount, pserverID);
	}
}

void CLoginManager::onException(int ExceptionCode)
{
	m_bCoreLogged = false;
	m_bIMTokening = false;
	bool bReconnect = false;

	CoreLogout();//should disconnect previous connection at beginning but not at the end.

	if(ExceptionCode == ERR_NETFAIL)
	{
		bReconnect = true;
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_ReconnectRequest(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
	else if(ExceptionCode == ERR_KICKED)
	{
        bReconnect = true; // avoid thread safety problem ,"LOGIN_AccountBeKicked" has implemented "LOGIN_CloseWindow"
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_AccountBeKicked(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
	
	if(!bReconnect)
	{
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_CloseWindow(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
}

void CLoginManager::NotifyClientExit()
{
	CoreLib_NotifyExit();
}

void CLoginManager::CoreLogout()
{
	m_bCoreLogged = false;
	m_bIMTokening = false;
	CoreLib_Logout(m_sUser.toUTF8());
}

bool CLoginManager::StartLoginMgr()
{
	int nRet = 0;

#if defined(_WINDOWS)
	WORD wVersion = MAKEWORD(2, 0);
	WSADATA WD;
	WSAStartup(wVersion, &WD); 
#endif

	if(!m_bInit)
	{
		if(!initCoreLib((LoginMsg*)this, &m_tokenConf))
		{
			m_lockListener.lock();
			set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
			while(iter != m_setLoginListener.end())
			{
				(*iter)->LOGIN_ReconnectRequest(0, 0);
				++ iter;
			}
			m_lockListener.unlock();
			return false;
		}
		m_bInit = true;
	}
	Sleep(500);

	CSystemInfo sSysString ;
	sSysString.Init(COINIT_MULTITHREADED) ;
	_tstring sEnv=_T("arcversion:")+ ARC_CLIENT_VERSION+_T(" os:") + sSysString.GetOSInfo();
	sSysString.Uninit();

	_tstring sMacIdMd5 = GetMacIdMd5();

	if (theDataPool.IsStandardAccount())
	{
         if (m_bIsReconnect && (!theDataPool.GetLoginCookie().empty()))
         {
            //standard user account reconnecting ,use login cookie.
            nRet = CoreLib_LoginRequest(m_sUser.toUTF8(), theDataPool.GetLoginCookie().c_str(),sEnv.toUTF8(),FACEBOOK_RECONNECT,sMacIdMd5.toUTF8(),CSystemInfo::GetPCName().toUTF8());
         }
		 else
		 {   //standard user account connecting ,use login password directly.
			 SetLoginInfo(theDataPool.GetUserInput().c_str());//ensure the account is compatible with re-connecting.
			 _tstringA szPassword = theDataPool.GetPassword();
			 nRet = CoreLib_LoginRequest(m_sUser.toUTF8(), szPassword,sEnv.toUTF8(),COMMONUSER,sMacIdMd5.toUTF8(),CSystemInfo::GetPCName().toUTF8());
			 szPassword.clear();
		 }
	}
	else if (theDataPool.IsFacebookAccount())
	{
       int nConnType = (m_bIsReconnect == true)? FACEBOOK_RECONNECT:FACEBOOKUSER;
       nRet = CoreLib_LoginRequest(theDataPool.GetUserName().toUTF8(), theDataPool.GetFBToken().toUTF8(),sEnv.toUTF8(),nConnType,sMacIdMd5.toUTF8(),CSystemInfo::GetPCName().toUTF8());
	}
	else
	{
		//for other type in future ;
	}
	if(nRet)
	{
		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_ReconnectRequest(0, 0);
			++ iter;
		}
		m_lockListener.unlock();
	}
	return nRet;
}

void CLoginManager::VerifyPIN(_tstring szDevName,_tstring szPin)
{
	LOGIN_INFO* pInfo = new LOGIN_INFO;
	//memset(pInfo, 0, sizeof(LOGIN_INFO));
	pInfo->nType = INFO_LOGIN_VERIFYPIN;
	pInfo->sParam = szDevName;
    pInfo->sParam1 = szPin;
	m_wTaskQueue.enter(pInfo);
}

 void CLoginManager::RequestPIN()
{
	LOGIN_INFO* pInfo = new LOGIN_INFO;
	//memset(pInfo, 0, sizeof(LOGIN_INFO));
	pInfo->nType = INFO_LOGIN_REQPIN;
	m_wTaskQueue.enter(pInfo);
}
 _tstring CLoginManager::GetMacIdMd5()
 {
	 gloox::MD5 md5;
	 _tstring sMacIdMd5(_T(""));
	 md5.feed(CSystemInfo::GetMacAddress().toUTF8().c_str( ));
	 md5.finalize( );
	 sMacIdMd5.fromUTF8(md5.hex().c_str());
     //sMacIdMd5 = _T("ffdfsssssfdfdsffjfftft");//just for test different devices.
	 return sMacIdMd5;
 }

bool CLoginManager::GetIMToken()
{
    if (m_bIMTokening)
    {
       return false;// the previous im token requirement is not response yet.
    }
    m_bIMTokening =true;

	LOGIN_INFO* pInfo = new LOGIN_INFO;
	//memset(pInfo, 0, sizeof(LOGIN_INFO));
	pInfo->nType = INFO_GET_TOKEN;
	pInfo->sParam = ARC_IM_TOKEN;
	m_wTaskQueue.enter(pInfo);
	return true;
}

bool CLoginManager::GetGameToken(_tstring sAbbr)
{
	static DWORD dwQueryToeknEscapeTime = 0;
    if ((::GetTickCount()-dwQueryToeknEscapeTime) < 1000)
    {
       return false;// the requirement is too frequent,ignore.
    }
    dwQueryToeknEscapeTime = ::GetTickCount();

	LOGIN_INFO* pInfo = new LOGIN_INFO;
	//memset(pInfo, 0, sizeof(LOGIN_INFO));
	pInfo->nType = INFO_GET_TOKEN;
	pInfo->sParam = sAbbr;
	m_wTaskQueue.enter(pInfo);
	return true;
}

bool CLoginManager::LoginReconnect(ILoginListener* pListener)
{
	m_bIMTokening =false;
	RegisterListener(pListener);
	LOGIN_INFO* pInfo = new LOGIN_INFO;
	//memset(pInfo, 0, sizeof(LOGIN_INFO));
	pInfo->nType = INFO_LOGIN_RECONNECT;
	m_wTaskQueue.enter(pInfo);
	return true;
}

long CLoginManager::run()
{
	while(!m_bStop)
	{
		LOGIN_INFO* pInfo = m_wTaskQueue.leave();
		if(pInfo)
		{
			switch(pInfo->nType)
			{
			case INFO_GET_TOKEN:
				{
					CoreLib_SetCallback(this);
					
					if(pInfo->sParam.compare(_tstring(ARC_IM_TOKEN)))
					{ //query game token.
						if(CoreLib_GetToken(theDataPool.GetLoginType(),pInfo->sParam.toUTF8()))
						{
							m_lockListener.lock();
							set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
							while(iter != m_setLoginListener.end())
							{
								(*iter)->LOGIN_GameTokenFail(0, 0);
								++ iter;
							}
							m_lockListener.unlock();
						}
					}
					else
					{  //query IM token.
						if(CoreLib_GetToken(LOGIN_USERNAME,pInfo->sParam.toUTF8()))
						{
							m_bIMTokening=false;
							m_lockListener.lock();
							set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
							while(iter != m_setLoginListener.end())
							{
								(*iter)->LOGIN_IMTokenFail(0, 0);
								++ iter;
							}
							m_lockListener.unlock();
							
						}
					}
					break;
				}
			case INFO_LOGIN_RECONNECT:
				{
					//SetLoginInfo(theDataPool.GetUserInput().c_str());
					SetConfig(theDataPool.GetLoginServer().toUTF8().c_str(), theDataPool.GetLoginPort(), 4);
					StartLoginMgr();
					break;
				}
			case INFO_LOGIN_VERIFYPIN:
				{
                    if (m_bCoreLogged)
                    {
                       break;// arc is login successfully now,useless.
                    }
					bool bRet = CoreLib_GuardVerify(GetMacIdMd5().toUTF8(),pInfo->sParam.toUTF8(),pInfo->sParam1.toUTF8());
                    if (!bRet)
                    {
                        Sleep(2000);//let loading bar UI is displayed completely to avoid disappearing fast.
						m_lockListener.lock();
						set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
						while(iter != m_setLoginListener.end())
						{
							(*iter)->LOGIN_DefenderREQFail(ERR_LOGIN_ERR_PIN_INERNAL_ERR);
							++ iter;
						}
						m_lockListener.unlock();
                    }
				}
				break;
			case INFO_LOGIN_REQPIN:
				{
					if (m_bCoreLogged)
					{
						break;// arc is login successfully now,useless.
					}
					bool bRet = CoreLib_SendNewPinRequest(GetMacIdMd5().toUTF8());
					if (!bRet)
					{
						m_lockListener.lock();
						set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
						while(iter != m_setLoginListener.end())
						{
							(*iter)->LOGIN_DefenderREQFail(ERR_LOGIN_ERR_NEWPIN_REQUEST_ERR);
							++ iter;
						}
						m_lockListener.unlock();
					}
				}
				break;
			default:
				break;
			}

			delete pInfo;
			pInfo = NULL;
		}
	}
	return 0;
}

bool CLoginManager::PostEmergentNotif(_tstring sData)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		sData.Replace(_T("["), _T("<")) ; //replace [ with <, ] with > to restore xml format
		sData.Replace(_T("]"), _T(">")) ; //since xmlrpc used in core server does not support xml content

		xmlDoc.Parse(sData.toNarrowString().c_str()) ;

		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(!pRoot)
		{
			break ;
		}

		if (pRoot->FirstChildElement("id") && pRoot->FirstChildElement("for") && pRoot->FirstChildElement("product") && pRoot->FirstChildElement("type") && pRoot->FirstChildElement("msg"))
		{
			//ZeroMemory(&m_EmergencyMessage,sizeof(EMERGENCY_MESSAGE_DATA));
            m_EmergencyMessage.Reset();
			m_EmergencyMessage._szId.fromUTF8(pRoot->FirstChildElement("id")->GetText());
			m_EmergencyMessage._szMode.fromUTF8(pRoot->FirstChildElement("for")->GetText());
			m_EmergencyMessage._szProduct.fromUTF8(pRoot->FirstChildElement("product")->GetText());
			m_EmergencyMessage._szType.fromUTF8(pRoot->FirstChildElement("type")->GetText());
			m_EmergencyMessage._szText.fromUTF8(pRoot->FirstChildElement("msg")->GetText());

			if (pRoot->FirstChildElement("updated"))
			{
				m_EmergencyMessage._szUpdateTimeStamp.fromUTF8(pRoot->FirstChildElement("updated")->GetText());
			}
		}
		else
		{
			break;
		}

		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_EmergencyNotify(0, 0);
			++ iter;
		}
		m_lockListener.unlock();

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CLoginManager::PostBetaAccessNotif(_tstring sData)
{
	_tstring sAbbr;
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		sData.Replace(_T("["), _T("<")) ; //replace [ with <, ] with > to restore xml format
		sData.Replace(_T("]"), _T(">")) ; //since xmlrpc used in core server does not support xml content

		xmlDoc.Parse(sData.toNarrowString().c_str()) ;

		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(!pRoot)
		{
			break ;
		}

		if (pRoot->FirstChildElement("game"))
		{
			sAbbr.fromUTF8(pRoot->FirstChildElement("game")->GetText());
		}
		else
		{
			break;
		}

		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_BetaAccessNotify(sAbbr);
			++ iter;
		}
		m_lockListener.unlock();

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CLoginManager::PostBetaOfflineNotif(_tstring sData)
{
	_tstring sAbbr, sFlag, sMsg;
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		sData.Replace(_T("["), _T("<")) ; //replace [ with <, ] with > to restore xml format
		sData.Replace(_T("]"), _T(">")) ; //since xmlrpc used in core server does not support xml content

		xmlDoc.Parse(sData.toNarrowString().c_str()) ;

		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(!pRoot)
		{
			break ;
		}

		if (pRoot->FirstChildElement("status") && pRoot->FirstChildElement("game"))
		{
			sFlag.fromUTF8(pRoot->FirstChildElement("status")->GetText());
			sAbbr.fromUTF8(pRoot->FirstChildElement("game")->GetText());

			if (pRoot->FirstChildElement("msg"))
			{
				sMsg.fromUTF8(pRoot->FirstChildElement("msg")->GetText());
			}
		}
		else
		{
			break;
		}

		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_BetaOfflineNotify(sAbbr, sFlag, sMsg);
			++ iter;
		}
		m_lockListener.unlock();

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CLoginManager::PostEarlyBetaNofif(_tstring sData)
{
	_tstring sAbbr;
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		sData.Replace(_T("["), _T("<")) ; //replace [ with <, ] with > to restore xml format
		sData.Replace(_T("]"), _T(">")) ; //since xmlrpc used in core server does not support xml content

		xmlDoc.Parse(sData.toNarrowString().c_str()) ;

		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(!pRoot)
		{
			break ;
		}

		if (pRoot->FirstChildElement("game"))
		{
			sAbbr.fromUTF8(pRoot->FirstChildElement("game")->GetText());
		}
		else
		{
			break;
		}

		m_lockListener.lock();
		set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
		while(iter != m_setLoginListener.end())
		{
			(*iter)->LOGIN_EarlyBetaNotify(sAbbr);
			++ iter;
		}
		m_lockListener.unlock();

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CLoginManager::PostGameinfoNotif(_tstring sData)
{
	sData.Replace(_T("["), _T("<")) ; //replace [ with <, ] with > to restore xml format
	sData.Replace(_T("]"), _T(">")) ; //since xmlrpc used in core server does not support xml content

	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while(iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_GameinfoNotify(sData);
		++ iter;
	}
	m_lockListener.unlock();

	return true;
}

bool CLoginManager::PostAvatarUpdate()
{
	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while(iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_AvatarUpdate();
		++ iter;
	}
	m_lockListener.unlock();

	return true;
}

bool CLoginManager::PostEmsArrive(const std::string sEmsData)
{
    _tstringA sData(sEmsData);
	sData.Replace('[', '<') ; //replace [ with <, ] with > to restore xml format
	sData.Replace(']', '>') ; //since xmlrpc used in core server does not support xml content
    
	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while(iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_EMSArrive(sData.c_str());
		++ iter;
	}
	m_lockListener.unlock();

	return true;
}

bool CLoginManager::PostShoppingCartNotif(_tstring sData)
{
	sData.Replace(_T("["), _T("<")) ; //replace [ with <, ] with > to restore xml format
	sData.Replace(_T("]"), _T(">")) ; //since xmlrpc used in core server does not support xml content

	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while(iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_ShoppingCartNotify(sData);
		++ iter;
	}
	m_lockListener.unlock();
	
	return true;
}

bool CLoginManager::PostPromotionCampaignNotif()
{
	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while(iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_PromotionCampaignNotify();
		++ iter;
	}
	m_lockListener.unlock();

	return true;
}

bool CLoginManager::PostInboxMessage(_tstring sData)
{
	sData.Replace(_T("["), _T("<"));
	sData.Replace(_T("]"), _T(">"));

	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while (iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_InboxMessage(sData);
		++iter;
	}
	m_lockListener.unlock();

	return true;
}
bool CLoginManager::PostFriendRequest(_tstring sData)
{
	sData.Replace(_T("["), _T("<"));
	sData.Replace(_T("]"), _T(">"));

	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while (iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_FriendRequest(sData);
		++iter;
	}
	m_lockListener.unlock();

	return true;

}

bool CLoginManager::PostFriendStatus(_tstring sData)
{
	sData.Replace(_T("["), _T("<"));
	sData.Replace(_T("]"), _T(">"));

	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while (iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_FriendStatus(sData, 1);
		++iter;
	}
	m_lockListener.unlock();

	return true;
}

bool CLoginManager::PostNotification(_tstring sData, uint16 showCmd, uint16 serverCount, uint32 *pserverId)
{
	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while (iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_Notification(sData, showCmd, serverCount, pserverId);
		++iter;
	}
	m_lockListener.unlock();

	return true;
}

bool CLoginManager::PostWebDataUpdate(_tstring sType)
{
	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while (iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_WebDataUpdate(sType);
		++iter;
	}
	m_lockListener.unlock();

	return true;

}
bool CLoginManager::PostAutoInstall()
{
	
	m_lockListener.lock();
	set<ILoginListener*>::iterator iter = m_setLoginListener.begin();
	while (iter != m_setLoginListener.end())
	{
		(*iter)->LOGIN_AutoInstall();
		++iter;
	}
	m_lockListener.unlock();

	return true;

}