/*
 * corelib.h
 */
 
/*
 *  增加语言表标示 
 *  增加服务器列表
 *  
 */
 

#pragma once

#include <string>

struct LibLoginSrv
{
    std::string  coreHost;
    int corePort;
};

struct LibConfig
{
    LibLoginSrv *psrvList;
    int srvNums;    
    unsigned     logLevel;
    std::string language; //英语 en 法语 fr 德语 de
};

#define ERR_NOERR  0
#define ERR_UNDEFINED 1
#define ERR_NETFAIL_AND_EXIT 2
#define ERR_NETFAIL 3
#define ERR_NOTINIT 4
#define ERR_KICKED 5

// loginmgr code 
#define LOGIN_ERR_OK				0
#define LOGIN_ERR_BUSY				1
#define LOGIN_ERR_INNER				2
#define LOGIN_ERR_PWD				3
#define LOGIN_ERR_UPGRADE			4
#define LOGIN_ERR_TEMPBANNED		5
#define LOGIN_ERR_REGNBANNED		6
#define LOGIN_ERR_FLOOD				7
#define LOGIN_ERR_INACTIVE 			8
#define LOGIN_ERR_NONICKNAME 		9
#define LOGIN_ERR_ACCTBANNED		10
#define LOGIN_ERR_FB_LOGINERROR		11
#define LOGIN_ERR_FB_INNERERROR		12
#define LOGIN_ERR_COOKIE_INVALID	13
#define LOGIN_ERR_MID_INTERNAL_ERR	14
#define LOGIN_ERR_NEED_CHECK_PIN	15
#define LOGIN_ERR_MID_BLOCKED		16
#define LOGIN_ERR_PIN_INERNAL_ERR	17
#define LOGIN_ERR_PIN_BLOCKED		18
#define LOGIN_ERR_PIN_EXPIRED		19
#define LOGIN_ERR_PIN_INVALIDE		20
#define LOGIN_ERR_PIN_VERIFY_FAILED	21
#define LOGIN_ERR_NEWPIN_REQUEST_OK 22
#define LOGIN_ERR_NEWPIN_BLOKED		23
#define LOGIN_ERR_NEWPIN_SENDEMAIL_FAILED 24
#define LOGIN_ERR_NEWPIN_REQUEST_ERR	25



#define ERR_LOGIN_BASE 5

#define ERR_LOGIN_ERR_BUSY			(ERR_LOGIN_BASE + LOGIN_ERR_BUSY)
#define ERR_LOGIN_ERR_INNER			(ERR_LOGIN_BASE + LOGIN_ERR_INNER)
#define ERR_LOGIN_ERR_PWD			(ERR_LOGIN_BASE + LOGIN_ERR_PWD)
#define ERR_LOGIN_ERR_UPGRADE		(ERR_LOGIN_BASE + LOGIN_ERR_UPGRADE)
#define ERR_LOGIN_ERR_TEMPBANNED	(ERR_LOGIN_BASE + LOGIN_ERR_TEMPBANNED)
#define ERR_LOGIN_ERR_REGNBANNED	(ERR_LOGIN_BASE + LOGIN_ERR_REGNBANNED)
#define ERR_LOGIN_ERR_FLOOD			(ERR_LOGIN_BASE + LOGIN_ERR_FLOOD)
#define ERR_LOGIN_ERR_INACTIVE		(ERR_LOGIN_BASE + LOGIN_ERR_INACTIVE)
#define ERR_LOGIN_ERR_NONICKNAME	(ERR_LOGIN_BASE + LOGIN_ERR_NONICKNAME)
#define ERR_LOGIN_ERR_ACCTBANNED	(ERR_LOGIN_BASE + LOGIN_ERR_ACCTBANNED)
//facebook response error
#define ERR_LOGIN_ERR_FB_LOGINERROR	(ERR_LOGIN_BASE + LOGIN_ERR_FB_LOGINERROR)
//facebook login arc server is error
#define ERR_LOGIN_ERR_FB_INNERERROR	(ERR_LOGIN_BASE + LOGIN_ERR_FB_INNERERROR)
//reconnect cookie is invalid
#define ERR_LOGIN_ERR_COOKIE_INVALID (ERR_LOGIN_BASE + LOGIN_ERR_COOKIE_INVALID)

// mid verify err
#define ERR_LOGIN_ERR_MID_INTERNAL_ERR	(ERR_LOGIN_BASE + LOGIN_ERR_MID_INTERNAL_ERR)
#define ERR_LOGIN_ERR_NEED_CHECK_PIN	(ERR_LOGIN_BASE + LOGIN_ERR_NEED_CHECK_PIN)
#define ERR_LOGIN_ERR_MID_BLOCKED		(ERR_LOGIN_BASE + LOGIN_ERR_MID_BLOCKED)

// pin verify err
#define ERR_LOGIN_ERR_PIN_INERNAL_ERR		(ERR_LOGIN_BASE + LOGIN_ERR_PIN_INERNAL_ERR)
#define ERR_LOGIN_ERR_PIN_BLOCKED			(ERR_LOGIN_BASE + LOGIN_ERR_PIN_BLOCKED)
#define ERR_LOGIN_ERR_PIN_EXPIRED			(ERR_LOGIN_BASE + LOGIN_ERR_PIN_EXPIRED)
#define ERR_LOGIN_ERR_INVALIDE_PIN			(ERR_LOGIN_BASE + LOGIN_ERR_PIN_INVALIDE)
#define ERR_LOGIN_ERR_PIN_VERIFY_FAILED		(ERR_LOGIN_BASE + LOGIN_ERR_PIN_VERIFY_FAILED)

