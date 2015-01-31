#include "stdafx.h"
#include "browser/BrowserImplbase.h"
#include "cef/cef_app.h"
#include "data/UIString.h"
#include "data/DataPool.h"
#include "cef/cef_runnable.h"

CBrowserImplbase::CBrowserImplbase()
{

}
CBrowserImplbase::~CBrowserImplbase()
{

}

BOOL CBrowserImplbase::BrowserInit(HINSTANCE hInstance,LPCTSTR lpstruseagent,CefRefPtr<CefApp> app,CefSettings & settings)
{
	//CefSettings settings ;
	CefMainArgs main_args(hInstance);

	settings.multi_threaded_message_loop = true;
	settings.log_severity = LOGSEVERITY_DISABLE ;

	CefString(&settings.accept_language).FromString(_tstring(L"en-US,en").toUTF8().c_str());
	CefString(&settings.user_agent).FromString(_tstring(lpstruseagent).toUTF8().c_str());

	BOOL bInit =  CefInitialize(main_args,settings, app.get());
	_tstring strPath = theDataPool.GetBaseDir() + _T("plugins\\NPSWF32.dll");
	CefAddWebPluginPath(CefString(strPath.toNarrowString().c_str()));
	return bInit;
}


void CBrowserImplbase::CursorInit(HINSTANCE hInstance)
{
	//LoadCursor(hInstance,IDC_ARROW);
	m_CursorMap[LoadCursor(hInstance,IDC_ARROW)] = 32512;
	m_CursorMap[LoadCursor(hInstance,IDC_IBEAM)] = 32513;
	m_CursorMap[LoadCursor(hInstance,IDC_WAIT)] = 32514;
	m_CursorMap[LoadCursor(hInstance,IDC_CROSS)] = 32515;
	m_CursorMap[LoadCursor(hInstance,IDC_UPARROW)] = 32516;
	m_CursorMap[LoadCursor(hInstance,IDC_SIZE)] = 32640;
	m_CursorMap[LoadCursor(hInstance,IDC_ICON)] = 32641;
	m_CursorMap[LoadCursor(hInstance,IDC_SIZENWSE)] = 32642;
	m_CursorMap[LoadCursor(hInstance,IDC_SIZENESW)] = 32643;
	m_CursorMap[LoadCursor(hInstance,IDC_SIZEWE)] = 32644;
	m_CursorMap[LoadCursor(hInstance,IDC_SIZENS)] = 32645;
	m_CursorMap[LoadCursor(hInstance,IDC_SIZEALL)] = 32646;
	m_CursorMap[LoadCursor(hInstance,IDC_NO)] = 32648;

	m_CursorMap[LoadCursor(hInstance,IDC_HAND)] = 32649;
	m_CursorMap[LoadCursor(hInstance,IDC_APPSTARTING)] = 32650;
	m_CursorMap[LoadCursor(hInstance,IDC_HELP)] = 32651;
}

