/**
 * @file      XmppClient.h
   @brief     The header file for xmpp client robert connecting.
   @author    xushenghui@pwrd.com
   @date      2012/08/29
   @version   01.00.00
 */

/* 
 *  Copyright(C) 2012. Perfect World Entertainment Inc
 *
 *  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
 *  its licensors and is protected by copyright and other intellectual 
 *  property laws and treaties.  We and our licensors own all rights, 
 *  title and interest in and to the SOURCE CODE, including all copyright 
 *  and other intellectual property rights in the SOURCE CODE.
 */

/*
 * $LastChangedBy: xushenghui@pwrd.com
 * $LastChangedDate: 
 * $Revision: 
 * $Id: 2
 * $notes:
 */

#pragma once 
#include <gloox/gloox.h>
#include <gloox/jid.h>
#include <gloox/client.h>
#include <gloox/connectionlistener.h>
#include <gloox/disco.h>
#include <gloox/eventhandler.h>
#include <gloox/message.h>
#include <gloox/messagehandler.h>
#include <gloox/messagesession.h>
#include <gloox/rostermanager.h>
#include <gloox/flexoffhandler.h>
#include <gloox/flexoff.h>

#include <atltime.h>

#include "XmppClientTaskType.h"
#include "FriendManager.h"
#include "thread/thread.h"
#include "thread/WQueue.h"

using namespace String;
using namespace gloox;

#define SAFE_DELETE(p) if(p) {delete (p); (p) = NULL;}

class CIMManager;

/********************************************************************************************
 *@ _IM_CONFIG Struct
 *@ used store the infomation of IM system, the object of this struct can only be 
 *@ used by UI thread and the recv thread.
********************************************************************************************/
struct _IM_CONFIG
{
	_tstring sUserName;
	_tstring sDomain;
	_tstring sServer;
	_tstring sResource;
	_tstring chatLogDir;//chat history saved directory.
	UINT nPort;
	DWORD dwLoginTimeOut;
	DWORD dwKeepAliveTimeout;
	_IM_CONFIG()
	{
		sUserName = _T("");
		sDomain = _T("");
		sServer = _T("");
		sResource = _T("");
		chatLogDir = _T("");
		nPort = 0;
		dwLoginTimeOut= 0;
		dwKeepAliveTimeout = 0;
	}
	_IM_CONFIG& operator =(_IM_CONFIG config)
	{
		sUserName = config.sUserName;
		sDomain = config.sDomain;
		sServer = config.sServer;
		sResource = config.sResource;
		chatLogDir = config.chatLogDir;
		nPort = config.nPort;
		dwLoginTimeOut= config.dwLoginTimeOut;
		dwKeepAliveTimeout = config.dwKeepAliveTimeout;
		return *this;
	}
};

/********************************************************************************************
 *@ OfflineMessageExtension Class
 *@ The Extension from the StanzaExtension in gloox-lib.
 *@ Just used for offline message.
********************************************************************************************/
class OfflineMessageExtension : public StanzaExtension
{
public:
    OfflineMessageExtension( const Tag* tag = 0 );
	virtual ~OfflineMessageExtension();

	// reimplemented from StanzaExtension
	virtual const std::string& filterString() const;

	// reimplemented from StanzaExtension
	virtual StanzaExtension* newInstance( const Tag* tag ) const
	{
		return new OfflineMessageExtension( tag );
	}

	// reimplemented from StanzaExtension
	virtual StanzaExtension* clone() const
	{
		return new OfflineMessageExtension( m_pTag );
	}

	// reimplemented from StanzaExtension
	virtual Tag* tag() const;

private:
	Tag *m_pTag;
	int m_context;
	StringList m_msgs;
};

