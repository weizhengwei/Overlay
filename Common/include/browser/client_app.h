#include "cef/cef_app.h"

class ClientApp : public CefApp,
	public CefBrowserProcessHandler,
	public CefRenderProcessHandler 
{
public:
	 ClientApp();
	 ~ClientApp();
	 IMPLEMENT_REFCOUNTING(ClientApp);
};