// new pin err
#define ERR_LOGIN_ERR_NEWPIN_REQUEST_OK			(ERR_LOGIN_BASE + LOGIN_ERR_NEWPIN_REQUEST_OK)
#define ERR_LOGIN_ERR_NEWPIN_BLOKED				(ERR_LOGIN_BASE + LOGIN_ERR_NEWPIN_BLOKED)
#define ERR_LOGIN_ERR_NEWPIN_SENDEMAIL_FAILED	(ERR_LOGIN_BASE + LOGIN_ERR_NEWPIN_SENDEMAIL_FAILED)
#define ERR_LOGIN_ERR_NEWPIN_REQUEST_ERR		(ERR_LOGIN_BASE + LOGIN_ERR_NEWPIN_REQUEST_ERR)
	   
enum UsERR_erType
{
	COMMONUSER,			//普通用户登陆
	FACEBOOKUSER,		//facebook用户登陆
	FACEBOOK_RECONNECT  //facebook重登陆用户
};

#define CLIENT_LIB_INVALID_ERROR_PARAM (-1)

struct ResponseData
{
	ResponseData()
		:devIDState(0),
		 errorCode(CLIENT_LIB_INVALID_ERROR_PARAM),
		 serverTime(-1)
	{
	}
	std::string		sessionID;	//use for login web as cookie
	std::string		nickname;	//display name
	std::string		userName;	//account
	std::string		password;	//use for facebook account bypass login cryptic games.
	std::string		email;		//user email
	std::string		photoUrl;	//facebook account photo URL
	int				devIDState; // 0 defender pass, 1 defender initialized, 2 defender disabled
	int				errorCode;	// when error occur, error code indicate error code.
	uint64			serverTime; // indicate server time
};

class LoginMsg{
public:
	/* 
	*  注意此处bool 改成了int
	*  result = 0表示登陆成功
	*  result = ERR_NETFAIL 表示登陆失败可能网络失败
	*  其他错误码待定
	*/
	virtual void onLoginResponse(int result, const ResponseData& data) = 0;
	/* 
	*  返回Token 如获取异常,可能会掉onException
	*  
	*/
	virtual void onTokenResponse(const std::string& token, const std::string& tokenType) = 0;
	/*
	*   通知消息
	*/
	virtual void onNotif(const std::string& title, const std::string& text, uint16 showCmd, uint16 target, uint16 serverCount, uint32 *pserverID) = 0;
	/*
	* 	 发生异常 时回调此接口
	*     连接断开如如网络异常:ERR_NetFail
	*     	
	*/
	virtual void onException(int ExceptionCode) = 0;

	/**
	 * brief 取token超时操作
	 * tokenType表示token的类型(各游戏的简写+IM token类型)
	 */
	virtual void onTokenTimeout(const std::string& tokenType) = 0;
};
/*
 * 返回 ERR_NOERR = 0 表示发送成功
 * 会在 onLoginResponse 中返回登陆结果
 * 返回 ERR_NetFail  表示网络失败
 * env用于客户端软硬件环境的收集
 * accountType 标示用户类型，详见USERTYPE
 * accountType=FACEBOOK时，pwd代表的是登陆fb成功后反馈的token
 */
int  CoreLib_LoginRequest(const std::string& account, const std::string& pwd, 
						  const std::string& env, const uint16 accountType, 
						  const std::string& devId, const std::string& devName);

/*
 *	返回 ERR_NOERR = 0 表成功
 */
int  CoreLib_Logout(const std::string& account);
/*
 *  para emailLogin 标示是否通过email登陆来获取token
 *  para tokenType  标示此token请求源的名字
 *	返回 ERR_NOERR = 0  表示发送成功
 *    会在 onTokenResponse 中返回登陆结果
 *    返回 ErrNetFai = 2 表示网络失败
 */
int  CoreLib_GetToken(const uint32 emailLogin, const std::string& tokenType="");
/*
 *	初始化
 */
bool initCoreLib(LoginMsg* loginMsg, LibConfig* config );
/*
 *	清理lib库
 */
void uninitCoreLib();
/*
 *
 */
void CoreLib_SetCallback(LoginMsg *loginMsg);

/*
 *\brief 客户端关键日志的记录
 *\ret 返回true表示发送成功，否则发送失败
 */
bool CoreLib_SendClientLog(const std::string& logData);

/*
 *\brief 客户端回到登陆界面或者退出
 *\ret 返回true表示发送成功，否则发送失败
 */
bool CoreLib_NotifyExit();

/*
 *\brief 向server验证Pin码
 *\ret 返回true表示发送成功，否则发送失败
 */
bool CoreLib_GuardVerify(const std::string& devId, const std::string& devName,const std::string& pin);

/*
 *\brief 向server发送new pin 的请求
 *\ret 返回true表示发送成功，否则发送失败
 */
bool CoreLib_SendNewPinRequest(const std::string& devId);

