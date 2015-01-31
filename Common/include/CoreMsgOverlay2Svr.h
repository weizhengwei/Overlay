#pragma once
#include "CoreMsgBase.h"

#define NOTIFY_CLICK	_T("notification click")
#define INBOX_CLICK		_T("inbox click")
#define FR_ACCEPT		_T("friend request accept")
#define FR_IGNORE		_T("friend request ignore")
enum
{
	CORE_MSG_QUERYTOKEN = 2,
	CORE_MSG_TOKEN,
	CORE_MSG_COOKIE,
	CORE_MSG_NOTIFICATION,
	CORE_MSG_INBOX,
	CORE_MSG_FRIENDREQ,
	CORE_MSG_FRSTATUS,
	CORE_MSG_FIRST_FIXED_NOTIFICATION,
	CORE_MSG_CHECK_GAME_WINDOWED,
	CORE_MSG_TRACK,
	CORE_MSG_FR_RESULT,
	CORE_MSG_READY_PLAY,
	CORE_MSG_GAMESTATE,
};


//////////////////////////////////////////////////////////////////////////
// this packet is used for a message server reply a client with game token
// dwRet = 0 means success
struct core_msg_token : public core_msg_header
{
	DWORD dwClientId;	// a client id is generally an user-define identity number which should be unique.
	// in CoreClient this value should the game id.
	char csGameToken[1024];	//
	core_msg_token()
	{
		memset(this, 0, sizeof(core_msg_token));
		dwSize = sizeof(core_msg_token);
		dwCmdId = CORE_MSG_TOKEN;
		//bGet = TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////
// this packet is used for a message server reply a client with overlay string
// dwRet = 0 means success
struct core_msg_cookie : public core_msg_header
{
	DWORD dwClientId;	// a client id is generally an user-define identity number which should be unique.
	// in CoreClient this value should the game id.
	HANDLE hWnd;		// handle of Core main window
	union
	{
		struct 
		{
			WCHAR csCookiePage[1024];	
			WCHAR csBillingPage[1024];	
			WCHAR csSupportPage[1024];	
			WCHAR csWebPage[1024];	
			WCHAR csMediaPage[1024];	
			WCHAR csNewsPage[1024];	
			WCHAR csForumPage[1024];
			WCHAR csMyAccountPage[1024];
			WCHAR csFriendSearchPage[1024];
			WCHAR csGameName[1024];
			WCHAR csBackToGame[1024];	
			WCHAR csClose[1024];	
			WCHAR csNews[1024];	
			WCHAR csBilling[1024];	
			WCHAR csSupport[1024];	
			WCHAR csForum[1024];
			WCHAR csWeb[1024];	
			WCHAR csMedia[1024];	
			WCHAR csInvite[1024];	
			WCHAR csChatView[1024];	
			WCHAR csDialogTitle[1024];
			WCHAR csInGameTitle[1024];
			WCHAR csCurrentPlay[1024];
			WCHAR csText2[1024];
			WCHAR csOk[1024];
			WCHAR csCancel[1024];
			WCHAR csUserName[1024];
			WCHAR csGameId[1024];
			WCHAR csUserIcon[1024];
			WCHAR csAccept[1024];
			WCHAR csIgnore[1024];
			WCHAR csMyFriendUrl[1024];
			WCHAR csAcceptText[1024];
			WCHAR csIgnortext[1024];
			WCHAR csYourFriend[1024];
			WCHAR csPlayGame[1024];
			WCHAR csOnLine[1024];
			WCHAR csReadyPlay[1024];
			WCHAR csHi[1024];
			WCHAR csGameAbbrName[1024];
			WCHAR csMonth[12][1024];
			WCHAR csWeek[7][1024];
		};
		WCHAR csCookie[59][1024];
	};

	WCHAR csSessionId[1024];

	core_msg_cookie()
	{
		memset(this, 0, sizeof(core_msg_cookie));
		dwSize = sizeof(core_msg_cookie);
		dwCmdId = CORE_MSG_COOKIE;
	}
};

//////////////////////////////////////////////////////////////////////////
// this packet is used for a message server send notification to a client
struct core_msg_notification : public core_msg_header
{
	DWORD dwClientId;	// a client id is generally an user-define identity number which should be unique.
	// in CoreClient this value should the game id.

	WCHAR sLink[1024];
	WCHAR sNotifId[1024];
	WCHAR sTitle[1024];
	WCHAR sText[1024];

	core_msg_notification()
	{
		memset(this, 0, sizeof(core_msg_notification));
		dwSize = sizeof(core_msg_notification);
		dwCmdId = CORE_MSG_NOTIFICATION;
		//bGet = TRUE;
	}
};

struct core_msg_inbox : public core_msg_header
{
	DWORD dwClientId;

	TCHAR sType[1024];
	TCHAR sId[1024];
	TCHAR sNickName[1024];
	TCHAR sContent[1024];
	TCHAR sFriendLink[1024];
	TCHAR sContentLink[1024];
	TCHAR sTitle[1024];
	TCHAR sArcMailSubject[1024];
	core_msg_inbox()
	{
		memset(this, 0, sizeof(core_msg_inbox));
		dwSize = sizeof(core_msg_inbox);
		dwCmdId = CORE_MSG_INBOX;
	}
};

struct core_msg_friendreq : public core_msg_header
{
	DWORD dwClientId;

	TCHAR sId[1024];
	TCHAR sNickName[1024];
	TCHAR sTitle[1024];
	TCHAR sContent[1024];
	TCHAR sFriendLink[1024];
	int nSenderId;

	core_msg_friendreq()
	{
		memset(this, 0, sizeof(core_msg_friendreq));
		dwSize = sizeof(core_msg_friendreq);
		dwCmdId = CORE_MSG_FRIENDREQ;
	}
};

struct core_msg_FRStatus : public core_msg_header
{
	DWORD dwClientId;

	TCHAR sNickName[1024];
	TCHAR sGameName[1024];
	TCHAR sGameUrl[1024];
	int nType;

	core_msg_FRStatus()
	{
		memset(this, 0, sizeof(core_msg_FRStatus));
		dwSize = sizeof(core_msg_FRStatus);
		dwCmdId = CORE_MSG_FRSTATUS;
	}
};

struct core_msg_ready_play : public core_msg_header
{
	DWORD dwClientId;

	TCHAR szGameName[1024];
	TCHAR szGameAbbr[1024];
	TCHAR szGameLang[1024];
	TCHAR szContent[1024];
	TCHAR szGameProPage[1024];
	TCHAR szGameLogoPath[1024];
	core_msg_ready_play()
	{
		memset(this, 0, sizeof(core_msg_ready_play));
		dwSize = sizeof(core_msg_ready_play);
		dwCmdId = CORE_MSG_READY_PLAY;
	}
};

//////////////////////////////////////////////////////////////////////////
// this packet is to retrieve the first fixed notification to prompt user how to open the overlay
// this packet will be sent to the server every time when the game starts
struct core_msg_first_fixed_notification : public core_msg_header
{
	DWORD dwClientId;		// a client id is generally an user-define identity number which should be unique.
							// in CoreClient this value should the game id.
	WCHAR szTitle[512];		// title for this notification
	WCHAR szText[512];		// Shift + Tab notification text

	core_msg_first_fixed_notification()
	{
		memset(this, 0, sizeof(core_msg_first_fixed_notification));
		dwSize = sizeof(core_msg_first_fixed_notification);
		dwCmdId = CORE_MSG_FIRST_FIXED_NOTIFICATION;
		bGet = TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////
// this packet is to query if the game is in window mode
// dwRet is 1 if in window mode else 0
struct core_msg_check_game_windowed : public core_msg_header
{
	DWORD dwClientId;		// a client id is generally an user-define identity number which should be unique.
							// in CoreClient this value should the game id.

	core_msg_check_game_windowed()
	{
		memset(this, 0, sizeof(core_msg_check_game_windowed));
		dwSize = sizeof(core_msg_check_game_windowed);
		dwCmdId = CORE_MSG_CHECK_GAME_WINDOWED;
	}
};

//////////////////////////////////////////////////////////////////////////
// this packet is for tracking user action in overlay, such as buttons being clicked
struct core_msg_track : public core_msg_header
{
	union{
		DWORD dwClientId;
		DWORD dwSenderId; // for response friend request
		DWORD dwId;		  // for inbox message and notification
	};
	DWORD dwReceiverId;  // for response friend request

	WCHAR csTrack[256];
	core_msg_track()
	{
		memset(this,0,sizeof(core_msg_track));
		dwSize = sizeof(core_msg_track);
		dwCmdId = CORE_MSG_TRACK;
	}
};

struct core_msg_FR_result : public core_msg_header
{
	DWORD dwClientId;
	BOOL bAccept;
	BOOL bResult;
	core_msg_FR_result()
	{
		memset(this, 0, sizeof(core_msg_FR_result));
		dwSize = sizeof(core_msg_track);
		dwCmdId = CORE_MSG_FR_RESULT;
	}

};
////////////////////////////////////////////
// notify arc for game state ,0 for start,1 for close.
struct core_msg_gamestate : public core_msg_header
{
	WCHAR csGameId[128];
	int   nState;
	core_msg_gamestate()
	{
		memset(this,0,sizeof(core_msg_gamestate));
		dwSize = sizeof(core_msg_gamestate);
		dwCmdId = CORE_MSG_GAMESTATE;
	}
};