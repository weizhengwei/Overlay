/**
 * @file      UIChatDlg.cpp
   @brief     The implementation file for xmpp client.
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

#include "stdafx.h"
#include "im/XmppClient.h"
#include "im/IMManager.h"
#include "data/DataPool.h"
#include "login/XmppLogin.h"
#include "im/im_constant.h"

#define OFFLINE_MESSAGE_EXTENSION_TYPE	100
#define COMPANY_NAME		_T("Perfect World")

// ---- OfflineExtension begin ----
OfflineMessageExtension::OfflineMessageExtension(const Tag* tag)
	: StanzaExtension( OFFLINE_MESSAGE_EXTENSION_TYPE ),m_pTag(NULL)
{
	if (tag)
	{
		m_pTag = tag->clone();
	}
}

OfflineMessageExtension::~OfflineMessageExtension()
{
	SAFE_DELETE(m_pTag);
}

const std::string& OfflineMessageExtension::filterString() const
{
	static const std::string filter = "/message/offline[@xmlns='" + XMLNS_OFFLINE + "']";
	return filter;
}

Tag* OfflineMessageExtension::tag() const
{
	return m_pTag;
}
// ---- OfflineExtension end ----
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
// ---- CXmppClient begin ----

/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
// base functions begin;

CXmppClient::CXmppClient(CIMManager* pMgr)
:m_pMgr(pMgr)
{
	m_nDisconnectStatus = (int)ArcPresenceType::Available;
	m_nLoginWithStatus = (int)ArcPresenceType::Available;
	ResetXmppClient();
}
CXmppClient::~CXmppClient(){}

void CXmppClient::ResetXmppClient()
{
	m_pOfflineClient = NULL;
	m_pClient = NULL;
	m_pOMExtension = NULL;
	m_hThread = NULL;

	m_bHasWaitForRun = false;
	m_bRosterArrive = false;
	m_bExitRecvThread = false;
	m_nStatus = (int)ArcPresenceType::Unavailable;

	m_bOfflineMessageReturnFinished = false;
	m_nOfflineMessageLeftNum = 0;
	m_bAlive = false;
	m_nlastKeepaliveTime = m_nlastLoginTime = ::GetTickCount();

	TMapUserMessageItr userItr = m_mapUserMessage.begin();
	for (; userItr != m_mapUserMessage.end(); ++userItr)
	{
		TMapTimeMessageItr timeItr = userItr->second.begin();
		for (; timeItr != userItr->second.end(); ++timeItr)
		{
			TMapIndexMessageItr indexItr = timeItr->second.begin();
			for (; indexItr != timeItr->second.end(); ++indexItr)
			{
				SAFE_DELETE(indexItr->second.second);
			}
		}
	}
	m_mapUserMessage.clear();

	while(m_wChatMessageQueue.size() > 0) {
		LPMSG_LOG_ELEMENT lpElement = m_wChatMessageQueue.leave();
		SAFE_DELETE(lpElement);
	}

	while(m_wOffllineMsgTaskQueue.size() > 0) {
		LPXMPP_TASK_HEADER lpTask = m_wOffllineMsgTaskQueue.leave();
		SAFE_DELETE(lpTask);
	}

	while(m_wTaskQueue.size() > 0) {
		LPXMPP_TASK_HEADER lpTask = m_wTaskQueue.leave();
		SAFE_DELETE(lpTask);
	}

	while(m_wMsgTaskQueue.size() > 0) {
		LPXMPP_TASK_HEADER lpTask = m_wMsgTaskQueue.leave();
		SAFE_DELETE(lpTask);
	}
}

// base functions end
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
// Gloox-Recv thread functions begin;

void CXmppClient::handleMessage(const Message& msg, MessageSession * session)
{
	switch(msg.subtype())
	{
	case Message::Chat:
		{
			_tstring tsDate;
			_tstring tsTime;
			ATL::CTime tmNow;

			Tag *pTag = msg.tag();
			if (!pTag) return;

			bool bOfflineMessage = false;
			const ConstTagList& match = pTag->findTagList(m_pOMExtension->filterString());

			std::string strNode;
			int nIndex = 0;
			if (match.size() > 0)
			{
				Tag* pItemTag = (*match.begin())->findChild("item");
				if (!pItemTag) return;
				strNode = pItemTag->findAttribute("node");
				if (strNode.size() < 20) return;
				std::string strYear = strNode.substr(0, 4);
				std::string strMonth = strNode.substr(5, 2);
				std::string strDay = strNode.substr(8, 2);
				std::string strHour = strNode.substr(11, 2);
				std::string strMin = strNode.substr(14, 2);
				std::string strSec = strNode.substr(17, 2);
				std::string strMillisec = strNode.substr(20, 3);
				
				SYSTEMTIME sm = {0};
				sm.wYear = atoi(strYear.c_str());
				sm.wMonth = atoi(strMonth.c_str());
				sm.wDay = atoi(strDay.c_str());
				sm.wHour = atoi(strHour.c_str());
				sm.wMinute = atoi(strMin.c_str());
				sm.wSecond = atoi(strSec.c_str());
				nIndex = atoi(strMillisec.c_str());

				FILETIME ft = {0};
				if (SystemTimeToFileTime(&sm, &ft))
				{
					//offline message is using UTC time;
					//the CTime constructor of CTime( const FILETIME& ) is using UTC file time:
					CTime offlineTime(ft);
					tmNow = offlineTime;
				}
				else
				{
					tmNow = CTime(::_time64(NULL));
				}

				bOfflineMessage = true;
			}
			else
			{
				tmNow = theDataPool.GetServerTimer(); 
			}

			tsTime = tmNow.Format(_T("%H:%M:%S")); 
			tsDate.Format(3, _T("%d:%d,%d"), tmNow.GetMonth(), tmNow.GetDay(), tmNow.GetYear());

			_tstring sUserName = msg.from().bare();
			sUserName=sUserName.substr(0,sUserName.Find('@'));

			_tstring sUserNick = theDataPool.GetUserNick().empty() ? theDataPool.GetUserName().c_str(): theDataPool.GetUserNick().c_str( );

			_tstring sContent;
			sContent.fromUTF8(msg.body().c_str());

			CHAT_FRIEND ChatFreind;
			if (!m_pMgr->GetFriendManager().GetFriendByName(sUserName,ChatFreind))
				return;

			LPMSG_LOG_ELEMENT lpMsgLog = new MSG_LOG_ELEMENT;
			lstrcpyn(lpMsgLog->_szUserName,		sUserName.c_str(),				MAX_PATH-1);
			lstrcpyn(lpMsgLog->_szSenderNick,	ChatFreind.szNick,	MAX_PATH-1);
			lstrcpyn(lpMsgLog->_szRecvNick,		sUserNick.c_str(),				MAX_PATH-1);
			lstrcpyn(lpMsgLog->_szContent,		sContent.c_str(),				MAX_PATH-1);
			lstrcpyn(lpMsgLog->_szTime,			tsTime.c_str(),					MAX_PATH-1);
			lstrcpyn(lpMsgLog->_szDate,			tsDate.c_str(),					MAX_PATH-1);

			if (bOfflineMessage)
			{
				//error msg, should never be here
				if (m_nOfflineMessageLeftNum <= 0)
				{
					SAFE_DELETE(lpMsgLog);
					return;
				}

				//save the message to the message-map(key:ctime, value:LPMSG_LOG_ELEMENT)
				m_mapUserMessage[sUserName][tmNow][nIndex] = TPairNodeMessage(strNode, lpMsgLog);

				//send the off-line message and the saved chat-message by time if the current message is the last one
				if (m_nOfflineMessageLeftNum == 1)
				{
					m_bOfflineMessageReturnFinished = true;
					LPXMPP_TASK_HEADER pHeader = new XMPP_TASK_RECV_OFFLINE_MESSAGE;
					m_wOffllineMsgTaskQueue.enter(pHeader);
				}

				//del the num
				--m_nOfflineMessageLeftNum;
			}
			else
			{
				m_wChatMessageQueue.enter(lpMsgLog);
				LPXMPP_TASK_HEADER pHeader = new XMPP_TASK_RECV_MESSAGE;
				m_wMsgTaskQueue.enter(pHeader);
			}

			break;
		}
	default:
		break;
	}
}

void CXmppClient::handleEvent(const Event& event)
{
	MutexGuard m(&m_thdLock); 
	m_bAlive = true;
}

void CXmppClient::handleFlexibleOfflineSupport( bool support )
{
	if (support && m_pOfflineClient)
	{
		try
		{
			m_pOfflineClient->getMsgCount();
		}
		catch(...)
		{
			m_bOfflineMessageReturnFinished = true;
			SAFE_DELETE(m_pOfflineClient);
		}
	}
	else
	{
		m_bOfflineMessageReturnFinished = true;
		SAFE_DELETE(m_pOfflineClient);
	}
}

void CXmppClient::handleFlexibleOfflineMsgNum( int num )
{
	if (num > 0 && m_pOfflineClient)
	{
		try
		{
			m_nOfflineMessageLeftNum = num;
			m_pOfflineClient->fetchHeaders();
		}
		catch(...)
		{
			m_bOfflineMessageReturnFinished = true;
		}
	}
	else
	{
		m_bOfflineMessageReturnFinished = true;
	}
}

void CXmppClient::handleFlexibleOfflineMessageHeaders( const Disco::ItemList& headers )
{
	if (headers.size() > 0 && m_pOfflineClient)
	{
		try
		{
			StringList list;
			m_pOfflineClient->fetchMessages(list);
		}
		catch(...)
		{
			m_bOfflineMessageReturnFinished = true;
		}
	}
	else
	{
		m_bOfflineMessageReturnFinished = true;
	}
}

void CXmppClient::handleFlexibleOfflineResult( FlexibleOfflineResult foResult )
{
	if (!m_pOfflineClient) return;

	if (foResult == FomrRequestSuccess) 
	{
	} 
	else if (foResult == FomrRemoveSuccess) 
	{
	} 
	else 
	{
		//error
	}
}

void CXmppClient::HandleRosterArrive()
{
	MutexGuard m(&m_thdLock); 
	m_bAlive = true;
	m_bRosterArrive = true;
	SetPresence((ArcPresenceType)m_nLoginWithStatus);
}

ConnectionError CXmppClient::ClientRecv( bool& bHasExcept, int nTimeOut /*= 20*/ )
{
	bHasExcept = false;
	ConnectionError ce = ConnNoError;
	__try
	{
		ce = m_pClient->recv(nTimeOut); 
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bHasExcept = true;
	}

	return ce;
}

