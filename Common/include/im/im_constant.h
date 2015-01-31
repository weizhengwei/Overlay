#pragma once
#include "CoreMsgBase.h"
#include "thread/thread.h"
#include "tstring/tstring.h"
#include <map>


//定向构造模板;
template<class T>
inline void constructInPlace(T *_Ptr)
{
#ifdef new
#undef new
	new ((void *)_Ptr) T( T() );
#define new DEBUG_NEW
#else
	new ((void *)_Ptr) T( T() );
#endif
}

#define POINTER_DESTROY(pPointer) if(pPointer){ delete pPointer; pPointer = NULL; }

/*common child control resource id.
*
*Note: the reason that enum number is not increased automatically is 
* to avoid some control id is not useless any more at some time point.
*
*/
//IDC
#define 	IDC_LISTBOX_FRIEND_LIST     20000
#define 	IDC_RICHEDIT_FRIEND_SEARCH  20001
#define 	IDC_BTN_CLEAR_HISTORY       20002
#define 	IDC_LISTBOX_SET_PRESENCE    20003
#define 	IDC_EDIT_CHATDLG_INPUT      20004
#define 	IDC_EDIT_CHATDLG_OUTPUT     20005

//IDD
#define		IDD_FRIEND_DLG				20000
#define		IDD_TAB_DLG					20001
#define		IDD_CHAT_DLG				20002
#define		IDD_PRESENCE_DLG			20003


enum ArcPresenceType
{
	Available,                  /**< The entity is online. */
	Chat,                       /**< The entity is 'available for chat'. */
	Away,                       /**< The entity is away. */
	DND,                        /**< The entity is DND (Do Not Disturb). */
	XA,                         /**< The entity is XA (eXtended Away). */
	Unavailable,                /**< The entity is offline. */
	Probe,                      /**< This is a presence probe. */
	Error,                      /**< This is a presence error. */
	Invalid                     /**< The stanza is invalid. */
};

//common string
#define ARC_IM_SERVER_NAME			_T("ArcIMServer")

//common struct.
struct _IM_BASE_CONFIG
{
public:
	bool bShowTimeStamp;
    bool bDisableIM;
    bool bMultiTabMgr;
    bool bDisableFlashWindow;
	bool bChatMsgNotify;
	_IM_BASE_CONFIG()
	{
		bShowTimeStamp =true;
        bDisableIM = false;
        bMultiTabMgr = false;
        bDisableFlashWindow = false;
		bChatMsgNotify = true;
	}
};

typedef struct _CHAT_FRIEND
{
	TCHAR szJid[MAX_PATH];
	TCHAR szUserName[MAX_PATH];//= m_tsJid.substr( 0,mInfo._szJid.Find('@'));
	TCHAR szNick[MAX_PATH];
	TCHAR szAvatarPath[MAX_PATH];
	ArcPresenceType nPresence;

	_CHAT_FRIEND()
	{
		memset(this, 0, sizeof(_CHAT_FRIEND));
		nPresence = ArcPresenceType::Unavailable/*Presence::Unavailable*/;
	}

	_CHAT_FRIEND& operator =(_CHAT_FRIEND info)
	{
		nPresence = info.nPresence;
		lstrcpyn(szJid,			info.szJid,			MAX_PATH-1);
		lstrcpyn(szUserName,	info.szUserName,	MAX_PATH-1);
		lstrcpyn(szNick,		info.szNick,		MAX_PATH-1);
		lstrcpyn(szAvatarPath,	info.szAvatarPath,	MAX_PATH-1);
		return *this;
	}

} CHAT_FRIEND, *LPCHAT_FRIEND;

typedef struct _ITEM_BASE_INFO 
{
	TCHAR _szUser[MAX_PATH];
	TCHAR _szNick[MAX_PATH];
	TCHAR  _szAvatarPath[MAX_PATH];
    TCHAR  _szServer[MAX_PATH];
    TCHAR  _szCharater[MAX_PATH];
	int iType;
	_ITEM_BASE_INFO()
	{
		memset(this, 0, sizeof(_ITEM_BASE_INFO));
		iType = 5/*Presence::Unavailable*/;
	}

	_ITEM_BASE_INFO& operator =(_ITEM_BASE_INFO info)
	{
		iType = info.iType;
		lstrcpyn(_szUser,info._szUser,MAX_PATH-1);
		lstrcpyn(_szNick,info._szNick,MAX_PATH-1);
		lstrcpyn(_szAvatarPath,info._szAvatarPath,MAX_PATH-1);
        lstrcpyn(_szServer,info._szServer,MAX_PATH-1);
		lstrcpyn(_szCharater,info._szCharater,MAX_PATH-1);
		return *this;
	}
}ITEM_BASE_INFO, *LPITEM_BASE_INFO;

