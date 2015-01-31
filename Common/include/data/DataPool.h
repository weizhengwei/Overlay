#pragma once

#include <time.h>
#include <vector>
#include <map>
#include "tstring/tstring.h"
#include "datatype.h"

using namespace String;
using namespace std;

class CDataPool
{
public:
	CDataPool();
	~CDataPool();
	enum
	{
		TIMER_NID_SET_SPAMMING = 3000 ,
		TIMER_NID_LOGIN_TIMEOUT,
		TIMER_NID_CHECK_UPDATE,
		TIMER_NID_TRACK_UPDATE_SESSION,
		TIMER_NID_TRACK_NOTIFICATION_DISPLAY,
		TIMER_NID_LOGIN_RECONNECT,
		TIMER_NID_XMPP_ALIVE,
		TIMER_NID_SET_COOKIE,
		TIMER_NID_GET_WEBSERVICE_FIRSTTIME,
		TIMER_NID_GET_WEBSERVICE,
		TIMER_NID_PROGRESSBAR_DOWNLOAD_STATUS,
		TIMER_NID_XMPP_RECONNECT,
		TIMER_NID_UNINSTALL_GAME,
		TIMER_NID_START_TUTORIAL,
		TIMER_NID_MSG_BLINK,
		TIMER_NID_INSTALL_GAME,
		TIMER_NID_CC_LOGIN,
		TIMER_NID_GAME_UPDATE_CHECK,
		TIMER_NID_GAME_GET_BETAKEYFLAG,
		TIMER_NID_GAME_GET_BETAOFFLINEFLAG,
		TIMER_NID_GAME_SEARCH,
		TIMER_NID_GAME_GAMECLIENT_RUNNING,
		TIMER_NID_DETECT_UNINSTALL,
		TIMER_NID_MAINWINDOW_SHOW,
        TIMER_NID_EMS_AVAILABLE,
		TIMER_NID_EMS_TIMEOUT,
		TIMER_NID_DISPLAY_TIP,
		TIMER_NID_PLUGIN_START,
		TIMER_NID_CCU_NEW,
	};

private:
	_tstring            m_tsBaseDir;
	_tstring            m_sUserProfilePath ;
	_tstring            m_sIniConfigPath ;
	UINT                m_nLang;
	int                 m_nLangCount;
	_tstring            m_sCCVersion ;
	_tstring            m_sEulaVersion ;

	//xmpp data;
	__int64				m_nLocalTimeTick;//millisecond
	__int64				m_nTime;//second
	_tstring            m_tsUserInput; //support two type:account name
	_tstring            m_tsUserEmail; //move to login manager
	_tstring            m_tsUserName;  //move to login manager
	_tstring            m_sNickName ;  //move to login manager
	_tstringA           m_tsPassword;  //move to login manager
	_tstringA			m_tsPassword2; //move to login manager, in case m_tsPassword cannot be set correctly
	_tstring            m_tsXmppDomain;//move to im login manager
	int                 m_iLoginPort;  //move to login manager
	int                 m_iXmppPort;   //move to im login manager
	_tstring		    m_sLoginServer; //move to login manager
	_tstring		    m_sXmppServer;  //move to login manager
    _tstring            m_sAvatarUrl;  // the user's avatar url(may be from the third resource,such as facebook).
    _tstring            m_sfbToken; // the facebook auth token ,used to login client server.
	_tstring            m_sByPassToken; // the token is produced by our server,and used to bypass login cryptic game client.
	_tstringA           m_tsIMToken;   //move to im login manager
	_tstringA           m_tsGameToken ;//move to app manager
	_tstringA		    m_sLoginCookie; //used for standard user reconnecting.

	TCHAR               m_szPartner[128] ;      // pando system partner name, move to download mgr
	TCHAR               m_szPandoVersion[128] ; //pando required minimum version, move to download mgr

	BOOL                m_bFullMode;   //move to app
    int                 m_nLoginType;
    int                 m_nAccountType; //1,pwe standard account;2, facebook linked account;100,new account.
    int                 m_nGuardState;

	//User registered-from info
	_tstring			m_sRegisterGame;
	_tstring			m_sRegisterLang;
	_tstring			m_sRegisterType;
	_tstring			m_sOfferID;				//For hasoffers
	_tstring			m_sTransactionID;		//For hasoffers
	int					m_nHasOffersPostback;	//For hasoffers

	vector<_tstring>                m_vecDllList ; //move to app manager
	map<UINT, UINT_PTR>             m_mapLocalTimer ; /* save all of local timer id and corresponding interval*/												  
	vector<_tstring>                m_vecMd5FileList;
    map<_tstring,_tstringA>         m_mapGametoken;
public:

	bool Init() ;
	bool Uninit() ;

	// get/set base user data
	void     SetBaseDir(LPCTSTR lpszStr);		
	_tstring GetBaseDir();

	_tstring GetConfigPath() ;
	_tstring GetCommonPath() ;

	void     SetUserProfilePath(TCHAR* pszUserProfilePath) ;
	_tstring GetUserProfilePath() ;

	void     SetLangCount(int nLangCount){m_nLangCount = nLangCount;};
	int      GetLangCount(){return m_nLangCount;};

	void     SetLangId(int id, bool bFlush = false);
	int      GetLangId(bool bFile = false);

