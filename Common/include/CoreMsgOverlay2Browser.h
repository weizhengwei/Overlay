#pragma once
#include "CoreMsgBase.h"

enum
{
	CORE_MSG_OVERLAY_INIT = 40,
	CORE_MSG_NAVIGATE,
	CORE_MSG_RESIZE,
	CORE_MSG_CURSORCHANGE,
	CORE_MSG_SHOWOVERLAY,
};

//////////////////////////////////////////////////////////////////////////
// this packet is used by the browser process to notify the overlay to record 
// the message client in browser process
struct core_msg_overlay_init : public core_msg_header
{
	HWND hMsgClient;	// the message client in browser process
	char szSessionId[256];	// session id with login info
	core_msg_overlay_init()
	{
		memset(this, 0, sizeof(core_msg_overlay_init));
		dwSize = sizeof(core_msg_overlay_init);
		dwCmdId = CORE_MSG_OVERLAY_INIT;
		bGet = TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////
// notify the browser to navigate a url in the background
struct core_msg_navigate : public core_msg_header
{
	char szUrl[512];	// the url needs to be navigated
	core_msg_navigate()
	{
		memset(this, 0, sizeof(core_msg_navigate));
		dwSize = sizeof(core_msg_navigate);
		dwCmdId = CORE_MSG_NAVIGATE;
	}
	
};

//////////////////////////////////////////////////////////////////////////
// notify the browser to change its size to fit the size of in-game webpage region
struct core_msg_resize : public core_msg_header
{
	int nWidth;
	int nHeight;
	core_msg_resize()
	{
		memset(this, 0, sizeof(core_msg_resize));
		dwSize = sizeof(core_msg_resize);
		dwCmdId = CORE_MSG_RESIZE;
	}
};

struct core_msg_cursorchange : public core_msg_header
{
	unsigned long lCursorId;
	core_msg_cursorchange()
	{
		memset(this,0,sizeof(core_msg_cursorchange));
		dwSize = sizeof(core_msg_cursorchange);
		dwCmdId = CORE_MSG_CURSORCHANGE;
	}
};

//////////////////////////////////////////////////////////////////////////
// show or hide the overlay
struct core_msg_showoverlay : public core_msg_header
{
	BOOL bShow;	// FALSE hide, TRUE show
	core_msg_showoverlay()
	{
		memset(this, 0, sizeof(core_msg_showoverlay));
		dwSize = sizeof(core_msg_showoverlay);
		dwCmdId = CORE_MSG_SHOWOVERLAY;
	}
};

