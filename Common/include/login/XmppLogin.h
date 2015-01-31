#pragma once

#include <set>
#include "login/CoreLib.h"
#include "tstring/tstring.h"
#include "thread/thread.h"
#include "thread/WQueue.h"
#include "datatype.h"

#define GAME_DEFAULT_TOEKN _T("ArcGameSharedToken")
#define ARC_IM_TOKEN          "ArcIMToken"

using namespace String;
using namespace std;

class ILoginListener
{
public:
	virtual bool LOGIN_ResponseSuccess(WPARAM wParam, LPARAM lParam)	{ return true; };
	virtual bool LOGIN_ResponseFail(WPARAM wParam, LPARAM lParam)		{ return true; };
	virtual bool LOGIN_ReconnectRequest(WPARAM wParam, LPARAM lParam)	{ return true; };

	virtual bool LOGIN_IMTokenSuccess(WPARAM wParam, LPARAM lParam)		{ return true; };
	virtual bool LOGIN_IMTokenFail(WPARAM wParam, LPARAM lParam)		{ return true; };
	virtual bool LOGIN_GameTokenSuccess(WPARAM wParam, LPARAM lParam)	{ return true; };
	virtual bool LOGIN_GameTokenFail(WPARAM wParam, LPARAM lParam)		{ return true; };

	virtual bool LOGIN_AccountBeKicked(WPARAM wParam, LPARAM lParam)	{ return true; };
	virtual bool LOGIN_CloseWindow(WPARAM wParam, LPARAM lParam)		{ return true; };
	virtual bool LOGIN_EmergencyNotify(WPARAM wParam, LPARAM lParam)	{ return true; };
	virtual bool LOGIN_BetaAccessNotify(_tstring& sAbbr)				{ return true; };
	virtual bool LOGIN_EarlyBetaNotify(_tstring& sAbbr)					{ return true; };
	virtual bool LOGIN_BetaOfflineNotify(_tstring& sAbbr, _tstring& sFlag, _tstring& sMsg) { return true; };

	virtual bool LOGIN_GameinfoNotify(_tstring sData) { return true; };
	virtual bool LOGIN_AvatarUpdate()	              { return true; };
    virtual bool LOGIN_EMSArrive(const std::string sEmsData){ return true; };

	virtual bool LOGIN_ShoppingCartNotify(_tstring sData) { return true; };
	virtual bool LOGIN_PromotionCampaignNotify() { return true; };
	virtual bool LOGIN_InboxMessage(_tstring sData) { return true; };
	virtual bool LOGIN_FriendRequest(_tstring sData) { return true; };
	virtual bool LOGIN_FriendStatus(_tstring sData, int nType) { return true; };
	virtual bool LOGIN_Notification(_tstring sData, uint16 nShowCmd, uint16 nSvrCount, uint32* nSvrIds){ return true; };
	virtual bool LOGIN_WebDataUpdate(_tstring sType){ return true; }
	virtual bool LOGIN_AutoInstall(){ return true; }
    virtual bool LOGIN_DefenderREQFail(int nErr){ return true; }
};

class CLoginManager : public LoginMsg, public Thread
{
public:
	enum
	{
		INFO_GET_TOKEN = 0,
		INFO_LOGIN_RECONNECT,
        INFO_LOGIN_VERIFYPIN,
		INFO_LOGIN_REQPIN,
	};

	enum
	{
		LOGIN_USERNAME = 0,
		LOGIN_EMAIL,
		LOGIN_LAST,
	};

	enum
	{
		LOGIN_STANDARD = 1,
		LOGIN_FACEBOOK,
		LOGIN_NEWUSER = 100,
	};
	typedef struct _LOGIN_INFO
	{
		int	nType;
		int nRetryNum;
		_tstring sParam;
		_tstring sParam1;
		_LOGIN_INFO()
		{
           nType = nRetryNum =0;
           sParam = sParam1 = _T("");
		}
	}LOGIN_INFO;

	CLoginManager();
	~CLoginManager();

	virtual void onException(int ExceptionCode);
	virtual void onTokenResponse(const std::string& token, const std::string& tokenType);
	virtual void onLoginResponse(int result, const ResponseData& data);
	virtual void onNotif(const std::string& title, const std::string& text, uint16 showCmd, uint16 target, uint16 serverCount, uint32 *pserverID);
	virtual void onTokenTimeout(const std::string& tokenType);
	virtual long run();

	bool ClearAllListener();
	bool RegisterListener(ILoginListener* pListener);
	bool UnRegisterListener(ILoginListener* pListener);

	bool Init();
	void UnInit(int code = 0);
	void CoreLogout();
	bool StartLoginMgr();
	bool GetGameToken(_tstring sAbbr);
    bool GetIMToken();
	void SetConfig(LPCSTR lpszServer, unsigned int uPort, int iLevel);
	bool LoginReconnect(ILoginListener* pListener);
    void NotifyClientExit();
	void SetLoginInfo(LPCTSTR lpszUser)		{	m_sUser = lpszUser;		};
	void ResetInitFlag(bool bInit = false)	{	m_bInit = bInit;		};
    void ResetReconnectFlag(bool bIsReConnect = false)	{	m_bIsReconnect = bIsReConnect;};
	bool GetCoreLogged()                    {   return m_bCoreLogged;   };
    void VerifyPIN(_tstring szDevName,_tstring szPin);
    void RequestPIN();
    _tstring GetMacIdMd5();
	bool PostEmergentNotif(_tstring sData);
	bool PostEarlyBetaNofif(_tstring sData);
	bool PostBetaAccessNotif(_tstring sData);
	bool PostBetaOfflineNotif(_tstring sData);
	bool PostGameinfoNotif(_tstring sData);
	bool PostAvatarUpdate();
	bool PostEmsArrive(const std::string sEmsData);
	bool PostShoppingCartNotif(_tstring sData);
	bool PostPromotionCampaignNotif();
	bool PostInboxMessage(_tstring sData);
	bool PostFriendRequest(_tstring sData);
	bool PostFriendStatus(_tstring sData);
	bool PostNotification(_tstring sData, uint16 showCmd, uint16 serverCount, uint32 *pserverId);
	bool PostWebDataUpdate(_tstring sType);
	bool PostAutoInstall();
	EMERGENCY_MESSAGE_DATA* GetEmergencyMessage() { return &m_EmergencyMessage; };
private:
	bool		m_bInit; //flag for the login lib initialize
	bool		m_bStop;
	bool        m_bIMTokening;
	bool        m_bCoreLogged; //flag for the current login status.
    bool        m_bIsReconnect; //flag for reconnecting. it's reconnect after the first successful login.
	_tstring	m_sUser;
	LibConfig	m_tokenConf;
	Mutex		m_lockListener;
	EMERGENCY_MESSAGE_DATA		m_EmergencyMessage;
	WQueue<LOGIN_INFO*>			m_wTaskQueue;
	set<ILoginListener*>		m_setLoginListener;
};

extern CLoginManager g_theLoginMgr;