	_tstring GetLangAbbrString() ;
	_tstring GetLangString() ;

	//void     SetCCVersion(_tstring sVer) {m_sCCVersion = sVer ;} ;
	//_tstring GetCCVersion() {return m_sCCVersion ;} ;
	
	void     SetEulaVersion(_tstring sVer) ;
	_tstring GetEulaVersion() ;

	void	 SetServerTimer(__int64 nTime);
	__int64	 GetServerTimer();

	void     SetUserEmail(LPCTSTR lpszEmail);
	_tstring GetUserEmail();

	void     SetUserName(LPCTSTR lpszUser);		
	_tstring GetUserName( );

	void     SetUserInput(LPCTSTR lpszInput){m_tsUserInput =lpszInput;};		
	_tstring GetUserInput( ){return m_tsUserInput;};

	void     SetUserNick(_tstring sNickName){m_sNickName = sNickName ;};
	_tstring GetUserNick(){return m_sNickName.c_str() ;};

	void      SetPassword(LPCSTR lpszStr)		;
	_tstringA GetPassword( );		
	_tstringA GetCredential() ;

	void      SetXmppDomain(LPCTSTR lpszDomain );		
	_tstring  GetXmppDomain( );

	void      SetXmppServer( LPCTSTR lpszServer);
	_tstring  GetXmppServer( );

	void      SetXmppPort( int iPort);		
	int       GetXmppPort( );

	void      SetLoginServer( LPCTSTR lpszServer);
	_tstring  GetLoginServer( );

	void      SetLoginPort( int iPort);		
	int       GetLoginPort( );		
	
	void      SetLoginType(int nType){m_nLoginType = nType;};		
	bool      GetLoginType(){return m_nLoginType;};

	void      SetIMToken(LPCSTR lpszStr){m_tsIMToken = lpszStr;};
	_tstring  GetIMToken(){ return m_tsIMToken;};

	void      SetGameToken(_tstring sGameAbbr,LPCSTR lpszStr) ;
	_tstring  GetGameToken(_tstring sGameAbbr) ;

	void      SetUserAvatarUrl(_tstring sAvatarUrl) {m_sAvatarUrl = sAvatarUrl;};
	_tstring  GetUserAvatarUrl(){return m_sAvatarUrl;} ;

	void      SetFBToken(_tstring sFBToken) {m_sfbToken = sFBToken;};
	_tstring  GetFBToken(){return m_sfbToken;};

	void      SetLoginCookie(_tstringA sLoginCookie) {m_sLoginCookie = sLoginCookie;};
	_tstringA  GetLoginCookie(){return m_sLoginCookie;};

	void      SetByPassToken(_tstring sByPassToken) {m_sByPassToken = sByPassToken;};
	_tstring  GetByPassToken(){return m_sByPassToken;};

	void      SetAccountType( int nType){m_nAccountType=nType;};		
	int       GetAccountType(){return m_nAccountType;};	
    bool      IsNewAccount(){return m_nAccountType>100;};
    bool      IsStandardAccount(){ return (m_nAccountType == 1)||(m_nAccountType-100==1);};
    bool      IsFacebookAccount(){ return (m_nAccountType == 2)||(m_nAccountType-100==2);};
    void      SetGuardState(int nState){m_nGuardState = nState;};
    int       GetGuardState(){return m_nGuardState;};

	//getter/setter for User Registered-from info
	void		SetRegisterGame(_tstring sRegisterGame) { m_sRegisterGame = sRegisterGame;};
	void		SetRegisterLang(_tstring sRegisterLang) { m_sRegisterLang = sRegisterLang;};
	void		SetRegisterType(_tstring sRegisterType) { m_sRegisterType = sRegisterType;};
	void		SetOfferID(_tstring sOfferID) { m_sOfferID = sOfferID;};
	void		SetTransactionID(_tstring sTransactionID) { m_sTransactionID = sTransactionID;};
	void		SetHasOffersPostback(int nHasOffersPostback) { m_nHasOffersPostback = nHasOffersPostback;};
	_tstring	GetRegisterGame() {return m_sRegisterGame;};
	_tstring	GetRegisterLang() {return m_sRegisterLang;};
	_tstring	GetRegisterType() {return m_sRegisterType;};
	_tstring	GetOfferID() {return m_sOfferID;};
	_tstring	GetTransactionID() {return m_sTransactionID;};
	int			GetHasOffersPostback() {return m_nHasOffersPostback;};

	//Format register info for register page
	bool		FormatRegisterInfo(_tstring& sRegister);

	//others
	BOOL      LoadLocalTimerInfo(_tstring) ;
	UINT      GetTimerInterval(UINT_PTR) ;

	void      SetFullMode(BOOL bFullMode){ m_bFullMode =bFullMode;};
	BOOL      GetFullMode(){return m_bFullMode;};

	_tstring  GetPandoVersion() ;

	bool      ReadMD5FileList(LPCTSTR lpsCfgFilePath);
	bool      WriteMD5ListToLog(LPCTSTR szBaseDir,LPCTSTR lpsLogFileName);

    vector<_tstring> GetNotifyDllList() const { return m_vecDllList ; } ;

	_tstring GetLangAbbrById(int nId);
	int GetLangIdByAbbr(_tstring sLangAbbr);
};

extern CDataPool theDataPool ;
extern char      g_szKey[17] ;