#pragma once
#include "CoreMsgBase.h"

#define CORECLIENT_MAINBROWSER_COMMANDLINE  _T("CoreClientMainBrowserCommandLine")
#define CORECLIENT_GAMEBROWSER_COMMANDLINE  _T("CoreClientGameBrowserCommandLine")

//////////////////////////////////////////////////////////////////////////////////
///use 0 for default client id, positive id is used for game id, negative id is for internal use.
#define CORECLIENT_MAINBROWSER  -1 
#define CORECLIENT_GAMEBROWSER  -2 

enum enCoreMsgType
{
	CORE_MSG_NAVIGATE_E = 100,
	CORE_MSG_NAVIGATE_URL,
	CORE_MSG_BROWSER_EVENT,
	CORE_MSG_BROWSER_INITCONNET,
	CORE_MSG_BROWSER_CRASH,
	CORE_MSG_BROWSER_MESSAGE,
	CORE_MSG_BROWSER_INFO,
	CORE_MSG_BROWSER_SETCOOOKIE,
	CORE_MSG_BROWSER_MSGEVENT,
	CORE_MSG_BROWSER_DEFRECT,
	CORE_MSG_BROWSER_PAINT,
	CORE_MSG_BROWSER_ADDRESSCHANGE,
	CORE_MSG_BROWSER_DELCOOOKIE,
};

enum enCoreBrowserEvent
{
	BROWSER_GOBACK = 0,
	BROWSER_GOFORWORD,
	BROWSER_REFRESH,
	BROWSER_SHOWDEVTOOLS,
};
enum enCoreBrowserType{
	TYPE_BASEBROWSER = 0,
	TYPE_MAINBROWSER,
	TYPE_GAMEBROWSER,
    TYPE_LOGINBROWSER,
};

struct core_msg_navigate_e : public core_msg_header
{
	DWORD dwMode;
	char scUrl [2048];
	core_msg_navigate_e()
	{
		memset(this,0,sizeof(core_msg_navigate_e));
		dwSize = sizeof(core_msg_navigate_e);
		dwCmdId = CORE_MSG_NAVIGATE_E;
		dwMode = 1;
	}
};
struct core_msg_navigateurl : public core_msg_header
{
	DWORD dwMode;
	char scUrl [2048];
	BOOL bUrlGameInfo;
	BOOL bUrlInGameList;
	core_msg_navigateurl()
	{
		memset(this,0,sizeof(core_msg_navigateurl));
		dwSize = sizeof(core_msg_navigateurl);
		dwCmdId = CORE_MSG_NAVIGATE_URL;
		dwMode = 1;
		bUrlGameInfo = FALSE;
		bUrlInGameList = FALSE;
		bGet = TRUE;
	}
};
struct core_msg_browserevent : public core_msg_header
{
	DWORD dwBrowserEventId;
	core_msg_browserevent()
	{
		memset(this,0,sizeof(core_msg_browserevent));
		dwSize = sizeof(core_msg_browserevent);
		dwCmdId = CORE_MSG_BROWSER_EVENT;
	}
};
struct core_msg_browsercrash :public core_msg_header
{
	DWORD dwClientId;
	core_msg_browsercrash()
	{
		memset(this,0,sizeof(core_msg_browsercrash));
		dwSize = sizeof(core_msg_browsercrash);
		dwCmdId = CORE_MSG_BROWSER_CRASH;
	}
};

struct core_msg_initbaseinfo : public core_msg_header
{
	UINT  uType;
	HWND  hBrowserWnd;
	HWND  hSelfWnd;
	DWORD dwMode;
	RECT  rcDefBrowser;
	SIZE  szBrowser;
	char  csAllGameUrl[1024];
	char  csStartUrl[1024];
	char  csCookieUrl [1024];
	char  csWebsite_Pwrd_Oldnews[1024];
	char  csWebsite_perfectworld[1024];
	char  csSessionId[128];
	char  csCookieFolder[256];
	core_msg_initbaseinfo()
	{
		memset(this,0,sizeof(core_msg_initbaseinfo));
		dwSize = sizeof(core_msg_initbaseinfo);
		dwCmdId = CORE_MSG_BROWSER_INITCONNET;
		bGet = TRUE;
	}
};
struct core_msg_msginfo : public core_msg_header
{
	UINT msgId;
	WPARAM wParam;
	LPARAM lParam;
	core_msg_msginfo()
	{
		memset(this,0,sizeof(core_msg_msginfo));
		dwSize = sizeof(core_msg_msginfo);
		dwCmdId = CORE_MSG_BROWSER_MESSAGE;
	}
};
struct core_msg_browserinfo : public core_msg_header
{
	BOOL bLoading;
	core_msg_browserinfo()
	{
		memset(this,0,sizeof(core_msg_browserinfo));
		dwSize = sizeof(core_msg_browserinfo);
		dwCmdId = CORE_MSG_BROWSER_INFO;
		bGet = TRUE;
		bLoading = FALSE;
	}
};
struct core_msg_setcookie : public core_msg_header
{
	char csSessionId[128];
	core_msg_setcookie()
	{
		memset(this,0,sizeof(core_msg_setcookie));
		dwSize = sizeof(core_msg_setcookie);
		dwCmdId = CORE_MSG_BROWSER_SETCOOOKIE;
	}
};
struct core_msg_msgevent : public core_msg_header
{
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	core_msg_msgevent()
	{
		memset(this,0,sizeof(core_msg_msgevent));
		dwSize = sizeof(core_msg_msgevent);
		dwCmdId = CORE_MSG_BROWSER_MSGEVENT;
	}
};
struct core_msg_defrect : public core_msg_header
{
	RECT rcdef;
	core_msg_defrect()
	{
		memset(this,0,sizeof(core_msg_defrect));
		dwSize = sizeof(core_msg_defrect);
		dwCmdId = CORE_MSG_BROWSER_DEFRECT;
	}
};
struct core_msg_paint : public core_msg_header
{
	RECT rect;
	core_msg_paint()
	{
		memset(this,0,sizeof(core_msg_paint));
		dwSize = sizeof(core_msg_paint);
		dwCmdId = CORE_MSG_BROWSER_PAINT;
	}
};
struct core_msg_addresschange : public core_msg_header
{
	char scUrl [2048];
	core_msg_addresschange()
	{
		memset(this,0,sizeof(core_msg_addresschange));
		dwSize = sizeof(core_msg_addresschange);
		dwCmdId = CORE_MSG_BROWSER_ADDRESSCHANGE;
	}
};

struct core_msg_delcookie : public core_msg_header
{
	char scUrl[2048];
	char scCookieName[512];
	char csCookieFolder[256];
	core_msg_delcookie()
	{
		memset(this,0,sizeof(core_msg_delcookie));
		dwSize = sizeof(core_msg_delcookie);
		dwCmdId = CORE_MSG_BROWSER_DELCOOOKIE;
	}
};