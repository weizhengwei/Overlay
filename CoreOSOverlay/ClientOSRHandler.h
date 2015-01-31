#pragma once
#include "cef/cef_client.h"
#include "cef/cef_life_span_handler.h"
#include "cef/cef_load_handler.h"
#include "cef/cef_render_handler.h"
#include "cef/cef_display_handler.h"
#include "cef/cef_keyboard_handler.h"
#include "cef/cef_jsdialog_handler.h"
#include "cef/cef_request_handler.h"
#include "cef/cef_app.h"
#include "util.h"
#include "tstring/tstring.h"
using String::_tstring;
#pragma  comment(lib,"libcef.lib")
#pragma  comment(lib,"libcef_dll_wrapper.lib")
class ClientOSRHandler;
// Class holding pointers for the client plugin window.
#define WM_COREBROWSERMESSAGE			WM_USER + 0x8222
#define WM_COREBROWSEREVENTMESSAGE		WM_USER + 0x8223

enum
{
	BROWSER_HANDLER_BEFORECREATE = 1,
	BROWSER_HANDLER_POPUPCHANGE,
	BROWSER_HANDLER_LOAD,
	BROWSER_HANDLER_TITLECHANGE,
	BROWSER_HANDLER_AFTERCREATE,
	BROWSER_HANDLER_ADDRESSCHANGE,
	BROWSER_HANDLER_CURSORCHANGE,
	BROWSER_HANDLER_HANDLEPAINT,
	BROWSER_HANDLER_CLOSE,
};
typedef struct tagBrowserMsgData
{
	int  msgid;
	int  type;
	bool show;
	ClientOSRHandler *pHandler;
	ClientOSRHandler *pParentHandler;
	const void* buffer;
	CefRect rect;
	CefBrowser *browser;
	bool loading;
	TCHAR szTitle[512];
	HCURSOR hCursor;
	tagBrowserMsgData()
	{
		memset(this,0,sizeof(tagBrowserMsgData));
	}
}BROWSERMSGDATA,*LPBROWSERMSGDATA;

class ClientOSRHandler : public CefClient
	,public CefLifeSpanHandler
	,public CefLoadHandler
	,public CefRequestHandler
	,public CefDisplayHandler
	,public CefKeyboardHandler
	,public CefRenderHandler
	,public CefJSDialogHandler
	
