/**
 * @file      FriendManager.cpp
   @brief     The implementation file for friend list manager.
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
#include "im/FriendManager.h"
#include "im/IMManager.h"

CFriendManager::CFriendManager(CIMManager* pMgr):m_pMgr(pMgr)
{
	Reset();
}

CFriendManager::~CFriendManager(void)
{
	Reset();
}
void CFriendManager::Reset()
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);
	m_mapFriends.clear();
	m_mapTmpFriends.clear();
}
void CFriendManager::handleRoster(const Roster& roster)
{
	//m_bRosterArrive = true;
	m_lockFriendList.lock();
	Roster::const_iterator iter=roster.begin();
	for(;roster.end()!=iter;iter++)
	{   
		_tstring sJid(iter->second->jid());
		_tstring sName = sJid.substr( 0,sJid.Find('@'));
		_tstring sNick(iter->second->name());
		if (sNick.empty())
		{
			sNick = sName;
		}
		
		if (m_mapFriends.find(sNick) != m_mapFriends.end())
		{
			lstrcpyn( m_mapFriends[sNick].szJid,		sJid.c_str(),	MAX_PATH-1 );
			lstrcpyn( m_mapFriends[sNick].szUserName,	sName.c_str(),	MAX_PATH-1 );
			lstrcpyn( m_mapFriends[sNick].szNick,		sNick.c_str(),	MAX_PATH-1 );
		}
		else
		{
			CHAT_FRIEND chatFriend;
			lstrcpyn( chatFriend.szJid,			sJid.c_str(),	MAX_PATH-1 );
			lstrcpyn( chatFriend.szUserName,	sName.c_str(),	MAX_PATH-1 );
			lstrcpyn( chatFriend.szNick,		sNick.c_str(),	MAX_PATH-1 );
			chatFriend.nPresence = (ArcPresenceType)(Presence::Unavailable);
	        m_mapFriends.insert(std::pair<_tstring,CHAT_FRIEND>(sNick, chatFriend));
		}
	}
	m_lockFriendList.unlock();

	UpdatePresenceFromTempVec();

	if (m_pMgr)
	{
        m_pMgr->HandleRosterArrive();
	}

}

void CFriendManager::handleNonrosterPresence(const Presence& presence )
{
	//if(!m_bRosterArrive)
	{	
		LockGuard<Mutex> guardLock(&m_lockFriendList);

		_tstring sJid(presence.from().bare());
		_tstring sName = sJid.substr( 0,sJid.Find('@'));

		CHAT_FRIEND chatFriend;
		lstrcpyn( chatFriend.szJid,			sJid.c_str(),	MAX_PATH-1 );
		lstrcpyn( chatFriend.szUserName,	sName.c_str(),	MAX_PATH-1 );
        chatFriend.nPresence = (ArcPresenceType)(presence.subtype());
		m_mapTmpFriends.insert(std::pair<_tstring,CHAT_FRIEND>(sName, chatFriend));

	}
}

void CFriendManager::handleRosterPresence(const RosterItem& item, const std::string& resource,
									   Presence::PresenceType presence, const std::string&  msg)
{
	_tstring sName(item.jid());
	sName=sName.substr(0,sName.Find('@'));
	_tstring sNick = item.name();
	if (sNick.empty())
	{
		sNick = sName;
	}
	
	{
		LockGuard<Mutex> guardLock(&m_lockFriendList);

		if (m_mapFriends.find(sNick) != m_mapFriends.end() && m_mapFriends[sNick].nPresence == presence)
		{
			//filter the second time
			return;
		}

		if (m_mapFriends.find(sNick) != m_mapFriends.end())
		{
			m_mapFriends[sNick].nPresence = (ArcPresenceType)presence;
		}
		else
		{
			CHAT_FRIEND chatFriend;
			lstrcpyn( chatFriend.szJid,			_tstring(item.jid()).c_str(),	MAX_PATH-1 );
			lstrcpyn( chatFriend.szUserName,	sName.c_str(),					MAX_PATH-1 );
			lstrcpyn( chatFriend.szNick,		sNick.c_str(),	MAX_PATH-1 );
			chatFriend.nPresence = (ArcPresenceType)presence;
			m_mapFriends.insert(std::pair<_tstring,CHAT_FRIEND>(sNick,chatFriend));
		}
	}


	if (m_pMgr)
	{
		m_pMgr->HandlePresence(sName,presence);
	}
}

void CFriendManager::handleItemRemoved(const JID& jid)
{
    //Note: we just need deal a removing message as an  off-line presence message according to the NPI at present.It should be changed in future according to a new design.
	_tstring tsName(jid.bare());
	tsName = tsName.substr(0,tsName.Find('@'));

	_tstring tsNick;
	bool bNickExist = false;

	{
		LockGuard<Mutex> guardLock(&m_lockFriendList);

		std::TMapFriendItr itr = m_mapFriends.begin();
		for (; itr != m_mapFriends.end(); itr++)
		{
			if (_tstring(itr->second.szUserName).CompareNoCase(tsName.c_str()) == 0)
			{
				tsNick = itr->second.szNick;
				bNickExist = true;
				m_mapFriends.erase(itr);
				break;
			}
		}
	}

	if (m_pMgr && bNickExist)
	{
		m_pMgr->HandleDivorce(tsName, tsNick);
	}

}


int CFriendManager::GetOnlineFriendsCount()
{
   int nCount =0;
   LockGuard<Mutex> guardLock(&m_lockFriendList);
   std::TMapFriendItr iter = m_mapFriends.begin();
   for (; iter!= m_mapFriends.end(); iter++)
   {
	   if (iter->second.nPresence < Presence::XA)
	   {
          nCount++;
	   }
   }
   return nCount;
}
void CFriendManager::UpdatePresenceFromTempVec()
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	std::map<_tstring,CHAT_FRIEND>::iterator itrTmp = m_mapTmpFriends.begin();
	for (;itrTmp != m_mapTmpFriends.end();itrTmp++)
	{
		std::TMapFriendItr itr = m_mapFriends.begin();
		for (; itr != m_mapFriends.end(); itr++)
		{
			if (_tstring(itr->second.szUserName).CompareNoCase(itrTmp->first) == 0)
			{
				m_mapFriends[itr->first].nPresence = itrTmp->second.nPresence;
				break;
			}
		}
	}
	m_mapTmpFriends.clear();

};
bool CFriendManager::GetFriendByName(_tstring sName,CHAT_FRIEND& Friend)
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	bool bRet = false;
	std::TMapFriendItr iter = m_mapFriends.begin();
	for (; iter != m_mapFriends.end(); iter++)
	{
		if (_tstring(iter->second.szUserName).CompareNoCase(sName) == 0)
		{
			Friend = iter->second;
			bRet = true;
			break;
		}
	}

	return bRet;
}

bool CFriendManager::GetFriendByNick(_tstring sNick,CHAT_FRIEND& Friend)
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	bool bRet = false;
	if (m_mapFriends.find(sNick) != m_mapFriends.end())
	{
		Friend = m_mapFriends[sNick];
		bRet = true;
	}

	return bRet;
}

bool CFriendManager::GetFriendNickByName( _tstring tsName, _tstring& tsNick )
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	bool bRet = false;
	std::TMapFriendItr iter = m_mapFriends.begin();
	for (; iter != m_mapFriends.end(); iter++)
	{
		if (_tstring(iter->second.szUserName).CompareNoCase(tsName) == 0)
		{
			tsNick = iter->second.szNick;
			bRet =true;
			break;
		}
	}

	return bRet;
}

int CFriendManager::GetFriendPresenceByName( LPCTSTR lpUserName )
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	std::TMapFriendItr itr = m_mapFriends.begin();
	for (; itr != m_mapFriends.end(); itr++)
	{
		if (_tstring(itr->second.szUserName).CompareNoCase(lpUserName) == 0)
		{
			return (int)itr->second.nPresence;
		}
	}

	return -1;
}

void CFriendManager::UpdateAvartar(_tstring sUserName,_tstring szPath)
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	std::TMapFriendItr itr = m_mapFriends.begin();
	for (; itr != m_mapFriends.end(); itr++)
	{
		if (_tstring(itr->second.szUserName).CompareNoCase(sUserName) == 0)
		{
			lstrcpyn(m_mapFriends[itr->first].szAvatarPath, szPath.c_str(), MAX_PATH-1);
			break;
		}
	}
}

bool CFriendManager::GetFriendList( std::vector<CHAT_FRIEND> &friends )
{
	LockGuard<Mutex> guardLock(&m_lockFriendList);

	bool bRet = false;

	friends.clear();
	if (m_mapFriends.size() > 0) bRet = true;
	for (std::TMapFriendItr itr = m_mapFriends.begin(); itr != m_mapFriends.end(); ++itr)
	{
		friends.push_back(itr->second);
	}

	return bRet;
}

int CFriendManager::GetFriendsCount()
{
	LockGuard<Mutex> guardLock(&m_lockFriendList); 
	return m_mapFriends.size();
}