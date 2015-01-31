/**
 * @file      IMManager.h
   @brief     The header file for managing the whole IM logic system.
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
#include "im/XmppClient.h"
#include "im/FriendManager.h"
#include "im/MsgLog.h"
#include "im/im_constant.h"

class CIMEventCallBack
{
public:
    virtual void HandleReConnect(bool bImmediately=false){};
    virtual void HandleRosterArrive(){};
	virtual void HandlePresence(_tstring sUserName, Presence::PresenceType presence){};
	virtual void HandleDivorce(_tstring tsName, _tstring tsNick){}
	virtual void HandleSendChatMsgDone(im_msg_send_msg_done msgDone){};
	virtual void HandleRcvChatMsg(MSG_LOG_ELEMENT msgEelement, bool bOfflineMessage){};
	virtual void HandleAvartarChanged(_tstring sUserName,_tstring szPath){};
};

class CIMManager
{
public:

public:
	CIMManager(void);
	~CIMManager(void);
public:
	bool InitIM(_IM_CONFIG conf);
	bool UnitIM();

	CXmppClientMgr&     GetXmppClient(){ return m_Client; };
	CFriendManager&  GetFriendManager(){return m_FriendMgr;};
	CMsgLog&         GetMsgLogMgr(){return m_MsgLogMgr;};
public:
	void RegisterIMCallback(CIMEventCallBack* pCallback);
    void UnRegisterIMCallback(CIMEventCallBack* pCallback);

public: // deliver CIMEventCallBack listener
	void HandleReConnect(bool bImmediately=false);
	void HandleRosterArrive();
	void HandlePresence(_tstring sUserName, Presence::PresenceType presence);
	void HandleDivorce(_tstring tsName, _tstring tsNick);
	void HandleSendChatMsgDone(im_msg_send_msg_done msgDone);
	void HandleRcvChatMsg(MSG_LOG_ELEMENT msgEelement, bool bOfflineMessage = false);
	void HandleAvartarChanged(_tstring sUserName,_tstring szPath);

public:
	void CheckOfflineMessage();

private:
	CXmppClientMgr m_Client;
	CFriendManager m_FriendMgr;
	CMsgLog m_MsgLogMgr;
    
	Mutex m_lock;
    std::map<CIMEventCallBack*,int> m_mapCallBacks;
};
//extern CIMManager g_theIMManager;