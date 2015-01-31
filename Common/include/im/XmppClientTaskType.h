#pragma once
#include <tchar.h>
#include <tstring/tstring.h>

using namespace String;

enum E_XMPPCLIENT_TASK_TYPE
{
	XMPP_TT_INVALID=-1,
	XMPP_TT_UNINSTALL,
	XMPP_TT_LOGOUT,
	XMPP_TT_LOGIN,
	XMPP_TT_RELOGIN,
	XMPP_TT_RECONNECT,
	XMPP_TT_WAIT_FOR_RUN,
	XMPP_TT_CHECK_OFFLINE,
	XMPP_TT_EXIT_THREAD,
	XMMP_TT_PING,
	XMPP_TT_SET_PRESENCE,
	XMPP_TT_SEND_MESSAGE,
	XMPP_TT_RECV_OFFLINE_MESSAGE,
	XMPP_TT_RECV_MESSAGE,
};

enum E_XMPPCLIENT_TASK_STOP_TYPE
{
	XMPP_ST_STOP_RUN = 1,
	XMPP_ST_NOT_STOP_RUN,
	XMPP_ST_DELETE_SELF,
};

/*
 *@ _XMPP_TASK_HEADER Struct
 *@ used store the infomation of IM system, the object of this struct can only be 
 *@ used by UI thread and the recv thread.
 */
typedef struct _XMPP_TASK_HEADER
{
	_XMPP_TASK_HEADER()
	{
        memset(this,0,sizeof(_XMPP_TASK_HEADER));
		_iType = XMPP_TT_INVALID;
	}

	virtual ~_XMPP_TASK_HEADER(){}

	int _iType;

} XMPP_TASK_HEADER, *LPXMPP_TASK_HEADER;

/*
 *@ _XMPP_TASK_UNINSTALL Struct
 */
typedef struct _XMPP_TASK_UNINSTALL : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_UNINSTALL()
	{
		_iType = XMPP_TT_UNINSTALL;
	}

} XMPP_TASK_UNINSTALL, *LPXMPP_TASK_UNINSTALL;

/*
 *@ XMPP_TASK_IM_LOGIN Struct
 *@ 
 */
typedef struct _XMPP_TASK_IM_LOGIN : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_IM_LOGIN()
	{
		_iType = XMPP_TT_LOGIN;
	}
	_tstring tsToken;

} XMPP_TASK_IM_LOGIN, *LPXMPP_TASK_IM_LOGIN;

/*
 *@ XMPP_TASK_IM_LOGOUT Struct
 *@ just for disconnect from the IM Server
 */
typedef struct _XMPP_TASK_IM_LOGOUT : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_IM_LOGOUT()
	{
		_iType = XMPP_TT_LOGOUT;
	}

} XMPP_TASK_IM_LOGOUT, *LPXMPP_TASK_IM_LOGOUT;


/*
 *@ XMPP_TASK_IM_RELOGIN Struct
 *@ 
 */
typedef struct _XMPP_TASK_IM_RELOGIN : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_IM_RELOGIN()
	{
		_iType = XMPP_TT_RELOGIN;
	}
	_tstring tsToken;

} XMPP_TASK_IM_RELOGIN, *LPXMPP_TASK_IM_RELOGIN;

/*
 *@ _XMPP_TASK_RECONNECT Struct
 *@ can been sent by any thread for reconnect
 */
typedef struct _XMPP_TASK_RECONNECT : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_RECONNECT()
		: _bImmediately(false)
		, _bOperatorByUser(false)
	{
		_iType = XMPP_TT_RECONNECT;
	}
	bool _bImmediately;
	bool _bOperatorByUser;

} XMPP_TASK_RECONNECT, *LPXMPP_TASK_RECONNECT;

/*
 *@ _XMPP_TASK_WAIT_FOR_RUN Struct
 *@ just sent by run thread it self when has no task to process;
 *@ has not used now(has some problems);
 */
typedef struct _XMPP_TASK_WAIT_FOR_RUN : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_WAIT_FOR_RUN()
	{
		_iType = XMPP_TT_WAIT_FOR_RUN;
	}

} XMPP_TASK_WAIT_FOR_RUN, *LPXMPP_TASK_WAIT_FOR_RUN;

/*
 *@ _XMPP_TASK_CHECK_OFFLINE Struct
 *@ used by ui thread after all of the online friends's presnce has been handled
 */
typedef struct _XMPP_TASK_CHECK_OFFLINE : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_CHECK_OFFLINE()
	{
		_iType = XMPP_TT_CHECK_OFFLINE;
	}

} XMPP_TASK_CHECK_OFFLINE, *LPXMPP_TASK_CHECK_OFFLINE;

/*
 *@ _XMPP_TASK_SET_PRESENCE Struct
 *@ can be sent by any thread when needed 
 */
typedef struct _XMPP_TASK_SET_PRESENCE : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_SET_PRESENCE()
	{
		_iType = XMPP_TT_SET_PRESENCE;
	}
	int _iPresence;

} XMPP_TASK_SET_PRESENCE, *LPXMPP_TASK_SET_PRESENCE;

/*
 *@ _XMPP_TASK_SEND_MESSAGE Struct
 *@ sent by ui thread when send chat message
 */
typedef struct _XMPP_TASK_SEND_MESSAGE : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_SEND_MESSAGE()
	{
		_iType = XMPP_TT_SEND_MESSAGE;
	}
	_tstring _szUserName;
	_tstring _szContent;

} XMPP_TASK_SEND_MESSAGE, *LPXMPP_TASK_SEND_MESSAGE;

/*
 *@ _XMPP_TASK_RECV_MESSAGE Struct
 *@ sent by recv thread when the offline message return finished
 */
typedef struct _XMPP_TASK_RECV_OFFLINE_MESSAGE : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_RECV_OFFLINE_MESSAGE()
	{
		_iType = XMPP_TT_RECV_OFFLINE_MESSAGE;
	}

} XMPP_TASK_RECV_OFFLINE_MESSAGE, *LPXMPP_TASK_RECV_OFFLINE_MESSAGE;

/*
 *@ _XMPP_TASK_RECV_MESSAGE Struct
 *@ sent by recv thread when the offline message return finished
 */
typedef struct _XMPP_TASK_RECV_MESSAGE : public _XMPP_TASK_HEADER
{
	_XMPP_TASK_RECV_MESSAGE()
	{
		_iType = XMPP_TT_RECV_MESSAGE;
	}

} XMPP_TASK_RECV_MESSAGE, *LPXMPP_TASK_RECV_MESSAGE;

