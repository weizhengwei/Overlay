#pragma once

#define MAX_NOTIFY_TITLE            64       /*< the maximum length of title text for notificaion is 64bytes*/
#define MAX_NOTIFY_TEXT             512      /*< the maximum length of text for notification is 512bytes*/

//product name
#define PRODUCT_NAME		_T("Arc")
#define SERVICE_NAME        _T("ArcService")

//file names
#define FILE_EXE_CLIENT		_T("Arc.exe")
#define FILE_EXE_PATCHER	_T("ArcUpdate.exe")
#define	FILE_EXE_LAUNCHER	_T("ArcLauncher.exe")
#define FILE_EXE_SERVICE	_T("ArcService.exe")
#define	FILE_EXE_REPAIR		_T("ArcRepair.exe")
#define FILE_EXE_BROWSER	_T("ArcBrowser.exe")
#define FILE_EXE_OSBROWSER  _T("ArcOSBrowser.exe")
#define FILE_EXE_DEPENDS	_T("ArcDepends.exe")
#define FILE_EXE_REPORT		_T("ArcErrRep.exe")
#define	FILE_DLL_OVERLAY8	_T("ArcD3D8Overlay.dll")
#define	FILE_DLL_OVERLAY9	_T("ArcD3D9Overlay.dll")
#define	FILE_DLL_OVERLAY11	_T("ArcD3D11Overlay.dll")
#define	FILE_DLL_OVERLAYSB	("\ArcOverlayStub.dll")
#define FILE_DLL_PLUGINFF	_T("npArcPluginFF.dll")
#define FILE_DLL_PUGLINIE	_T("ArcPluginIE.dll")
#define FILE_DLL_SDK		_T("ArcSDK.dll")
#define FILE_EXE_OSOVERLAY	_T("ArcOSOverlay.exe")
#define FILE_DLL_FLASH      _T("NPSWF32.dll")

#define FILE_CFG_CLIENT		_T("config.ini")
#define FILE_MD5_LIST_FILE  _T("md5Report.txt")
#define FILE_MD5_REPORT_ZIP _T("md5Report.zip")
#define FILE_LOG_CLIENT		_T("clientlog.txt")
#define FILE_LOG_CLIENT_C	_T("clientlog_copy.txt")
#define FILE_CFG_STRINGTABLE	_T("Client_String_Table.dat")
#define FILE_VER_CLIENT		_T("version.ini")
#define FILE_VER_SEED		_T("arcversionseed.xml")
#define FILE_INI_REGISTER		_T("register.ini")

//other text
#define TEXT_USER_PASSWORD	L"User Password"
#define TEXT_IM_RESOURCE   _T("PWPClient")
#define MSG_SERVER_MAIN		_T("MsgServerMain")