DWORD WINAPI CXmppClient::RecvDataProc(void* param)
{
	do 
	{
		CXmppClient* pThis = (CXmppClient*)param;
		if (!pThis) break; 

		ConnectionError ce = ConnNoError;
		while(ce == ConnNoError)
		{
			if (pThis->m_bExitRecvThread)
			{//the thread killed by set m_bStopInRecvThread/m_bReconnectInRecvThread value;
				break;
			}

			if (!pThis->m_pClient)
			{
				break;
			}

			bool bHasExcept = false;
			ce = pThis->ClientRecv(bHasExcept, 20);
			if (bHasExcept)
			{
				break;
			}
		} 
		
		pThis->m_bRosterArrive = false;
		
		if (!pThis->m_bExitRecvThread)
		{//NOT killed by run thread(user self), the killer may be the server or the local-network;
			LPXMPP_TASK_RECONNECT lpTask = new _XMPP_TASK_RECONNECT();
			lpTask->_bImmediately = false;
			pThis->m_wSeniorTaskQueue.enter((LPXMPP_TASK_HEADER&)lpTask);
			pThis->m_nDisconnectStatus = pThis->m_nStatus;
		}
		else
		{
			pThis->m_nDisconnectStatus = ArcPresenceType::Unavailable;
		}

		pThis->m_hThread = NULL;
	} while (false);

	return 0;
}

