#pragma once

enum eBrowserPluginRequestType
{
	PluginRequest_StartClient=0,
	PluginRequest_GetStatus,
	PluginRegqest_GetAllStatus,
	PluginRequest_StartInfoPop,
	PluginRequest_SetFullScreenMode,
	PluginRequest_GetGameToken,
    PluginRequest_GoToLoginDialog,
	PluginRequest_LoginClient,
	PluginRequest_GotoWebPage,
    PluginRequest_GetAccountName,
    PluginRequest_GetGameTokenEx,
    PluginRequest_GetAccountNameEx,
};

#define TOKEN_LENGTH                        16
#define MAP_BUF_SIZE                        128
#define FILE_CFG_CLIENT                    _T("config.ini")
#define	INI_CFG_CLIENT_INGAMEDLL		   _T("InGameDll")
#define INI_CFG_CLIENT_INGAMEDLL_COUNT	   _T("NotificationCount")

#ifndef _WIN64
#define INI_CFG_CLIENT_INGAMEDLL_LIST	   _T("Notification")
#else
#define INI_CFG_CLIENT_INGAMEDLL_LIST	   _T("Notification_64")
#endif

#define BROWSER_PLUGIN_SERVER_PIPE_NAME    _T("BrowserPluginServerPipe")

#define REG_ITEM_CLIENT				_T("Software\\Perfect World Entertainment\\Arc")
#define REG_ITEM_APP				_T("Software\\Perfect World Entertainment\\App")
#define REG_ITEM_KEY_CLIENT			_T("client")
#define REG_ITEM_KEY_PATCHER		_T("patcher")
#define REG_ITEM_KEY_LAUNCHER		_T("launcher")

#define EVENT_CLIENT_START		    _T("Global\\CoreClient_StartEvent")
#define EVENT_PATCHER_START		    _T("Global\\CoreUpdate_StartEvent")
#define EVENT_REPAIR_START		    _T("Global\\CoreRepair_StartEvent")
#define EVENT_CLIENT_INSTALL		_T("Global\\Event_ArcInstaller_Running")