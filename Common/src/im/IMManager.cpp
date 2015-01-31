/**
 * @file      IMManager.cpp
   @brief     The implementation file for the whole IM system logic manager.
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

#include "im/IMManager.h"

//CIMManager g_theIMManager;
CIMManager::CIMManager(void):
m_Client(this),m_FriendMgr(this)
{
}

CIMManager::~CIMManager(void)
{

}

bool CIMManager::InitIM(_IM_CONFIG conf)
{
    m_Client.Initialize(conf);
	m_MsgLogMgr.Init(conf.chatLogDir.c_str(),conf.sUserName.c_str());
	return true;
}
bool CIMManager::UnitIM()
{
	m_lock.lock();
	m_mapCallBacks.clear();
	m_lock.unlock();

	m_Client.Unintialize();
	m_MsgLogMgr.UnInit();

	return true;
}

void CIMManager::RegisterIMCallback(CIMEventCallBack* pCallback)
{
	m_lock.lock();
	if (pCallback)
	{
		m_mapCallBacks.insert(std::pair<CIMEventCallBack*,int>(pCallback,1));
	}
	m_lock.unlock();

}

void CIMManager::UnRegisterIMCallback(CIMEventCallBack* pCallback)
{
	m_lock.lock();
	if (m_mapCallBacks.find(pCallback) != m_mapCallBacks.end())
	{
		m_mapCallBacks.erase(pCallback);
	}
	m_lock.unlock();
}

void CIMManager::HandleReConnect(bool bImmediately)
{
	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandleReConnect(bImmediately);
	}
	m_lock.unlock();
}

void CIMManager::HandleRosterArrive()
{
	m_Client.HandleRosterArrive();

	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandleRosterArrive();
	}
	m_lock.unlock();
}

void CIMManager::HandlePresence(_tstring sUserName, Presence::PresenceType presence)
{
	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandlePresence(sUserName,presence);
	}
	m_lock.unlock();
}

void CIMManager::HandleDivorce( _tstring tsName, _tstring tsNick )
{
	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandleDivorce(tsName, tsNick);
	}
	m_lock.unlock();
}

void CIMManager::HandleSendChatMsgDone(im_msg_send_msg_done msgDone)
{
	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandleSendChatMsgDone(msgDone);
	}
	m_lock.unlock();
}
void CIMManager::HandleRcvChatMsg(MSG_LOG_ELEMENT msgEelement, bool bOfflineMessage /*= false*/)
{
	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandleRcvChatMsg(msgEelement, bOfflineMessage);
	}
	m_lock.unlock();
}
void CIMManager::HandleAvartarChanged(_tstring sUserName,_tstring szPath)
{
    m_FriendMgr.UpdateAvartar(sUserName,szPath);
	m_lock.lock();
	std::map<CIMEventCallBack*,int>::iterator iter = m_mapCallBacks.begin();
	for (;iter != m_mapCallBacks.end();iter++)
	{
		iter->first->HandleAvartarChanged(sUserName,szPath);
	}
	m_lock.unlock();
}

void CIMManager::CheckOfflineMessage()
{
	m_Client.CheckOfflineMessage();
}