// Gloox-Recv thread functions end
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
// run thread begin

/*
 *@ main loop
 *@ this function just return at the end, not allowed return at any other place.
 */
long CXmppClient::run()
{
	LPXMPP_TASK_HEADER pTask = NULL;

	while (1)
	{
		/*
		 *@ check the connect of the xmpp client;
		 */
		if (KeepAlive() && CheckLoginTimeOut()) {
			//WaitForRun();
		}else {
			ReRun(false);
		}

		/*
		 *@ get the next process task;
		 */
		if (m_wSeniorTaskQueue.size() > 0) {
			pTask = m_wSeniorTaskQueue.leave();
		}else if (m_wTaskQueue.size() > 0) {
			pTask = m_wTaskQueue.leave();
		}else if (m_wOffllineMsgTaskQueue.size() > 0){
			pTask = m_wOffllineMsgTaskQueue.leave();
		}else if (m_wMsgTaskQueue.size() > 0 && m_bOfflineMessageReturnFinished) {
			pTask = m_wMsgTaskQueue.leave();
		}else {
			pTask = new XMPP_TASK_WAIT_FOR_RUN();
		}

		if (pTask == NULL) continue;

		/*
		 *@ process the task;
		 */
		switch(pTask->_iType)
		{
		case XMPP_TT_UNINSTALL:
			{
				StopClient();
				delete this;
			}
			break;
		case XMPP_TT_LOGIN:
			StartClient(((LPXMPP_TASK_IM_LOGIN)pTask)->tsToken);
			break;
		case XMPP_TT_LOGOUT:
			StopClient();
			break;
		case XMPP_TT_RECONNECT:
			ReconnectClient(((LPXMPP_TASK_RECONNECT)pTask)->_bImmediately);
			break;
		case XMPP_TT_WAIT_FOR_RUN:
			Sleep(100);
			break;
		case XMPP_TT_CHECK_OFFLINE:
			CheckOfflineSupport();
			break;
		case XMPP_TT_SET_PRESENCE:
			SetClientPresence(((LPXMPP_TASK_SET_PRESENCE)pTask)->_iPresence);
			break;
		case XMPP_TT_SEND_MESSAGE:
			SendXmppMsg(((LPXMPP_TASK_SEND_MESSAGE)pTask)->_szUserName,((LPXMPP_TASK_SEND_MESSAGE)pTask)->_szContent);
			break;
		case XMPP_TT_RECV_MESSAGE:
			RecvMessage();
			break;
		case XMPP_TT_RECV_OFFLINE_MESSAGE:
			RecvOffLineMessage();
			break;
		default:
			break;
		}

		SAFE_DELETE(pTask);
	}//while loop end;

	m_bRunning = false;
	return 0;
}

