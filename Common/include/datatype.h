#pragma once

#include <vector>
#include <tchar.h>
#include "tstring/tstring.h"

using namespace String ;

typedef void (*CALLBACK_FUNC_YESNO)(PVOID lpParam) ;
#pragma pack(push, 1)



/*
** notification type
** send to client, to game, or both
*/
enum _NOTIFY_TYPE
{
	NOTIFY_CLIENT = 1,
	NOTIFY_GAME,
	NOTIFY_BOTH,
};

/*
** notification status
** read or un-read
*/
enum _NOTIFY_STATUS
{
	NOTIFY_READ = 0,
	NOTIFY_UNREAD ,
};

/*
** notification data
*/
typedef struct _NOTIFY_ITEM
{
	_tstring  m_sId ;  //unique
	_tstring  m_sTitle;
	_tstring  m_sContent;
	_tstring  m_sTime;
	_tstring  m_sLink;
	_tstring  m_sGameList;
	_tstring  m_sInnerType;
	int       m_nShowCmd;
}NOTIFY_ITEM, EMS_ITEM;

/*
** emergency message data
*/
typedef struct _EMERGENCY_MESSAGE_DATA
{
	_tstring  _szId ;  //unique
	_tstring  _szMode ;
	_tstring  _szUpdateTimeStamp ;
	_tstring  _szProduct ;
	_tstring  _szType;
	_tstring  _szText;
	void Reset()
	{
       _szId = _szMode = _szUpdateTimeStamp = _szProduct = _szType = _szText = _T("");
	}
}EMERGENCY_MESSAGE_DATA;

typedef struct _GAME_DOWNLOAD_COMPLETE{
	_tstring m_sId;
	_tstring m_sGameName;
	_tstring m_sGameAbbr;
	_tstring m_sGameLang;
	_tstring m_sGameProdPageUrl;
	_tstring m_sTimeStamp;
	_tstring m_sGameLogoPath;
}GAME_DOWNLOAD_COMPLETE;

typedef struct _HIDE_ITEM{
	_tstring m_sId;
	_tstring m_sType;
}HIDE_ITEM;

typedef struct _PDL_ITEM{
	_tstring m_sId;
	_tstring m_sTitle;
	_tstring m_sContent;
	_tstring m_sTimeStamp;
	_tstring m_sGameAbbr;
	_tstring m_sGameName;
}PDL_ITEM;


typedef struct _FR_STATUS_NEW{
	_tstring m_sId;
	_tstring m_sTimeStamp;
	_tstring m_sAccount;
	_tstring m_sGameAbbr;
	_tstring m_sNickName;
	_tstring m_sGameName;
	_tstring m_sUrl;
	_tstring m_sGameLan;
	int      m_nType;

}FR_STATUS_NEW;

typedef struct _INBOX_DATA_NEW{

	_tstring m_sType;
	_tstring m_sId ;
	_tstring m_sNickName ;
	_tstring m_sContent ;
	_tstring m_sTimestamp ;
	_tstring m_sFriendLink ;
	_tstring m_sInboxLink ;
	_tstring m_sArcMailSubject ;

}INBOX_DATA_NEW, *PINBOX_DATA_NEW;

typedef struct _FRIREQ_DATA_NEW{

	_tstring m_sId ;
	_tstring m_sTitle;
	_tstring m_sNickName ;
	_tstring m_sAvatarPath ;
	_tstring m_sContent ;
	_tstring m_sFriendLink ;
	int      m_nSenderId ;
	int      m_nReceiverId ;

}FRIREQ_DATA_NEW, *PFRIREQ_DATA_NEW ;

typedef struct _ALERT_ITEM{
	_tstring m_sType;
	_tstring m_sId;
	_tstring m_sTitle;
	_tstring m_sContent;
	_tstring m_sTimeStamp;
	_tstring m_sLink;
	_tstring m_sGameList; //for ace alert
	_tstring m_sGameName; //for game play alerts
	_tstring m_sGameAbbr; //for game play alerts
	_tstring m_sGameLang; //for game play alerts
	_tstring m_sGameLogoPath;//for game play alerts
	int      m_nShowCmd;     //for notification alerts
	
}ALERT_ITEM;

typedef struct _CATER_DATA{
	int m_nType;
	int m_nNum;
}CATER_DATA;

typedef struct _JUMPLIST
{
	HWND g_hMainWnd;
	HWND g_hLoginWnd;
	HWND g_hUpdateWnd;
	_JUMPLIST()
	{
		memset(this, 0, sizeof(*this));
	}
}JUMPLIST, *PJUMPLIST;

#pragma pack(pop)