typedef struct _MSG_LOG_ELEMENT
{
	TCHAR _szUserName[MAX_PATH]; //friend's name 
	TCHAR  _szRecvNick[MAX_PATH];//receiver's NikeName.
	TCHAR _szSenderNick[MAX_PATH];//sender's NikeName 
	TCHAR _szTime[MAX_PATH];//msg time
	TCHAR _szDate[MAX_PATH];//msg date
	TCHAR _szContent[MAX_PATH];//msg
	_MSG_LOG_ELEMENT()
	{
		memset(this, 0, sizeof(_MSG_LOG_ELEMENT));
	}
	_MSG_LOG_ELEMENT& operator =(_MSG_LOG_ELEMENT msg)
	{
		memset(this, 0, sizeof(_MSG_LOG_ELEMENT));
		lstrcpyn(_szUserName,	msg._szUserName,	MAX_PATH-1);
		lstrcpyn(_szRecvNick,	msg._szRecvNick,	MAX_PATH-1);
		lstrcpyn(_szSenderNick,	msg._szSenderNick,	MAX_PATH-1);
		lstrcpyn(_szTime,		msg._szTime,		MAX_PATH-1);
		lstrcpyn(_szDate,		msg._szDate,		MAX_PATH-1);
		lstrcpyn(_szContent,	msg._szContent,		MAX_PATH-1);

		return *this;
	}
} MSG_LOG_ELEMENT, *LPMSG_LOG_ELEMENT;

// chat system massage id.
enum ARC_CHAT_MSG
{
	ARC_CHAT_MSG_CONNECT_DONE=10000,
	ARC_CHAT_MSG_ITEM_INFO,
    ARC_CHAT_MSG_SELF_INFO,
	ARC_CHAT_MSG_SEND_MSG,
	ARC_CHAT_MSG_SEND_MSG_DONE,
	ARC_CHAT_MSG_RCV_MSG,
	ARC_CHAT_MSG_SET_PRESENCE,
	ARC_CHAT_MSG_PRESENCE_CHANGED,
	ARC_CHAT_MSG_FRIEND_DIVORCE,
	ARC_CHAT_MSG_GET_PRESENCE,
	ARC_CHAT_MSG_AVATAR_CHANGED,
	ARC_CHAT_MSG_LOAD_HISTOPRY_RECORDS,
	ARC_CHAT_MSG_CLEAR_HISTOPRY_RECORDS,
    ARC_CHAT_MSG_LOAD_STRING,
    ARC_CHAT_MSG_FRIEND_LIST_REQ,
    ARC_CHAT_MSG_FRIEND_LIST_RESP,
	ARC_CHAT_MSG_FRIENDLIST_INIT_FINISHED,
	ARC_CHAT_MSG_ROSTER_ARRIVE_FOR_OVERLAY,
	ARC_CHAT_MSG_GET_TIME,
	ARC_CHAT_MSG_SYNC_LOGIN_STATUS,
	ARC_CHAT_MSG_GET_FRIENDS_COUNT,
	ARC_CHAT_MSG_HANDLE_SYNC_SENT_MESSAGE,
	ARC_CHAT_MSG_SYNC_SENT_MESSAGE,

};

struct im_dialog_params
{
	bool bCreated;// whether create dialog if it is not existed.
	UINT nShowCmd;
	CWindow* pParentWnd;
	RECT   rt;
	im_dialog_params()
	{
		memset(this, 0, sizeof(im_dialog_params));
	}
	bool IsWinPosChanging()
	{
		return rt.left>0 || rt.top>0 || rt.right>0 || rt.bottom>0;
	}
};

// chat system message
struct im_msg_connect_done: public core_msg_header
{
	int iType;
	im_msg_connect_done()
	{
		memset(this, 0, sizeof(im_msg_connect_done));
		dwSize = sizeof(im_msg_connect_done);
		dwCmdId = ARC_CHAT_MSG_CONNECT_DONE;
	}
};

struct im_msg_item_info : public core_msg_header
{
	bool bByNick;
	_ITEM_BASE_INFO info;
	im_msg_item_info()
	{
		memset(this, 0, sizeof(im_msg_item_info));
		dwSize = sizeof(im_msg_item_info);
		dwCmdId = ARC_CHAT_MSG_ITEM_INFO;
		bGet = TRUE;
		info.iType = ArcPresenceType::Unavailable;
		bByNick = false;
	}
};