//section names in config.ini
#define INI_CFG_CLIENT_CONFIG			_T("config")
#define INI_CFG_CLIENT_CONFIG_DOMAIN	_T("domain")
#define INI_CFG_CLIENT_CONFIG_SERVER	_T("server")
#define	INI_CFG_CLIENT_CONFIG_PORT		_T("port")
#define	INI_CFG_CLIENT_LOGIN			_T("Login")
#define	INI_CFG_CLIENT_LOGIN_SERVER		_T("server")
#define	INI_CFG_CLIENT_LOGIN_PORT		_T("port")
#define	INI_CFG_CLIENT_LOGIN_REMPSW		_T("remember_pwd")
#define INI_CFG_CLIENT_LOGIN_USER		_T("user")
#define INI_CFG_CLIENT_LOGIN_PASSWORD	_T("pwd")
#define	INI_CFG_CLIENT_LOGIN_AUTOSTART	_T("auto_start")
#define	INI_CFG_CLIENT_LOGIN_FIRST		_T("first_login")
#define	INI_CFG_CLIENT_SHOW_INIT		_T("show_init")
#define INI_CFG_CLIENT_LOGIN_ERRCODE	_T("ErrorCode")
#define INI_CFG_CLIENT_LOGIN_FIRSTEN	_T("first_en_login")
#define INI_CFG_CLIENT_LOGIN_FIRSTDE	_T("first_de_login")
#define INI_CFG_CLIENT_LOGIN_FIRSTFR	_T("first_fr_login")
#define INI_CFG_CLIENT_LOGIN_FIRSTPT	_T("first_pt_login")
#define INI_CFG_CLIENT_NETWORK			_T("NetWork")
#define INI_CFG_CLIENT_NETWORK_P2P		_T("use_p2p")			
#define INI_CFG_CLIENT_NETWORK_DOWNPATH	_T("folder")
#define INI_CFG_CLIENT_GENERAL			_T("General")
#define INI_CFG_CLIENT_GENERAL_0		_T("General_0")
#define INI_CFG_CLIENT_GENERAL_1		_T("General_1")
#define INI_CFG_CLIENT_GENERAL_2		_T("General_2")
#define INI_CFG_CLIENT_GENERAL_3		_T("General_3")
#define INI_CFG_CLIENT_LOCALE			_T("Locale")
#define INI_CFG_CLIENT_LOCALE_COUNT		_T("Count")
#define INI_CFG_CLIENT_LOCALE_LANGUAGE	_T("Language")
#define INI_CFG_CLIENT_MULTILANGUAGE	_T("MultiLang")
#define INI_CFG_CLIENT_MULTILANGABBR	_T("MultiLangAbbr")
#define INI_CFG_CLIENT_INGAME			_T("InGameShortcut")
#define INI_CFG_CLIENT_INGAME_FUNCTION	_T("Function")
#define INI_CFG_CLIENT_INGAME_SHORTCUT	_T("Shortcut")
#define INI_CFG_CLIENT_NOTIFY			_T("Notify")
#define INI_CFG_CLIENT_NOTIFY_ONCLOSE	_T("notifyonclose")
#define INI_CFG_CLIENT_NOTIFY_DRAGGAME	_T("DragNotification")
#define	INI_CFG_CLIENT_INGAMEDLL		_T("InGameDll")
#define INI_CFG_CLIENT_INGAMEDLL_COUNT	_T("NotificationCount")
#define INI_CFG_CLIENT_INGAMEDLL_LIST	_T("Notification")
#define INI_CFG_CLIENT_INGAMEDLL_WINDOW	_T("DisplayInWindowMode")
#define INI_CFG_CLIENT_UPDATE			_T("CoreVersionURL")
#define INI_CFG_CLIENT_UPDATE_SEEDURL	_T("URL")
#define INI_CFG_CLIENT_UPDATE_PARTNER	_T("PartnerName")
#define INI_CFG_CLIENT_UPDATE_PARTNERVER	_T("PandoVersion")
#define	INI_CFG_CLIENT_TIMER			_T("LocalTimer")
#define INI_CFG_CLIENT_PATCHER			_T("Launcher")
#define INI_CFG_CLIENT_PATCHER_NAME		_T("AppName")
#define INI_CFG_CLIENT_VERSION			_T("LocalVersion")
#define INI_CFG_CLIENT_VERSION_TEXT		_T("build")
#define INI_CFG_CLIENT_WINDOWSIZE		_T("WindowSize")
#define INI_CFG_CLIENT_WINDOWSIZE_X		_T("x")
#define INI_CFG_CLIENT_WINDOWSIZE_Y		_T("y")
#define INI_CFG_CLIENT_WINDOWSIZE_FULL	_T("FullSize")
#define INI_CFG_VERSION_INFO			_T("versioninfo")
#define INI_CFG_VERSION_INFO_VERSION	_T("version")
#define INI_CFG_VERSION_INFO_SEEDURL	_T("url")
#define INI_CFG_ERRORREPORT_TEMP        _T("ErrRepTemp")
#define INI_CFG_ERRORREPORT_TIME        _T("time")
#define INI_CFG_ERRORREPORT_TOTAL       _T("total")
#define INI_CFG_UPDATE					_T("Update")
#define INI_CFG_UPDATE_SILENTUPDATE		_T("SilentUpdate")
#define INI_CFG_DEVTOOLS				_T("DevTools")
#define INI_CFG_NOTIFY                  _T("notification")
#define INI_CFG_NOTIFY_FRIEND_REQ		_T("friend_requests")
#define INI_CFG_NOTIFY_DWONLOAD_PRO     _T("download_progress")
#define INI_CFG_NOTIFY_FRIEND_ACTIVITY  _T("friend_activity")
#define INI_CFG_NOTIFY_NEW_MESSAGE      _T("new_messages")
#define INI_CFG_NOTIFY_CHAT_MESSAGE     _T("chat_messages")
#define INI_CFG_NOTIFY_PLAY_SOUND       _T("play_sound")
#define INI_CFG_CHAT					_T("chat")
#define INI_CFG_CHAT_OPEN_IN_NEW_WND	_T("chat_open_in_new_window")
#define INI_CFG_CHAT_DISFLASH_NEW_MSG	_T("chat_disable_flashing_new_message")
#define INI_CFG_USER_PRIVILEGES			_T("UserPrivileges")
#define	INI_CFG_ADMIN_PRIVILEGES		_T("AdminPrivileges")
#define	INI_CFG_TRACKING				_T("Tracking")

