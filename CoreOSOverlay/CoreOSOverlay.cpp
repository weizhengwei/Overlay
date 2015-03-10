#include "stdafx.h"
#include "OverlayMainWnd.h"
#include "data/DataPool.h"
#include "BrowserImpl.h"
#include "OverlayImpl.h"
#include "data\ErrorReport.h"
#include "OverlayImImpl.h"
#include "browser/client_app.h"

CAppModule _Module;

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{

	CefMainArgs main_args(hInstance);
	CefRefPtr<ClientApp> app(new ClientApp);
	int exit_code = CefExecuteProcess(main_args, app.get());
	if (exit_code >= 0)
		return exit_code;

	InitErrorReport(theDataPool.GetBaseDir().c_str(),I_CRASHREPORT);

    //::MessageBox(NULL,lpCmdLine,lpCmdLine,0);
	_tstring sFontPath = theDataPool.GetBaseDir() + _T("font\\");
	_tstring sFontFile = sFontPath + _T("*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind=::FindFirstFile(sFontFile.c_str(),&FindFileData);
	while(INVALID_HANDLE_VALUE != hFind)
	{
		if(FindFileData.cFileName[0] != '.')
		{
			sFontFile = sFontPath + FindFileData.cFileName;
			if(AddFontResourceEx(sFontFile.c_str(), FR_PRIVATE, 0) == 0)
			{
				OutputDebugString(_T("add font failed\n"));
			}
		}
		if(!FindNextFile(hFind, &FindFileData))
		{
			break;
		}
	}
	
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
	}

	_Module.Init(NULL, hInstance);
	//::MessageBox(NULL,lpCmdLine,lpCmdLine,0);

	//theDataPool.SetLangId(0, false) ;
	//initialize core ui engine
	int nLangId = GetPrivateProfileInt(INI_CFG_CLIENT_LOCALE,INI_CFG_CLIENT_LOCALE_LANGUAGE, 1,theDataPool.GetUserProfilePath().c_str());
	_tstring sUIConfigPath = theDataPool.GetBaseDir() + OVERLAY_FILECONFIG + _tstring(--nLangId);

    int nInitRes = GetCoreUI()->Initialize(sUIConfigPath.c_str(), COverlayImpl::GetInstance());
	if (irOK != nInitRes && irImagesFromZipFailed != nInitRes)
	{
		return FALSE ;
	}
	//

	//CefRefPtr<CefApp> app;
	CBrowserImpl::GetInstance()->BrowserInit(hInstance,app.get());
	CBrowserImpl::GetInstance()->CursorInit(NULL);
	TCHAR ch[256] = {0};
	if (__targv[0])
	{
		lstrcpyn(ch,__targv[0],256);
	}

	COverlayImpl::GetInstance()->SetClientHwnd((HWND)_wtoi(ch));

	// create main window
	if (GetSonicUI()->GetOffscreenUICallback())
	{
		COverlayImpl::GetInstance()->Init();
		_OverlayMainWnd.Create(NULL, CRect(0, 0, COverlayImpl::GetInstance()->GetOverlaySize().cx, COverlayImpl::GetInstance()->GetOverlaySize().cy), NULL, WS_POPUP|WS_SYSMENU);
		overlay_sm_header * pHeader = COverlayImpl::GetInstance()->GetMenMgr().GetHeader();
		if (pHeader && pHeader->bShow)
		{
			//_OverlayMainWnd.shor(SW_SHOWNA);
			COverlayImpl::GetInstance()->ShowOverlay(TRUE,FALSE);
			if (pHeader->csPage[0] != '\0')
			{
				USES_CONVERSION;
				COverlayImpl::GetInstance()->ShowBrowserMainWnd(A2T(pHeader->csPage));
			}
			
		}
		
	}
	else
	{
		_OverlayMainWnd.Create(NULL, CRect(0, 0, 1024, 768), NULL, WS_POPUP|WS_SYSMENU);
		_OverlayMainWnd.ShowWindow(SW_SHOWNA);
	//	COverlayImpl::GetInstance()->ShowBrowserMainWnd(L"http://baidu.com");
	//	_OverlayMainWnd.ShowBrowserMainWnd(_T("www.baidu.com"));
	//	CBrowserImpl::GetInstance()->SetCookie();
	}
	//_OverlayMainWnd.ShowWindow(SW_SHOWNA);
	

	// create message loop
	CMessageLoop loop;
	_Module.AddMessageLoop(&loop);
	loop.Run();
	_Module.Term();

	COverlayImImpl::GetUIChatMgr()->UnInitChatSystem();
	CBrowserImpl::GetInstance()->UnInit();
	return 0;
} 