struct im_msg_self_info : public core_msg_header
{
	_ITEM_BASE_INFO info;
	im_msg_self_info()
	{
		memset(this, 0, sizeof(im_msg_self_info));
		dwSize = sizeof(im_msg_item_info);
		dwCmdId = ARC_CHAT_MSG_SELF_INFO;
		bGet = TRUE;
		info.iType = ArcPresenceType::Unavailable;
	}
};

struct im_msg_send_msg : public core_msg_header
{
	MSG_LOG_ELEMENT msg;
	bool bOfflineMessage;
	im_msg_send_msg()
	{
		memset(this, 0, sizeof(im_msg_send_msg));
		dwSize = sizeof(im_msg_send_msg);
		dwCmdId = ARC_CHAT_MSG_SEND_MSG;
		bOfflineMessage = false;
	}
	im_msg_send_msg(MSG_LOG_ELEMENT msg)
	{
		memset(this, 0, sizeof(im_msg_send_msg));
		dwSize = sizeof(im_msg_send_msg);
		dwCmdId = ARC_CHAT_MSG_SEND_MSG;
		bOfflineMessage = false;
		this->msg = msg;
	}
};

struct im_msg_send_msg_done : public core_msg_header
{
	int nRet;
	MSG_LOG_ELEMENT msg;
	im_msg_send_msg_done()
	{
		memset(this, 0, sizeof(im_msg_send_msg_done));
		dwSize = sizeof(im_msg_send_msg_done);
		dwCmdId = ARC_CHAT_MSG_SEND_MSG_DONE;
	}
};

struct im_msg_rcv_msg : public core_msg_header
{
	MSG_LOG_ELEMENT msg;
	bool bOfflineMessage;
	im_msg_rcv_msg()
	{
		memset(this, 0, sizeof(im_msg_rcv_msg));
		dwSize = sizeof(im_msg_rcv_msg);
		dwCmdId = ARC_CHAT_MSG_RCV_MSG;
		bOfflineMessage = false;
	}
};

struct im_msg_set_presence : public core_msg_header
{
	int iType;
	im_msg_set_presence()
	{
		memset(this, 0, sizeof(im_msg_set_presence));
		dwSize = sizeof(im_msg_set_presence);
		dwCmdId = ARC_CHAT_MSG_SET_PRESENCE;
	}
};

struct im_msg_presence_changed : public core_msg_header
{
	TCHAR szUserName[MAX_PATH];
	int iType;
	im_msg_presence_changed()
	{
		memset(this, 0, sizeof(im_msg_presence_changed));
		dwSize = sizeof(im_msg_presence_changed);
		dwCmdId = ARC_CHAT_MSG_PRESENCE_CHANGED;
	}
};

struct im_msg_friend_divorce : public core_msg_header
{
	TCHAR szNickName[MAX_PATH];
	TCHAR szUserName[MAX_PATH];
	im_msg_friend_divorce()
	{
		memset(this, 0, sizeof(im_msg_friend_divorce));
		dwSize = sizeof(im_msg_friend_divorce);
		dwCmdId = ARC_CHAT_MSG_FRIEND_DIVORCE;
	}
};

struct im_msg_get_presence : public core_msg_header
{
	TCHAR szUserName[MAX_PATH];
	int iType;
	im_msg_get_presence()
	{
		memset(this, 0, sizeof(im_msg_get_presence));
		dwSize = sizeof(im_msg_get_presence);
		dwCmdId = ARC_CHAT_MSG_GET_PRESENCE;
		bGet =true;
	}
};

struct im_msg_avatar_changed : public core_msg_header
{
	TCHAR szUserName[MAX_PATH];
	TCHAR szPath[MAX_PATH];
	im_msg_avatar_changed()
	{
		memset(this, 0, sizeof(im_msg_avatar_changed));
		dwSize = sizeof(im_msg_avatar_changed);
		dwCmdId = ARC_CHAT_MSG_AVATAR_CHANGED;
	}
};

#define  MAX_HISTORY_RECORDS_COUNT 100
struct im_msg_load_history_records : public core_msg_header
{
	TCHAR szUserName[MAX_PATH];
	UINT  nCount;
	MSG_LOG_ELEMENT aRecords[MAX_HISTORY_RECORDS_COUNT];
	im_msg_load_history_records()
	{
		memset(this, 0, sizeof(im_msg_load_history_records));
		dwSize = sizeof(im_msg_load_history_records);
		dwCmdId = ARC_CHAT_MSG_LOAD_HISTOPRY_RECORDS;
		bGet = true;
	}
};

struct im_msg_clear_history_records : public core_msg_header
{
	TCHAR szUserName[MAX_PATH];
	im_msg_clear_history_records()
	{
		memset(this, 0, sizeof(im_msg_clear_history_records));
		dwSize = sizeof(im_msg_clear_history_records);
		dwCmdId = ARC_CHAT_MSG_CLEAR_HISTOPRY_RECORDS;
	}
};

