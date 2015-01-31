#ifndef _BROWSERIMPLBASE_H
#define _BROWSERIMPLBASE_H
#include "cef/cef_app.h"
#include "cef/cef_client.h"
#include <map>
#include "data/UIString.h"
using String::_tstring;
class CBrowserImplbase 
{
public:
	CBrowserImplbase();
	~CBrowserImplbase();

	bool isKeyDown(WPARAM wparam);
	int GetCefMouseModifiers(WPARAM wparam);
	int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) ;
    void SetBrowserEvent(CefRefPtr<CefBrowser> Browser,UINT message, WPARAM wParam,LPARAM lParam);
	BOOL BrowserInit(HINSTANCE hInstance,LPCTSTR lpstruseagent,CefRefPtr<CefApp> app,CefSettings & settings);
	template <class TClientHandler>
	void CreateBrowser(LPCTSTR szUrl,TClientHandler ptr,CefWindowInfo &Info)
	{
		CefBrowserSettings browserDefaults ;

		_tstringA sCssStylePath = theDataPool.GetBaseDir() + _T("scrollbar.css") ;

		FILE* fp;
		char chCssStyle[10240] = {0} ;
		fopen_s(&fp, sCssStylePath.c_str(), ("rb"));
		if (fp)
		{
			fread_s(chCssStyle, sizeof(chCssStyle), 1, 10240, fp) ;
			fclose(fp);
		}

		_tstringA sCssStyle(chCssStyle);
		sCssStyle = sCssStyle.toBASE64();
		const char kDataUrlPrefix[] = "data:text/css;charset=utf-8;base64,";
		sCssStyle = kDataUrlPrefix + sCssStyle;
		CefString(&browserDefaults.user_style_sheet_location).FromASCII(sCssStyle.c_str());
		BOOL bcreate =CefBrowserHost::CreateBrowser(Info,static_cast<CefRefPtr<CefClient>>(ptr),szUrl,browserDefaults,NULL);
		return;
	}

	void CursorInit(HINSTANCE hInstance);
	void SetCefCookie(LPCTSTR lpstrSessionId,LPCTSTR lpstrCookieUrl,LPCTSTR lpCookieFolder);

	static void ActiveThreadWindow(HWND hParentWnd);

protected:
	std::map<HCURSOR,unsigned long> m_CursorMap;
	static void IOT_Set(const CefString& url,const CefCookie& cookie){
		CefCookieManager::GetGlobalManager()->SetCookie(url,cookie);
	}
private:
	
		
};
#endif