bool CXmppClient::KeepAlive()
{
	MutexGuard m(&m_thdLock); 
	if (!m_bRosterArrive)
	{
		return true;
	}
	if ((::GetTickCount()- m_nlastKeepaliveTime) >= m_config.dwKeepAliveTimeout)
	{
		if (!m_bAlive)
		{
			return false;
		}
		m_bAlive = false;
		try
		{
			m_pClient->xmppPing(m_jid,this);
		}
		catch(...)
		{
			return false;
		}
		m_nlastKeepaliveTime = ::GetTickCount();
	}
	return true;
}

void CXmppClient::WaitForRun()
{
	if (!m_bHasWaitForRun)
	{
		LPXMPP_TASK_HEADER pHeader = new XMPP_TASK_WAIT_FOR_RUN();
		m_wTaskQueue.enter(pHeader);
		m_bHasWaitForRun = true;
	}
}

void CXmppClient::ReRun(bool bImmediately)
{
	LPXMPP_TASK_RECONNECT pTask = new XMPP_TASK_RECONNECT();
	pTask->_bImmediately = bImmediately;
	m_wSeniorTaskQueue.enter((LPXMPP_TASK_HEADER&)pTask);
}

bool CXmppClient::CheckLoginTimeOut()
{
	MutexGuard m(&m_thdLock); 
	if (m_hThread == NULL)
	{//not start login, not need to check;
		return true;
	}
	if (m_bRosterArrive)
	{
		return true;
	}
	if ((::GetTickCount() - m_nlastLoginTime) >= m_config.dwLoginTimeOut)
	{
		return m_bRosterArrive;
	}
	return true;
}

