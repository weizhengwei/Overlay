/**
 * @file      FriendManager.h
   @brief     The header file for friend list manager.
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
#include <tstring/tstring.h>
#include <gloox/gloox.h>
#include <gloox/jid.h>
#include <gloox/rosterlistener.h>
#include "gloox/presence.h"
#include <map>
#include <vector>
#include "thread/thread.h"
#include "im_constant.h"

using namespace String;
using namespace gloox;


class CIMManager;

class CFriendManager: public RosterListener
{
public:
	CFriendManager(CIMManager* pMgr);
	~CFriendManager(void);
public:
	bool GetFriendList(std::vector<CHAT_FRIEND> &friends);
	bool GetFriendByName(_tstring sName,CHAT_FRIEND& Friend);
	bool GetFriendByNick(_tstring sNick,CHAT_FRIEND& Friend);
	bool GetFriendNickByName(_tstring tsName, _tstring& tsNick);
	int  GetFriendPresenceByName(LPCTSTR lpUserName);
	int  GetOnlineFriendsCount();
	int  GetFriendsCount();
    void Reset(); //we should reset some initial state when IM is reconnected.
    void UpdateAvartar(_tstring sUserName,_tstring szPath);
    
protected:
	virtual void handleItemAdded( const JID& jid ){};
	virtual void handleItemSubscribed( const JID& jid ){};
	virtual void handleItemRemoved( const JID& jid );
	virtual void handleItemUpdated( const JID& jid ){};
	virtual void handleItemUnsubscribed( const JID& jid ){};
	virtual void handleSelfPresence( const RosterItem& item, const std::string& resource,\
		Presence::PresenceType presence, const std::string& msg ){};
	virtual void handleRosterError( const IQ& iq ){};
	virtual void handleRoster(const Roster& roster );
	virtual void handleRosterPresence(const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string&  msg );
	virtual void handleNonrosterPresence(const Presence& presence );
	virtual bool handleSubscriptionRequest(const JID& jid, const std::string& /*msg*/ ){return true ;};
	virtual bool handleUnsubscriptionRequest(const JID& jid, const std::string& /*msg*/ ){return true ;};
protected:
	void UpdatePresenceFromTempVec();

private:
	std::TMapFriend m_mapFriends;//<nickname , CFriend>pair map.
	std::map<_tstring,CHAT_FRIEND> m_mapTmpFriends;
	Mutex m_lockFriendList ;

	CIMManager* m_pMgr;
};