struct im_msg_load_string : public core_msg_header
{
	TCHAR szId[MAX_PATH];
    TCHAR szContent[2048];
	im_msg_load_string()
	{
		memset(this, 0, sizeof(im_msg_load_string));
		dwSize = sizeof(im_msg_load_string);
		dwCmdId = ARC_CHAT_MSG_LOAD_STRING;
        bGet = TRUE;
	}
};


struct im_msg_friend_list_response : public core_msg_header
{
	DWORD size(){ return sizeof(im_msg_friend_list_response) + sizeof(_ITEM_BASE_INFO) * dwLen; }
	DWORD dwLen;
	CHAT_FRIEND aFriendList[0];
};

struct im_msg_friend_list_req : public core_msg_header
{
	im_msg_friend_list_req()
	{
		memset(this, 0, sizeof(im_msg_friend_list_req));
		dwCmdId = ARC_CHAT_MSG_FRIEND_LIST_REQ;
		dwSize = sizeof(im_msg_friend_list_req);
	}
};

struct im_msg_friend_list_is_inited_req : public core_msg_header
{
	bool bIsInitFinished;
	im_msg_friend_list_is_inited_req()
	{
		memset(this, 0, sizeof(im_msg_friend_list_is_inited_req));
		dwCmdId = ARC_CHAT_MSG_FRIENDLIST_INIT_FINISHED;
		dwSize = sizeof(im_msg_friend_list_is_inited_req);
		bGet = true;
	}
};

struct im_msg_roster_arrive_for_overlay : public core_msg_header
{
	im_msg_roster_arrive_for_overlay()
	{
		memset(this, 0, sizeof(im_msg_roster_arrive_for_overlay));
		dwCmdId = ARC_CHAT_MSG_ROSTER_ARRIVE_FOR_OVERLAY;
		dwSize = sizeof(im_msg_roster_arrive_for_overlay);
	}
};

struct im_msg_get_time : public core_msg_header
{
	__int64 nTime;//seconds
	im_msg_get_time()
	{
		memset(this, 0, sizeof(im_msg_get_time));
		dwSize = sizeof(im_msg_get_time);
		dwCmdId = ARC_CHAT_MSG_GET_TIME;
		bGet = TRUE;
	}
};

struct im_msg_sync_login_status : public core_msg_header
{
	int nStatus;//seconds
	im_msg_sync_login_status()
	{
		memset(this, 0, sizeof(im_msg_sync_login_status));
		dwSize = sizeof(im_msg_sync_login_status);
		dwCmdId = ARC_CHAT_MSG_SYNC_LOGIN_STATUS;
	}
};

struct im_msg_get_friends_count : public core_msg_header
{
	int nCount;
	im_msg_get_friends_count()
	{
		memset(this, 0, sizeof(im_msg_get_friends_count));
		dwSize = sizeof(im_msg_get_friends_count);
		dwCmdId = ARC_CHAT_MSG_GET_FRIENDS_COUNT;
		bGet = TRUE;
	}
};

struct im_msg_handle_sync_sent_message : public core_msg_header
{
	DWORD size() { return sizeof(im_msg_handle_sync_sent_message) + sizeof(TCHAR) * dwLen; }

	__int64		nTime;
	int			nType;
	int			nSyncID;
	DWORD		dwLen;
	TCHAR		szSelfName[MAX_PATH];
	TCHAR		szUser[MAX_PATH];
	TCHAR		szContent[0];
};

struct im_msg_sync_sent_message : public core_msg_header
{
	DWORD size() { return sizeof(im_msg_sync_sent_message) + sizeof(TCHAR) * dwLen; }
	
	__int64		nTime;
	int			nType;
	int			nSyncID;
	DWORD		dwLen;
	TCHAR		szSelfName[MAX_PATH];
	TCHAR		szUser[MAX_PATH];
	TCHAR		szContent[0];
};


////////////////////////////////////////////////////////////////////////
_STD_BEGIN
using namespace String;

//template function for compareFunc in map of stl;
template<class _Ty>
struct lessStr
	: public binary_function<_Ty, _Ty, bool>
{	// functor for operator<
	bool operator()(const _Ty& _Left, const _Ty& _Right) const
	{	// apply operator< to operands
		return _Left.CompareNoCase(_Right) < 0;
	}
};

typedef map<_tstring, CHAT_FRIEND, lessStr<_tstring>>	TMapFriend;
typedef TMapFriend::iterator							TMapFriendItr;

_STD_END
////////////////////////////////////////////////////////////////////////