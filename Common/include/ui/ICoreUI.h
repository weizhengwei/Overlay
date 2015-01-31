#pragma once

//////////////////////////////////////////////////////////////////////////
// file:	ICoreUI.h
// author:	Sonic Guan
// date:	2012-06-29
// intro:	ICoreUI is a lite UI engine with which you could simply accomplish UI
//			with XML scripts. XML is used to load image resource and generate pre-defined
//			static UI elements, you can also create UI elements dynamically with ISonicUI.h and 
//			CoreWidght.h to meet different needs.
//			necessary files: ICoreUI.h, ISonicUI.h, CoreUI.lib, CoreUI.dll
//			optional files: WTL, CoreWidght.h
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//			XML attributes
//////////////////////////////////////////////////////////////////////////
//			image.item
// id:		global unique id
// file:	relative path to where CoreUI.dll exists
// colorkey:the pixel with color equals to colorkey will be transparent
// tile:	tile state, a positive number n indicates the source image is horizontally tiled into n blocks
//			while a negative number means it's vertically tiled 
//////////////////////////////////////////////////////////////////////////
//			MainWnd
// id:		global unique id
// x, y:	in screen coordinates if the MainWnd is a popup window, otherwise client relative.
// width:	width of the window
// height:	height of the window
// image:	background image, auto tiled
// min:		id of an image, used as skin of the minimizing box, which will not appear without this attribute
// max:		id of an image, used as skin of the maximizeing box, which will not appear without this attribute
// restore: id of an image, used as skin of the restoring box, which will not appear without this attribute
// close:	id of an image, used as skin of the closing button, which will not appear without this attribute
// tileheight:	specify the title region of window
// skintype:	1 for normal window without layer, 2 for layered window, 3 for alpha-per-pixel layered window, to implement shadow effect
// customdraw:	specify if the window requirs custom render, if set to zero, the window will not receive WM_PAINT message, otherwize it will handle the WM_PAINT
// closeeffect:	1 for fade out, 2 for shutter, 3 for slide, 4 for aero effect
//////////////////////////////////////////////////////////////////////////
//			SonicString
// id:		global unique id
// x, y:	client relative coordinates
// width:	width of the string
// height:	height of the string
// arg%d:	specify the dynamic arguments when formating the string. %d means integer numbers. it's one to one correspondence between the
//			arguments list and format-control string.
// image:	id of an image, with this attribute SonicString can be specified as a simple image holder with no text value.
//////////////////////////////////////////////////////////////////////////
//			SonicPaint
// id:		global unique id
// x, y:	client relative coordinates
// width:	width of the paint
// height:	height of the paint
// withmemory: the paint could be created as virtual container of ISonic elements, or be created with memory and act as a canvas
//////////////////////////////////////////////////////////////////////////
//			CoreListBox
// id:		global unique id
// x, y:	client relative coordinates
// width:	width of the listbox
// height:	height of the listbox
// bordercolor: border color
// bgcolor:	background color
// selectcolor: background color when a item is selected and hilighted
// hilightselectedtext: the text will no be hilighted when selected if this attribute is set to "no"
//////////////////////////////////////////////////////////////////////////
//       edit
// id:		global unique id
// x, y:	client relative coordinates
// width:	width of the listbox
// height:	height of the listbox
// bordercolor: border color
// bgcolor:	background color
// textcolor: text color
// style:    edit style // ES_READONLY|ES_PASSWORD|ES_CENTER|ES_LEFT|ES_RIGHT

#include "ISonicUI.h"
using namespace sonic_ui;

#include "atlbase.h"
#include "atlwin.h"

#ifdef COREUI_EXPORTS
#define COREUI_API _declspec(dllexport)
#else
#define COREUI_API
#endif

// application window or dialog responding this message with return value 1 to cancel the closing procedure, or return 0 to continue closing.
#define WM_PROMPTCLOSE			     WM_USER + 0x1112
#define WM_WINDPOSCHANGED            WM_USER + 0x1113