//content in register.ini
#define  INI_REGISTER_FROM				_T("registered_from")
#define  INI_REGISTER_FROM_FLAG			_T("registered")
#define  INI_REGISTER_FROM_TRANSID		_T("trans_id")
#define  INI_REGISTER_FROM_OFFERID		_T("offer_id")
#define  INI_REGISTER_FROM_VENDORID		_T("vendor_id")
#define  INI_REGISTER_FROM_LANG			_T("region")
#define  INI_REGISTER_FROM_GAMEABBR		_T("game_abbr")

//command names
#define CMD_UPDATE_CLIENT	_T("noupdate")
#define CMD_AUTO_RUN	_T("/autorun")
#define CMD_GAME_CUSTOM	_T("gamecustom")
//reg items
#define REG_ITEM_SGI				_T("SOFTWARE\\Perfect World Entertainment\\Core")

//file paths
#define FILEPATH_UICONFIG		_T("themes\\sonic.xml_")
#define FILEPATH_UI				_T("themes\\sonic\\")
#define FILEPATH_DEFAULT_DOWNLOAD	_T("Perfect World Entertainment\\")
#define FILEPATH_ARCTEMP_DOWNLOAD	_T("ArcTemp\\")
#define APP_PATH                _T("Application Data\\Arc\\")
#define OVERLAY_FILECONFIG       _T("themes\\overlay.xml_")
#define PDL_CONFIG_DIR			_T("pdlconfig")
//global event object names
#define EVENT_SYNCHRONIZE_GAME_LAUNCHING	_T("COREOverlay")
#define EVENT_LAUNCHFAIL			        _T("Global\\LaunchFailedEvent")

//urls
#define URL_DEFAULT_SEEDURL		_T("http://cc-ns.perfectworld.com/updates/arcversionseed.xml")

//alert items
#define ALERT_ITEM_NOTIFICATION        _T("notification")
#define ALERT_ITEM_GAME_DC             _T("GameDownloadComplete")
#define ALERT_ITEM_PDL                 _T("PDL")
#define ALERT_ITEM_EMS                 _T("EMS")