bool CXmppClient::StartClient(_tstring tsToken)
{
	if (m_bRosterArrive) return true;
	if (!m_pMgr) return false; 
	m_tsLoginToken = tsToken;

	try
	{
		/*
		 *@ clear m_pOfflineClient data
		 */
		if (m_pOfflineClient){
			m_pOfflineClient->removeFlexibleOfflineHandler();
		}

		/*
		 *@ clear client data
		 */
		if (m_pClient){
			m_pClient->disconnect();
			m_pClient->rosterManager()->removeRosterListener();
			m_pClient->disableRoster();
			m_pClient->removeMessageHandler(this) ;
			m_pClient->removeConnectionListener(this);
			m_pClient->removeStanzaExtension(OFFLINE_MESSAGE_EXTENSION_TYPE);
		}

		/*
		 *@ release obj
		 */
		SAFE_DELETE(m_pOfflineClient);
		SAFE_DELETE(m_pClient);

		/*
		 *@ reset client member variable
		 */
		ResetXmppClient();

		/*
		 *@ init client
		 */
		m_pClient= new Client(m_jid,m_tsLoginToken.toUTF8().c_str() ,m_config.nPort);
		if (!m_pClient) return false; 
		m_pClient->bindResource(_tstring(COMPANY_NAME).toNarrowString().c_str());
		m_pClient->setServer(m_config.sServer.toNarrowString().c_str());
		m_pClient->disco()->setVersion(_tstring(PRODUCT_NAME).toNarrowString().c_str(), gloox_version());
		m_pClient->disco()->setIdentity("client",_tstring(COMPANY_NAME).toNarrowString().c_str());
		m_pOMExtension = new OfflineMessageExtension;
		m_pClient->registerStanzaExtension(m_pOMExtension);
		m_pClient->registerConnectionListener(this);
		m_pClient->rosterManager()->registerRosterListener(&(m_pMgr->GetFriendManager()));
		m_pClient->registerMessageHandler(this) ;
		m_pClient->disco()->addFeature(XMLNS_CHAT_STATES);
		m_pClient->disco()->addFeature(XMLNS_SI);
		m_pClient->disco()->addFeature(XMLNS_BYTESTREAMS);
		m_pClient->disco()->addFeature(XMLNS_FT_FASTMODE);
		m_pClient->disco()->addFeature(XMLNS_OFFLINE);

		/*
		 *@ init offline client
		 */
		m_pOfflineClient = new FlexibleOffline(m_pClient);
		if (m_pOfflineClient) m_pOfflineClient->registerFlexibleOfflineHandler(this);

		/*
		 *@ begin socket
		 */
		bool bRet = false;
		if(m_pClient->connect(false))
		{
			m_hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RecvDataProc,this,0,NULL);
			if (m_hThread != NULL) bRet = true;
		}

		if (!bRet)
		{//error
			SAFE_DELETE(m_pOfflineClient);
			SAFE_DELETE(m_pClient);
			m_pOMExtension = NULL;
		}
		return bRet;

	}
	catch (...){
		SAFE_DELETE(m_pOfflineClient);
		SAFE_DELETE(m_pClient);
		m_pOMExtension = NULL;
		return false;
	}

	return false;
}

bool CXmppClient::StopClient()
{
	if (m_hThread != NULL)
	{
		m_bExitRecvThread = true;
		WaitForSingleObject(m_hThread, INFINITE);
	}

	SetClientPresence((int)ArcPresenceType::Unavailable);
	DestroyClient();
	ResetXmppClient();

	return true;
}

bool CXmppClient::ReconnectClient(bool bImmediately)
{
	StopClient();
	GetIMToken(bImmediately);

	return true;
}

bool CXmppClient::DestroyClient()
{
	m_bExitRecvThread = false;
	m_bRosterArrive = false;

	try
	{
		if (m_pOfflineClient)
		{
			m_pOfflineClient->removeFlexibleOfflineHandler();
		}

		if (m_pClient)
		{
			m_pClient->disconnect();
			m_pClient->rosterManager()->removeRosterListener();
			m_pClient->disableRoster();
			m_pClient->removeMessageHandler(this);
			m_pClient->removeConnectionListener(this);
			m_pClient->removeStanzaExtension(OFFLINE_MESSAGE_EXTENSION_TYPE);
		}

		SAFE_DELETE(m_pOfflineClient);
		SAFE_DELETE(m_pClient);
	}
	catch (...)
	{
		
	}

	return true;
}

bool CXmppClient::GetIMToken( bool bImmediately )
{
	m_pMgr->HandleReConnect(bImmediately);
	return true;
}

bool CXmppClient::SetClientPresence( int nPresence )
{
	m_nStatus = nPresence;
	m_pMgr->HandlePresence(m_config.sUserName, (Presence::PresenceType)nPresence);
	try
	{
		if(m_pClient)
		{
			m_pClient->setPresence((Presence::PresenceType)nPresence,1,EmptyString );
			return true;
		}
	}
	catch (...)
	{
		return false;
	}

	return false;
}