{
public:
	ClientOSRHandler(void);
	~ClientOSRHandler(void);


	// CefClient methods
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
		return this;
	}

	virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() OVERRIDE {
		return this;
	}

	// Called before a new popup window is created. The |parentBrowser| parameter
	// will point to the parent browser window. The |popupFeatures| parameter will
	// contain information about the style of popup window requested. Return false
	// to have the framework create the new popup window based on the parameters
	// in |windowInfo|. Return true to cancel creation of the popup window. By
	// default, a newly created popup window will have the same client and
	// settings as the parent window. To change the client for the new window
	// modify the object that |client| points to. To change the settings for the
	// new window modify the |settings| structure.
	///
	/*--cef(optional_param=url)--*/
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		bool* no_javascript_access)  ;

	///
	// Called after a new window is created.
	///
	/*--cef()--*/
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser);

	// Called when the browser begins loading a frame. The |frame| value will
	// never be empty -- call the IsMain() method to check if this frame is the
	// main frame. Multiple frames may be loading at the same time. Sub-frames may
	// start or continue loading after the main frame load has ended. This method
	// may not be called for a particular frame if the load request for that frame
	// fails.
	///
	/*--cef()--*/
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame) ;

	///
	// Called when the browser is done loading a frame. The |frame| value will
	// never be empty -- call the IsMain() method to check if this frame is the
	// main frame. Multiple frames may be loading at the same time. Sub-frames may
	// start or continue loading after the main frame load has ended. This method
	// will always be called for all frames irrespective of whether the request
	// completes successfully.
	///
	/*--cef()--*/
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int httpStatusCode) ;


	///
	// Called when a frame's address has changed.
	///
	/*--cef()--*/
	virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& url);
	///
	// Called when the page title changes.
	///
	/*--cef(optional_param=title)--*/
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
		const CefString& title) ;


	virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
		const CefKeyEvent& event,
		CefEventHandle os_event,
		bool* is_keyboard_shortcut);
	///
	// Called to retrieve the view rectangle which is relative to screen
	// coordinates. Return true if the rectangle was provided.
	///
	/*--cef()--*/
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser,
		CefRect& rect) ;

	///
	// Called to retrieve the simulated screen rectangle. Return true if the
	// rectangle was provided.
	///
	/*--cef()--*/
	virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
		CefRect& rect) ;
	///
	// Called to retrieve the translation from view coordinates to actual screen
	// coordinates. Return true if the screen coordinates were provided.
	///
	/*--cef()--*/
	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
		int viewX,
		int viewY,
		int& screenX,
		int& screenY){return false;}

	///
	// Called when the browser wants to show or hide the popup widget. The popup
	// should be shown if |show| is true and hidden if |show| is false.
	///
	/*--cef()--*/
	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser,
		bool show) ;

	///
	// Called when the browser wants to move or resize the popup widget. |rect|
	// contains the new location and size.
	///
	/*--cef()--*/
	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser,
		const CefRect& rect) ;

	///
	// Called when an element should be painted. |type| indicates whether the
	// element is the view or the popup widget. |buffer| contains the pixel data
	// for the whole image. |dirtyRects| contains the set of rectangles that need
	// to be repainted. On Windows |buffer| will be width*height*4 bytes in size
	// and represents a BGRA image with an upper-left origin. The
	// CefBrowserSettings.animation_frame_rate value controls the rate at which
	// this method is called.
	///
	/*--cef()--*/
	virtual void OnPaint(CefRefPtr<CefBrowser> browser,
		PaintElementType type,
		const RectList& dirtyRects,
		const void* buffer,
		int width, int height) ;

	///
	// Called when the browser window's cursor has changed.
	///
	/*--cef()--*/
	virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
		CefCursorHandle cursor);

	///
	// Called just before a window is closed. If this is a modal window and a
	// custom modal loop implementation was provided in RunModal() this callback
	// should be used to exit the custom modal loop.
	///
	/*--cef()--*/
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) ;


	///
	// Called  to run a JavaScript alert message. Return false to display the
	// default alert or true if you displayed a custom alert.
	///
	/*--cef(optional_param=message)--*/
	virtual bool OnJSAlert(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& message) { return true; }

	///
	// Called to run a JavaScript confirm request. Return false to display the
	// default alert or true if you displayed a custom alert. If you handled the
	// alert set |retval| to true if the user accepted the confirmation.
	///
	/*--cef(optional_param=message)--*/
	virtual bool OnJSConfirm(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& message,
		bool& retval) {
			retval = true;
			return true;
	}

	///
	// Called to run a JavaScript prompt request. Return false to display the
	// default prompt or true if you displayed a custom prompt. If you handled
	// the prompt set |retval| to true if the user accepted the prompt and request
	// and |result| to the resulting value.
	///
	/*--cef(optional_param=message,optional_param=defaultValue)--*/
	virtual bool OnJSPrompt(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& message,
		const CefString& defaultValue,
		bool& retval,
		CefString& result) { return true; }


	
public:
	CefRefPtr<CefBrowser> GetBrowser(){return m_Browser;}
	void SetBrowserSize(PaintElementType type, int width, int height);
	CefRect GetPopupCefRect(){ return popup_rect_;}
	void Invalidate(CefRect *rect);
	void SetWnd(HWND hwnd){m_BrowserHwnd = hwnd;}
	void GetTitle(_tstring & strTitle){
		Lock();
		strTitle = m_strTitle;
		Unlock();
	}
	
protected:
	void SetRGB(const void* src, int width, int height, bool view);
	void SetBufferSize(int width, int height, bool view);
	static void ConvertToRGB(const unsigned char* src, unsigned char* dst,
		int width, int height);
	
	
// 	void EnableGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
// 	// Disable GL.
//	void DisableGL(HWND hWnd, HDC hDC, HGLRC hRC);

	// The child browser window
	CefRefPtr<CefBrowser> m_Browser;
	// The child browser window handle
	CefWindowHandle m_BrowserHwnd;
	_tstring m_strTitle;
	
private:
	CefRect popup_rect_;
	int m_nWidth;
	int m_nHeight;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(ClientOSRHandler);
	// Include the default locking implementation.
	IMPLEMENT_LOCKING(ClientOSRHandler);

};