bool CBrowserImplbase::isKeyDown(WPARAM wparam) {
	return (GetKeyState(wparam) & 0x8000) != 0;
}
int CBrowserImplbase::GetCefMouseModifiers(WPARAM wparam) {
	int modifiers = 0;
	if (wparam & MK_CONTROL)
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (wparam & MK_SHIFT)
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (isKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;
	if (wparam & MK_LBUTTON)
		modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
	if (wparam & MK_MBUTTON)
		modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
	if (wparam & MK_RBUTTON)
		modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

	// Low bit set from GetKeyState indicates "toggled".
	if (::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if (::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;
	return modifiers;
}
int CBrowserImplbase::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {
	int modifiers = 0;
	if (isKeyDown(VK_SHIFT))
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (isKeyDown(VK_CONTROL))
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (isKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;

	// Low bit set from GetKeyState indicates "toggled".
	if (::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if (::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;

	switch (wparam) {
  case VK_RETURN:
	  if ((lparam >> 16) & KF_EXTENDED)
		  modifiers |= EVENTFLAG_IS_KEY_PAD;
	  break;
  case VK_INSERT:
  case VK_DELETE:
  case VK_HOME:
  case VK_END:
  case VK_PRIOR:
  case VK_NEXT:
  case VK_UP:
  case VK_DOWN:
  case VK_LEFT:
  case VK_RIGHT:
	  if (!((lparam >> 16) & KF_EXTENDED))
		  modifiers |= EVENTFLAG_IS_KEY_PAD;
	  break;
  case VK_NUMLOCK:
  case VK_NUMPAD0:
  case VK_NUMPAD1:
  case VK_NUMPAD2:
  case VK_NUMPAD3:
  case VK_NUMPAD4:
  case VK_NUMPAD5:
  case VK_NUMPAD6:
  case VK_NUMPAD7:
  case VK_NUMPAD8:
  case VK_NUMPAD9:
  case VK_DIVIDE:
  case VK_MULTIPLY:
  case VK_SUBTRACT:
  case VK_ADD:
  case VK_DECIMAL:
  case VK_CLEAR:
	  modifiers |= EVENTFLAG_IS_KEY_PAD;
	  break;
  case VK_SHIFT:
	  if (isKeyDown(VK_LSHIFT))
		  modifiers |= EVENTFLAG_IS_LEFT;
	  else if (isKeyDown(VK_RSHIFT))
		  modifiers |= EVENTFLAG_IS_RIGHT;
	  break;
  case VK_CONTROL:
	  if (isKeyDown(VK_LCONTROL))
		  modifiers |= EVENTFLAG_IS_LEFT;
	  else if (isKeyDown(VK_RCONTROL))
		  modifiers |= EVENTFLAG_IS_RIGHT;
	  break;
  case VK_MENU:
	  if (isKeyDown(VK_LMENU))
		  modifiers |= EVENTFLAG_IS_LEFT;
	  else if (isKeyDown(VK_RMENU))
		  modifiers |= EVENTFLAG_IS_RIGHT;
	  break;
  case VK_LWIN:
	  modifiers |= EVENTFLAG_IS_LEFT;
	  break;
  case VK_RWIN:
	  modifiers |= EVENTFLAG_IS_RIGHT;
	  break;
	}
	return modifiers;
}
void CBrowserImplbase::SetBrowserEvent(CefRefPtr<CefBrowser> Browser,UINT message, WPARAM wParam,LPARAM lParam)
{
	bool mouseRotation = false;
	bool mouseTracking = false;
	static unsigned long  gLastClickCount = 0;
	switch(message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		if (wParam & MK_SHIFT) {
			// Start rotation effect.
			mouseRotation = true;
		} 
		else 
		{
			if (Browser) 
			{
				CefBrowserHost::MouseButtonType lbtntype = MBT_LEFT;
				gLastClickCount = 1;
				if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK)
				{

					lbtntype = MBT_RIGHT;
				}
				if (message == WM_RBUTTONDBLCLK || message == WM_LBUTTONDBLCLK)
				{
					gLastClickCount = 2;
				}

				// 				m_Browser->GetHost()->SendMouseClickEvent(LOWORD(lParam), HIWORD(lParam),
				// 					(CefBrowser::MouseButtonType)lbtntype, false, 1);
				CefMouseEvent mouse_event;
				mouse_event.x = GET_X_LPARAM(lParam);
				mouse_event.y = GET_Y_LPARAM(lParam);

				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				Browser->GetHost()->SendMouseClickEvent(mouse_event, lbtntype, false,
					gLastClickCount);
			}
		}
		if (Browser)
			Browser->GetHost()->SendFocusEvent(true);
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (mouseRotation) {
			// End rotation effect.
			mouseRotation = false;
		} else {
			if (Browser) {
				CefBrowserHost::MouseButtonType lbtntype =  (message == WM_LBUTTONUP) ? MBT_LEFT : MBT_RIGHT ;
				CefMouseEvent mouse_event;
				mouse_event.x = GET_X_LPARAM(lParam);
				mouse_event.y = GET_Y_LPARAM(lParam);

				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				Browser->GetHost()->SendMouseClickEvent(mouse_event, lbtntype, true,
					gLastClickCount);
			}
		}
		break;

	case WM_MOUSEMOVE:
		if(mouseRotation) {
			// Apply rotation effect.
			// 			 curMousePos.x = LOWORD(lParam);
			// 			 curMousePos.y = HIWORD(lParam);
			// 			 lastMousePos.x = curMousePos.x;
			// 			 lastMousePos.y = curMousePos.y;
		} else {
			if (!mouseTracking) {
				// Start tracking mouse leave. Required for the WM_MOUSELEAVE event to
				// be generated.
				// 				 TRACKMOUSEEVENT tme;
				// 				 tme.cbSize = sizeof(TRACKMOUSEEVENT);
				// 				 tme.dwFlags = TME_LEAVE;
				// 				 tme.hwndTrack = hWnd;
				// 				 TrackMouseEvent(&tme);
				// 				 mouseTracking = true;
			}
			if (Browser) {
				CefMouseEvent mouse_event;
				mouse_event.x =GET_X_LPARAM(lParam);
				mouse_event.y = GET_Y_LPARAM(lParam);
				//window->ApplyPopupOffset(mouse_event.x, mouse_event.y);
				mouse_event.modifiers = GetCefMouseModifiers(wParam);
				Browser->GetHost()->SendMouseMoveEvent(mouse_event, false);
			}
		}
		break;

	case WM_MOUSELEAVE:
		if (mouseTracking) {
			// Stop tracking mouse leave.
			// 			 TRACKMOUSEEVENT tme;
			// 			 tme.cbSize = sizeof(TRACKMOUSEEVENT);
			// 			 tme.dwFlags = TME_LEAVE & TME_CANCEL;
			// 			 tme.hwndTrack = hWnd;
			// 			 TrackMouseEvent(&tme);
			// 			 mouseTracking = false;
		}
		if (Browser)
		{
			CefMouseEvent mouse_event;
			mouse_event.x = 0;
			mouse_event.y = 0;
			mouse_event.modifiers = GetCefMouseModifiers(wParam);
			Browser->GetHost()->SendMouseMoveEvent(mouse_event, true);
		}
		break;

	case WM_MOUSEWHEEL:
		if (Browser) {
			// m_Browser->SendMouseWheelEvent(LOWORD(lParam), HIWORD(lParam),0,
			//	 GET_WHEEL_DELTA_WPARAM(wParam));
			CefMouseEvent mouse_event;
			mouse_event.x = GET_X_LPARAM(lParam);
			mouse_event.y = GET_Y_LPARAM(lParam);
			int len = GET_WHEEL_DELTA_WPARAM(wParam);
			Browser->GetHost()->SendMouseWheelEvent(mouse_event,
				isKeyDown(VK_SHIFT) ? len : 0,
				!isKeyDown(VK_SHIFT) ? len : 0);
		}
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		if (Browser)
			Browser->GetHost()->SendFocusEvent(message==WM_SETFOCUS);
		break;

	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
		if(!mouseRotation) {
			if (Browser)
				Browser->GetHost()->SendCaptureLostEvent();
		}
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
	case WM_SYSCHAR:
	case WM_IME_CHAR:
		if (Browser) {
			CefKeyEvent event;
			event.windows_key_code = wParam;
			event.native_key_code = lParam;
			event.is_system_key = message == WM_SYSCHAR ||
				message == WM_SYSKEYDOWN ||
				message == WM_SYSKEYUP;

			if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
				event.type = KEYEVENT_RAWKEYDOWN;
			else if (message == WM_KEYUP || message == WM_SYSKEYUP)
				event.type = KEYEVENT_KEYUP;
			else
				event.type = KEYEVENT_CHAR;
			event.modifiers = GetCefKeyboardModifiers(wParam, lParam);
			Browser->GetHost()->SendKeyEvent(event);
		}
		break;
	}
}

void CBrowserImplbase::SetCefCookie(LPCTSTR lpstrSessionId,LPCTSTR lpstrCookieUrl,LPCTSTR lpCookieFolder)
{
	/*if (*lpstrSessionId =='\0')
	{
		return;
	}*/
	////set cookie path
	_tstring sCefCache = theDataPool.GetBaseDir() + lpCookieFolder;
	CefCookieManager::GetGlobalManager()->SetStoragePath(CefString(sCefCache.toUTF8().c_str()),true);

	_tstring scookieURL = lpstrCookieUrl;
	//parse url domain.
	_tstring sDomin = scookieURL.Left(scookieURL.rfind(L"."));
	sDomin = sDomin.Left(sDomin.rfind(L"."));
	sDomin = scookieURL.Right(scookieURL.length()-sDomin.length()-1);

	_tstring strSessionId(lpstrSessionId);
	CefCookie cookie;
	CefString(&cookie.name).FromASCII("PWRD");
	CefString(&cookie.value).FromASCII(strSessionId.toNarrowString().c_str());
	CefString(&cookie.path).FromASCII("/");
	CefString(&cookie.domain).FromASCII(sDomin.toNarrowString().c_str());


	CefPostTask(TID_IO, NewCefRunnableFunction(&CBrowserImplbase::IOT_Set, CefString(scookieURL.toNarrowString().c_str()), cookie));
}
void CBrowserImplbase::ActiveThreadWindow(HWND hParentWnd)
{
	DWORD dwArcID;
	DWORD dwCurID;
	dwArcID = GetWindowThreadProcessId(hParentWnd,NULL);
	dwCurID = GetCurrentThreadId();
	AttachThreadInput( dwCurID, dwArcID, TRUE); 
	WNDCLASSEX wc;
	memset(&wc,0,sizeof(WNDCLASSEX));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = _T("BROWSERBASE");
	RegisterClassEx(&wc);
	HWND hWnd = CreateWindow(wc.lpszClassName,NULL,WS_POPUP,CW_USEDEFAULT,0,CW_USEDEFAULT,0,NULL,NULL,wc.hInstance,NULL);
	::SetFocus(hWnd);
	::DestroyWindow(hWnd);
	::SetFocus(hParentWnd);
}