//user defined messages
#define ICON_MAIN_TRAY            1016
#define WM_NOTIFY_MAIN_TRAY       WM_USER+102
#define WM_UPDATE_DONE            WM_USER+111
#define WM_UPDATE_USERINFO		  WM_USER+121
#define WM_XMPP_NOTIFY_MSG        WM_USER+122
#define WM_CORE_LOGIN_SUCCESS     WM_USER+123
#define WM_CORE_LOGIN_FAIL        WM_USER+124
#define WM_IM_TOKEN_FAILED        WM_USER+125
#define WM_IM_TOKEN_SUCCESS       WM_USER+126
#define WM_GAME_TOKEN_FAIL        WM_USER+127
#define WM_GAME_TOKEN_SUCCESS     WM_USER+128
#define WM_UPDATE_XMPP_STATUS     WM_USER+129
#define WM_UPDATE_PROFILE         WM_USER+130
#define WM_UPDATE_WEBSERVICE      WM_USER+151	
#define WM_SERVICE_MORE_FEED      WM_USER+152
#define WM_SELECTED_SERVICE       WM_USER+153
#define WM_WEB_NAVIGATE_URL       WM_USER+154
#define WM_LOGIN_RECONNECT        WM_USER+159
#define WM_PLUGIN_STARTCLIENT     WM_USER+160
#define WM_FOOTDIALOG_HIDE        WM_USER+161
#define WM_NOTIFICATION_IS_READ   WM_USER+202
#define WM_SHOW_GAMEINFO          WM_USER+203
#define WM_SETWEBPLAYERFULLSCREEN WM_USER+205
#define WM_ACCOUNT_BE_KICKED	  WM_USER+207
#define WM_UPLOAD_FINISHED        WM_USER+208
#define WM_SHOW_CHAT_MOVE_TAB     WM_USER+209
#define WM_UPDATE_CHAT_STATUS     WM_USER+210
#define WM_SHOW_SUB_WINDOW        WM_USER+211
#define WM_HANDLE_CHAT_MSG        WM_USER+212
#define WM_UPDATE_PROC            WM_USER+213
#define WM_FOOTPOP_DELETE_ITEM    WM_USER+214
#define WM_SUBMIT_FEEDBACK_FAIL   WM_USER+215
#define WM_CHECKFORUPDATE         WM_USER+216
#define WM_SHOWFOREGROUND         WM_USER+217
#define WM_WEBPAGE_LOAD_START     WM_USER+218
#define WM_WEBPAGE_LOAD_END       WM_USER+219
#define WM_BROWSER_NAVIGATE       WM_USER+220
#define WM_LOGIN_EMERGENT_NOTIFY  WM_USER+221
#define WM_AUTOINSTALL_GAME		WM_USER+222
#define WM_DISPLAYNAME_DONE_CHAT		WM_USER+223
#define WM_DISPLAYNAME_DONE_FRIEND_REQ	WM_USER+224
#define WM_DISPLAYNAME_SET_SUCCESS		WM_USER+225
#define WM_DISPLAYNAME_SET_FAIL			WM_USER+226
#define WM_USERINFO_FROM_WEB_DONE		WM_USER+227
#define WM_CHAT_PANNEL_RESIZE		    WM_USER+228
#define WM_WEBPAGE_HIDEWND				WM_USER+229
#define WM_UPDATE_SESSIONID				WM_USER+230
#define WM_HANDLE_PIPE_REQ				WM_USER+231
#define WM_VERSION_UPDATE               WM_USER+232
#define WM_WEBPAGE_SETRECT              WM_USER+233
#define WM_UPDATE_GAMEINFO				WM_USER+234
#define WM_UPDATE_GAMELIST				WM_USER+235
#define WM_BROWSER_DESTROY              WM_USER+236   
#define WM_GOTOLOGINDIALOG              WM_USER+237
#define WM_LOGINCLIENT                  WM_USER+238
#define WM_URL_GAME						WM_USER+239
#define WM_UPDATE_AVATAR				WM_USER+240
#define WM_UPDATE_SHOPPINGCART			WM_USER+241
#define WM_RECEIVE_SNSMESSAGE           WM_USER+242
#define WM_GAME_READY_TOPLAY			WM_USER+243
#define WM_PLAY_GAME                    WM_USER+244
#define WM_UPDATE_NOTIFY_CENTER         WM_USER+245
#define WM_RESPONSE_FR_SUCCEED			WM_USER+246
#define WM_RESPONSE_FR_FAIL				WM_USER+247
#define WM_ADD_ALERT_ITEM				WM_USER+248
#define WM_EMS_UPDATE			        WM_USER+249
#define WM_EMS_ARRIVE			        WM_USER+250
#define WM_UPDATE_PROMOTION				WM_USER+251
#define WM_UPDATE_BKGND					WM_USER+252
#define WM_POST_UIITEM_MESSAGE			WM_USER+253
#define WM_GOTOWEBPAGE					WM_USER+254
#define WM_SHOWTIPWINDOW				WM_USER+255
#define WM_SHOWPROMOTIONWINDOW			WM_USER+256
#define WM_ARC_DEFENDER	                WM_USER+257
#define WM_BROWSER_DELCOOKIE            WM_USER+258
#define WM_INSTALL_PROGRESS				WM_USER+259
#define WM_HANDLE_IM_PRESENCE			WM_USER+300

#define WM_INGAME_ISWINDOWED               WM_USER+0x0fff
#define WM_INGAME_TRACK_USE_OVERLAY        WM_USER+0x1000
#define WM_INGAME_TRACK_CLICK_NOTIFICATION WM_USER+0x1001 //message id should be incremented.

#define WM_OPENFILEWND                  WM_USER+0x2014 //overlay open filedialog
#define WM_CHANGESIZDE                  WM_USER+0X2015 //overlay change size 
//web service parameters
#define CRASHREPORT		_T("crashreport")
#define I_CRASHREPORT	_T("i_crashreport")

//common string
#define SERVICE_INTERNAL_FLAG           _T("/ArcInternal")
#define TSTRING_ABBR_LANGUAGE_EN		_T("en")
#define TSTRING_ABBR_LANGUAGE_DE		_T("de")
#define TSTRING_ABBR_LANGUAGE_FR		_T("fr")
#define TSTRING_ABBR_LANGUAGE_PT		_T("pt")
#define STRING_ABBR_LANGUAGE_EN			"en"
#define STRING_ABBR_LANGUAGE_DE			"de"
#define STRING_ABBR_LANGUAGE_FR			"fr"
#define STRING_ABBR_LANGUAGE_PT			"pt"

#define DECLARE_APPID	const TCHAR APP_ID[] = _T("PERFECTWORLD.ARC.2.0");\
	typedef HRESULT (WINAPI *PROCSETAPPID)(PCWSTR);
#define ARCCLIENT_FILEMAP	_T("ArcClient")
#define ARCUPDATE_FILEMAP	_T("ArcUpdate")
#define JUMPLIST_COMMAND_START	0

//#define LOSHORT(l)           ((short)(((LONG_PTR)(l)) & 0xffff))
//#define HISHORT(l)           ((short)((((LONG_PTR)(l)) >> 16) & 0xffff))