enum enCoreObjectType
{
	CoreTypeNone = 0,
	CoreTypeSonicImage,
	CoreTypeSonicString,
	CoreTypeSonicPaint,
	CoreTypeSonicAnimation,
	CoreTypeSonicTextScrollBar,
	CoreTypeSonicProgressBar,
	CoreTypeSonicTab,
	CoreTypeSonicMenuBar,
	CoreTypeSonic,
	CoreTypeWindow,
	CoreTypeCoreListBox,
	CoreTypeCoreComboBox,
	CoreTypeCoreEdit,
	CoreTypeCoreRichEdit,
	CoreTypeCoreMenu,
	CoreTypeCoreDriveList,
	CoreTypeContrl,
};

typedef struct tagCoreObjDescriptor
{
	DWORD dwObjType;
	union
	{
		CWindow * pWnd;
		ISonicBase * pBase;
	};
}CORE_OBJ_DESCRIPTOR;
typedef void (*MenuCallback)(void *);

enum enInitResult
{
	irOK = 0,
	irXMLNotExists,
	irParseXMLFailed,
	irImagesFromZipFailed,
};

class ICoreUI
{
public:
	// brief:	this function must be called before any other interfaces.
	//			lpszCfgPath is the path of a pre-define xml file, which defined the UI appearance of windows and controls
	//			if pOSUICallback is TRUE, CoreUI engine will work in off-screen rendering mode(not reversible), 
	//			all windows attached by ICoreUI will be set to hidden and render all GUI buffer with the callback interface pUICallback. 
	//			You must relay all input messages (from WM_MOUSEFIRST to WM_MOUSELAST, and from WM_KEYFIRST to WM_KEYLAST, and also WM_MOUSELEAVE if needed) 
	//			to the panel window manually (IOffscreenUICallBack::GetPanel).
	// return:	a type of enInitResult
	// remarks:	it is highly recommended that using off-screen rendering mode in a separate process, otherwise the hidden window (in fact, it is visible but 
	//			completely transparent) may grab focus from the foreground window while receiving some messages (WM_LBUTTONDOWN, etc). 
	//			subsequent created windows should be children of the panel window.
	virtual int Initialize(LPCTSTR lpszCfgPath, IOffscreenUICallBack * pOSUICallback = NULL) = 0;
	virtual void Uninitialize() = 0;
	virtual ISonicImage * GetImage(LPCTSTR szID) = 0;
	virtual BOOL AttachWindow(HWND hWnd, LPCTSTR szID) = 0;
	virtual BOOL GetObjDescriptorByID(HWND hWnd, LPCTSTR szID, CORE_OBJ_DESCRIPTOR & objDesp) = 0;
	virtual CWindow * FromHandle(HWND hWnd) = 0;
	virtual BOOL CreateCtrlForXml(HWND hParent,LPCTSTR szID,CWindow*pWindow) = 0;
	virtual void *CreateCoreMenu(LPCTSTR szID) = 0;
	virtual DWORD CreateCoreMenu(HWND hWnd,LPCTSTR szID,POINT point,MenuCallback pMenuCallback = NULL,UINT uFlags = TPM_VERPOSANIMATION ) = 0;

	template<class T>
	T * GetObjByID(HWND hWnd, LPCTSTR szID)
	{
		CORE_OBJ_DESCRIPTOR objDesc;
		if(!GetObjDescriptorByID(hWnd, szID, objDesc))
		{
			return NULL;
		}
		if(objDesc.dwObjType > CoreTypeNone && objDesc.dwObjType < CoreTypeSonic)
		{
			return (T *)objDesc.pBase;
		}
		else if(objDesc.dwObjType >= CoreTypeWindow && objDesc.dwObjType < CoreTypeContrl)
		{
			return (T *)objDesc.pWnd;
		}
		return NULL;
	}
};

extern "C" COREUI_API ICoreUI * GetCoreUI();
typedef ICoreUI * (*FUNC_GET_CORE_UI)();