bool CXmppClient::SendXmppMsg(_tstring sUserName,_tstring sContent)
{
	if (!m_bRosterArrive)
		return false;

	try
	{
		if(!m_pClient)
		{
			return false ;
		}

		CHAT_FRIEND chatFriend;
		if (!m_pMgr || !m_pMgr->GetFriendManager().GetFriendByName(sUserName,chatFriend))
		{
			return false;
		}

		MessageSession* pSession = new MessageSession( m_pClient, JID(_tstring(chatFriend.szJid).toUTF8().c_str()).full( )) ;
		if(pSession==NULL)
		{
			return false ;
		}
		pSession->send(sContent.toUTF8().c_str()) ;
		m_pClient->disposeMessageSession(pSession) ;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool CXmppClient::RecvMessage()
{
	if (!m_bRosterArrive)
		return false;

	if (m_wChatMessageQueue.size() <= 0)
		return false;

	LPMSG_LOG_ELEMENT lpElement = m_wChatMessageQueue.leave();
	if (lpElement != NULL)
	{
		m_pMgr->HandleRcvChatMsg(*lpElement, false);
		m_pMgr->GetMsgLogMgr().AppendLog(
			lpElement->_szUserName,
			lpElement->_szRecvNick,
			_tstring(lpElement->_szSenderNick).toUTF8().c_str(),
			_tstring(lpElement->_szTime).toNarrowString().c_str(),
			_tstring(lpElement->_szDate).toNarrowString().c_str(),
			_tstring(lpElement->_szContent).c_str()
			);
	}

	return true;
}

bool CXmppClient::RecvOffLineMessage()
{
	if (!m_bRosterArrive)
		return false;

	int nRecvNumMax = 20;
	int nRecvNum = 0;

	TMapUserMessageItr userItr = m_mapUserMessage.begin();
	if (userItr == m_mapUserMessage.end())
		return false;

	TMapTimeMessageItr timeItr = userItr->second.begin();
	while (timeItr != userItr->second.end())
	{
		TMapIndexMessageItr indexItr = timeItr->second.begin();
		while (indexItr != timeItr->second.end())
		{
			/*
			 *@ verify the max num of the off-line messages that will be show at one time
			 */
			if (nRecvNum++ > nRecvNumMax)
				goto go_end;

			/*
			 *@ get the node message
			 */
			TPairNodeMessage& nodeMsg = indexItr->second;

			/*
			 *@ process the message
			 */
			if (nodeMsg.second != NULL)
			{
				m_pMgr->HandleRcvChatMsg(*(nodeMsg.second), true);
				m_pMgr->GetMsgLogMgr().AppendLog(
					nodeMsg.second->_szUserName,
					nodeMsg.second->_szRecvNick,
					_tstring(nodeMsg.second->_szSenderNick).toUTF8().c_str(),
					_tstring(nodeMsg.second->_szTime).toNarrowString().c_str(),
					_tstring(nodeMsg.second->_szDate).toNarrowString().c_str(),
					_tstring(nodeMsg.second->_szContent).c_str()
					);

				if (m_pOfflineClient)
				{
					try
					{
						StringList nodeList;
						nodeList.push_back(nodeMsg.first);
						m_pOfflineClient->removeMessages(nodeList);
					}
					catch(...)
					{

					}
				}
				SAFE_DELETE(nodeMsg.second);
			}
			
			/*
			 *@ erase the index message from the save-map of off-line messages
			 */
			timeItr->second.erase(indexItr);
			
			/*
			 *@ erase the time message if current user's time-message is empty
			 */
			if(timeItr->second.size() <= 0)
			{
				userItr->second.erase(timeItr);
				break;
			}
			
			/*
			 *@ set the next index message
			 */
			indexItr = timeItr->second.begin();
		}
		
		/*
		 *@ erase the user message if current user's message is empty
		 */
		if (userItr->second.size() <= 0)
		{
			m_mapUserMessage.erase(userItr);
			break;
		}

		/*
		 *@ set the next index message
		 */
		timeItr = userItr->second.begin();
	}

go_end:
	if (m_mapUserMessage.size() > 0)
	{
		Sleep(100);
		LPXMPP_TASK_HEADER pHeader = new XMPP_TASK_RECV_OFFLINE_MESSAGE;
		m_wOffllineMsgTaskQueue.enter(pHeader);
	}

	return true;
}

void CXmppClient::CheckOfflineSupport()
{
	if (!m_bRosterArrive)
		return;

	if (m_bRunning && m_pOfflineClient)
	{
		try
		{
			m_pOfflineClient->checkSupport();
		}
		catch (...)
		{
			m_bOfflineMessageReturnFinished = true;
		}
	}
	else
	{
		m_bOfflineMessageReturnFinished = true;
	}
}


// run thread end
/********************************************************************************************/
/********************************************************************************************/
/********************************************************************************************/
//UI Thread functions begin

CXmppClient * CXmppClient::s_pClient = NULL;
CXmppClient* CXmppClient::GetInstance( CIMManager* pMgr, _IM_CONFIG& config )
{
	if (s_pClient == NULL)
	{
		s_pClient = new CXmppClient(pMgr);
		if (s_pClient == NULL) return NULL;

		s_pClient->m_config = config;
		s_pClient->m_jid.setUsername(s_pClient->m_config.sUserName.toNarrowString());
		s_pClient->m_jid.setServer(s_pClient->m_config.sDomain.toNarrowString());
		s_pClient->m_jid.setResource(s_pClient->m_config.sResource.toNarrowString());

		if (!s_pClient->start()){
			SAFE_DELETE(s_pClient);
		}

		s_pClient->m_bRunning = true;
	}
	return s_pClient;
}

void CXmppClient::Unintialize()
{
	if (m_bRunning)
	{
		if (m_wTaskQueue.size() > 0) {
			LPXMPP_TASK_HEADER lpTask = m_wTaskQueue.leave();
			SAFE_DELETE(lpTask);
		}

		while (m_wSeniorTaskQueue.size() > 0){
			LPXMPP_TASK_HEADER lpTask = m_wSeniorTaskQueue.leave();
			SAFE_DELETE(lpTask);
		}

		LPXMPP_TASK_UNINSTALL lpTask = new XMPP_TASK_UNINSTALL();
		m_wSeniorTaskQueue.enter(((LPXMPP_TASK_HEADER&)lpTask));
	}
	else
	{
		delete this;
	}
}

bool CXmppClient::IMLogout()
{
	if (m_bRunning)
	{
		while (m_wSeniorTaskQueue.size() > 0){
			LPXMPP_TASK_HEADER lpTask = m_wSeniorTaskQueue.leave();
			SAFE_DELETE(lpTask);
		}

		LPXMPP_TASK_IM_LOGOUT lpTask = new XMPP_TASK_IM_LOGOUT();
		m_wSeniorTaskQueue.enter(((LPXMPP_TASK_HEADER&)lpTask));
		return true;
	}
	return false;
}

bool CXmppClient::IMLogin( _tstring tsToken )
{
	if (m_bRunning)
	{
		while (m_wSeniorTaskQueue.size() > 0){
			LPXMPP_TASK_HEADER lpTask = m_wSeniorTaskQueue.leave();
			SAFE_DELETE(lpTask);
		}

		LPXMPP_TASK_IM_LOGIN lpTask = new XMPP_TASK_IM_LOGIN();
		lpTask->tsToken = tsToken; 
		m_wSeniorTaskQueue.enter(((LPXMPP_TASK_HEADER&)lpTask));
		return true;
	}
	return false;
}

bool CXmppClient::IMReconnect( bool bImmediately )
{
	if (m_bRunning)
	{
		while (m_wSeniorTaskQueue.size() > 0){
			LPXMPP_TASK_HEADER lpTask = m_wSeniorTaskQueue.leave();
			SAFE_DELETE(lpTask);
		}

		LPXMPP_TASK_RECONNECT pTask = new XMPP_TASK_RECONNECT();
		pTask->_bImmediately = bImmediately;
		m_wSeniorTaskQueue.enter((LPXMPP_TASK_HEADER&)pTask);
		return true;
	}

	return false;
}

void CXmppClient::SetPresence(ArcPresenceType iType)
{
	if (m_bRunning)
	{
		LPXMPP_TASK_HEADER pHeader = new XMPP_TASK_SET_PRESENCE();
		((LPXMPP_TASK_SET_PRESENCE)pHeader)->_iPresence = iType;
		m_wTaskQueue.enter(pHeader);
	}
}

bool CXmppClient::SendChatMessage(_tstring sUserName,_tstring sContent)
{
	if (m_bRosterArrive)
	{
		LPXMPP_TASK_SEND_MESSAGE pHeader = new _XMPP_TASK_SEND_MESSAGE();
		pHeader->_szUserName.assign(sUserName);
		pHeader->_szContent.assign(sContent);
		m_wTaskQueue.enter((LPXMPP_TASK_HEADER&)pHeader);
		return true;
	}
	return false;	
}

void CXmppClient::CheckOfflineMessage()
{
	if (m_bRunning)
	{
		LPXMPP_TASK_CHECK_OFFLINE lpTask = new _XMPP_TASK_CHECK_OFFLINE();
		m_wTaskQueue.enter((LPXMPP_TASK_HEADER&)lpTask);
	}
}

//UI Thread functions end
/********************************************************************************************/

// ---- CXmppClient end ----
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
// ---- CXmppClientMgr begin ----


CXmppClientMgr::CXmppClientMgr(CIMManager* pMgr)
:m_pMgr(pMgr),
m_pCurClient(NULL)
{

}
CXmppClientMgr::~CXmppClientMgr()
{

}

void CXmppClientMgr::Initialize(const _IM_CONFIG config)
{
	m_config = config;
	hostent* host_entry = gethostbyname(m_config.sServer.toUTF8().c_str()) ;
	char sBuf [256] = {0} ;
	if (host_entry != NULL)
	{
		sprintf(sBuf , "%d.%d.%d.%d",
			( host_entry->h_addr_list [0][0]&0x00ff),
			( host_entry->h_addr_list [0][1]&0x00ff),
			( host_entry->h_addr_list [0][2]&0x00ff),
			( host_entry->h_addr_list [0][3]&0x00ff));
	}

	m_config.sServer=sBuf;
	m_config.nPort =(m_config.nPort>0)? m_config.nPort:-1;
}	

void CXmppClientMgr::Unintialize()
{
	try {  
		if (NULL != m_pCurClient) {
			m_pCurClient->Unintialize();
			m_pCurClient = NULL;
		}
	}catch(...){
	}
}

bool CXmppClientMgr::StartClient(const _tstring tsToken)
{
	try {    
		if (!m_pMgr || tsToken.empty()) return false; 

		if (NULL != m_pCurClient) {
			m_pCurClient->IMLogout();
		}

		if (NULL == m_pCurClient) {
			m_pCurClient = CXmppClient::GetInstance(m_pMgr, m_config);
			if (m_pCurClient == NULL) return false;
		}

		m_pMgr->GetFriendManager().Reset();
		return m_pCurClient->IMLogin(tsToken);

	}catch(...){
		return false;
	}

	return false;
}

void CXmppClientMgr::StopClient()
{
	try {
		if (NULL != m_pCurClient){
			m_pCurClient->IMLogout();
		}
	}catch(...){
	}
}

bool  CXmppClientMgr::SendChatMessage(_tstring sUserName,_tstring sContent)
{
	try {    
		if (NULL != m_pCurClient){
			return m_pCurClient->SendChatMessage(sUserName,sContent);
		}
	}catch(...){
		return false;
	}
}
void  CXmppClientMgr::SetPresece(ArcPresenceType iType)
{
	try {
		if (GetPresence() == ArcPresenceType::Unavailable) {
			if (iType < ArcPresenceType::XA) {
				//need to login
				if (m_pCurClient != NULL) {
					m_pCurClient->IMReconnect(true);
				}else {
					m_pMgr->HandleReConnect(true);
				}
				return;
			}
		}
		if (m_pCurClient != NULL) {
			if (GetPresence() < ArcPresenceType::XA) {
				if (iType == ArcPresenceType::Unavailable) {
					StopClient();
					return;
				}
			}
			m_pCurClient->SetPresence((ArcPresenceType)iType);
		}
	}catch(...){
	}
}

ArcPresenceType CXmppClientMgr::GetPresence()
{
	try {    
		if (NULL != m_pCurClient){
			return (ArcPresenceType)(m_pCurClient->GetPresence());
		}
	}catch(...){
		return ArcPresenceType::Unavailable;
	}
	return ArcPresenceType::Unavailable;
}

void CXmppClientMgr::HandleRosterArrive()
{
	try {    
		if (NULL != m_pCurClient){
			return m_pCurClient->HandleRosterArrive();
		}
	}catch(...){
	}
}

void CXmppClientMgr::CheckOfflineMessage()
{
	try {    
		if (NULL != m_pCurClient){
			return m_pCurClient->CheckOfflineMessage();
		}
	}catch(...){
	}
}

int CXmppClientMgr::GetDisconnectPresence()
{
	try {
		if(NULL != m_pCurClient) {
			return m_pCurClient->GetDisconnectPresence(); 
		}
	}catch(...){
	}
	
	return (int)ArcPresenceType::Available;
}

void CXmppClientMgr::SetLoginWithStatus( int status )
{
	if (status >= (int)ArcPresenceType::XA) return;

	try {
		if(NULL == m_pCurClient) {
			m_pCurClient = CXmppClient::GetInstance(m_pMgr, m_config);
			if (NULL == m_pCurClient) return;
		} 
		m_pCurClient->SetLoginWithStatus(status); 
	}catch(...){
	}
}

bool CXmppClientMgr::IsSupportOfflineMessage()
{
	try {
		if (NULL != m_pCurClient) {
			return m_pCurClient->IsSupportOfflineMessage(); 
		}
	}catch(...){
	}
	
	return false;
}