/********************************************************************************************
 *@ CXmppClient Class
 *@ The class is a singleton, and it will be constructed by UI thread when starting 
 *@ IM system first time.The singleton will never be destroyed until the Arc-Client exit.
********************************************************************************************/
class CXmppClient : 
	public ConnectionListener,
	public MessageHandler,
	public FlexibleOfflineHandler, 
	public EventHandler,
	public Thread
{
//singleton use protected
protected:
	CXmppClient(CIMManager* pMgr);
    ~CXmppClient();

private:
	CXmppClient(const CXmppClient&);  
	const CXmppClient& operator=(const CXmppClient&); 

	void ResetXmppClient();

/*
 *@ functions used in Gloox-Recv thread:
 *@ the Gloox-Recv thread will start by run thread when start the IM system;
 *@ the Gloox-Recv thread will stop by run thread or it's self;
 *@ the Gloox-Recv do two things mainly: 
 *@		1) Recv message from gloox and store the data needed by the IM system;
 *@		2) Recv message from gloox and handle the message to the UI thread;
 */
public:
    //virtual event handlers
	virtual void onConnect(){}
	virtual void onDisconnect(ConnectionError e){}
	virtual bool onTLSConnect(const CertInfo& info){return true;};
	virtual void handleMessage(const Message& msg, MessageSession * /*session*/);
	virtual void handleEvent(const Event& event) ;

	virtual void handleFlexibleOfflineSupport( bool support );
    virtual void handleFlexibleOfflineMsgNum( int num );
    virtual void handleFlexibleOfflineMessageHeaders( const Disco::ItemList& headers );
    virtual void handleFlexibleOfflineResult( FlexibleOfflineResult foResult );

public:
	void HandleRosterArrive(); //as the flag that xmpp client has connected.

protected:
	ConnectionError ClientRecv(bool& bHasExcept, int nTimeOut = 20);
	static DWORD WINAPI RecvDataProc(void* param);

/*
 *@ functions used in run thread:
 *@ functions for checking self when loop;
 *@ functions for processing the task;
 *@ the run thread just do the following things:
 *@		1) check self for looping;
 *@		2) processing the tasks sent by UI thread and Recv thread;
 *@		3) handle reconnnect and handle presence;
 */
public:
	virtual long run();

protected:
	//functions for checking self when loop;
	bool KeepAlive();
	bool CheckLoginTimeOut();
	void WaitForRun();
	void ReRun(bool bImmediately=false);
	
	//functions for processing the task
	bool StartClient(_tstring tsToken);
	bool StopClient();
	bool ReconnectClient(bool bImmediately);
	bool DestroyClient();
	bool GetIMToken(bool bImmediately);
	bool SetClientPresence(int nPresence);
	bool SendXmppMsg(_tstring sUserName,_tstring sContent);
	bool RecvMessage();
	bool RecvOffLineMessage();
	void CheckOfflineSupport();

/*
 *@ functions used in UI thread,and called by the XmppClientMgr Object:
 *@ functions used to post message to run thread,and support some get-functions;
 */

public:
	static CXmppClient* GetInstance(CIMManager* pMgr, _IM_CONFIG& config);

public:
	void Unintialize();
	bool IMLogout();
	bool IMLogin(_tstring tsToken);
	bool IMReconnect(bool bImmediately);
	bool IsSupportOfflineMessage(){ return m_pOfflineClient != NULL; }
	bool SendChatMessage(_tstring sUserName,_tstring sContent);
	void CheckOfflineMessage();
	void SetPresence(ArcPresenceType iType);
	void SetLoginWithStatus(int status){ m_nLoginWithStatus = status; }
	int GetPresence(){ return m_nStatus; }
	int GetDisconnectPresence(){ return m_nDisconnectStatus; }

protected:
    
 
private:
//status can be used by run thread(not need lock):
	bool volatile m_bRunning;
	bool volatile m_bRosterArrive;
	bool volatile m_bExitRecvThread;
	bool volatile m_bOfflineMessageReturnFinished;
	int  volatile m_nOfflineMessageLeftNum;
	int  volatile m_nStatus;//used to record the user status(contain the offline status)
	int  volatile m_nDisconnectStatus;//used to record the status when disconnet from the server
	int  volatile m_nLoginWithStatus;//used to record the status used in roster-arrived to init the login presence
	HANDLE volatile m_hThread;

private:
//status can be used by run thread(need lock):
	bool  volatile m_bAlive;
	unsigned long  volatile m_nlastKeepaliveTime;
	unsigned long  volatile m_nlastLoginTime;

private:
//member variable just used in the run thread(not need lock):
	Client    *m_pClient;
	FlexibleOffline *m_pOfflineClient;
	OfflineMessageExtension *m_pOMExtension; //offline message extension (inherit from the StanzaExtension by user)

private:
//lock
	Mutex     m_thdLock;

private:
//just used in the tun thread
	bool m_bHasWaitForRun;

private:
//common-thread member variable used without lock;
	CIMManager* m_pMgr;
	_IM_CONFIG m_config;
	_tstringA  m_tsLoginToken;
	JID        m_jid;

private:
	typedef std::pair<std::string, LPMSG_LOG_ELEMENT>	TPairNodeMessage;
	typedef std::map<int, TPairNodeMessage>				TMapIndexMessage;
	typedef std::map<ATL::CTime, TMapIndexMessage>		TMapTimeMessage;
	typedef std::map<_tstring, TMapTimeMessage>			TMapUserMessage;

	typedef TMapIndexMessage::iterator					TMapIndexMessageItr;
	typedef TMapTimeMessage::iterator					TMapTimeMessageItr;
	typedef TMapUserMessage::iterator					TMapUserMessageItr;

	TMapUserMessage					m_mapUserMessage;
	WQueue<LPMSG_LOG_ELEMENT>		m_wChatMessageQueue;
	WQueue<LPXMPP_TASK_HEADER>		m_wOffllineMsgTaskQueue;
    WQueue<LPXMPP_TASK_HEADER>		m_wTaskQueue;
	WQueue<LPXMPP_TASK_HEADER>		m_wMsgTaskQueue;
	WQueue<LPXMPP_TASK_HEADER>		m_wSeniorTaskQueue;

private:
	static CXmppClient *s_pClient;
};

/********************************************************************************************
 *@ CXmppClientMgr Class
 *@ functions in this class can just be used in UI thread
********************************************************************************************/
class CXmppClientMgr
{
public:
	CXmppClientMgr(CIMManager* pMgr);
	~CXmppClientMgr();
public:
	void  Initialize(const _IM_CONFIG config);
	void  Unintialize();
   	bool  StartClient(const _tstring tsToken);
    void  StopClient();
	bool  SendChatMessage(_tstring sUserName,_tstring sContent);
	bool  IsSupportOfflineMessage();
	void  SetPresece(ArcPresenceType iType);
	ArcPresenceType GetPresence();
    void HandleRosterArrive(); 
	void CheckOfflineMessage();

public:
	void SetLoginWithStatus(int status);
	int GetDisconnectPresence();
private:
	CXmppClient*	m_pCurClient; 
    CIMManager*		m_pMgr;
    _IM_CONFIG		m_config;
};
