#pragma once

//////////////////////////////////////////////////////////////////////////
// file:	CCoreWidght.h
// author:	Sonic Guan
// date:	2012-7-2
// intro:	this is a head file with implementation of kinds of controls based on WTL and SonicUI
//			these controls are conjunction of directUI and win32 controls. you can freely add hyperlinks,
//			images,self-draw buttons or colorful texts to the controls.
// remark:	while using this file, some precompiled macros must be defined in different cases, 
//			for more details check the precomplied macros following
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// this file includes the following classes
// CCorePaintDC
// CWindowImplEx
// CWindowWithReflectorImplFixed
// CCoreScrollBar
// CWindowImplWithScrollBar
// CWindowWithDragDrop
// CBalloontip
// CCoreListBox
// CCoreComboBox
// CCoreTreeCtrl
// CCoreHeaderCtrl
// CCoreListCtrl
// CCoreMenu

//////////////////////////////////////////////////////////////////////////
// this macro must be defined if you use this head file in different PE modules, for example, 
// when you need to create one CoreUI Control in a dll but have to operate it in an exe. 
// In this situation, some stl components must be exported because different PE modules 
// maintain their own static CRT respectively(if compiled option /MT is assigned), 
// operation on a stl container created by another module may cause heap crash or unpredictable errors.
// not all the stl containers can be exported, never export the following objects:
// map, set, queue, list, deque
// for more details check: http://support.microsoft.com/kb/q168958/
#define 	CORE_CROSS_MODULE

//////////////////////////////////////////////////////////////////////////
// define this macro to work compatibly with MFC
#define		CORE_COMPATIBLE_WITH_MFC

//////////////////////////////////////////////////////////////////////////
// since wtl head files automatically merged the "WTL" name space into global name space, and lots of WTL types
// have the same names with MFC types, while conjunctively using MFC and WTL, this macro must be defined to avoid
// declaration conflicts. add WTL:: prefix to types while using, like WTL::XXX.
// define this macro in your stdafx.h or in preprocessor definitions, don't modify the comment here
//#define		_WTL_NO_AUTOMATIC_NAMESPACE

#include "ISonicUI.h"
using namespace sonic_ui;
#include "atlbase.h"
#include "atlwin.h"
#ifdef CORE_CROSS_MODULE
#pragma warning(disable : 4275)
#pragma warning(disable : 4251)
#pragma warning(disable : 4231)
#ifdef CORE_DLL
#define COREWIDGHT_API	__declspec(dllexport)
#define EXP_STL
#else
#define COREWIDGHT_API	__declspec(dllimport)
#define EXP_STL extern
#endif	// CORE_EXPORT_STL
#endif // CORE_CROSS_MODULE

#ifndef CORE_COMPATIBLE_WITH_MFC
#include "atlstr.h"
#include "atltypes.h"
#else
#undef __ATLSTR_H__
#endif // COMPATIBLE_WITH_MFC, not with MFC, use simple ATL classes

#include "wtl/atlapp.h"
// to assure work normally with MFC, dirty work
#ifdef CORE_COMPATIBLE_WITH_MFC
#ifdef _AFX
#undef __IStream_INTERFACE_DEFINED__
#endif // _AFX
#ifdef CORE_CROSS_MODULE
namespace WTL
{ 
	// export WTL::CString to avoid heap crash
	class COREWIDGHT_API CString; 
};
#endif	// CORE_CROSS_MODULE
#include "wtl/atlmisc.h"
#endif	// COMPATIBLE_WITH_MFC

#include "wtl/atlctrls.h"
#include "wtl/atlcrack.h"

#include <vector>

namespace WTL{
//////////////////////////////////////////////////////////////////////////
// Global Constant Definitions
//////////////////////////////////////////////////////////////////////////
#define CORE_WINDOW_WITH_REFLECTOR		_T("CoreWindowWithReflectorFixed")
#define CORE_WINDOW_SUBCLASSED			_T("CoreWindowSubclassed")
#define CORE_BORDER_COLOR				RGB(172, 168, 153)
#define ODT_SELFDRAW					0x11c0
#define CORE_ATTR_NOHILIGHT				_T("CoreAttrNoHilight")
#define CORE_ATTR_WIDTH					_T("CoreAttrWidth")
#define CORE_INPUT_BOX					_T("CoreInputBox")

//////////////////////////////////////////////////////////////////////////
// this message is sent by control to its parent when doing a drop operation
// if wParam is 0, this message is using for inquiry if the drop operation should be allowed. return a non-zero value to block the operation
// if wParam is non-zero, it means a drop operation has successfully been done
// lParam is a structure of drop data
//////////////////////////////////////////////////////////////////////////
enum enCoreDropType
{
	cdtSelf = 0,	// drag source is itself
	cdtFile,		// drag source is file
	cdtOther,		// drag source is from other core controls
};
typedef struct tagCoreDropData
{
	UINT uDropType;
	HWND hCtrl;
	union
	{
		struct
		{
			int nFileNum;
			LPCTSTR lpszFilePath;
		}file;
		HWND hSrcCtrl;
	};
	union
	{
		int nOldIndex;
		HTREEITEM hOldItem;
	};
	union
	{
		int nNewIndex;
		HTREEITEM hNewItem;
	};
}CORE_DROP_DATA;
#define WM_COREDROP						WM_USER + 0x3557

//////////////////////////////////////////////////////////////////////////
// Global inline functions
//////////////////////////////////////////////////////////////////////////
inline void HilightPaintString(ISonicPaint * pPaint, BOOL bHilight, DWORD dwColor = (DWORD)-1)
{
	int nCount = pPaint->GetCount();
	for(int i = 0; i < nCount; i++)
	{
		PAINT_ITEM * pItem = pPaint->GetPaintItem(i, TRUE);
		if(pItem->pBase->GetType() == BASE_TYPE_STRING)
		{
			ISonicString * pStr = (ISonicString * )pItem->pBase;
			if(!pStr->GetAttr(CORE_ATTR_NOHILIGHT))
			{
				if(!bHilight && pStr->IsForcingColor())
				{
					pStr->ForceColor(FALSE, 0, FALSE);
				}
				if(bHilight && dwColor != (DWORD)-1)
				{
					pStr->ForceColor(TRUE, dwColor, FALSE);
				}
				if(bHilight)
				{
					pStr->Highlight(TRUE, FALSE);
				}
				else
				{
					pStr->Highlight(FALSE, FALSE);
				}
			}				
		}
		if(pItem->pBase->GetType() == BASE_TYPE_PAINT)
		{
			HilightPaintString((ISonicPaint *)pItem->pBase, bHilight, dwColor);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// inherit this callback interface to implement sort operation 
// return 0 to indicate the two item is equal, return 1 to indicate item2 is after item1
// return -1 to indicate item2 is before item1
template<class T, typename TItem = int>
class ICoreSorter
{
public:
	virtual int Compare(T * pCtrl, TItem item1, TItem item2) = 0;
};
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CCorePaintDC : public CPaintDC
{
private:
	ISonicImage * m_pCanvas;
	HDC m_hHostDC;
public:
	CCorePaintDC(HWND hWnd, DWORD dwColor = 0) : CPaintDC(hWnd), m_pCanvas(NULL)
	{
		if(m_hDC)
		{
			CRect rt;
			GetClientRect(hWnd, rt);
			m_pCanvas = GetSonicUI()->CreateImage();
			m_pCanvas->Create(rt.Width(), rt.Height(), dwColor);
			m_hHostDC = m_hDC;
			m_hDC = m_pCanvas->GetDC();
		}
	}
	~CCorePaintDC()
	{
		if(m_pCanvas)
		{
			m_pCanvas->Draw(m_hHostDC);
			SONICUI_DESTROY(m_pCanvas);
		}
	}
};
//////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowImplEx;
template <class T, class TBase /* = CWindow */, class TWinTraits /* = CControlWinTraits */>
class ATL_NO_VTABLE CWindowImplEx : public CWindowImpl< T, TBase, TWinTraits >, public CSafeCrtBuffer
{
public:
	typedef std::vector<CString> PROP_VEC;
private:
	PROP_VEC m_vecProp;
	BOOL m_bDeleteSelfOnDestroy;
	// differ from the conception of owner window and parent window, a host window is responsible for command and notify messages
	// or as the host served by this window, default as parent window
	CWindow m_Host;
public:
	// message map and handlers
	typedef CWindowImplEx< T, TBase, TWinTraits >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_NCDESTROY, OnNcDestroy)
	END_MSG_MAP()

	CWindowImplEx() : m_bDeleteSelfOnDestroy(FALSE){}

	void SetHost(HWND hWnd)
	{
		m_Host = hWnd;
	}

	CWindow GetHost()
	{
		if(!m_Host)
		{
			m_Host = GetParent();
		}
		return m_Host;
	}

	BOOL SetAutoRemoveProp(LPCTSTR szPropName, LPVOID pData)
	{
		if(SetProp(m_hWnd, szPropName, pData))
		{
			BOOL bFound = FALSE;
			for(PROP_VEC::iterator it = m_vecProp.begin(); it != m_vecProp.end(); it++)
			{
				if(*it == szPropName)
				{
					bFound = TRUE;
					break;
				}
			}
			if(!bFound)
			{
				m_vecProp.push_back(szPropName);
			}
			return TRUE;
		}
		return FALSE;
	}

	LRESULT OnNcDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		ClearAutoRemoveProp();
		bHandled = FALSE;
		return 1;
	}

	BOOL SubclassWindowOnce(HWND hWnd)
	{
		if(GetProp(hWnd, CORE_WINDOW_SUBCLASSED))
		{
			// already subclassed
			return FALSE;
		}
		if(SubclassWindow(hWnd))
		{
			SetAutoRemoveProp(CORE_WINDOW_SUBCLASSED, this);
			return TRUE;
		}
		return FALSE;
	}

	HWND UnsubclassWindow(BOOL bForce = FALSE)
	{
		HWND hRet = CWindowImpl::UnsubclassWindow(bForce);
		if(hRet)
		{
			// succeeded
			ClearAutoRemoveProp();
		}
		return hRet;
	}

	void SetDeleteSelfOnDestroy(BOOL b){ m_bDeleteSelfOnDestroy = b; }

	// caution: overrided OnFinalMessage in child classes must call the function of their base class at the very end	
	virtual void OnFinalMessage(HWND)
	{
		if(m_bDeleteSelfOnDestroy)
		{
			delete this;
		}
	}
protected:
	void ClearAutoRemoveProp()
	{
		for(PROP_VEC::iterator it = m_vecProp.begin(); it != m_vecProp.end(); it++)
		{
			RemoveProp(m_hWnd, *it);
		}
		m_vecProp.clear();
	}
};

//////////////////////////////////////////////////////////////////////////
// class:	CWindowWithReflectorImplFixed
// intro:	controls derived from this class will create a proxy window between the real parent and child control, 
//			to make an automatic message reflection. Other than ordinary ATL reflection, WM_COMMAND, WM_NOTIFY 
//			and WM_PARENTNOTIFY are forwarded to the real parent instead of reflecting to child control.
// remark:	this section of code is for the purpose of fixing a bug of CWindowWithReflectorImpl in atlwin.h
//			the original code is like following:
//			RECT rcPos = { 0, 0, rect.m_lpRect->right, rect.m_lpRect->bottom };
//			which calculates region size of the child control but mistakes the width and height
//////////////////////////////////////////////////////////////////////////
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowWithReflectorImplFixed;
template <class T, class TBase /* = CWindow */, class TWinTraits /* = CControlWinTraits */>
class ATL_NO_VTABLE CWindowWithReflectorImplFixed : public CWindowImplEx< T, TBase, TWinTraits >
{
protected:
	void CalcClientRect(LPRECT lpRect)
	{
		CRect rt;
		m_wndReflector.GetWindowRect(rt);
		rt.OffsetRect(-rt.left, -rt.top);
		rt.DeflateRect(GetBorderWidth() + m_rtMargin.left, GetBorderWidth() + m_rtMargin.top, 
			GetBorderWidth() + m_rtMargin.right, GetBorderWidth() + m_rtMargin.bottom);
		*lpRect = rt;
	}
public:
	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		if(dwStyle & WS_BORDER)
		{
			if(GetBorderColor() == -1)
			{
				SetBorderColor(0);
			}
		}
		dwStyle = dwStyle & ~WS_BORDER;
		m_wndReflector.Create(hWndParent, rect.m_lpRect, NULL, (dwStyle & (WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP))| WS_CLIPCHILDREN | ((dwStyle & WS_POPUP) ? WS_POPUP : WS_CHILD),
			dwExStyle & WS_EX_TOOLWINDOW);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_CHILD | WS_VISIBLE;
		dwExStyle &= ~WS_EX_TOOLWINDOW;
		CRect rcPos;
		CalcClientRect(rcPos);
		HWND hRet = CWindowImplEx< T, TBase, TWinTraits >::Create(m_wndReflector, rcPos, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		if(hRet)
		{
			SetAutoRemoveProp(CORE_WINDOW_WITH_REFLECTOR, this);
		}
		SetHost(m_wndReflector);
		return hRet;
	}

	// relay the GetParent()
	CWindow GetParent() const throw()
	{
		return m_wndReflector.GetParent();
	}

	// message map and handlers
	typedef CWindowWithReflectorImplFixed< T, TBase, TWinTraits >	thisClass;
	BEGIN_MSG_MAP(thisClass)
		MESSAGE_HANDLER(WM_NCDESTROY, OnNcDestroy)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnWindowPosChanging)
		CHAIN_MSG_MAP(CWindowImplEx)
	END_MSG_MAP()

	void SetBorderColor(DWORD dwColor)
	{
		BOOL bResize = FALSE;
		if(m_wndReflector.m_dwBorderColor == -1)
		{
			// the first time set border color, change the size of the internal window
			bResize = TRUE;
		}
		if(dwColor == -1)
		{
			// remove the border
			bResize = TRUE;
		}
		m_wndReflector.m_dwBorderColor = dwColor;
		if(bResize && IsWindow())
		{
			CRect rtWnd;
			GetWindowRect(rtWnd);
			if(m_wndReflector.GetParent())
			{
				m_wndReflector.GetParent().ScreenToClient(rtWnd);
			}
			SetWindowPos(NULL, rtWnd, SWP_NOZORDER | SWP_NOACTIVATE);
		}		
	}
	DWORD GetBorderColor()
	{
		return m_wndReflector.m_dwBorderColor;
	}

	int GetBorderWidth(){ return m_wndReflector.m_dwBorderColor == (DWORD)-1 ? 0 : 1; }

	BOOL ShowWindow(int nCmdShow)
	{
		return m_wndReflector.ShowWindow(nCmdShow);
	}
	BOOL ShowWindowAsync(int nCmdShow)
	{
		return m_wndReflector.ShowWindowAsync(nCmdShow);
	}
	LRESULT OnNcDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_wndReflector.DestroyWindow();
		bHandled = FALSE;
		return 1;
	}
	LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		WINDOWPOS* pWP = (WINDOWPOS*)lParam;
		m_wndReflector.SetWindowPos(pWP->hwndInsertAfter, pWP->x, pWP->y, pWP->cx, pWP->cy, pWP->flags);
		pWP->x = GetBorderWidth() + m_rtMargin.left;
		pWP->y = GetBorderWidth() + m_rtMargin.top;
		pWP->cx -= GetBorderWidth() * 2 + m_rtMargin.left + m_rtMargin.right;
		pWP->cy -= GetBorderWidth() * 2 + m_rtMargin.top + m_rtMargin.bottom;
		pWP->flags &= ~SWP_HIDEWINDOW;
		return DefWindowProc(uMsg, wParam, lParam);
	}
	BOOL GetWindowRect(LPRECT lpRect)
	{
		return m_wndReflector.GetWindowRect(lpRect);
	}
	BOOL GetClientRect(LPRECT lpRect)
	{
		if(!lpRect)
		{
			return FALSE;
		}
		CRect rtHost, rtClient;
		m_wndReflector.GetWindowRect(rtHost);
		CWindow::GetClientRect(rtClient);
		ClientToScreen(&rtClient.TopLeft());
		ClientToScreen(&rtClient.BottomRight());
		rtClient.OffsetRect(-rtHost.left, -rtHost.top);
		*lpRect = rtClient;
		return TRUE;
	}
	void SetMargin(int nLeft, int nTop, int nRight, int nBottom)
	{
		m_rtMargin.left = nLeft;
		m_rtMargin.top = nTop;
		m_rtMargin.right = nRight;
		m_rtMargin.bottom = nBottom;
		if(IsWindow())
		{
			CRect rtWnd;
			GetWindowRect(rtWnd);
			if(m_wndReflector.GetParent())
			{
				m_wndReflector.GetParent().ScreenToClient(rtWnd);
			}
			SetWindowPos(NULL, rtWnd, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	// reflector window stuff
	class Reflector : public CWindowImplEx<Reflector>
	{
	public:
		DWORD m_dwBorderColor;
	public:
		Reflector() : m_dwBorderColor((DWORD)-1){}
		enum { REFLECTOR_MAP_ID = 69 };
		DECLARE_WND_CLASS_EX(_T("ATLReflectorWindow"), 0, -1)

		LRESULT ReflectNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			// override the default ReflectNotifications to reflect specified messages but forward WM_COMMAND, WM_NOTIFY and WM_PARENTNOTIFY to the real parent
			HWND hWndChild = NULL;
			switch(uMsg)
			{
			case WM_COMMAND:
				if(lParam != NULL)	// not from a menu
					hWndChild = (HWND)lParam;
				break;
			case WM_NOTIFY:
				hWndChild = ((LPNMHDR)lParam)->hwndFrom;
				break;
			case WM_PARENTNOTIFY:
				switch(LOWORD(wParam))
				{
				case WM_CREATE:
				case WM_DESTROY:
					hWndChild = (HWND)lParam;
					break;
				default:
					hWndChild = GetDlgItem(HIWORD(wParam));
					break;
				}
				break;
			case WM_DRAWITEM:
				if(wParam)	// not from a menu
					hWndChild = ((LPDRAWITEMSTRUCT)lParam)->hwndItem;
				break;
			case WM_MEASUREITEM:
				if(wParam)	// not from a menu
					hWndChild = GetDlgItem(((LPMEASUREITEMSTRUCT)lParam)->CtlID);
				break;
			case WM_COMPAREITEM:
				if(wParam)	// not from a menu
					hWndChild = ((LPCOMPAREITEMSTRUCT)lParam)->hwndItem;
				break;
			case WM_DELETEITEM:
				if(wParam)	// not from a menu   
					hWndChild = ((LPDELETEITEMSTRUCT)lParam)->hwndItem;

				break;
			case WM_VKEYTOITEM:
			case WM_CHARTOITEM:
			case WM_HSCROLL:
			case WM_VSCROLL:
				hWndChild = (HWND)lParam;
				break;
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORDLG:
			case WM_CTLCOLOREDIT:
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORMSGBOX:
			case WM_CTLCOLORSCROLLBAR:
			case WM_CTLCOLORSTATIC:
				hWndChild = (HWND)lParam;
				break;
			default:
				break;
			}

			if(hWndChild == NULL)
			{
				bHandled = FALSE;
				return 1;
			}
			else
			{
				switch(uMsg)
				{
				case WM_COMMAND:
					{
						switch(HIWORD(wParam))
						{
						case LBN_SELCHANGE:
						case EN_CHANGE:
						case EN_UPDATE:
							::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
							break;
						default:
							break;
						}
						return GetParent().SendMessage(uMsg, wParam, lParam);
					}
					break;
				case WM_PARENTNOTIFY:
					return GetParent().SendMessage(uMsg, wParam, lParam);
				case WM_NOTIFY:
					{
						NMHDR * pNMHdr = (NMHDR *)lParam;
						switch(pNMHdr->code)
						{
						case NM_CUSTOMDRAW:
						case TVN_BEGINDRAG:
						case LVN_BEGINDRAG:
							break;
						case LBN_SELCHANGE:
						case EN_LINK:
							::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
						default:
							return GetParent().SendMessage(uMsg, wParam, lParam);
						}
					}
					break;
				}
				return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
			}			
		}
		BEGIN_MSG_MAP(Reflector)
			MESSAGE_HANDLER(WM_PAINT, OnPaint);
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MSG_WM_SETFOCUS(OnSetFocus)
 			REFLECT_NOTIFICATIONS()
 			CHAIN_MSG_MAP(CWindowImplEx)
		END_MSG_MAP()

		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
		{
			if(m_dwBorderColor != (DWORD)-1)
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(&ps);
				CRect rt;
				GetClientRect(rt);
				GetSonicUI()->Rectangle(hdc, 0, 0, rt.Width(), rt.Height(), m_dwBorderColor);
				EndPaint(&ps);
			}
			else
			{
				bHandled = FALSE;
			}
			return 0;
		}
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;
			ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
			pSkin->Attach(m_hWnd);
			return 0;
		}
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;
			GetSonicUI()->DestroyAttachedObject(m_hWnd);
			return 0;
		}
		LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			return TRUE;
		}
		void OnSetFocus(CWindow wndOld)
		{
			if (GetWindowLong(GWL_STYLE) & WS_TABSTOP)
			{
				::SetFocus(GetNextDlgTabItem(NULL));
			}
		}
	};
protected:
	Reflector m_wndReflector;
	CRect m_rtMargin;
};
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CCoreScrollBar : public CWindowImplEx<CCoreScrollBar, CScrollBar>
{
public:
	enum enTimer
	{
		TIMER_ARROW_DELAY = 1,
		TIMER_ARROW_PUSHED,
		TIMER_CHANNEL_DELAY,
		TIMER_CHANNEL_PUSHED,
	};

	enum enElapse
	{
		ELAPSE_ARROW_DELAY = 250,
		ELAPSE_ARROW_PUSHED = 50,
		ELAPSE_CHANNEL_DELAY = 250,
		ELAPSE_CHANNEL_PUSHED = 80,
	};

	enum enConstantDef
	{
		MIN_BOX_LENGTH = 10,
	};

	BEGIN_MSG_MAP_EX(CCoreScrollBar)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(CWindowImplEx)
	END_MSG_MAP()

	CCoreScrollBar()
	{
		m_pBtnUpArrow = NULL;
		m_pBtnDownArrow = NULL;
		m_pBtnThumb = NULL;
		m_pImgChannel = NULL;
		m_pImgThumb = NULL;
		m_bIsDragging = FALSE;
		m_si.fMask = SIF_ALL;
		m_si.cbSize = sizeof(SCROLLINFO);

	}
	~CCoreScrollBar()
	{
		SONICUI_DESTROY(m_pBtnUpArrow);
		SONICUI_DESTROY(m_pBtnDownArrow);
		SONICUI_DESTROY(m_pBtnThumb);
	}
	BOOL IsSkined()
	{
		return (m_pImgChannel != NULL);
	}

	void SetHost(HWND hWnd)
	{
		CWindowImplEx<CCoreScrollBar, CScrollBar>::SetHost(hWnd);
		ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
		if(pSkin)
		{
			pSkin->SetAttr(SATTR_ACCEPT_FOCUS, hWnd);
		}
	}

	DWORD GetChannelColor()
	{
		if(!IsSkined())
		{
			return 0;
		}
		return m_pImgChannel->GetPixel(m_pImgChannel->GetWidth() / 2, m_pImgChannel->GetHeight() / 2) & 0x00ffffff;
	}

	int GetScrollImageWidth()
	{
		if(!m_pImgChannel)
		{
			return 0;
		}
		return m_pImgChannel->GetWidth();
	}

	int GetScrollImageHeight()
	{
		if(!m_pImgChannel)
		{
			return 0;
		}
		return m_pImgChannel->GetHeight();
	}

	void SetScrollImage(ISonicImage * pUpArrow, ISonicImage * pDownArrow, ISonicImage * pChannel, ISonicImage * pThumb)
	{
		// up arrow
		if(!m_pBtnUpArrow)
		{
			m_pBtnUpArrow = GetSonicUI()->CreateString();
		}
		if(pUpArrow)
		{
			m_pBtnUpArrow->Format(_T("/a, p=%d, linkc=0/"), pUpArrow);
			m_pBtnUpArrow->Delegate(DELEGATE_EVENT_LBUTTONDOWN, NULL, this, &CCoreScrollBar::OnUpArrowPush);
			m_pBtnUpArrow->Delegate(DELEGATE_EVENT_RAWLBUTTONUP, NULL, this, &CCoreScrollBar::OnArrowPop);
		}

		// down arrow
		m_pImgThumb = pThumb;
		if(!m_pBtnDownArrow)
		{
			m_pBtnDownArrow = GetSonicUI()->CreateString();
		}
		if(pDownArrow)
		{
			m_pBtnDownArrow->Format(_T("/a, p=%d, linkc=0/"), pDownArrow);
			m_pBtnDownArrow->Delegate(DELEGATE_EVENT_LBUTTONDOWN, NULL, this, &CCoreScrollBar::OnDownArrowPush);
			m_pBtnDownArrow->Delegate(DELEGATE_EVENT_RAWLBUTTONUP, NULL, this, &CCoreScrollBar::OnArrowPop);
		}

		// thumb
		if(!m_pBtnThumb)
		{
			m_pBtnThumb = GetSonicUI()->CreateString();
		}
		// channel, just a point, don't need to create instance
		m_pImgChannel = pChannel;
		ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
		if(pSkin)
		{
			pSkin->SetSkin(_T("bg"), _T("image:%d"), m_pImgChannel->GetObjectId());
		}
	}

	void OnSetScrollPos(LPSCROLLINFO lpsi)
	{
		if(lpsi->fMask & SIF_POS)
		{
			m_si.nPos = lpsi->nPos;
		}
		if(lpsi->fMask & SIF_PAGE)
		{
			m_si.nPage = lpsi->nPage;
		}
		if(lpsi->fMask & SIF_TRACKPOS)
		{
			m_si.nTrackPos = lpsi->nTrackPos;
		}
		if(lpsi->fMask & SIF_RANGE)
		{
			m_si.nMax = lpsi->nMax;
			m_si.nMin = lpsi->nMin;
		}
		if(!IsSkined())
		{
			return;
		}
		if(!m_pBtnThumb->IsPushed())
		{
			if(IsWindowVisible())
			{
				InvalidateRect(NULL);
			}
		}
		ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(GetHost());
		if(pSkin && pSkin->GetSkin(_T("filter"), _T("src")))
		{
			// a remote filter installed, disable the system rendering clip
			if(GetHost().IsWindowVisible())
			{
				GetHost().InvalidateRect(NULL);
			}
		}
	}
protected:
	//////////////////////////////////////////////////////////////////////////
	// these two mouse functions must be overrided otherwise the ugly original item will appear
	void OnLButtonDblClk(UINT nFlags, CPoint point)
	{

	}
	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		if(IsVertical())
		{
			CRect rtBox = m_pBtnThumb->GetRect();
			if(rtBox.top > point.y)
			{
				// up scroll one page
				GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), (LPARAM)m_hWnd);
			}
			else if(rtBox.bottom < point.y)
			{
				// down scroll one page
				GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), (LPARAM)m_hWnd);
			}
		}
		else
		{
			CRect rtBox = m_pBtnThumb->GetRect();
			if(rtBox.left > point.x)
			{
				// up scroll one page
				GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_PAGELEFT, 0), (LPARAM)m_hWnd);
			}
			else if(rtBox.right < point.x)
			{
				// down scroll one page
				GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_PAGERIGHT, 0), (LPARAM)m_hWnd);
			}
		}
		SetTimer(TIMER_CHANNEL_DELAY, ELAPSE_CHANNEL_DELAY);
		SetCapture();
	}
	//////////////////////////////////////////////////////////////////////////
	void OnLButtonUp(UINT nFlags, CPoint point)
	{
	  	
	}
	BOOL IsVertical()
	{
		return (GetWindowLong(GWL_STYLE) & SBS_VERT) != 0;
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		switch(nIDEvent)
		{
		case TIMER_ARROW_DELAY:
			{
				KillTimer(TIMER_ARROW_DELAY);
				if(m_pBtnUpArrow->IsPushed() || m_pBtnDownArrow->IsPushed())
				{
					SetTimer(TIMER_ARROW_PUSHED, ELAPSE_ARROW_PUSHED);
				}
			}
			break;
		case TIMER_ARROW_PUSHED:
			{
				POINT pt;
				GetSonicUI()->GetCursorPosInWindow(m_hWnd, &pt);
				if(IsVertical())
				{
					if(m_pBtnUpArrow->IsPushed() && PtInRect(m_pBtnUpArrow->GetRect(), pt))
					{
						GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), (LPARAM)m_hWnd);
					}
					else if(m_pBtnDownArrow->IsPushed() && PtInRect(m_pBtnDownArrow->GetRect(), pt))
					{
						GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), (LPARAM)m_hWnd);
					}
				}
				else
				{
					if(m_pBtnUpArrow->IsPushed() && PtInRect(m_pBtnUpArrow->GetRect(), pt))
					{
						GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), (LPARAM)m_hWnd);
					}
					else if(m_pBtnDownArrow->IsPushed() && PtInRect(m_pBtnDownArrow->GetRect(), pt))
					{
						GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), (LPARAM)m_hWnd);
					}
				}
			}
			break;
		case TIMER_CHANNEL_DELAY:
			{
				KillTimer(TIMER_CHANNEL_DELAY);
				SetTimer(TIMER_CHANNEL_PUSHED, ELAPSE_CHANNEL_PUSHED);
			}
			break;
		case TIMER_CHANNEL_PUSHED:
			{
				POINT pt;
				GetSonicUI()->GetCursorPosInWindow(m_hWnd, &pt);
				CRect rt;
				GetClientRect(rt);
				if(IsVertical())
				{
					rt.DeflateRect(0, m_pBtnUpArrow->GetHeight(), 0, m_pBtnDownArrow->GetHeight());
				}
				else
				{
					rt.DeflateRect(m_pBtnUpArrow->GetWidth(), 0, m_pBtnDownArrow->GetWidth(), 0);
				}
				if(!rt.PtInRect(pt))
				{
					break;
				}
				if(PtInRect(m_pBtnThumb->GetRect(), pt))
				{
					KillTimer(TIMER_CHANNEL_PUSHED);
					break;
				}
				if(IsVertical())
				{					
					if(m_pBtnThumb->GetRect()->top > pt.y)
					{
						// up scroll one page
						GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), (LPARAM)m_hWnd);
					}
					else if(m_pBtnThumb->GetRect()->bottom < pt.y)
					{
						// down scroll one page
						GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), (LPARAM)m_hWnd);
					}
				}
				else
				{
					if(m_pBtnThumb->GetRect()->left > pt.x)
					{
						// up scroll one page
						GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_PAGELEFT, 0), (LPARAM)m_hWnd);
					}
					else if(m_pBtnThumb->GetRect()->right < pt.x)
					{
						// down scroll one page
						GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_PAGERIGHT, 0), (LPARAM)m_hWnd);
					}
				}
			}
			break;
		}
	}

	void OnUpArrowPush(ISonicString *, LPVOID)
	{
		if(IsVertical())
		{
			GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), (LPARAM)m_hWnd);
		}
		else
		{ 
			GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINELEFT, 0), (LPARAM)m_hWnd);
		}
		SetTimer(TIMER_ARROW_DELAY, ELAPSE_ARROW_DELAY);
	}

	void OnArrowPop(ISonicString *, LPVOID)
	{
		KillTimer(TIMER_ARROW_DELAY);
		KillTimer(TIMER_ARROW_PUSHED);
	}

	void OnDownArrowPush(ISonicString *, LPVOID)
	{
		if(IsVertical())
		{
			GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), (LPARAM)m_hWnd);
		}
		else
		{
			GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_LINERIGHT, 0), (LPARAM)m_hWnd);
		}
		SetTimer(TIMER_ARROW_DELAY, ELAPSE_ARROW_DELAY);
	}

	void OnThumbPush(ISonicString *, LPVOID)
	{
		POINT pt;
		GetSonicUI()->GetCursorPosInWindow(m_hWnd, &pt);
		m_ptBeginDragCursor = pt;
		m_ptBeginDragBox = CPoint(m_pBtnThumb->GetRect()->left, m_pBtnThumb->GetRect()->top);
		m_ptDragingBox = m_ptBeginDragBox;
		m_bIsDragging = TRUE;
	}

	void OnThumbMouseMove(ISonicString *, LPVOID)
	{
		if(!m_pBtnThumb->IsPushed() || !m_bIsDragging)
		{
			return;
		}
		BOOL bDragingBoxMoved = FALSE;
		int nTrackPos = GetTrackPos(&bDragingBoxMoved);
		if(IsVertical())
		{
			GetHost().SetScrollPos(SB_VERT, nTrackPos, FALSE);
			GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, nTrackPos), (LPARAM)m_hWnd);
		}
		else
		{
			GetHost().SetScrollPos(SB_HORZ, nTrackPos, FALSE);
			GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, nTrackPos), (LPARAM)m_hWnd);
		}
		if(bDragingBoxMoved)
		{
			InvalidateRect(NULL);
		}
	}

	void OnThumbPop(ISonicString * pStr, LPVOID)
	{
		ReleaseCapture();
		KillTimer(TIMER_CHANNEL_DELAY);
		KillTimer(TIMER_CHANNEL_PUSHED);
		GetHost().SetFocus();
		if(!m_bIsDragging)
		{			
			return;
		}
		int nTrackPos = GetTrackPos();
		if(IsVertical())
		{
			GetHost().SetScrollPos(SB_VERT, nTrackPos, FALSE);
			GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nTrackPos), (LPARAM)m_hWnd);
			GetHost().SendMessage(WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), (LPARAM)m_hWnd);
		}
		else
		{
			GetHost().SetScrollPos(SB_HORZ, nTrackPos, FALSE);
			GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nTrackPos), (LPARAM)m_hWnd);
			GetHost().SendMessage(WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), (LPARAM)m_hWnd);
		}
		InvalidateRect(NULL);
		m_bIsDragging = FALSE;
	}

	void OnDestroy()
	{
		GetSonicUI()->DestroyObject(GetSonicUI()->SkinFromHwnd(m_hWnd));
		SetMsgHandled(FALSE);		
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
		pSkin->Attach(m_hWnd);
		SetMsgHandled(FALSE);
		return 0;
	}

	BOOL OnEraseBkgnd(CDCHandle dc)
	{
		return TRUE;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		CRect rt;
		GetClientRect(rt);

		CPaintDC dc(m_hWnd);
		if(IsVertical())
		{	
			// render up arrow
			m_pBtnUpArrow->TextOut(dc, 0, 0, m_hWnd);
 			// render down arrow
 			m_pBtnDownArrow->TextOut(dc, 0, rt.Height() - m_pBtnDownArrow->GetHeight(), m_hWnd);
			// vertically tiled render channel
 			int nChannelHeight = rt.Height() - m_pBtnUpArrow->GetHeight() - m_pBtnDownArrow->GetHeight();
			// render the scroll box
			int nMaxPos = m_si.nMax - max(m_si.nPage - 1, 0);
			int nPos = m_si.nPos < 0 ? 0 : m_si.nPos;
			int nBoxHeight = (int)((rt.Height() - m_pBtnUpArrow->GetHeight() - m_pBtnDownArrow->GetHeight()) * m_si.nPage/ (double)(m_si.nMax - m_si.nMin + 1));
			nBoxHeight = nBoxHeight > MIN_BOX_LENGTH ? nBoxHeight : MIN_BOX_LENGTH;
			if(nBoxHeight != m_pBtnThumb->GetHeight())
			{
				MakeThumb(nBoxHeight);
			}
			
			nPos = nPos > nMaxPos ? nMaxPos : nPos;
			int y;
			if(m_pBtnThumb->IsPushed())
			{
				y = m_ptDragingBox.y;
			}
			else
			{
				y = (int)(m_pBtnUpArrow->GetHeight() + nPos * (nChannelHeight - m_pBtnThumb->GetHeight()) / (double)nMaxPos);
			}
			m_pBtnThumb->TextOut(dc, 0, y, m_hWnd);
		}
		else
		{
			// render left arrow
			m_pBtnUpArrow->TextOut(dc, 0, 0, m_hWnd);
			// render right arrow
			m_pBtnDownArrow->TextOut(dc, rt.Width() - m_pBtnDownArrow->GetWidth(), 0, m_hWnd);
			// horizontally tiled render channel
 			int nChannelWidth = rt.Width() - m_pBtnDownArrow->GetWidth() - m_pBtnDownArrow->GetWidth();
			// render the scroll box
			int nMaxPos = m_si.nMax - max(m_si.nPage - 1, 0);
			int nPos = m_si.nPos < 0 ? 0 : m_si.nPos;
			int nBoxWidth = (int)((rt.Width() - m_pBtnUpArrow->GetWidth() - m_pBtnDownArrow->GetWidth()) * m_si.nPage/ (double)(m_si.nMax - m_si.nMin + 1));
			nBoxWidth = nBoxWidth > MIN_BOX_LENGTH ? nBoxWidth : MIN_BOX_LENGTH;
			if(nBoxWidth != m_pBtnThumb->GetWidth())
			{
				MakeThumb(nBoxWidth);
			}
			nPos = nPos > nMaxPos ? nMaxPos : nPos;
			int x;
			if(m_pBtnThumb->IsPushed())
			{
				x = m_ptDragingBox.x;
			}
			else
			{
				x = (int)(m_pBtnUpArrow->GetWidth() + nPos * (nChannelWidth - m_pBtnThumb->GetWidth()) / (double)nMaxPos);
			}
			m_pBtnThumb->TextOut(dc, x, 0, m_hWnd);
		}
		return 0;
	}
	void MakeThumb(int nLength)
	{
		nLength = nLength > MIN_BOX_LENGTH ? nLength : MIN_BOX_LENGTH;
		m_pBtnThumb->Clear();
		if(IsVertical())
		{
			m_pBtnThumb->Format(_T("/a, p=%d, linkc=0, btn_height=%d/"), m_pImgThumb->GetObjectId(), nLength);
		}
		else
		{
			m_pBtnThumb->Format(_T("/a, p=%d, linkc=0, btn_width=%d/"), m_pImgThumb->GetObjectId(), nLength);
		}
		m_pBtnThumb->Delegate(DELEGATE_EVENT_LBUTTONDOWN, NULL, this, &CCoreScrollBar::OnThumbPush);
		m_pBtnThumb->Delegate(DELEGATE_EVENT_RAWMOUSEMOVE, NULL, this, &CCoreScrollBar::OnThumbMouseMove);
		m_pBtnThumb->Delegate(DELEGATE_EVENT_RAWLBUTTONUP, NULL, this, &CCoreScrollBar::OnThumbPop);
	}

	int GetTrackPos(BOOL * bDragingBoxMoved = NULL)
	{
		POINT pt;
		GetSonicUI()->GetCursorPosInWindow(m_hWnd, &pt);
		CRect rt;
		GetClientRect(rt);
		if(IsVertical())
		{
			int nPlus = pt.y - m_ptBeginDragCursor.y;
			int nOldDragingBoxY = m_ptDragingBox.y;
			m_ptDragingBox.y = m_ptBeginDragBox.y + nPlus;
			if(m_ptDragingBox.y < m_pBtnUpArrow->GetHeight())
			{
				m_ptDragingBox.y = m_pBtnUpArrow->GetHeight();
			}
			if(m_ptDragingBox.y + m_pBtnThumb->GetHeight() > rt.Height() - m_pBtnDownArrow->GetHeight())
			{
				m_ptDragingBox.y = rt.Height() - m_pBtnDownArrow->GetHeight() - m_pBtnThumb->GetHeight();
			}
			if(nOldDragingBoxY != m_ptDragingBox.y && bDragingBoxMoved)
			{
				*bDragingBoxMoved = TRUE;
			}
			int nMaxPos = m_si.nMax - max(m_si.nPage - 1, 0);
			int nPos = (m_ptDragingBox.y - m_pBtnUpArrow->GetHeight()) * nMaxPos / (rt.Height() - m_pBtnUpArrow->GetHeight() - m_pBtnDownArrow->GetHeight() - m_pBtnThumb->GetHeight());
			return nPos;
		}
		else
		{
			int nPlus = pt.x - m_ptBeginDragCursor.x;
			int nOldDragingBoxX = m_ptDragingBox.x;
			m_ptDragingBox.x = m_ptBeginDragBox.x + nPlus;
			if(m_ptDragingBox.x < m_pBtnUpArrow->GetWidth())
			{
				m_ptDragingBox.x = m_pBtnUpArrow->GetWidth();
			}
			if(m_ptDragingBox.x + m_pBtnThumb->GetWidth() > rt.Width() - m_pBtnDownArrow->GetWidth())
			{
				m_ptDragingBox.x = rt.Width() - m_pBtnDownArrow->GetWidth() - m_pBtnThumb->GetWidth();
			}
			if(nOldDragingBoxX != m_ptDragingBox.x && bDragingBoxMoved)
			{
				*bDragingBoxMoved = TRUE;
			}
			int nMaxPos = m_si.nMax - max(m_si.nPage - 1, 0);
			int nPos = (m_ptDragingBox.x - m_pBtnUpArrow->GetWidth()) * nMaxPos / (rt.Width() - m_pBtnUpArrow->GetWidth() - m_pBtnDownArrow->GetWidth() - m_pBtnThumb->GetWidth());
			return nPos;
		}
	}
private:
	ISonicString * m_pBtnUpArrow;
	ISonicString * m_pBtnDownArrow;
	ISonicString * m_pBtnThumb;
	ISonicImage * m_pImgChannel;
	ISonicImage * m_pImgThumb;
	BOOL m_bIsDragging;
	POINT m_ptBeginDragCursor;
	POINT m_ptBeginDragBox;
	POINT m_ptDragingBox;
	SCROLLINFO m_si;
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#define SET_SCROLL_INFO_USERDATA	_T("SetScrollInfoUserData")
template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowImplWithScrollBar;
template<class T, class TBase, class TWinTraits>
class ATL_NO_VTABLE CWindowImplWithScrollBar : public CWindowWithReflectorImplFixed<T, TBase, TWinTraits>
{
private:
	static LPVOID m_spOldSetScrollInfo;
	static int WINAPI SetScrollInfo(HWND hwnd, int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw)
	{
		CWindowImplWithScrollBar * pWnd = (CWindowImplWithScrollBar *)GetProp(hwnd, SET_SCROLL_INFO_USERDATA);
		if(pWnd)
		{
			pWnd->OnSetScrollInfo(fnBar, lpsi, fRedraw);
		}
		__asm
		{
			push [ebp + 14h]
			push [ebp + 10h]
			push [ebp + 0ch]
			push [ebp + 8h]
			call [m_spOldSetScrollInfo]
		}
	}
protected:
	CCoreScrollBar m_VScroll;
	CCoreScrollBar m_HScroll;
public:
	BEGIN_MSG_MAP_EX(CWindowImplWithScrollBar)
		MSG_WM_NCCALCSIZE(OnNcCalcSize)
		MSG_WM_NCPAINT(OnNcPaint)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(CWindowWithReflectorImplFixed)
	END_MSG_MAP()

	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, 
				_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		return CWindowWithReflectorImplFixed::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		if(!CWindowWithReflectorImplFixed::SubclassWindow(hWnd))
		{
			return FALSE;
		}
		Init();
		return TRUE;
	}

	void SetScrollImage(ISonicImage * pUpArrow, ISonicImage * pDownArrow, ISonicImage * pChannel, ISonicImage * pThumb, BOOL bVScroll = TRUE)
	{
		if(bVScroll)
		{
			m_VScroll.SetScrollImage(pUpArrow, pDownArrow, pChannel, pThumb);
		}
		else
		{
			m_HScroll.SetScrollImage(pUpArrow, pDownArrow, pChannel, pThumb);
		}
	}

	// SB_VERT or SB_HORZ
	HWND GetCoreScrollWnd(int nBar)
	{
		if(nBar == SB_VERT)
		{
			return m_VScroll;
		}
		else if(nBar == SB_HORZ)
		{
			return m_HScroll;
		}
		else
		{
			return NULL;
		}
	}

protected:
	void OnNcPaint(CRgn rgn)
	{
		if(IsFootVisible() && m_VScroll.IsSkined())
		{
			CRect rt;
			GetWindowRect(rt);
			rt.OffsetRect(-rt.TopLeft());
			CWindowDC hdc(m_hWnd);
 			rt.top = rt.bottom - m_VScroll.GetScrollImageWidth();
			GetSonicUI()->FillSolidRect(hdc, rt, m_VScroll.GetChannelColor());
		}
		if(!m_VScroll.IsSkined() && !m_HScroll.IsSkined())
		{
			SetMsgHandled(FALSE);
		}
	}

	void OnSetScrollInfo(int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw)
	{
		if(fnBar == SB_VERT)
		{
			m_VScroll.OnSetScrollPos(lpsi);
		}
		if(fnBar == SB_HORZ)
		{
			m_HScroll.OnSetScrollPos(lpsi);
		}
	}

	void OnDestroy()
	{
		m_VScroll.DestroyWindow();
		m_HScroll.DestroyWindow();
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		Init();
		SetMsgHandled(FALSE);
		return 0;
	}

	BOOL IsFootVisible()
	{
		DWORD dwStyle = GetWindowLong(GWL_STYLE);
		return (dwStyle & WS_VSCROLL) && (dwStyle & WS_HSCROLL);
	}

	LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
	{
		if(bCalcValidRects && (m_VScroll.IsSkined() || m_HScroll.IsSkined()))
		{
			NCCALCSIZE_PARAMS FAR* lpncsp = (NCCALCSIZE_PARAMS FAR*)lParam;
			CRect rtWnd, rtClient;
			m_wndReflector.GetClientRect(rtWnd);
			rtWnd.DeflateRect(GetBorderWidth(), GetBorderWidth(), GetBorderWidth(), GetBorderWidth());
			rtClient = rtWnd;
			if(m_VScroll.IsSkined())
			{
				if((GetWindowLong(GWL_STYLE) & WS_VSCROLL))
				{
					int cy = rtClient.Height() - (IsFootVisible() ? m_VScroll.GetScrollImageWidth() : 0);
					if(GetWindowLong(GWL_EXSTYLE) & WS_EX_LEFTSCROLLBAR)
					{
						m_VScroll.SetWindowPos(NULL, rtClient.left, rtClient.top,
							m_VScroll.GetScrollImageWidth(), cy, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
						rtClient.DeflateRect(m_VScroll.GetScrollImageWidth(), 0, 0, 0);
					}
					else
					{
						m_VScroll.SetWindowPos(NULL, rtClient.right - m_VScroll.GetScrollImageWidth(), rtClient.top,
							m_VScroll.GetScrollImageWidth(), cy, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
						rtClient.DeflateRect(0, 0, m_VScroll.GetScrollImageWidth(), 0);
											
					}
				}
				else
				{
					m_VScroll.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			if(m_HScroll.IsSkined())
			{
				if((GetWindowLong(GWL_STYLE) & WS_HSCROLL))
				{
					m_HScroll.SetWindowPos(NULL, rtClient.left, rtClient.bottom - m_HScroll.GetScrollImageHeight(),
						rtClient.Width(), m_HScroll.GetScrollImageHeight(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);
					rtClient.DeflateRect(0, 0, 0, m_HScroll.GetScrollImageHeight());
				}
				else
				{
					m_HScroll.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			DefWindowProc();
			lpncsp->rgrc[0] = rtClient;
			return 0;
		}
		else
		{
			return DefWindowProc();
		}
	}

	void Init()
	{
		if(!m_spOldSetScrollInfo)
		{
			m_spOldSetScrollInfo = GetSonicUI()->DetourFunctionEntry(GetProcAddress(GetModuleHandle(_T("user32.dll")), "SetScrollInfo"),
				&CWindowImplWithScrollBar::SetScrollInfo);
		}

		SetAutoRemoveProp(SET_SCROLL_INFO_USERDATA, this);
		// why not create the scroll bar as the child of controls? the scroll bar will scroll itself with the client region of controls by doing that
		m_VScroll.Create(m_wndReflector, CRect(0, 0, 1, 1), NULL, WS_CHILD | SBS_VERT);
		m_VScroll.SetHost(m_hWnd);
		m_HScroll.Create(m_wndReflector, CRect(0, 0, 1, 1), NULL, WS_CHILD | SBS_HORZ);
		m_HScroll.SetHost(m_hWnd);

		// because the the region of scrollbars intersects with control windows, we have to change the z-order of control window, 
		// put it under the scrollbars to assure the scrollbars could response their input messages normally
		SetWindowPos(m_HScroll, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
};
template<class T, class TBase, class TWinTraits>
__declspec(selectany) LPVOID CWindowImplWithScrollBar<T, TBase, TWinTraits>::m_spOldSetScrollInfo = NULL;

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

template<class T>
class CStaticUnknownImpl : public T
{
public:
	virtual HRESULT STDMETHODCALLTYPE T::QueryInterface(REFIID riid, void **ppvObject)
	{
		*ppvObject = NULL;
		HRESULT hr = S_OK;
		if(riid == IID_IUnknown || riid == __uuidof(T))
		{
			*ppvObject = this;
		}
		else
		{
			hr = E_NOINTERFACE;
		}
		return hr;
	}
	virtual ULONG STDMETHODCALLTYPE T::AddRef(void)
	{
		return 1;
	}
	virtual ULONG STDMETHODCALLTYPE T::Release(void)
	{
		return 0;
	}
};

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowWithDragDrop;
template<class T, class TBase, class TWinTraits>
class ATL_NO_VTABLE CWindowWithDragDrop : public CWindowImplWithScrollBar<T, TBase, TWinTraits>,
	public CStaticUnknownImpl<IDropSource>, 
	public CStaticUnknownImpl<IDataObject>,
	public CStaticUnknownImpl<IDropTarget>
{
public:
	class CDragDropHelperWnd : public CWindowImplEx<CDragDropHelperWnd>
	{
	public:
		DECLARE_WND_CLASS(_T("dragdrophelper12"))
		BEGIN_MSG_MAP_EX(CDragDropHelperWnd)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_DESTROY(OnDestroy)
			CHAIN_MSG_MAP(CWindowImplEx)
			END_MSG_MAP()
			CDragDropHelperWnd()
		{
			m_pEffect = NULL;
			m_pImg = NULL;
		}
		void SetImage(ISonicImage * pImg)
		{
			m_pImg = pImg;
		}
	private:
		ISonicWndEffect * m_pEffect;
		ISonicImage * m_pImg;
	protected:
		int OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			m_pEffect = GetSonicUI()->CreateWndEffect();
			m_pEffect->Attach(m_hWnd);
			m_pEffect->SetShapeByImage(m_pImg);
			m_pEffect->SetLayeredAttributes(0, 0x80, 2);
			SetMsgHandled(FALSE);
			return 0;
		}
		void OnDestroy()
		{
			GetSonicUI()->DestroyObject(m_pEffect);
		}
	};
public:
	typedef struct tagDragInfo
	{
		ISonicImage * pImg;
		union
		{
			int nItem;
			HTREEITEM hItem;
		};
		POINT ptStartItem;
		tagDragInfo()
		{
			memset(this, 0, sizeof(tagDragInfo));
			pImg = GetSonicUI()->CreateImage();
		}
		~tagDragInfo()
		{
			GetSonicUI()->DestroyObject(pImg);
		}
	}DRAG_INFO;
	BEGIN_MSG_MAP_EX(CWindowWithDragDrop)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		CHAIN_MSG_MAP(CWindowImplWithScrollBar)
	END_MSG_MAP()
public:
	CWindowWithDragDrop()
	{
		m_bTrackMouse = FALSE;
		m_bDragging = FALSE;
		m_bAllowDrag = FALSE;
		m_bEnableDrag = FALSE;
		m_dwTimeStamp = 0;
		m_bManualCancel = FALSE;
	}
	virtual void OnDragLeave(){};
	virtual void OnDragOver(DWORD grfKeyState, POINT pt){};
	virtual void OnDrop(IDataObject * pDataObject, DWORD grfKeyState, POINT pt, DWORD dwOKEffect){};

	// if you enable this option, remember to call OleInitialize at the beginning of application
	void EnableDrag(BOOL b)
	{
		m_bEnableDrag = b;
	}

	HRESULT DoDragDrop(DWORD dwOKEffect, LPDWORD pdwEffect)
	{
		if(!OnSetDragInfo(&m_DragInfo))
		{
			return 0;
		}
		m_bManualCancel = FALSE;	
		m_dwTimeStamp = GetTickCount();
		ClientToScreen(&m_DragInfo.ptStartItem);

		m_wndHelper.SetImage(m_DragInfo.pImg);
		m_wndHelper.Create(NULL, CRect(0, 0, 1, 1), NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
		m_wndHelper.SetWindowPos(NULL, m_DragInfo.ptStartItem.x, m_DragInfo.ptStartItem.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
		HRESULT hRet = ::DoDragDrop(this, this, DROPEFFECT_MOVE, pdwEffect);
		m_bDragging = FALSE;
		m_wndHelper.DestroyWindow();
		return hRet;
	}
private:
	BOOL m_bTrackMouse;
	POINT m_ptStartMouse;
	DWORD m_dwTimeStamp;
	CDragDropHelperWnd m_wndHelper;
protected:
	static UINT s_uCoreFmtType;
	BOOL m_bEnableDrag;
	BOOL m_bDragging;
	BOOL m_bAllowDrag;
	BOOL m_bManualCancel;
	DRAG_INFO m_DragInfo;
protected:
	virtual BOOL OnSetDragInfo(DRAG_INFO * pInfo) = 0;
	virtual BOOL TestCanDrag(POINT pt){ return TRUE; }
	void OnMouseLeave()
	{
		m_bTrackMouse = FALSE;
		SetMsgHandled(FALSE);
	}
	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if(!m_bTrackMouse && !GetSonicUI()->GetOffscreenUICallback())
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.hwndTrack = m_hWnd;
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = 0;
			TrackMouseEvent(&tme);
			m_bTrackMouse = TRUE;
		}
		if(m_bDragging)
		{
			CPoint pt = point;
			ClientToScreen(&pt);
			const int nDragOffset = 2;
			if(abs(pt.x - m_ptStartMouse.x) > nDragOffset || abs(pt.y - m_ptStartMouse.y) > nDragOffset)
			{
				// begin drag
				DWORD dwRetEffect;
				DoDragDrop(DROPEFFECT_MOVE, &dwRetEffect);	
			}
		}
		SetMsgHandled(FALSE);
	}
	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		m_bDragging = FALSE;
		SetMsgHandled(FALSE);
	}
	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		if(m_bEnableDrag)
		{
			GetCursorPos(&m_ptStartMouse);
			if(TestCanDrag(m_ptStartMouse))
			{
				m_bDragging = TRUE;
			}
		}
		SetMsgHandled(FALSE);
	}
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		RegisterDragDrop(m_hWnd, this);
		if(!s_uCoreFmtType)
		{
			s_uCoreFmtType = RegisterClipboardFormat(_T("CoreFmtType"));
		}
		SetMsgHandled(FALSE);
		return 0;
	}
	void OnDestroy()
	{
		RevokeDragDrop(m_hWnd);
		SetMsgHandled(FALSE);
	}

	// com interface implementations
	// IDataObject
	STDMETHODIMP SetData ( FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease ){return E_NOTIMPL;}
	STDMETHODIMP GetData ( FORMATETC* pformatetcIn, STGMEDIUM* pmedium ){return E_NOTIMPL;}
	STDMETHODIMP EnumFormatEtc ( DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc ){return E_NOTIMPL;}
	STDMETHODIMP GetDataHere(FORMATETC* pformatetc, STGMEDIUM *pmedium){return E_NOTIMPL;}
	STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut){return E_NOTIMPL;}
	STDMETHODIMP DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection){return E_NOTIMPL;}
	STDMETHODIMP DUnadvise(DWORD dwConnection){return E_NOTIMPL;}
	STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise){return E_NOTIMPL;}
	STDMETHODIMP QueryGetData ( FORMATETC* pformatetc )
	{
		if(pformatetc->cfFormat == s_uCoreFmtType)
		{
			return S_OK;
		}
		return DV_E_FORMATETC;
	}

	// IDropSource
	STDMETHODIMP QueryContinueDrag ( BOOL fEscapePressed, DWORD grfKeyState )
	{
		POINT pt;
		GetCursorPos(&pt);
		m_wndHelper.SetWindowPos(NULL, pt.x + (m_DragInfo.ptStartItem.x - m_ptStartMouse.x), pt.y + (m_DragInfo.ptStartItem.y - m_ptStartMouse.y), 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		// process auto scroll
		CRect rtClient;
		GetClientRect(rtClient);
		ScreenToClient(&pt);
		DWORD dwTime = GetTickCount();
		int nDistance = 0;
		if(pt.y > rtClient.bottom)
		{
			nDistance = pt.y - rtClient.bottom;
		}
		else if(pt.y < rtClient.top)
		{
			nDistance = rtClient.top - pt.y;
		}
		if(nDistance)
		{
			const int nMaxDis = 50;
			nDistance = nDistance > nMaxDis ? nMaxDis: nDistance;
			DWORD dwInterval = 10 + 4 * (nMaxDis - nDistance);
			if(dwTime - m_dwTimeStamp > dwInterval)
			{			
				m_dwTimeStamp = dwTime;	
				if(pt.y > rtClient.bottom)
				{
					SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0));
				}
				else if(pt.y < rtClient.top)
				{
					SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0));
				}
			}
		}

		if(fEscapePressed || (grfKeyState & MK_RBUTTON) || m_bManualCancel)
		{
			return DRAGDROP_S_CANCEL;
		}
		if(grfKeyState & MK_LBUTTON)
		{
			return S_OK;
		}
		else
		{
			return DRAGDROP_S_DROP;
		}
	}
	STDMETHODIMP GiveFeedback ( DWORD dwEffect )
	{
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

	// IDropTarget
	STDMETHOD(DragEnter)(IDataObject * pDataObject,DWORD grfKeyState,POINTL pt,DWORD * pdwEffect)
	{
		FORMATETC fmt = {s_uCoreFmtType, NULL, 0, 0, 0};
		FORMATETC fmtFile = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		if((m_bDragging && pDataObject->QueryGetData(&fmt) == S_OK) || pDataObject->QueryGetData(&fmtFile) == S_OK)
		{
			m_bAllowDrag = TRUE;
		}
		else
		{
			*pdwEffect = DROPEFFECT_NONE;
			m_bAllowDrag = FALSE;
		}
		return S_OK;
	}
	STDMETHOD(DragOver)(DWORD grfKeyState,POINTL ptl,DWORD * pdwEffect)
	{
		if(!m_bAllowDrag)
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
		else
		{
			POINT pt = {ptl.x, ptl.y};
			OnDragOver(grfKeyState, pt);
		}
		return S_OK;
	}
	STDMETHOD(DragLeave)()
	{
		if(m_bAllowDrag)
		{
			OnDragLeave();
		}
		m_bAllowDrag = FALSE;
		return S_OK;
	}
	STDMETHOD(Drop)(IDataObject * pDataObject,DWORD grfKeyState,POINTL ptl,DWORD * pdwEffect)
	{
		if(!m_bAllowDrag)
		{
			*pdwEffect = DROPEFFECT_NONE;
		}
		else
		{
		}
		POINT pt = {ptl.x, ptl.y};
		OnDrop(pDataObject, grfKeyState, pt, *pdwEffect);
		m_bAllowDrag = FALSE;
		return S_OK;
	}

};
template<class T, class TBase, class TWinTraits>
__declspec(selectany) UINT CWindowWithDragDrop<T, TBase, TWinTraits>::s_uCoreFmtType = 0;

//////////////////////////////////////////////////////////////////////////
// CBalloonTip, pop up a balloon tip to prompt user
//////////////////////////////////////////////////////////////////////////
class IBalloonTipCallback
{
public:
	enum enRetId
	{
		RI_NONE,
		RI_OK,
		RI_CANCEL,
		RI_OVERTIME,
	};
public:
	virtual BOOL OnBalloonEnd(UINT uID, LPVOID pParam) = 0;
};

class CBalloonTip : public CWindowImplEx<CBalloonTip>
{
public:
	enum enBalloonStyle
	{
		BS_NONE = 0,
		BS_TOPMOST = 0x1,
		BS_TRANPARENTFORMSG = 0x2,
	};
	enum 
	{
		CLIENT_HOR_MARGIN = 15,
		CLIENT_VER_MARGIN = 10,
		TIMER_LIVE = 1,
		TIMER_FOLLOW,
	};

	DECLARE_WND_CLASS(_T("CyboBalloonTip"));

	BEGIN_MSG_MAP(CBalloonTip)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		CHAIN_MSG_MAP(CWindowImplEx)
	END_MSG_MAP()

private:
	// clear时无需清除
	ISonicImage * m_pImgBg;
	ISonicImage * m_pImgClose;
	ISonicImage * m_pImgButton;
	int m_nWidth;
	CRect m_rtMargin;

	// 需清除的成员
	HWND m_hParent;		// 此成员变量位置不要变
	IBalloonTipCallback * m_pCallback;
	LPVOID m_pParam;
	WORD m_wLiveTimeOver;	// million seconds
	WORD m_wMouseIn;
	ISonicPaint * m_pClientPaint;
	POINT m_ptParentTopleft;
	POINT m_ptShowPos;
	ISonicWndEffect * m_pEffect;	// 此成员变量位置不要变


public:
	CBalloonTip(ISonicImage * pBg = NULL, ISonicImage * pClose = NULL, ISonicImage * pButton = NULL)
	{
		InitValue();
		SetImage(pBg, pClose, pButton);
		m_rtMargin.SetRect(CLIENT_HOR_MARGIN, CLIENT_VER_MARGIN, CLIENT_HOR_MARGIN, CLIENT_VER_MARGIN);
		m_nWidth = 0;
	}
	~CBalloonTip()
	{
		Clear();
	}

	void InitValue()
	{
		memset(&m_hParent, 0, (PBYTE)&m_pEffect - (PBYTE)&m_hParent);
	}
	void Clear()
	{
		SONICUI_DESTROY(m_pClientPaint);
		SONICUI_DESTROY(m_pEffect);
		InitValue();
	}
	void FixRightMargin()
	{
		if(m_pImgClose && m_pImgClose->GetWidth() + 3 > m_rtMargin.right)
		{
			m_rtMargin.right = m_pImgClose->GetWidth() + 3;
		}
	}
	void SetMargin(int l, int t, int r, int b)
	{
		m_rtMargin.SetRect(l, t, r, b);
		FixRightMargin();
	}
	void SetImage(ISonicImage * pBg, ISonicImage * pClose = NULL, ISonicImage * pButton = NULL)
	{
		m_pImgBg = pBg;
		m_pImgClose = pClose;
		m_pImgButton = pButton;
		FixRightMargin();
		
	}
	// if nWidth is greater than 0, the width of balloon tip is fixed.
	// if nWidth is 0, the width of balloon tip is no limited and auto fit for the width of text.
	// if nWidth is less than 0, the max width of balloon tip is limited to abs(nWidth)
	void SetWidth(int nWidth)
	{
		m_nWidth = nWidth;
	}	
	BOOL Create(HWND hWndParent, DWORD dwBalloonStyle = BS_TOPMOST, LPCTSTR lpszText = NULL, LPCTSTR lpszTitle = NULL, LPCTSTR lpszBtnOK = NULL, LPCTSTR lpszBtnCancel = NULL, IBalloonTipCallback * pCallback = NULL, LPVOID pParam = NULL)
	{
		if(m_pImgBg == NULL)
		{
			return FALSE;
		}
		if(m_hWnd && IsWindow())
		{
			DestroyWindow();
		}
		m_hParent = hWndParent;
		RECT rt = {0, 0, 1, 1};
		DWORD dwExStyle = WS_EX_TOOLWINDOW;
		if(dwBalloonStyle & BS_TOPMOST)
		{
			dwExStyle |= WS_EX_TOPMOST;
		}
		if(dwBalloonStyle & BS_TRANPARENTFORMSG)
		{
			dwExStyle |= WS_EX_TRANSPARENT;
		}
		if(!CWindowImplEx::Create(hWndParent, &rt, NULL, WS_POPUP, dwExStyle))
		{
			return FALSE;
		}

		m_pEffect = GetSonicUI()->CreateWndEffect();
		m_pEffect->Attach(m_hWnd, TRUE);
		m_pEffect->Delegate(DELEGATE_EVENT_SHOWGENTLY_OVER, NULL, this, &CBalloonTip::OnShowGentlyOver);
		m_pEffect->Delegate(DELEGATE_EVENT_MOUSEENTER, NULL, this, &CBalloonTip::OnMouseEnter);
		m_pEffect->Delegate(DELEGATE_EVENT_MOUSELEAVE, NULL, this, &CBalloonTip::OnMouseLeave);
		m_pEffect->GetSkin()->SetSkin(_T("bg"), _T("image:%d"), m_pImgBg->GetObjectId());

		// handle title
		if(lpszTitle)
		{
			const TCHAR szGlobal[] = _T("/global, align=1, font, font_face='Tahoma', font_height=14, font_bold=1, c=0x444444/");
			CString strTitle;
			strTitle.Format(_T("%s%s"), szGlobal, lpszTitle);
			m_pEffect->GetSkin()->SetSkin(_T("bg"), _T("title_text:%s"), strTitle);
		}
		if(m_pImgClose)
		{
			m_pEffect->GetSkin()->SetSkin(_T("bg"), _T("close:%d|Exit"), m_pImgClose->GetObjectId());
		}

		// set callback handler
		Delegate(pCallback, pParam);

		// create client paint
		m_pClientPaint = GetSonicUI()->CreatePaint();
		m_pClientPaint->Create(FALSE);

		// create text
		int x = -1;
		int y = 20;
		if(lpszText)
		{
			ISonicString * pText = GetSonicUI()->CreateString();
			pText->Format(_T("/global, c=555555, font, font_face='Tahoma', line_space=5/%s"), lpszText);
			m_pClientPaint->AddObject(pText->GetObjectId(), 0, 0, TRUE);
			y += pText->GetHeight();
		}

		// create OK and Cancel button
		if(lpszBtnCancel)
		{
			ISonicString * pBtnCancel = GetSonicUI()->CreateString();
			if(m_pImgButton)
			{
				pBtnCancel->Format(_T("/a, p=%d, btn_text='%s'/"), m_pImgButton->GetObjectId(), lpszBtnCancel);
			}
			else
			{
				pBtnCancel->Format(_T("/a, c=0xff0000, linkl=2/%s"), lpszBtnCancel);
			}
			pBtnCancel->Delegate(DELEGATE_EVENT_CLICK, (LPVOID)(INT_PTR)IBalloonTipCallback::RI_CANCEL, this, &CBalloonTip::OnClick);
			m_pClientPaint->AddObject(pBtnCancel->GetObjectId(), x, y, TRUE);
			x -= pBtnCancel->GetWidth() + 30;
		}
		if(lpszBtnOK)
		{
			ISonicString * pBtnOK = GetSonicUI()->CreateString();
			if(m_pImgButton)
			{
				pBtnOK->Format(_T("/a, p=%d, btn_text='%s'/"), m_pImgButton->GetObjectId(), lpszBtnOK);
			}
			else
			{
				pBtnOK->Format(_T("/a, c=0xff0000, linkl=2/%s"), lpszBtnOK);
			}
			pBtnOK->Delegate(DELEGATE_EVENT_CLICK, (LPVOID)(INT_PTR)IBalloonTipCallback::RI_OK, this, &CBalloonTip::OnClick);
			m_pClientPaint->AddObject(pBtnOK->GetObjectId(), x, y, TRUE);
		}
		// limit the text width width m_nWidth
		if(m_nWidth > 0 || (m_nWidth < 0 && m_pClientPaint->GetWidth() + m_rtMargin.left + m_rtMargin.right > -m_nWidth))
		{
			m_pClientPaint->Resize(abs(m_nWidth) - m_rtMargin.left - m_rtMargin.right);
		}
		ReposWithClientPaint();
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	// refer to FUNC_OnBalloonEnd format
	//////////////////////////////////////////////////////////////////////////
	void Delegate(IBalloonTipCallback * pCallback, LPVOID pParam)
	{
		m_pCallback = pCallback;
		m_pParam = pParam;
	}

	void AddObject(DWORD dwObjectId, int x = 0, int y = 0, BOOL bDestroyedOnClear = FALSE)
	{
		if(m_hWnd == NULL)
		{
			return;
		}
		m_pClientPaint->AddObject(dwObjectId, x, y, bDestroyedOnClear);
		ReposWithClientPaint();
		
	}

	void Show(int x, int y, int nLiveTime = 0)
	{
		if(m_hWnd == NULL)
		{
			return;
		}
        MoveBalloon(x, y);
		m_pEffect->ShowGently(TRUE);
		if(nLiveTime)
		{
			SetTimer(TIMER_LIVE, nLiveTime);
		}
		if(m_hParent)
		{
			CRect rtWnd;
			::GetWindowRect(m_hParent, rtWnd);
			m_ptParentTopleft = rtWnd.TopLeft();
			m_ptShowPos.x = x;
			m_ptShowPos.y = y;
			SetTimer(TIMER_FOLLOW, 50);
		}
	}

	void Destroy()
	{
		KillTimer(TIMER_LIVE);
		if(m_pEffect)
		{
			m_pEffect->ShowGently(FALSE);
		}
	}
	
protected:
	void ReposWithClientPaint()
	{
		int nTitleHeight;
		m_pEffect->GetSkin()->GetAttr(_T("SonicAttrTitleHeight"), nTitleHeight);
		CRect rtWnd(0, 0, m_pClientPaint->GetWidth() + m_rtMargin.left + m_rtMargin.right, m_pClientPaint->GetHeight() + m_rtMargin.top + m_rtMargin.bottom + nTitleHeight);
		SetWindowPos(NULL, rtWnd, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		if(!m_pEffect->GetSkin()->GetPaint()->GetPaintItem(m_pClientPaint->GetObjectId()))
		{
			m_pEffect->GetSkin()->GetPaint()->AddObject(m_pClientPaint->GetObjectId(), m_rtMargin.left, nTitleHeight + m_rtMargin.top);
		}
	}
	void OvertimeDestroy()
	{
		if(m_wLiveTimeOver)
		{
			if(m_pCallback)
			{
				m_pCallback->OnBalloonEnd(IBalloonTipCallback::RI_OVERTIME, m_pParam);
			}
			Destroy();
		}
	}
	void MoveBalloon(int x, int y)
	{
		SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}
	void OnClick(ISonicString * pStr, LPVOID pParam)
	{
		DWORD dwButtonID = (DWORD)(DWORD_PTR)pParam;
		if(!m_pCallback || !m_pCallback->OnBalloonEnd(dwButtonID, m_pParam))
		{
			Destroy();
		}
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		KillTimer(TIMER_LIVE);
		Clear();		
		return 0;
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		if(!m_pCallback || !m_pCallback->OnBalloonEnd(IBalloonTipCallback::RI_CANCEL, m_pParam))
		{
			Destroy();
		}
		return 0;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		switch (wParam)
		{
		case TIMER_LIVE:
			{
				KillTimer(wParam);
				m_wLiveTimeOver = TRUE;
				if(!m_wMouseIn)
				{
					OvertimeDestroy();
				}
			}
			break;
		case TIMER_FOLLOW:
			{
				CRect rtWnd;
				::GetWindowRect(m_hParent, rtWnd);
				int x = m_ptShowPos.x + rtWnd.left - m_ptParentTopleft.x;
				int y = m_ptShowPos.y + rtWnd.top - m_ptParentTopleft.y;
				MoveBalloon(x, y);
				if(IsWindowVisible() && !::IsWindowVisible(m_hParent))
				{
					ShowWindow(SW_HIDE);
				}
				if(!IsWindowVisible() && ::IsWindowVisible(m_hParent))
				{
					ShowWindow(SW_SHOWNA);
				}
			}
			break;
		}
		return 0;
	}

	void OnShowGentlyOver(ISonicWndEffect *, LPVOID)
	{
		if(IsWindowVisible() == FALSE)
		{
			DestroyWindow();
		}
	}
	void OnMouseEnter(ISonicWndEffect *, LPVOID)
	{
		m_wMouseIn = TRUE;
		m_pEffect->ShowGently(TRUE);
	}
	void OnMouseLeave(ISonicWndEffect *, LPVOID)
	{
		m_wMouseIn = FALSE;
		if(m_wLiveTimeOver)
		{
			OvertimeDestroy();
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// CoreListBox
//////////////////////////////////////////////////////////////////////////
#define CORE_CB_BTNIMAGE		_T("CoreCBBtnImage")
template<class TBase, BOOL bIsComboBox = FALSE>
class CCoreListBoxT : public CWindowWithDragDrop<CCoreListBoxT<TBase>, TBase>
{
public:
	enum enCLBStyle
	{
		CLBS_FOCUS_TEXT_COLOR_CHANGE = 0x2,		// hilight text when item selected
		CLBS_CANCELSEL_WHEN_NOFOCUS = 0x4,		// cancel selecting state when the listbox loses focus
		CLBS_RBUTTON_SELECT = 0x20,				// select an item with right click
	};
	enum
	{
		ITEM_TOP_MARGIN = 1,
	};
	typedef struct tagItemData : public CSafeCrtBuffer
	{
		ISonicPaint * pPaint;
		BYTE bSelectable;
		BOOL bCanDrag;
		tagItemData() : pPaint(0), bSelectable(TRUE), bCanDrag(TRUE)
		{
		}
	}ITEM_DATA;
public:
	typedef std::vector<ITEM_DATA> VEC_ITEM_DATA;
	int m_nItemTopMargin;
protected:
	VEC_ITEM_DATA m_vecItemData;
private:
	DWORD m_dwInternalBorderColor;
	DWORD m_dwSelectColor;
	DWORD m_dwHotItemColor;
	DWORD m_dwComboLBBorderColor;
	DWORD m_dwCoreStyle;
	int m_nLastSel;
	BOOL m_bInternalInserting;
	ISonicString * m_pBtnDrop;
	ISonicImage * m_pImgWhole;
	HBRUSH m_hBGBrush;
	ISonicPaint * m_pClientPaint;
	int m_nDragHotItem;
	int m_nHotItem;
	DWORD m_dwDragHightlightLineColor;
public:
	class CCoreComboListHooker : public CWindowImplEx<CCoreComboListHooker, CListBox>
	{
	private:
		CCoreListBoxT * m_pHost;
	public:
		CCoreComboListHooker(CCoreListBoxT * pHost) : m_pHost(pHost){}
		BEGIN_MSG_MAP_EX(CCoreComboListHooker)
			MSG_WM_NCCALCSIZE(OnNcCalcSize)
			MSG_WM_NCPAINT(OnNcPaint)
			CHAIN_MSG_MAP(CWindowImplEx)
		END_MSG_MAP()
	protected:
		void DrawBorder()
		{
			CRect rtWnd;
			GetWindowRect(rtWnd);
			rtWnd.OffsetRect(-rtWnd.TopLeft());
			CWindowDC dc(m_hWnd);
			GetSonicUI()->Rectangle(dc, rtWnd.left, rtWnd.top, rtWnd.right, rtWnd.bottom, m_pHost->GetComboLBBorderColor());
		}
		void OnNcPaint(CRgn rgn)
		{
			DrawBorder();
		}
		LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
		{
			LRESULT lRet = DefWindowProc();
			if(bCalcValidRects && IsWindowVisible())
			{
				DrawBorder();
			}
			return lRet;
		}
	};
	
public:
	CCoreListBoxT(int nItemTopMargin = ITEM_TOP_MARGIN) : m_nItemTopMargin(nItemTopMargin)
	{
		m_dwSelectColor = GetSysColor(COLOR_HIGHLIGHT);
		m_dwHotItemColor = (DWORD)-1;
		m_dwInternalBorderColor = RGB(127, 157, 185);
		m_dwCoreStyle = CLBS_FOCUS_TEXT_COLOR_CHANGE;
		m_bInternalInserting = FALSE;
		m_pBtnDrop = NULL;
		m_hBGBrush = NULL;
		m_pImgWhole = NULL;
		m_nLastSel = -1;
		m_pClientPaint = NULL;
		m_dwComboLBBorderColor = 0;
		m_nDragHotItem = -1;
		m_nHotItem = -1;
		m_dwDragHightlightLineColor = RGB(255, 0, 0);
	}
	virtual ~CCoreListBoxT()
	{ 
		if(m_hBGBrush)
		{
			DeleteObject(m_hBGBrush);
		}
		if(m_hWnd && IsWindow())
		{
			OnDestroy();
		} 
	}
	BEGIN_MSG_MAP_EX(CCoreListBoxT)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		REFLECTED_COMMAND_CODE_HANDLER_EX(LBN_SELCHANGE, OnNotifySelChange)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(LB_SETCURSEL, OnSetCurSel)
		MESSAGE_HANDLER(LB_ADDSTRING, OnAddString)
		MESSAGE_HANDLER(LB_INSERTSTRING, OnInsertString)
		MESSAGE_HANDLER(LB_DELETESTRING, OnDeleteString)
		MESSAGE_HANDLER(CB_SETCURSEL, OnSetCurSel)
		MESSAGE_HANDLER(CB_ADDSTRING, OnAddString)
		MESSAGE_HANDLER(CB_INSERTSTRING, OnInsertString)
		MESSAGE_HANDLER(CB_DELETESTRING, OnDeleteString)
		MSG_OCM_CTLCOLOREDIT(OnReflectedCtlColorEdit)
		MSG_OCM_DRAWITEM(OnDrawItem)
		MSG_OCM_MEASUREITEM(OnMeasureItem)
		MSG_WM_SETCURSOR(OnSetCursor)
		CHAIN_MSG_MAP(CWindowWithDragDrop)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClick)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	void SetBGColor(DWORD dwColor)
	{ 
		GetSonicUI()->SkinFromHwnd(m_hWnd)->SetSkin(_T("bg"), _T("bg_color:%d"), dwColor);
		if(m_hBGBrush)
		{
			DeleteObject(m_hBGBrush);
		}
		m_hBGBrush = CreateSolidBrush(dwColor);
	}
	DWORD GetBGColor()
	{
		DWORD dwBGColor;
		GetSonicUI()->SkinFromHwnd(m_hWnd)->GetAttr(_T("SonicAttrColor"), (int &)dwBGColor, RGB(255, 255, 255));
		return dwBGColor;
	}	
	
	DWORD GetDragHighlightLineColor(){ return m_dwDragHightlightLineColor; }
	void SetDragHighlightLineColor(DWORD dwColor){ m_dwDragHightlightLineColor = dwColor; }
	void SetSelectColor(DWORD dwColor){ m_dwSelectColor = dwColor; }
	DWORD GetSelectColor(){ return m_dwSelectColor; }
	void SetHotItemColor(DWORD dwColor){ m_dwHotItemColor = dwColor; }
	void SetHotItem(int nItem){ ChangeHotItem(nItem); }
	DWORD GetHotItemColor(){ return m_dwHotItemColor; }
	void SetCoreStyle(DWORD dwCoreStyle){ if(m_dwCoreStyle != dwCoreStyle){m_dwCoreStyle = dwCoreStyle; InvalidateRect(NULL);} }
	DWORD GetCoreStyle(){ return m_dwCoreStyle; }
	ISonicPaint * GetClientPaint(){ return m_pClientPaint; }
	void SetComboLBBorderColor(DWORD dwColor){ m_dwComboLBBorderColor = dwColor; }
	DWORD GetComboLBBorderColor(){ return m_dwComboLBBorderColor; }
	int GetItemTopMargin(){ return m_nItemTopMargin; }

	void SetBorderColor(DWORD dwColor)
	{
		if(bIsComboBox)
		{
			m_dwInternalBorderColor = dwColor;
		}
		else
		{
			CWindowImplWithScrollBar::SetBorderColor(dwColor);
		}
	}
	DWORD GetBorderColor()
	{
		if(bIsComboBox)
		{
			return m_dwInternalBorderColor;
		}
		else
		{
			return CWindowImplWithScrollBar::GetBorderColor();
		}
	}

	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, 
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		if(bIsComboBox)
		{
			dwStyle &= ~CBS_OWNERDRAWFIXED;
			dwStyle |= CBS_OWNERDRAWVARIABLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST;
		}
		else
		{
			dwStyle &= ~LBS_OWNERDRAWFIXED;
			dwStyle |= LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS;
		}
		HWND hRet = CWindowImplWithScrollBar::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		if(hRet)
		{
			if(bIsComboBox)
			{
				COMBOBOXINFO cbi;
				cbi.cbSize = sizeof(cbi);
				::GetComboBoxInfo(hRet, &cbi);
				if(cbi.hwndList)
				{
					CCoreComboListHooker * pHooker = new CCoreComboListHooker(this);
					pHooker->SubclassWindowOnce(cbi.hwndList);
					pHooker->SetDeleteSelfOnDestroy(TRUE);
				}
			}
		}
		return hRet;
	}

	BOOL Sort(ICoreSorter<CCoreListBoxT<TBase>> * pSorter)
	{
		if(!pSorter)
		{
			return FALSE;
		}
		for(int j = GetCount() - 1; j > 0; j--)
		{
			BOOL bSwaped = FALSE;
			for(int i = 0; i < j; i++)
			{
				if(pSorter->Compare(this, i, i + 1) < 0)
				{
					SwapItem(i, i + 1);
					bSwaped = TRUE;
				}
			}
			if(!bSwaped)
			{
				break;
			}
		}
		return TRUE;
	}

	void MakeBtnDrop()
	{
		if(!m_pBtnDrop)
		{
			return;
		}
		COMBOBOXINFO cbi;
		cbi.cbSize = sizeof(cbi);
		::GetComboBoxInfo(m_hWnd, &cbi);
		CRect rtClient;
		CRect rcButton = cbi.rcButton;
		GetClientRect(rtClient);
		if(rtClient.Width() != m_pImgWhole->GetWidth())
		{
			ISonicImage * pImgBtnDrop = (ISonicImage *)m_pBtnDrop->GetAttr(CORE_CB_BTNIMAGE);
			m_pImgWhole->Clear();
			int nTileStatus = abs(pImgBtnDrop->GetTileStatus());
			nTileStatus = nTileStatus > 0 ? nTileStatus : 1;
			m_pImgWhole->Create(rtClient.Width(), rtClient.Height() * nTileStatus, 0, TRUE);
			int nImgHeight = pImgBtnDrop->GetTileStatus() < 0 ? pImgBtnDrop->GetHeight() / nTileStatus : pImgBtnDrop->GetHeight();
			int nImgWidth = pImgBtnDrop->GetTileStatus() < 0 ? pImgBtnDrop->GetWidth() : pImgBtnDrop->GetWidth() / nTileStatus;
			for(int i = 0; i < nTileStatus; i++)
			{
				DRAW_PARAM dp;
				dp.dwMask = DP_SRC_CLIP;
				if(pImgBtnDrop->GetTileStatus() > 0)
				{
					dp.rtSrc = CRect(i * pImgBtnDrop->GetWidth() / nTileStatus, 0, (i + 1) * pImgBtnDrop->GetWidth() / nTileStatus, pImgBtnDrop->GetHeight());
					pImgBtnDrop->Draw(m_pImgWhole->GetDC(), rtClient.right - 19 , rcButton.top + i * rtClient.Height() + (rcButton.Height() - nImgHeight) / 2, &dp);
				}
				else
				{
					dp.rtSrc = CRect(0, i * pImgBtnDrop->GetHeight() / nTileStatus, pImgBtnDrop->GetWidth(), (i + 1) * pImgBtnDrop->GetHeight() / nTileStatus);
					pImgBtnDrop->Draw(m_pImgWhole->GetDC(), rtClient.right - 19, rcButton.top + i * rtClient.Height() + (rcButton.Height() - nImgHeight) / 2, &dp);
				}
			}
			if(pImgBtnDrop->GetTileStatus())
			{
				m_pImgWhole->SetTileStatus(-abs(pImgBtnDrop->GetTileStatus()));
			}
			m_pBtnDrop->Clear();
			m_pBtnDrop->Format(_T("/a, p=%d, linkc=0, btn_type=2, pass_msg=1/"), m_pImgWhole->GetObjectId());
			m_pBtnDrop->SetAttr(CORE_CB_BTNIMAGE, pImgBtnDrop);
		}
	}

	void SetDropBtnImage(ISonicImage * pImage)
	{
		if(!bIsComboBox)
		{
			return;
		}
		if(!m_pBtnDrop)
		{
			m_pBtnDrop = GetSonicUI()->CreateString();
		}
		if(!m_pImgWhole)
		{
			m_pImgWhole = GetSonicUI()->CreateImage();
		}
		m_pBtnDrop->Format(_T("/a, p=%d/"), pImage->GetObjectId());
		m_pBtnDrop->SetAttr(CORE_CB_BTNIMAGE, pImage);
	}

	int GetIndexByPaint(ISonicPaint * pPaint)
	{
		for (size_t i = 0; i < m_vecItemData.size(); i++)
		{
			if(m_vecItemData[i].pPaint == pPaint)
			{
				return (int)i;
			}
		}
		return -1;
	}

	int AddObject(int nIndex, ISonicBase * pBase, int x = 0, int y = 0, BOOL bDestoryedOnClear = FALSE, BOOL bNewItem = FALSE)
	{
		ISonicPaint * pPaint = NULL;
		int nCount = GetCount();
		nIndex = nIndex < 0 ? nCount : nIndex;

		CRect rtClient;
		GetClientRect(rtClient);
		if(nIndex >= (int)m_vecItemData.size())
		{
			nIndex = nCount;
			ITEM_DATA data;
			data.pPaint = GetSonicUI()->CreatePaint();
			data.pPaint->Create(FALSE, rtClient.Width());
			pPaint = data.pPaint;
			m_vecItemData.push_back(data);
			bNewItem = TRUE;
		}
		else
		{
			if(bNewItem)
			{
				ITEM_DATA data;
				data.pPaint = GetSonicUI()->CreatePaint();
				data.pPaint->Create(FALSE, rtClient.Width());
				pPaint = data.pPaint;
				m_vecItemData.insert(m_vecItemData.begin() + nIndex, data);
			}
			else
			{
				pPaint = m_vecItemData[nIndex].pPaint;
			}
		}
		pPaint->AddObject(pBase->GetObjectId(), x, y, bDestoryedOnClear);

		// delete and insert to trigger WM_MEASUREITEM to renew the item region
		m_bInternalInserting = TRUE;
		if(!bNewItem && (GetWindowLong(GWL_STYLE) & LBS_OWNERDRAWVARIABLE))
		{
			DeleteString(nIndex);
		}		
		ISonicString * pFirst = (ISonicString *)pPaint->GetPaintItem(0, TRUE)->pBase;
		if(pFirst && pFirst->GetType() == BASE_TYPE_STRING)
		{
			CWindowImplWithScrollBar::InsertString(nIndex, pFirst->GetTextWithoutCtrl());
		}
		else
		{
			CWindowImplWithScrollBar::InsertString(nIndex, _T(""));
		}
		m_bInternalInserting = FALSE;
		if((GetWindowLong(GWL_STYLE) & LBS_OWNERDRAWFIXED) && nIndex == 0)
		{
			SetItemHeight(nIndex, pPaint->GetHeight() + m_nItemTopMargin * 2);
		}
		return nIndex;
	}
	
	BOOL IsItemSelectable(int nIndex)
	{
		if(nIndex < 0 || nIndex >= GetCount())
		{
			return FALSE;
		}
		return m_vecItemData[nIndex].bSelectable;
	}
	void EnableItemSelectable(int nIndex, BOOL b)
	{
		if(nIndex < 0 || nIndex >= GetCount())
		{
			return;
		}
		m_vecItemData[nIndex].bSelectable = b;
	}

	BOOL IsItemCanDrag(int nIndex)
	{
		if(nIndex < 0 || nIndex >= GetCount())
		{
			return FALSE;
		}
		return m_vecItemData[nIndex].bCanDrag;
	}
	void EnableItemCanDrag(int nIndex, BOOL b)
	{
		if(nIndex < 0 || nIndex >= GetCount())
		{
			return;
		}
		m_vecItemData[nIndex].bCanDrag = b;
	}

	int AddString(LPCTSTR lpszItem, int x = 0, int y = 0)
	{
		ISonicString * pStr = GetSonicUI()->CreateString();
		pStr->Format(_T("%s"), lpszItem);
		return AddObject(GetCount(), pStr, x, y, TRUE, TRUE);
	}

	ISonicPaint * GetItemPaint(int nIndex)
	{
		if(nIndex < 0 || nIndex >= (int)m_vecItemData.size())
		{
			return NULL;
		}
		return m_vecItemData[nIndex].pPaint;
	}

	int InsertString(int nIndex, LPCTSTR lpszItem, int x = 0, int y = 0)
	{
		ISonicString * pStr = GetSonicUI()->CreateString();
		pStr->Format(_T("%s"), lpszItem);
		return AddObject(nIndex, pStr, x, y, TRUE, TRUE);
	}

	void DeleteAllItems()
	{
		while (GetCount())
		{
			DeleteString(0);
		}
	}

	int SetItemHeight(int nIndex, UINT cyItemHeight)
	{
		if(nIndex < 0 || nIndex >= (int)m_vecItemData.size())
		{
			return LB_ERR;
		}
		m_vecItemData[nIndex].pPaint->Resize(0, cyItemHeight - m_nItemTopMargin * 2);
		int nRet = CWindowImplWithScrollBar::SetItemHeight(nIndex, cyItemHeight);
		// this message force the system to call SetScrollPos
		SendMessage(WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0));
		return nRet;
	}
	virtual void SwapItem(int i, int j)
	{
		ITEM_DATA data = m_vecItemData[i];
		m_vecItemData[i] = m_vecItemData[j];
		m_vecItemData[j] = data;
		LPVOID pUserData = GetItemDataPtr(i);
		SetItemDataPtr(i, GetItemDataPtr(j));
		SetItemDataPtr(j, pUserData);
		InvalidateRect(NULL);
	}
	int HitTest(CPoint point)
	{
		CRect rtClient;
		GetClientRect(rtClient);
		int nStart = GetTopIndex();
		if(nStart == LB_ERR)
		{
			return LB_ERR;
		}
		int nCount = GetCount();
		if(nCount == LB_ERR)
		{
			return LB_ERR;
		}
		int nTop = 0;
		CRect rtItem;
		for(int i = nStart; i < nCount; i++)
		{
			rtItem.SetRect(0, nTop, rtClient.Width(), nTop + m_vecItemData[i].pPaint->GetHeight() + m_nItemTopMargin * 2);
			if(rtItem.PtInRect(point))
			{
				return i;
			}
			if(rtItem.bottom >= rtClient.bottom)
			{
				break;
			}
			nTop += rtItem.Height();
		}
		return LB_ERR;
	}
protected:
	virtual void OnDrop(IDataObject * pDataObject, DWORD grfKeyState, POINT pt, DWORD dwOKEffect)
	{
		if(dwOKEffect == DROPEFFECT_NONE)
		{
			// invalid drop target or operation is canceled
		}
		else
		{
			int nIndex = m_nDragHotItem;
			// there is a special case that dwOKEffect is ok but nIndex is -1, which is caused by dropping the item into the system recycle bin.
			if(m_bDragging)
			{
				// this drag operation was launched by listbox itself 
				if(nIndex >= 0 && nIndex != m_DragInfo.nItem && nIndex != m_DragInfo.nItem + 1)
				{
					CORE_DROP_DATA dp = {0};
					dp.hCtrl = m_hWnd;
					dp.nOldIndex = m_DragInfo.nItem;
					dp.nNewIndex = nIndex;
					// inquiry whether this drop operation should continue
					if(!GetParent().SendMessage(WM_COREDROP, 0, (LPARAM)&dp))
					{
						// a valid drag&drop, duplicate the original drag item
						m_bInternalInserting = TRUE;
						// here a "temp" variable is for stl safety purpose. vector will release its memory before inserting if it's not big enough
						ITEM_DATA temp = m_vecItemData[m_DragInfo.nItem];
						m_vecItemData.insert(m_vecItemData.begin() + nIndex, temp);
						CWindowImplWithScrollBar::InsertString(nIndex, _T(""));				
						if(nIndex < m_DragInfo.nItem)
						{
							// the inserting operation increases the original index by 1
							m_DragInfo.nItem++;
						}
						SetItemDataPtr(nIndex, GetItemDataPtr(m_DragInfo.nItem));
						// remove the original drag item
						m_vecItemData.erase(m_vecItemData.begin() + m_DragInfo.nItem);
						DeleteString(m_DragInfo.nItem);
						m_bInternalInserting = FALSE;
						if(m_DragInfo.nItem < nIndex)
						{
							nIndex--;
						}
						SendMessage(LB_SETCURSEL, -1, 1);
						SetCurSel(nIndex);
						// notify parent a drag&drop has been successfully completed
						memset(&dp, 0, sizeof(dp));
						dp.hCtrl = m_hWnd;
						dp.nNewIndex = nIndex;
						GetParent().SendMessage(WM_COREDROP, 1, (LPARAM)&dp);
					}
				}
			}
			else
			{
				FORMATETC fmtFile = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
				STGMEDIUM stgMedium;
				if(pDataObject->GetData(&fmtFile, &stgMedium) == S_OK)
				{
					HDROP hDrop = (HDROP)GlobalLock(stgMedium.hGlobal);
					if(hDrop)
					{
						UINT uFileNum = DragQueryFile(hDrop, -1, NULL, 0);
						if(uFileNum >= 0)
						{
							TCHAR szPath[MAX_PATH];
							if(DragQueryFile(hDrop, 0, szPath, MAX_PATH))
							{
								CORE_DROP_DATA dp = {0};
								dp.uDropType = cdtFile;
								dp.hCtrl = m_hWnd;
								dp.file.nFileNum = (int)uFileNum;
								dp.file.lpszFilePath = szPath;
								dp.nNewIndex = nIndex;
								GetParent().SendMessage(WM_COREDROP, 1, (LPARAM)&dp);
							}
						}
						GlobalUnlock(stgMedium.hGlobal);
					}
					ReleaseStgMedium(&stgMedium);
				}
			}
		}
		ChangeDragHotItem(-1);
	}
	void ChangeDragHotItem(int nItem)
	{
		if(m_nDragHotItem != nItem)
		{
			m_nDragHotItem = nItem;
			InvalidateRect(NULL);
		}
	}
	void ChangeHotItem(int nItem)
	{
		if(m_nHotItem != nItem)
		{
			m_nHotItem = nItem;
			if(m_dwHotItemColor != -1)
			{
				InvalidateRect(NULL);
			}
		}
	}
	virtual void OnDragLeave()
	{
		ChangeDragHotItem(-1);
	}
	virtual void OnDragOver(DWORD grfKeyState, POINT pt)
	{
		ScreenToClient(&pt);
		int nHover = HitTest(pt);
		nHover = nHover < 0 ? GetCount() - 1 : nHover;
		if(nHover < 0)
		{
			return;
		}
		CRect rt = GetItemPaint(nHover)->GetMsgRect();
		if(pt.y > rt.bottom - rt.Height() / 2)
		{
			ChangeDragHotItem(nHover + 1);
		}
		else
		{
			ChangeDragHotItem(nHover);
		}
	}
	virtual BOOL TestCanDrag(POINT pt)
	{
		ScreenToClient(&pt);
		int nIndex = HitTest(pt);
		if(nIndex >= 0 && nIndex < (int)m_vecItemData.size())
		{
			return m_vecItemData[nIndex].bCanDrag;
		}
		return FALSE;
	}
	virtual BOOL OnSetDragInfo(DRAG_INFO * pInfo)
	{
		pInfo->nItem = GetCurSel();
		if(pInfo->nItem < 0)
		{
			return FALSE;
		}
		CRect rt = GetItemPaint(pInfo->nItem)->GetMsgRect();
		pInfo->ptStartItem.x = rt.left;
		pInfo->ptStartItem.y = rt.top;
		pInfo->pImg->Clear();
		pInfo->pImg->Create(rt.Width(), rt.Height());
		GetItemPaint(pInfo->nItem)->Move(0, 0);
		GetItemPaint(pInfo->nItem)->Render(pInfo->pImg->GetDC());
		GetItemPaint(pInfo->nItem)->Move(rt.left, rt.top);
		return TRUE;
	}
	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		int nIndex = HitTest(point);
		if(nIndex >= 0)
		{
			DefWindowProc();
			NMHDR nm;
			nm.code = NM_CLICK;
			nm.hwndFrom = m_hWnd;
			nm.idFrom = GetDlgCtrlID();
			GetParent().SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	void OnRButtonDown(UINT nFlags, CPoint point)
	{
		DefWindowProc();
		int nIndex = HitTest(point);
		if(nIndex >= 0)
		{
			if(m_dwCoreStyle & CLBS_RBUTTON_SELECT)
			{			
				SetCurSel(nIndex);
			}
			NMHDR nm;
			nm.code = NM_RCLICK;
			nm.hwndFrom = m_hWnd;
			nm.idFrom = GetDlgCtrlID();
			GetParent().SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	void OnLButtonDblClick(UINT nFlags, CPoint point)
	{
		if(!bIsComboBox)
		{
			DefWindowProc();
		}
		int nIndex = HitTest(point);
		if(nIndex >= 0)
		{
			NMHDR nm;
			nm.code = NM_DBLCLK;
			nm.hwndFrom = m_hWnd;
			nm.idFrom = GetDlgCtrlID();
			GetParent().SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	LRESULT OnNotifySelChange(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		int nSel = GetCurSel();
		if(nSel >= 0 && nSel < GetCount())
		{
			if(!m_vecItemData[nSel].bSelectable)
			{
				SetCurSel(m_nLastSel);
			}
			else
			{
				m_nLastSel = nSel;
			}
			
		}
		return 0;
	}
	HBRUSH OnReflectedCtlColorEdit(CDCHandle dc, CEdit edit)
	{
		if(!m_hBGBrush)
		{
			SetMsgHandled(FALSE);
			return NULL;
		}
		return m_hBGBrush;
	}
	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
	{
		SetMsgHandled(FALSE);		
		return TRUE;
	}

	void OnKillFocus(CWindow wndFocus)
	{
		if(m_dwCoreStyle & CLBS_CANCELSEL_WHEN_NOFOCUS)
		{
			SetCurSel(-1);
			return;
		}
		SetMsgHandled(FALSE);
	}

	void OnMouseLeave()
	{
		SetMsgHandled(FALSE);
		ChangeHotItem(-1);
		return;
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		SetMsgHandled(FALSE);
		if(m_dwHotItemColor != -1)
		{
			int nIndex = HitTest(point);
			if(nIndex >= 0)
			{
				ChangeHotItem(nIndex);
			}
		}
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		SetMsgHandled(FALSE);
	}

	BOOL OnEraseBkgnd(CDCHandle dc)
	{
		SetMsgHandled(FALSE);
		return TRUE;
	}

	void InternalDraw(HDC hdc)
	{
		int nCount = GetCount();
		CRect rtClient;
		GetClientRect(rtClient);

		m_pClientPaint->Resize(rtClient.Width(), rtClient.Height());
		int nTop = 0;
		CRect rtTemp;
		int nSel = GetCurSel();
		int nTopIndex = GetTopIndex();

		for(int i = 0; i < nCount; i++)
		{
			if(i < nTopIndex)
			{
				m_vecItemData[i].pPaint->Show(FALSE, FALSE);
				continue;
			}
			rtTemp.SetRect(0, nTop, rtClient.Width(), nTop + m_vecItemData[i].pPaint->GetHeight() + m_nItemTopMargin * 2);
			CRect rtHelp;
			if(rtHelp.IntersectRect(rtTemp, rtClient))
			{
				m_vecItemData[i].pPaint->Show(TRUE, FALSE);				
				DRAWITEMSTRUCT di = {0};
				di.CtlType = ODT_SELFDRAW;
				di.hwndItem = m_hWnd;
				di.itemID = i;
				di.CtlID = GetDlgCtrlID();
				di.hDC = hdc;
				di.itemData = NULL;
				di.rcItem = rtTemp;
				if(nSel == i)
				{
					di.itemState = ODS_FOCUS;
				}				
				OnDrawItem(di.CtlID, &di);
			}
			else
			{
				m_vecItemData[i].pPaint->Show(FALSE, FALSE);		
			}
			nTop += rtTemp.Height();
		}
		m_pClientPaint->Move(0, 0);
		m_pClientPaint->Render(hdc);
	}

	LRESULT OnDeleteString(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		if(m_bDragging)
		{
			m_bManualCancel = TRUE;
		}
		int nIndex = (int)wParam;
		if(!m_bInternalInserting)
		{
			ClearItemData(nIndex);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnInsertString(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		if(m_bDragging)
		{
			m_bManualCancel = TRUE;
		}
		if(m_bInternalInserting)
		{
			bHandled = FALSE;
		}
		else
		{
			InsertString((int)wParam, (LPCTSTR)lParam);
		}
		return 0;
	}
	LRESULT OnAddString(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		if(m_bDragging)
		{
			m_bManualCancel = TRUE;
		}
		AddString((LPCTSTR)lParam);
		return 0;
	}
	LRESULT OnSetCurSel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		int nSel = GetCurSel();
		LRESULT lRet = DefWindowProc();
		if(nSel != (int)wParam && !lParam)
		{
			GetHost().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
		}
		return lRet;
	}

	void ComboDraw(HDC hdc)
	{
		CRect rt;
		GetClientRect(rt);
		COMBOBOXINFO cbi;
		cbi.cbSize = sizeof(cbi);
		::GetComboBoxInfo(m_hWnd, &cbi);
		GetSonicUI()->Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom, m_dwInternalBorderColor);

		BOOL bHighlight = FALSE;
		if(GetFocus() == m_hWnd && cbi.stateButton != STATE_SYSTEM_PRESSED && !::IsWindowVisible(cbi.hwndList))
		{
			GetSonicUI()->FillSolidRect(hdc, &cbi.rcItem, m_dwSelectColor);
			bHighlight = TRUE;
		}
		if(GetCurSel() >= 0)
		{
			ISonicPaint * pPaint = GetItemPaint(GetCurSel());
			if(pPaint)		
			{
				if(!bHighlight)
				{
					HilightPaintString(pPaint, FALSE);
				}
				else
				{
					HilightPaintString(pPaint, TRUE, RGB(255, 255, 255));
				}
				pPaint->Move(cbi.rcItem.left, cbi.rcItem.top);
				pPaint->Render(hdc, m_hWnd);
			}
		}

		MakeBtnDrop();
		if(m_pBtnDrop && m_pBtnDrop->IsValid())
		{
			m_pBtnDrop->TextOut(hdc, 0, 0, m_hWnd);
		}
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		if(bIsComboBox && !m_pBtnDrop)
		{
			bHandled = FALSE;
			return 0;
		}
		CPaintDC dc(m_hWnd);
		if(bIsComboBox)
		{
			ComboDraw(dc);
		}
		else
		{
			InternalDraw(dc);
		}
		return 0;
	}

	void ClearItemData(UINT nIndex)
	{
		if(nIndex >= m_vecItemData.size() || nIndex < 0)
		{
			return;
		}
		ISonicPaint * pPaint = m_vecItemData[nIndex].pPaint;
		if(pPaint)
		{
			pPaint->DelAllObject();
			SONICUI_DESTROY(pPaint);
		}
		m_vecItemData.erase(m_vecItemData.begin() + nIndex);
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
		pSkin->Attach(m_hWnd);
		if(!bIsComboBox)
		{
			pSkin->SetAttr(SATTR_SKIN_GAME_RENDERING, 1);
			pSkin->SetAttr(SATTR_ACCEPT_FOCUS, m_hWnd);
			CRect rt;
			GetClientRect(rt);
			m_pClientPaint = GetSonicUI()->CreatePaint();
			m_pClientPaint->Create(FALSE, rt.Width(), rt.Height());
		}
		
		SetMsgHandled(FALSE);
		return 0;
	}

	void OnDestroy()
	{
		SONICUI_DESTROY(m_pClientPaint);
		SONICUI_DESTROY(m_pImgWhole);
		SONICUI_DESTROY(m_pBtnDrop);
		GetSonicUI()->DestroyObject(GetSonicUI()->SkinFromHwnd(m_hWnd));
		while(GetCount())
		{
			DeleteString(0);
		}
	}

	virtual void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		if(lpDrawItemStruct->CtlType == ODT_SELFDRAW || bIsComboBox)
		{
			if(lpDrawItemStruct->itemID == (UINT)-1)
			{
				return;
			}
			ISonicPaint * pPaint = m_vecItemData[lpDrawItemStruct->itemID].pPaint;
			CRect rtItem = lpDrawItemStruct->rcItem;
			pPaint->Resize(rtItem.Width(), rtItem.Height() - 2 * m_nItemTopMargin);

			if(m_dwHotItemColor != -1)
			{
				if(lpDrawItemStruct->itemID == m_nHotItem)
				{
					GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, rtItem, m_dwHotItemColor);
					HilightPaintString(pPaint, TRUE, (m_dwCoreStyle & CLBS_FOCUS_TEXT_COLOR_CHANGE) ? RGB(255, 255, 255) : (DWORD)-1);
				}
				else
				{
					HilightPaintString(pPaint, FALSE);
				}

			}
			if((lpDrawItemStruct->itemState & (ODS_SELECTED | ODS_FOCUS)) && m_vecItemData[lpDrawItemStruct->itemID].bSelectable)
			{
				if(m_dwSelectColor != -1)
				{
					GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, rtItem, m_dwSelectColor);
				}
				HilightPaintString(pPaint, TRUE, (m_dwCoreStyle & CLBS_FOCUS_TEXT_COLOR_CHANGE) ? RGB(255, 255, 255) : (DWORD)-1);
			}
			else
			{
				if(bIsComboBox)
				{
					GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, GetBGColor());
				}
				HilightPaintString(pPaint, FALSE);
			}
			

			int x, y;
			x = rtItem.left;
			y = rtItem.top + m_nItemTopMargin;
			if(lpDrawItemStruct->itemState & ODS_COMBOBOXEDIT)
			{
				y -= m_nItemTopMargin;
			}
			pPaint->Move(x, y);
			if(bIsComboBox)
			{
				pPaint->Render(lpDrawItemStruct->hDC);
			}
			else
			{
				pPaint->Render(lpDrawItemStruct->hDC, m_hWnd);
			}
			if(m_bAllowDrag && m_nDragHotItem >= 0)
			{
				if(!m_bDragging || (m_nDragHotItem != m_DragInfo.nItem && m_nDragHotItem != m_DragInfo.nItem + 1))
				{
					int nHotIndex = m_nDragHotItem >= (int)m_vecItemData.size() ? m_nDragHotItem - 1 : m_nDragHotItem;
					if(lpDrawItemStruct->itemID == nHotIndex)
					{
						// draw drag highlight line
						CRect rtDragLine = lpDrawItemStruct->rcItem;
						if(m_nDragHotItem == nHotIndex)
						{
							rtDragLine.bottom = rtDragLine.top + 2;
						}
						else
						{
							rtDragLine.top = rtDragLine.bottom - 2;
						}
						GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, &rtDragLine, m_dwDragHightlightLineColor);
					}
				}
			}
		}
		else
		{
			if(lpDrawItemStruct->itemID == m_vecItemData.size() - 1)
			{
				InvalidateRect(NULL);
			}
			else
			{
				CRect rt = lpDrawItemStruct->rcItem;
				InvalidateRect(&lpDrawItemStruct->rcItem);
			}
		}
	}

	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
	{
		if((int)lpMeasureItemStruct->itemID == -1)
		{
			return;
		}
		CRect rtClient;
		GetClientRect(rtClient);
		lpMeasureItemStruct->itemWidth = rtClient.Width();
		if(m_vecItemData.empty())
		{
			// owner draw fixed
			lpMeasureItemStruct->itemHeight = GetItemHeight(0);
		}
		else
		{
			// owner draw variable
			ISonicPaint * pPaint = m_vecItemData[lpMeasureItemStruct->itemID].pPaint;
			lpMeasureItemStruct->itemHeight = pPaint->GetHeight() + m_nItemTopMargin * 2;
		}
	}
};

typedef CCoreListBoxT<CListBox> CCoreListBox;
typedef CCoreListBoxT<CComboBox, TRUE> CCoreComboBox;

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CCoreTreeCtrl :	public CWindowWithDragDrop<CCoreTreeCtrl, CTreeViewCtrl>,
						public CCustomDraw<CCoreTreeCtrl>
{
public:
	typedef struct tagTreeItem : public CSafeCrtBuffer
	{
		tagTreeItem() : nNormalImage(0), nExpandImage(0), lParam(0)
		{
			pPaint = GetSonicUI()->CreatePaint();
			pPaint->Create(FALSE);
		}
		tagTreeItem(const tagTreeItem & srcItem)
		{
			pPaint = NULL;
			Clone(srcItem);
		}
		tagTreeItem & operator = (const tagTreeItem & srcItem)
		{
			Clone(srcItem);
			return *this;
		}
		void Clone(const tagTreeItem & srcItem)
		{
			SONICUI_DESTROY(pPaint);
			memcpy(this, &srcItem, sizeof(tagTreeItem));
			pPaint = GetSonicUI()->CreatePaint();
			pPaint->CloneFrom(srcItem.pPaint);
		}
		~tagTreeItem()
		{
			SONICUI_DESTROY(pPaint);
		}
		int nNormalImage;
		int nExpandImage;
		LPARAM lParam;
		ISonicPaint * pPaint;
	}TREE_ITEM;
public:
	friend class CCustomDraw<CCoreTreeCtrl>;
	CCoreTreeCtrl(){}
	virtual ~CCoreTreeCtrl(){}

	BEGIN_MSG_MAP_EX(CCoreTreeCtrl)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDED, OnItemExpanded)
		MESSAGE_HANDLER(TVM_INSERTITEM, OnInsertItem)
		MESSAGE_HANDLER(TVM_DELETEITEM, OnDeleteItem)
		MESSAGE_HANDLER(TVM_SETITEM, OnSetItem)
		MESSAGE_HANDLER(TVM_GETITEM, OnGetItem)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(CWindowWithDragDrop)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CCoreTreeCtrl>, 1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
private:
public:
	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, 
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		dwStyle |= TVS_DISABLEDRAGDROP;
		HWND hRet = CWindowWithDragDrop::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		if(hRet)
		{
			SetFont(GetSonicUI()->GetDefaultFont(), FALSE);
		}
		return hRet;
	}
	ISonicPaint * GetItemPaint(HTREEITEM hItem)
	{
		TREE_ITEM * pTreeItem = GetItemDataEx(hItem);
		if(pTreeItem)
		{
			return pTreeItem->pPaint;
		}
		return NULL;
	}
	BOOL SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage, int nExpandImage)
	{
		TREE_ITEM * pTreeItem = GetItemDataEx(hItem);
		if(!pTreeItem)
		{
			return FALSE;
		}
		pTreeItem->nExpandImage = nExpandImage;
		pTreeItem->nNormalImage = nImage;
		return CTreeViewCtrl::SetItemImage(hItem, nImage, nSelectedImage);
	}
	typedef BOOL (CCoreTreeCtrl::*PROC_TRAVERSE_TREE)(HTREEITEM hItem);
	BOOL TraverseTree(HTREEITEM hRoot, PROC_TRAVERSE_TREE pProc)
	{
		if(hRoot && !(this->*pProc)(hRoot))
		{
			return FALSE;
		}
		HTREEITEM hItem = hRoot ? GetChildItem(hRoot) : GetRootItem();
		for(; hItem; hItem = GetNextSiblingItem(hItem))
		{
			if(!TraverseTree(hItem, pProc))
			{
				return FALSE;
			}
		}
		return TRUE;
	}
protected:
	virtual BOOL OnSetDragInfo(DRAG_INFO * pInfo)
	{
		// not implemented yet
		return FALSE;
	}
	BOOL TraverseHidePaint(HTREEITEM hItem)
	{
		TREE_ITEM * pTreeItem = GetItemDataEx(hItem);
		if(pTreeItem)
		{
			// adjust item text
			CRect rt, rtClient;
			GetItemRect(hItem, rt, TRUE);
			GetClientRect(rtClient);
			if(rt.Width() > 0 && rtClient.IntersectRect(rt, rtClient))
			{
				pTreeItem->pPaint->Show(TRUE, FALSE);
				static int nTextMargin = -1;
				if(nTextMargin < 0)
				{
					nTextMargin = rt.Width() % 6;
					if(nTextMargin == 1)
					{
						nTextMargin += 6;
					}
				}
				int nSpace = rt.Width() - pTreeItem->pPaint->GetWidth() - nTextMargin;
				if(nSpace < 0 || nSpace >= 6)
				{
					if(GetFont() == GetSonicUI()->GetDefaultFont())
					{
						CString strText;
						int nSpaceCount = (pTreeItem->pPaint->GetWidth() + 5) / 6;
						for(int i = 0; i < nSpaceCount; i++)
						{
							strText += _T(" ");
						}
						TVITEM item = {0};
						item.hItem = hItem;
						item.mask = TVIF_TEXT;
						item.pszText = (LPTSTR)(LPCTSTR)strText;
						item.cchTextMax = strText.GetLength();
						SendMessage(TVM_SETITEM, 1, (LPARAM)&item);
					}
				}
			}
			else
			{
				pTreeItem->pPaint->Show(FALSE, FALSE);
			}
		}
		return TRUE;
	}
	BOOL TraverDeleteItemData(HTREEITEM hItem)
	{
		TREE_ITEM * pTreeItem = GetItemDataEx(hItem);
		if(pTreeItem)
		{
			delete pTreeItem;
			TV_ITEM item;
			item.lParam = NULL;
			item.mask = TVIF_PARAM;
			item.hItem = hItem; 
			SendMessage(TVM_SETITEM, 1, (LPARAM)&item);
		}
		return TRUE;
	}
	void OnLButtonDblClk(UINT nFlags, CPoint pt)
	{
		UINT uFlag;
		HTREEITEM hItem = HitTest(pt, &uFlag);
		if((uFlag & TVHT_ONITEMLABEL) || (uFlag & TVHT_ONITEMICON))
		{
			SetMsgHandled(FALSE);
		}
	}
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		SetMsgHandled(FALSE);
		ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
		pSkin->Attach(m_hWnd);
		return 0;
	}
	void OnDestroy()
	{
		DeleteAllItems();
		GetSonicUI()->DestroyObject(GetSonicUI()->SkinFromHwnd(m_hWnd));
		SetMsgHandled(FALSE);
	}
	LRESULT OnInsertItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TVINSERTSTRUCT * pTVI = (TVINSERTSTRUCT *)lParam;
		ISonicString * pText = GetSonicUI()->CreateString();
		TREE_ITEM * pTreeItem = new TREE_ITEM;
		if(pTVI->itemex.mask & TVIF_TEXT)
		{
			pText->Format(_T("/global, c=%x/%s"), GetTextColor(), pTVI->itemex.pszText);
			pTVI->itemex.pszText = (LPTSTR)pText->GetTextWithoutCtrl();
			pTVI->itemex.cchTextMax = lstrlen(pText->GetTextWithoutCtrl());
		}
		if(pTVI->itemex.mask & TVIF_PARAM)
		{
			pTreeItem->lParam = pTVI->itemex.lParam;
		}
		if(pTVI->itemex.mask & TVIF_IMAGE)
		{
			pTreeItem->nNormalImage = pTVI->item.iImage;
			pTreeItem->nExpandImage = pTVI->item.iImage;
		}
		pTreeItem->pPaint->AddObject(pText->GetObjectId(), 0, 0, TRUE);
		pTVI->itemex.lParam = (LPARAM)pTreeItem;
		pTVI->itemex.mask |= TVIF_PARAM;
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnDeleteItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HTREEITEM hItem = (HTREEITEM)lParam;
		if(hItem)
		{
			if(hItem == TVI_ROOT)
			{
				// deleting all items
				TraverseTree(NULL, &CCoreTreeCtrl::TraverDeleteItemData);
			}
			else
			{
				TraverDeleteItemData(hItem);
			}
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		TV_ITEM * pItem = (TV_ITEM *)lParam;
		TREE_ITEM * pTreeItem = GetItemDataEx(pItem->hItem);
		if(pTreeItem && !wParam)
		{
			if(pItem->mask & TVIF_PARAM)
			{
				pTreeItem->lParam = pItem->lParam;
				pItem->mask &= ~TVIF_PARAM;
			}
			if(pItem->mask & TVIF_TEXT)
			{
				ISonicString * pText = (ISonicString *)pTreeItem->pPaint->GetPaintItem(0, TRUE)->pBase;
				pText->Format(_T("/global, c=%x/%s"), GetTextColor(), pItem->pszText);
				pItem->pszText = (LPTSTR)pText->GetTextWithoutCtrl();
				pItem->cchTextMax = lstrlen(pText->GetTextWithoutCtrl());
			}
		}
		return 0;
	}
	LRESULT OnGetItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TV_ITEM * pItem = (TV_ITEM *)lParam;
		UINT uOldMask = pItem->mask;
		pItem->mask |= TVIF_PARAM;
		LRESULT lRet = DefWindowProc();
		pItem->mask = uOldMask;
		if(pItem->lParam)
		{
			TREE_ITEM * pTreeItem = (TREE_ITEM *)pItem->lParam;
			if((uOldMask & TVIF_PARAM) && !wParam)
			{
				pItem->lParam =pTreeItem->lParam;
			}
			if(uOldMask & TVIF_TEXT)
			{
				ISonicString * pText = (ISonicString *)pTreeItem->pPaint->GetPaintItem(0, TRUE)->pBase;
				lstrcpy(pItem->pszText, pText->GetTextWithoutCtrl());
				pItem->cchTextMax = lstrlen(pText->GetTextWithoutCtrl());
			}
		}
		return lRet;
	}
	TREE_ITEM * GetItemDataEx(HTREEITEM hItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		if(!hItem)
		{
			return NULL;
		}
		TVITEM item = { 0 };
		item.hItem = hItem;
		item.mask = TVIF_PARAM;
		BOOL bRet = (BOOL)::SendMessage(m_hWnd, TVM_GETITEM, 1, (LPARAM)&item);
		return bRet ? (TREE_ITEM *)item.lParam : NULL;
	}
	LRESULT OnItemExpanded(int iCtrlId, NMHDR* pNMHDR, BOOL& bHandled)
	{
		bHandled = FALSE;
		LPNMTREEVIEW pNMTreeView = (LPNMTREEVIEW)pNMHDR;
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		TREE_ITEM * pTreeItem = GetItemDataEx(hItem);
		if(!pTreeItem)
		{
			return 0;
		}
		if(pNMTreeView->action & TVE_COLLAPSE)
		{
			CTreeViewCtrl::SetItemImage(hItem, pTreeItem->nNormalImage, pTreeItem->nNormalImage);
		}
		else if(pNMTreeView->action & TVE_EXPAND)
		{
			CTreeViewCtrl::SetItemImage(hItem, pTreeItem->nExpandImage, pTreeItem->nExpandImage);
		}
		return 0;
	}
	DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
	{
		TraverseTree(NULL, &CCoreTreeCtrl::TraverseHidePaint);
		return CDRF_NOTIFYITEMDRAW;
	}
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
	{
		return CDRF_NOTIFYPOSTPAINT;
	}
	DWORD OnItemPostPaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
	{
		if(!(lpNMCustomDraw->dwDrawStage & CDDS_ITEM))
		{
			return CDRF_DODEFAULT;
		}
		HTREEITEM hItem = (HTREEITEM)(DWORD_PTR)lpNMCustomDraw->dwItemSpec;
		ISonicPaint * pPaint = GetItemPaint(hItem);
		if(!pPaint)
		{
			return CDRF_DODEFAULT;
		}
		HDC hdc = lpNMCustomDraw->hdc;
		CRect rt;
		GetItemRect(hItem, rt, TRUE);
		if(lpNMCustomDraw->uItemState & CDIS_SELECTED)
		{
			GetSonicUI()->FillSolidRect(hdc, rt, GetSysColor(COLOR_HIGHLIGHT));
			HilightPaintString(pPaint, TRUE, RGB(255, 255, 255));
		}
		else
		{
			GetSonicUI()->FillSolidRect(hdc, rt, GetBkColor());
			HilightPaintString(pPaint, FALSE);
		}
		int x = rt.left;
		int y = (rt.Height() - pPaint->GetHeight()) / 2;
		if(y < 0)
		{
			y = 0;
		}
		y += rt.top;
		pPaint->Move(x, y);
		pPaint->Render(hdc, m_hWnd);
		return CDRF_SKIPDEFAULT;
	}
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CCoreHeaderCtrl : public CWindowImplEx<CCoreHeaderCtrl, CHeaderCtrl>
{
public:
	typedef struct tagHeaderItem : public CSafeCrtBuffer
	{
		tagHeaderItem() : lParam(0)
		{
			pText = GetSonicUI()->CreateString();
		}
		~tagHeaderItem()
		{
			SONICUI_DESTROY(pText);
		}
		LPARAM lParam;
		ISonicString * pText;
	}HEADER_ITEM;
private:
	BOOL m_bGetItemSwitch;
	int m_nHeaderHeight;
	DWORD m_dwBGColor;
	DWORD m_dwBorderColor;
public:
	BEGIN_MSG_MAP_EX(CCoreHeaderCtrl)
		MSG_WM_SETCURSOR(OnSetCursor)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_DESTROY(OnDestroy)
		MESSAGE_HANDLER(HDM_INSERTITEM, OnInsertItem)
		MESSAGE_HANDLER(HDM_DELETEITEM, OnDeleteItem)
		MESSAGE_HANDLER(HDM_SETITEM, OnSetItem)
		MESSAGE_HANDLER(HDM_GETITEM, OnGetItem)
		MESSAGE_HANDLER(HDM_LAYOUT, OnLayOut)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(HDN_BEGINTRACK, OnHDNBeginTrack)
		CHAIN_MSG_MAP(CWindowImplEx)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	CCoreHeaderCtrl(int nHeaderHeight = 0)
	{
		m_nHeaderHeight = nHeaderHeight;
		m_dwBorderColor = CORE_BORDER_COLOR;
		m_dwBGColor = GetSysColor(COLOR_BTNFACE);
		m_bGetItemSwitch = FALSE;
	}
	void SetBorderColor(DWORD dwBorderColor){ m_dwBorderColor = dwBorderColor; }
	DWORD GetBorderColor(){ return m_dwBorderColor; }
	void SetBGColor(DWORD dwBGColor){ m_dwBGColor = dwBGColor; }
	DWORD GetBGColor(){ return m_dwBGColor; }
protected:
	void OnDestroy()
	{
		SetMsgHandled(FALSE);
		while(GetItemCount())
		{
			DeleteItem(0);
		}
	}
	BOOL OnEraseBkgnd(CDCHandle dc)
	{
		return TRUE;
	}
	void OnPaint(CDCHandle dc)
	{
		CCorePaintDC hdc(m_hWnd, m_dwBGColor);
		CRect rtAll;
		GetClientRect(rtAll);
		// render border
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetParent().GetScrollInfo(SB_HORZ, &si);
		rtAll.right = si.nMax + 1;
		CRect rt = rtAll;
		CWindowWithReflectorImplFixed<CCoreHeaderCtrl> * pParent = (CWindowWithReflectorImplFixed<CCoreHeaderCtrl> *)GetProp(GetParent(), CORE_WINDOW_WITH_REFLECTOR);
		if(pParent && pParent->GetBorderWidth())
		{
			rt.InflateRect(1, 1, 1, 0);
		}
		GetSonicUI()->Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom, m_dwBorderColor);
		// render items
		int nCount = GetItemCount();
		for(int i = 0; i < nCount; i++)
		{
			CRect rtItem;
			GetItemRect(i, rtItem);
			const int nMargin = 3;
			rtItem.DeflateRect(nMargin, 0, nMargin, 0);
			HEADER_ITEM * pHeaderItem = GetItemDataEx(i);
			HDITEM item = {0};
			item.mask = HDI_FORMAT;
			GetItem(i, &item);
			int x = rtItem.left;
			int y = rtItem.top + (rtItem.Height() - pHeaderItem->pText->GetHeight()) / 2;
			if(rtItem.Width() > pHeaderItem->pText->GetWidth())
			{
				if((item.fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
				{
					x += (rtItem.Width() - pHeaderItem->pText->GetWidth()) / 2;
				}
				if((item.fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
				{
					x = rtItem.right - pHeaderItem->pText->GetWidth();
				}
			}
			pHeaderItem->pText->TextOut(hdc, x, y, m_hWnd);
			// render divider
			if(i != nCount - 1)
			{
				hdc.FillSolidRect(rtItem.right + nMargin, 3, 1, rtItem.Height() - 6, m_dwBorderColor);
			}
		}
	}
	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
	{
		return TRUE;
	}
	LRESULT OnHDNBeginTrack(LPNMHDR pnmh)
	{
		LPNMHEADER pHeader = (LPNMHEADER)pnmh;
		return TRUE;
	}
	void OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		HDHITTESTINFO info;
		info.pt = point;
		HitTest(&info);
		if(info.flags == HHT_ONDIVIDER)
		{
			SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
		}
		else
		{
			SetMsgHandled(FALSE);
		}
	}
	LRESULT OnLayOut(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		LRESULT lRet = DefWindowProc();
		LPHDLAYOUT pLayout = (LPHDLAYOUT)lParam;
		if(m_nHeaderHeight)
		{
			pLayout->prc->top = m_nHeaderHeight;
			pLayout->pwpos->cy = m_nHeaderHeight;
		}
		return lRet;
	}
	LRESULT OnInsertItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		bHandled = FALSE;
		int nIndex = (int)wParam;
		LPHDITEM pHDItem = (LPHDITEM)lParam;
		HEADER_ITEM * pItem = new HEADER_ITEM;
		pItem->lParam = pHDItem->lParam;
		if(pHDItem->mask & HDI_TEXT)
		{
			pItem->pText->Format(pHDItem->pszText);
		}
		pHDItem->mask |= HDI_LPARAM;
		pHDItem->lParam = (LPARAM)pItem;
		return 0;
	}
	LRESULT OnDeleteItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int nIndex = (int)wParam;
		delete GetItemDataEx(nIndex);
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		LPHDITEM pItem = (LPHDITEM)lParam;
		if(pItem->mask & HDI_LPARAM)
		{
			HEADER_ITEM * pHeaderItem = GetItemDataEx((int)wParam);
			if(pHeaderItem)
			{
				pHeaderItem->lParam = pItem->lParam;
				pItem->mask &= ~HDI_LPARAM;
			}
		}
		return 0;
	}
	LRESULT OnGetItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRet = DefWindowProc();
		LPHDITEM pItem = (LPHDITEM)lParam;
		if((pItem->mask & HDI_LPARAM) && pItem->lParam)
		{
			HEADER_ITEM * pHeaderItem = (HEADER_ITEM *)pItem->lParam;
			if(!m_bGetItemSwitch)
			{
				pItem->lParam = pHeaderItem->lParam;
			}
		}
		return lRet;
	}
	HEADER_ITEM * GetItemDataEx(int nIndex)
	{
		HDITEM item = {0};
		item.mask = HDI_LPARAM;
		m_bGetItemSwitch = TRUE;
		SendMessage(HDM_GETITEM, nIndex, (LPARAM)&item);
		m_bGetItemSwitch = FALSE;
		return (HEADER_ITEM *)item.lParam;
	}
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CCoreListCtrl :	public CWindowImplWithScrollBar<CCoreTreeCtrl, CListViewCtrl>,
						public CCustomDraw<CCoreListCtrl>
{
private:
	CCoreHeaderCtrl m_HeaderCtrl;
	BOOL m_bDrawingHilight;
	BOOL m_bDrawingFocused;
	DWORD m_dwSelectColor;
	CImageList m_ilNull;
public:
	BEGIN_MSG_MAP_EX(CCoreListCtrl)
		MSG_WM_VSCROLL(OnVScroll)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		REFLECT_NOTIFY_CODE(HDN_BEGINTRACK)
		CHAIN_MSG_MAP(CWindowImplWithScrollBar)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CCoreListCtrl>, 1)
	END_MSG_MAP()

	CCoreListCtrl(int nHeaderHeight = 0) : m_HeaderCtrl(nHeaderHeight)
	{
		m_dwSelectColor = (DWORD)-1;
		m_bDrawingHilight = FALSE;
		m_bDrawingFocused = FALSE;
	}

	void SetSelectColor(DWORD cr){ m_dwSelectColor = cr; }
	DWORD GetSelectColor(){ return m_dwSelectColor; }

	void SetItemHeight(int nHeight)
	{
		m_ilNull.Destroy();
		m_ilNull.Create(1, nHeight, ILC_COLOR32, 1, 0);
		SetImageList(m_ilNull, LVSIL_SMALL);
	}
	int GetItemHeight()
	{
		CRect rt;
		if(GetItemRect(0, rt, LVIR_BOUNDS))
		{
			return rt.Height();
		}
		return 0;
	}

	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, 
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		dwStyle |= WS_CLIPCHILDREN;
		HWND hRet = CWindowWithReflectorImplFixed::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		if(hRet)
		{
			m_HeaderCtrl.SubclassWindow(GetHeader());
		}
		return hRet;
	}

	CCoreHeaderCtrl & GetCoreHeader(){ return m_HeaderCtrl; }

	DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
	{
		return CDRF_NOTIFYITEMDRAW;
	}
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
	{
		NMLVCUSTOMDRAW * pCustomDraw = (NMLVCUSTOMDRAW *)lpNMCustomDraw;
		int nIndex = (int)pCustomDraw->nmcd.dwItemSpec;
		if(GetItemState(nIndex, LVIS_SELECTED) && m_dwSelectColor != (DWORD)-1)
		{
			m_bDrawingHilight = TRUE;
			pCustomDraw->clrTextBk = m_dwSelectColor;
			SetItemState(nIndex, 0, LVIS_SELECTED);
		}
		if(GetItemState(nIndex, LVIS_FOCUSED))
		{
			m_bDrawingFocused = TRUE;
			SetItemState(nIndex, 0, LVIS_FOCUSED);
		}
		return CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
	}
	DWORD OnItemPostPaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw)
	{
		int nIndex = (int)lpNMCustomDraw->dwItemSpec;
		if(m_bDrawingHilight)
		{
			m_bDrawingHilight = FALSE;
			SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
		}
		if(m_bDrawingFocused)
		{
			m_bDrawingFocused = FALSE;
			SetItemState(nIndex, LVIS_FOCUSED, LVIS_FOCUSED);
		}
		return CDRF_DODEFAULT;
	}
	void SetBGColor(DWORD dwColor)
	{
		SetBkColor(dwColor);
		SetTextBkColor(dwColor);
	}
protected:
	// other than listbox and treeview, listctrl is the only exception which cannot respond to WM_VSCROLL and WM_HSCROLL correctly when nSBCode is SB_THUMBxxx
	// so these two response functions are overrided here
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
	{
		if(nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
		{
			int nPlus = (nPos - GetScrollPos(SB_VERT)) * GetItemHeight();
			if(nPlus)
			{
				Scroll(CSize(0, nPlus));
			}
		}
		SetMsgHandled(FALSE);
	}
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
	{
		if(nSBCode == SB_THUMBTRACK || nSBCode == SB_THUMBPOSITION)
		{
			int nPlus = nPos - GetScrollPos(SB_HORZ);
			if(nPlus)
			{
				Scroll(CSize(nPlus, 0));
			}
		}
		SetMsgHandled(FALSE);
	}
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		SetMsgHandled(FALSE);
		return 0;
	}
	void OnDestroy()
	{
		SetMsgHandled(FALSE);
		m_ilNull.Destroy();
	}
};

//////////////////////////////////////////////////////////////////////////
// class:	CCoreMenu
// remark:	CCoreMenu must be created dynamically, so you cannot attach it to an existing
//			menu handle or load it from resource template
//			never operate menu data or menu item data with the system api because these two fields are used to keep internal data
//			use CCoreMenu::SetMenuItemData instead
//////////////////////////////////////////////////////////////////////////
#define CORE_MENU_HELPER_NAME			_T("MenuHelperProc2356")
class CCoreMenu : public CMenuHandle, public CSafeCrtBuffer
{
public:
	enum
	{
		CORE_MENU_MAGIC = 0x4352,
		SIDE_WIDTH = 22,
		MIN_SIDE_WIDTH = 12,
		MENU_MARGIN = 1,
		ORIGINAL_MARGIN_DEFLATE = 2,
		TEXT_MARGIN = 6,
		ITEM_VMARGIN = 8,
	};
	typedef struct tagMenuItemData
	{
		UINT wID;
		ISonicImage * pImg;
	}MENU_ITEM_DATA;

	typedef std::vector<MENU_ITEM_DATA> VEC_ITEM;
private:
	int m_nItemVMargin;
	int m_nSideWidth;
	int m_nFixedWidth;
	DWORD m_dwMagic;
	VEC_ITEM m_vecItem;
	DWORD m_dwBGColor;
	DWORD m_dwBorderColor;
	DWORD m_dwTextColor;
	ISonicString * m_pCheck;
	ISonicImage * m_pSideImg;
	ISonicImage * m_pArrowImg;
	CPoint m_ptTrackPopup;
	HFONT m_hFont;
	int m_nTabCount;		// character count for \t, default to 0x10
	DWORD m_dwSelectColor;
	DWORD m_dwHilightTextColor;
	DWORD m_dwSepColor;
private:
	class CCoreMenuArrow : public CWindowImplEx<CCoreMenuArrow>
	{
	private:
		ISonicImage * m_pImgArrow;
		ISonicWndEffect * m_pEffect;
	public:
		DECLARE_WND_CLASS(_T("CoreMenuArrow"))
		BEGIN_MSG_MAP_EX(CCoreMenuArrow)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_DESTROY(OnDestroy)
		END_MSG_MAP()
		CCoreMenuArrow()
		{
			m_pImgArrow = NULL;
			m_pEffect = NULL;
		}
		HWND Create(HWND hParent, int x, int y, ISonicImage * pImgArrow, BOOL bMirrorY = FALSE)
		{
			m_pImgArrow = GetSonicUI()->CreateImage();
			m_pImgArrow->CloneFrom(pImgArrow);
			if(bMirrorY)
			{
				m_pImgArrow->MirrorY();
			}
			return CWindowImplEx::Create(hParent, CRect(x, y, x + 1, y + 1), NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT);
		}
		int OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			SetMsgHandled(FALSE);
			m_pEffect = GetSonicUI()->CreateWndEffect();
			m_pEffect->Attach(m_hWnd, TRUE);
			m_pEffect->GetSkin()->SetSkin(_T("bg"), _T("image:%d; fit_image:1"), m_pImgArrow);
			return 0;
		}
		void OnDestroy()
		{
			SetMsgHandled(FALSE);
			SONICUI_DESTROY(m_pImgArrow);
			SONICUI_DESTROY(m_pEffect);
		}
	};
	class CCoreMenuHooker : public CWindowImplEx<CCoreMenuHooker>
	{
	private:
		static HHOOK s_Hook;
		CCoreMenuArrow m_wndArrow;
	public:
		// static
		static void InistallHooker()
		{
			if(!s_Hook)
			{
				s_Hook = SetWindowsHookEx(WH_CALLWNDPROC, WindowsHook, GetModuleHandle(NULL), GetCurrentThreadId());
			}
		}
		static CCoreMenu * MenuFromHWND(HWND hWnd)
		{
			HMENU hMenu = (HMENU)::SendMessage(hWnd, MN_GETHMENU, 0, 0);
			return CCoreMenu::FromHandle(hMenu);
		}
		static LRESULT CALLBACK WindowsHook(int code, WPARAM wParam, LPARAM lParam) 
		{
			CWPSTRUCT * pStruct = (CWPSTRUCT *)lParam;
			if(pStruct->message == 0x1e2)
			{
				TCHAR szClsName[MAX_PATH];
				int nCount = GetClassName(pStruct->hwnd, szClsName, MAX_PATH);
				if(nCount == 6 && lstrcmp(szClsName, _T("#32768")) == 0)
				{
					if(MenuFromHWND(pStruct->hwnd))
					{
 						CCoreMenuHooker * pHooker = new CCoreMenuHooker;
						if(pHooker->SubclassWindowOnce(pStruct->hwnd))
						{
							pHooker->SetDeleteSelfOnDestroy(TRUE);
						}
						else
						{
							delete pHooker;
						}
					}
				}
			}
			return CallNextHookEx(s_Hook, code, wParam, lParam);
		}
	public:
		BEGIN_MSG_MAP_EX(CCoreMenuHooker)
			MSG_WM_GETICON(OnGetIcon)
			MSG_WM_NCCALCSIZE(OnNcCalcSize)
			MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
			MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
			MSG_WM_PRINT(OnPrint)
			MSG_WM_PAINT(OnPaint)
			MSG_WM_NCPAINT(OnNcPaint)
		END_MSG_MAP()

	protected:
		void DrawBorder(CDCHandle hdc, CCoreMenu * pMenu)
		{
			if(!pMenu)
			{
				return;
			}
			CRect rtWnd;
			GetWindowRect(rtWnd);
			GetSonicUI()->Rectangle(hdc, 0, 0, rtWnd.Width(), rtWnd.Height(), pMenu->GetBorderColor());
		}
		void DrawClient(CDCHandle hdc, CCoreMenu * pMenu, BOOL bClientDC = FALSE)
		{
			if(!pMenu)
			{
				return;
			}
			CRect rtWnd, rtClient;
			GetWindowRect(rtWnd);
			rtWnd.OffsetRect(-rtWnd.TopLeft());
			rtClient = rtWnd;
			rtClient.DeflateRect(MENU_MARGIN, MENU_MARGIN, MENU_MARGIN, MENU_MARGIN);
			ISonicImage * pImg = GetSonicUI()->CreateImage();
			pImg->Create(rtClient.Width(), rtClient.Height());
			DefWindowProc(WM_PRINTCLIENT, (WPARAM)(HDC)pImg->GetDC(), PRF_CLIENT | PRF_ERASEBKGND);
			if(bClientDC)
			{
				pImg->Draw(hdc);
			}
			else
			{
				pImg->Draw(hdc, rtClient.left, rtClient.top);
			}
			SONICUI_DESTROY(pImg);
		}
		void OnNcPaint(CRgn rgn)
		{
			CCoreMenu * pMenu = MenuFromHWND(m_hWnd);
			if(!pMenu)
			{
				SetMsgHandled(FALSE);
				return;
			}
			CWindowDC hdc(m_hWnd);
			DrawBorder(hdc.m_hDC, pMenu);
		}
		void OnPaint(CDCHandle dc)
		{
			CCoreMenu * pMenu = MenuFromHWND(m_hWnd);
			if(!pMenu)
			{
				SetMsgHandled(FALSE);
				return;
			}
			CPaintDC hdc(m_hWnd);
			DrawClient(hdc.m_hDC, pMenu, TRUE);
		}
		void OnWindowPosChanging(LPWINDOWPOS lpWndPos)
		{
			if(!(lpWndPos->flags & SWP_NOSIZE))
			{
				lpWndPos->cx -= 2 * ORIGINAL_MARGIN_DEFLATE;
				lpWndPos->cy -= 2 * ORIGINAL_MARGIN_DEFLATE;
			}
			if(!(lpWndPos->flags & SWP_NOMOVE))
			{
				CCoreMenu * pMenu = MenuFromHWND(m_hWnd);
				if(pMenu)
				{
					if(pMenu->GetArrowImage())
					{
						if(lpWndPos->x >= pMenu->GetTrackPopupPos().x - pMenu->GetArrowImage()->GetWidth() / 2)
						{
							lpWndPos->x = pMenu->GetTrackPopupPos().x - pMenu->GetArrowImage()->GetWidth() / 2;
						}
						else
						{
							CRect rt;
							GetWindowRect(rt);
							if(lpWndPos->x + rt.Width() <= pMenu->GetTrackPopupPos().x + pMenu->GetArrowImage()->GetWidth() / 2)
							{
								lpWndPos->x = pMenu->GetTrackPopupPos().x - rt.Width() + pMenu->GetArrowImage()->GetWidth() / 2;
							}
						}
						if(lpWndPos->y < pMenu->GetTrackPopupPos().y)
						{
							lpWndPos->y -= pMenu->GetArrowImage()->GetHeight() - 2 * ORIGINAL_MARGIN_DEFLATE;
						}
						else
						{
							lpWndPos->y += pMenu->GetArrowImage()->GetHeight();
						}
					}
				}
			}
			SetMsgHandled(FALSE);
		}
		void OnWindowPosChanged(LPWINDOWPOS lpWndPos)
		{
			CCoreMenu * pMenu = MenuFromHWND(m_hWnd);
			if(pMenu && pMenu->GetArrowImage())
			{
				if(lpWndPos->flags & SWP_SHOWWINDOW)
				{
					if(m_wndArrow.IsWindow())
					{
						m_wndArrow.DestroyWindow();
					}
					CRect rt;
					GetWindowRect(rt);
					BOOL bUpArrow = (pMenu->GetArrowImage()->GetPixel(1, 1) == 0);
					int x = pMenu->GetTrackPopupPos().x - pMenu->GetArrowImage()->GetWidth() / 2;
					int y;
					if(rt.top > pMenu->GetTrackPopupPos().y)
					{						
						y = rt.top - pMenu->GetArrowImage()->GetHeight() + 1;						
						m_wndArrow.Create(m_hWnd, x, y, pMenu->GetArrowImage(), !bUpArrow);
					}
					else
					{
						y = rt.bottom - 1;
						m_wndArrow.Create(m_hWnd, x, y, pMenu->GetArrowImage(), bUpArrow);
					}
				}
				else if(lpWndPos->flags & SWP_HIDEWINDOW)
				{
					if(m_wndArrow.IsWindow())
					{
						m_wndArrow.DestroyWindow();
					}
				}
			}
			SetMsgHandled(FALSE);
		}
		LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
		{
			if(bCalcValidRects)
			{
				NCCALCSIZE_PARAMS FAR* lpncsp = (NCCALCSIZE_PARAMS FAR*)lParam;
				CRect rtWnd = lpncsp->rgrc[0];
				LRESULT lRet = DefWindowProc();
				CRect rt = lpncsp->rgrc[0];
				if(rtWnd.Width() - rt.Width() == (MENU_MARGIN + ORIGINAL_MARGIN_DEFLATE) * 2)
				{
					rt.InflateRect(ORIGINAL_MARGIN_DEFLATE, ORIGINAL_MARGIN_DEFLATE, ORIGINAL_MARGIN_DEFLATE, ORIGINAL_MARGIN_DEFLATE);
					lpncsp->rgrc[0] = rt;
				}
				return lRet;
			}
			else
			{
				SetMsgHandled(FALSE);
			}
			return 0;
		}
		HICON OnGetIcon(int)
		{
			return NULL;
		}
		void OnPrint(CDCHandle dc, UINT uFlags)
		{
			CCoreMenu * pMenu = MenuFromHWND(m_hWnd);
			if(!pMenu)
			{
				SetMsgHandled(FALSE);
				return;
			}
			DrawBorder(dc, pMenu);
			DrawClient(dc, pMenu);
		}
	};
	class CCoreMenuHelper : public CWindowImplEx<CCoreMenuHelper>
	{
	public:
		CCoreMenuHelper(){}
		~CCoreMenuHelper(){}

		BEGIN_MSG_MAP_EX(CCoreMenuHelper)
			MSG_WM_MEASUREITEM(OnMeasureItem)
			MSG_WM_DRAWITEM(OnDrawItem)
			CHAIN_MSG_MAP(CWindowImplEx)
		END_MSG_MAP()

		void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
		{
			SetMsgHandled(FALSE);
			if(nIDCtl || lpDrawItemStruct->CtlType != ODT_MENU)
			{				
				return;
			}
			CCoreMenu * pMenu = CCoreMenu::FromHandle((HMENU)lpDrawItemStruct->hwndItem);
			if(!pMenu)
			{
				return;
			}
			TCHAR szText[MAX_PATH];
			MENUITEMINFO info;
			info.cbSize = sizeof(info);
			info.fMask = MIIM_STRING | MIIM_STATE;
			info.dwTypeData = szText;
			info.cch = MAX_PATH;
			// render background and side
			CRect rt = lpDrawItemStruct->rcItem;
			CRect rtText = rt;
			rtText.DeflateRect(pMenu->GetSideWidth(), 0, 0, 0);
			CRect rtTextBk = rtText;
			rtTextBk.DeflateRect(TEXT_MARGIN, 0, pMenu->m_nItemVMargin / 2, 0);
			// render background
			if(pMenu->GetSideImage())
			{
				DRAW_PARAM dp ={0};
				dp.dwMask = DP_TILE;
				dp.cy = rt.Height();
				pMenu->GetSideImage()->Draw(lpDrawItemStruct->hDC, 0, rt.top, &dp);
			}
			else if(pMenu->GetSideWidth())
			{
				CRect rtSide = rt;
				rtSide.right = rtSide.left + pMenu->GetSideWidth();
				GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, &rtSide, pMenu->GetBGColor());
			}
			GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, &rtText, pMenu->GetBGColor());
			if(!lpDrawItemStruct->itemID)
			{
				// seperator
				CRect rtSep = rtText;
				rtSep.top = rtText.top + rtText.Height() / 2;
				rtSep.bottom = rtSep.top + 1;
				rtSep.left++;
				rtSep.right--;
				if(!pMenu->GetSideImage())
				{
					rtSep.left -= pMenu->GetSideWidth();
				}
				GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, &rtSep, pMenu->GetSepColor());
				return;

			}
			// get item info and state
			DWORD dwTextColor = pMenu->GetTextColor();
			pMenu->GetMenuItemInfo(lpDrawItemStruct->itemID, FALSE, &info);
			if(info.fState & MFS_DISABLED)
			{
				dwTextColor = RGB(128, 128, 128);
			}
			else
			{
				if(info.fState & MFS_HILITE)
				{
					if(pMenu->GetSelectColor() != (DWORD)-1)
					{
						GetSonicUI()->FillSolidRect(lpDrawItemStruct->hDC, &rt, pMenu->GetSelectColor());
					}
					if(pMenu->GetHilightTextColor() != (DWORD)-1)
					{
						dwTextColor = pMenu->GetHilightTextColor();
					}
				}
			}
			// render check icon
			if(info.fState & MFS_CHECKED)
			{
				pMenu->m_pCheck->ForceColor(TRUE, dwTextColor, FALSE);
				pMenu->m_pCheck->TextOut(lpDrawItemStruct->hDC, rt.left + 3, rt.top + (rt.Height() - pMenu->m_pCheck->GetHeight()) / 2);
			}
			else
			{
				// render icon
				MENU_ITEM_DATA * pData = pMenu->GetMenuItemData(lpDrawItemStruct->itemID);
				if(pData && pData->pImg)
				{
					pData->pImg->Draw(lpDrawItemStruct->hDC, rt.left + 3, rt.top + (rt.Height() - pData->pImg->GetHeight()) / 2);
				}
			}
			
			SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
			::SetTextColor(lpDrawItemStruct->hDC, dwTextColor);
			HFONT hFont = pMenu->m_hFont ? pMenu->m_hFont : GetSonicUI()->GetDefaultFont();
			HFONT hOldFont = (HFONT)SelectObject(lpDrawItemStruct->hDC, hFont);
			DrawText(lpDrawItemStruct->hDC, szText, lstrlen(szText), &rtTextBk, DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_TABSTOP | (pMenu->GetTabCount() << 8));
			SelectObject(lpDrawItemStruct->hDC, hOldFont);
			SetMsgHandled(TRUE);
		}

		void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
		{
			if(nIDCtl || lpMeasureItemStruct->CtlType != ODT_MENU)
			{
				return;
			}
			CCoreMenu * pMenu = (CCoreMenu *)lpMeasureItemStruct->itemData;
			if(!pMenu)
			{
				return;
			}
			if(lpMeasureItemStruct->itemID == 0)
			{
				// seperator
				lpMeasureItemStruct->itemHeight = pMenu->m_nItemVMargin / 2 + 1;
				return;
			}
			// calculate item region
			TCHAR szText[MAX_PATH];
			MENUITEMINFO info;
			info.cbSize = sizeof(info);
			info.fMask = MIIM_STRING;
			info.dwTypeData = szText;
			info.cch = MAX_PATH;			
			pMenu->GetMenuItemInfo(lpMeasureItemStruct->itemID, FALSE, &info);
			ISonicImage * pImg = GetSonicUI()->CreateImage();
			pImg->Create(1, 1);
			HFONT hFont = pMenu->m_hFont ? pMenu->m_hFont : GetSonicUI()->GetDefaultFont();
			HFONT hOldFont = (HFONT)SelectObject(pImg->GetDC(), hFont);
			DRAWTEXTPARAMS dtp = {0};
			dtp.cbSize = sizeof(dtp);
			dtp.uiLengthDrawn = lstrlen(szText);
			dtp.iTabLength = pMenu->GetTabCount();
			CRect rtText;
			DrawTextEx(pImg->GetDC(), szText, dtp.uiLengthDrawn, rtText, DT_CALCRECT | DT_SINGLELINE | DT_EXPANDTABS | DT_TABSTOP, &dtp);
			if(pMenu->m_nFixedWidth > 0)
			{
				lpMeasureItemStruct->itemWidth = pMenu->m_nFixedWidth;
			}
			else
			{
				lpMeasureItemStruct->itemWidth = rtText.Width() + pMenu->GetSideWidth() + 2 * TEXT_MARGIN;
			}
			lpMeasureItemStruct->itemHeight = rtText.Height() + pMenu->m_nItemVMargin;
			// adjust height with icon height
			MENU_ITEM_DATA * pData = pMenu->GetMenuItemData(lpMeasureItemStruct->itemID);
			if(pData && pData->pImg && GetSonicUI()->IsValidObject(pData->pImg))
			{
				lpMeasureItemStruct->itemHeight = max((int)lpMeasureItemStruct->itemHeight, pData->pImg->GetHeight());
			}
			SelectObject(pImg->GetDC(), hOldFont);
			SONICUI_DESTROY(pImg);
		}
	};
public:
	static CCoreMenu * FromHandle(HMENU hMenu)
	{
		if(!hMenu || !::IsMenu(hMenu))
		{
			return NULL;
		}
		MENUINFO mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_MENUDATA;
		if(::GetMenuInfo(hMenu, &mi))
		{
			CCoreMenu * pMenu = (CCoreMenu *)mi.dwMenuData;
			if(!IsBadReadPtr(pMenu, 8) && pMenu->m_dwMagic == CORE_MENU_MAGIC)
			{
				return pMenu;
			}
		}
		return NULL;
	}
	// static member functions
	static BOOL Initialize(HWND hWnd)
	{
		if(!hWnd)
		{
			return FALSE;
		}
		CCoreMenuHooker::InistallHooker();
		CCoreMenuHelper * pHelper = new CCoreMenuHelper;
		if(!pHelper->SubclassWindowOnce(hWnd))
		{
			delete pHelper;
			return FALSE;
		}
		pHelper->SetDeleteSelfOnDestroy(TRUE);
		pHelper->SetAutoRemoveProp(CORE_MENU_HELPER_NAME, pHelper);
		return TRUE;
	}
public:
	CCoreMenu(int nSideWidth = SIDE_WIDTH, int nItemVMargin = ITEM_VMARGIN, int nFixedWidth = 0) : m_dwMagic(CORE_MENU_MAGIC), m_nSideWidth(nSideWidth), m_nItemVMargin(nItemVMargin), m_nFixedWidth(nFixedWidth)
	{
		m_hFont = NULL;
		m_dwBGColor = RGB(255, 255, 255);
		m_dwTextColor = RGB(33, 33, 33);
		m_dwBorderColor = CORE_BORDER_COLOR;
		m_pSideImg = NULL;
		m_pArrowImg = NULL;
		m_pCheck = NULL;
		m_nTabCount = 0x10;
		m_dwHilightTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
		m_dwSelectColor = GetSysColor(COLOR_HIGHLIGHT);
		m_dwSepColor = CORE_BORDER_COLOR;
		if(m_nSideWidth < MIN_SIDE_WIDTH)
		{
			m_nSideWidth = MIN_SIDE_WIDTH;
		}
	}
	virtual ~CCoreMenu()
	{
		if(m_hMenu && IsMenu())
		{
			DestroyMenu();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// CCoreMenu must be created dynamically, so these functions are bypassed 
	void Attach(HMENU hMenuNew)
	{
		return;
	}
	HMENU Detach()
	{
		return NULL;
	}
	BOOL LoadMenu(ATL::_U_STRINGorID menu)
	{
		return FALSE;
	}

#ifndef _WIN32_WCE
	BOOL LoadMenuIndirect(const void* lpMenuTemplate)
	{
		FALSE;
	}
#endif
	CCoreMenu * GetSubMenu(int nPos)
	{
		return FromHandle(CMenuHandle::GetSubMenu(nPos));
	}
	//////////////////////////////////////////////////////////////////////////
	// attributes operation
	// caution: if the width of the side image is not equal to SIDE_WIDTH, the source image will be permanently horizontally stretched to fit the SIDE_WIDTH
	void SetSideImage(ISonicImage * pImg)
	{ 
		if(!pImg || !pImg->IsValid() || !m_nSideWidth)
		{
			return;
		}
		m_pSideImg = pImg; 
		if(m_pSideImg->GetWidth() != m_nSideWidth)
		{
			m_pSideImg->Stretch(m_nSideWidth, m_pSideImg->GetHeight());
		}
	}
	void SetArrowImage(ISonicImage * pImg)
	{
		m_pArrowImg = pImg;
	}
	POINT GetTrackPopupPos(){ return m_ptTrackPopup; }
	ISonicImage * GetArrowImage(){ return m_pArrowImg; }
	ISonicImage * GetSideImage(){ return m_pSideImg; }
	void SetBGColor(DWORD dwColor){ m_dwBGColor = dwColor; }
	DWORD GetBGColor(){ return m_dwBGColor; }
	void SetSepColor(DWORD dwColor){ m_dwSepColor = dwColor; }
	DWORD GetSepColor(){ return m_dwSepColor; }
	void SetBorderColor(DWORD dwColor){ m_dwBorderColor = dwColor; }
	DWORD GetBorderColor(){ return m_dwBorderColor; }
	void SetTextColor(DWORD dwColor){ m_dwTextColor = dwColor; }
	DWORD GetTextColor(){ return m_dwTextColor; }
	void SetSelectColor(DWORD dwColor){ m_dwSelectColor = dwColor; }
	DWORD GetSelectColor(){ return m_dwSelectColor; }
	void SetHilightTextColor(DWORD dwColor){ m_dwHilightTextColor = dwColor; }
	DWORD GetHilightTextColor(){ return m_dwHilightTextColor; }
	int GetTabCount(){ return m_nTabCount; }
	// note: if you need to change tab count(for '\t'), call this function before adding any menu items
	void SetTabCount(int nCount){ if(nCount<=0)nCount = 8; m_nTabCount = nCount; }
	int GetSideWidth(){ return m_nSideWidth; }
	void SetCheckImg(ISonicImage * pImgCheck)
	{
		if(pImgCheck && pImgCheck->IsValid())
		{
			m_pCheck->Clear();
			m_pCheck->Format(_T("/p=%d/"), pImgCheck->GetObjectId());
		}
	}
	BOOL TrackPopupMenu(UINT uFlags, int x, int y, HWND hWnd, LPCRECT lpRect = NULL)
	{
		PreTrackPopupMenu(uFlags, x, y);
		return CMenuHandle::TrackPopupMenu(uFlags, x, y, hWnd, lpRect);
	}
	BOOL TrackPopupMenuEx(UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm = NULL)
	{
		PreTrackPopupMenu(uFlags, x, y);
		return CMenuHandle::TrackPopupMenuEx(uFlags, x, y, hWnd, lptpm);
	}
	void SetFont(LPCTSTR szFaceName, int nFontHeight, BOOL bBold = FALSE, BOOL bItalic = FALSE)
	{
		if(m_hFont)
		{
			DeleteObject(m_hFont);
		}
		LOGFONT lf = {0};
		lf.lfHeight = -abs(nFontHeight);
		lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
		lf.lfItalic = bItalic;
		lf.lfQuality = DRAFT_QUALITY;
		lstrcpy(lf.lfFaceName, szFaceName);
		m_hFont = CreateFontIndirect(&lf);
	}
	//////////////////////////////////////////////////////////////////////////
	// Create/destroy methods
	BOOL CreateMenu()
	{
		// not implement, only support popup menu
		ATLASSERT(m_hMenu == NULL);
		return NULL;
	}

	BOOL CreatePopupMenu()
	{
		ATLASSERT(m_hMenu == NULL);
		m_hMenu = ::CreatePopupMenu();
		if(m_hMenu)
		{
			AssociateHandle();
		}
		return (m_hMenu != NULL) ? TRUE : FALSE;
	}

	MENU_ITEM_DATA * GetMenuItemData(UINT wID)
	{
		for(VEC_ITEM::iterator it = m_vecItem.begin(); it != m_vecItem.end(); it++)
		{
			if(it->wID == wID)
			{
				return &(*it);
			}
		}
		return NULL;
	}
	void SetMenuItemData(UINT wID, MENU_ITEM_DATA data)
	{
		MENU_ITEM_DATA * pData = GetMenuItemData(wID);
		if(pData)
		{
			*pData = data;
		}
		else
		{
			m_vecItem.push_back(data);
		}
	}
	BOOL DeleteMenuItemData(UINT wID)
	{
		for(VEC_ITEM::iterator it = m_vecItem.begin(); it != m_vecItem.end(); it++)
		{
			if(it->wID == wID)
			{
				m_vecItem.erase(it);
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL CoreInsertItem(int iIndex, UINT wID, LPCTSTR lpszText, ISonicImage * pImg = NULL, HMENU hSub = NULL)
	{
		if(iIndex == -1)
		{
			iIndex = GetMenuItemCount();
		}
		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_STRING | MIIM_ID | MIIM_FTYPE | MIIM_DATA;
		info.wID = wID;
		if(hSub)
		{
			info.fMask |= MIIM_SUBMENU;
			info.hSubMenu = hSub;
			info.wID = (UINT)(UINT_PTR)hSub;
		}
		info.fType = MFT_OWNERDRAW;
		info.dwTypeData = (LPTSTR)lpszText;
		info.cch = lstrlen(lpszText);
		info.dwItemData = (ULONG_PTR)this;
		if(InsertMenuItem((UINT)iIndex, TRUE, &info))
		{
			MENU_ITEM_DATA data = {info.wID, pImg};
			SetMenuItemData(info.wID, data);
			return TRUE;
		}
		return FALSE;
	}

	BOOL CoreInsertSepItem(int iIndex)
	{
		if(iIndex == -1)
		{
			iIndex = GetMenuItemCount();
		}
		MENUITEMINFO info;
		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask = MIIM_FTYPE | MIIM_DATA;
		info.fType = MFT_SEPARATOR | MFT_OWNERDRAW;
		info.dwItemData = (ULONG_PTR)this;
		return InsertMenuItem((UINT)iIndex, TRUE, &info);
	}

	BOOL CoreRemoveMenu(UINT wID)
	{
		if(CMenuHandle::RemoveMenu(wID, MF_BYCOMMAND))
		{
			DeleteMenuItemData(wID);
			return TRUE;
		}
		return FALSE;
	}
	BOOL CoreDeleteMenu(UINT wID)
	{
		if(CMenuHandle::DeleteMenu(wID, MF_BYCOMMAND))
		{
			DeleteMenuItemData(wID);
			return TRUE;
		}
		return FALSE;
	}
	BOOL DestroyMenu()
	{
		int nCount = GetMenuItemCount();
		for (int i = 0; i < nCount; i++)
		{
			CCoreMenu * pSub = GetSubMenu(i);
			if(pSub)
			{
				pSub->DestroyMenu();
			}
		}
		m_vecItem.clear();
		SONICUI_DESTROY(m_pCheck);
		if(m_hFont)
		{
			DeleteObject(m_hFont);
			m_hFont = NULL;
		}
		if(CMenuHandle::DestroyMenu())
		{			
			return TRUE;
		}
		return FALSE;
	}
protected:
	void PreTrackPopupMenu(UINT& uFlags, int& x, int& y)
	{
		m_ptTrackPopup.x = x;
		m_ptTrackPopup.y = y;
	}
	BOOL AssociateHandle()
	{
		if(!IsMenu())
		{
			return FALSE;
		}
		m_pCheck = GetSonicUI()->CreateString();
		m_pCheck->Format(_T("/font, font_face='宋体', font_height=12/√"));
		MENUINFO mi;
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_MENUDATA;
		mi.dwMenuData = (ULONG_PTR)this;
		return SetMenuInfo(&mi);
	}
};
__declspec(selectany) HHOOK CCoreMenu::CCoreMenuHooker::s_Hook = NULL;

//////////////////////////////////////////////////////////////////////////
// class: ICoreInputBoxCallback
//////////////////////////////////////////////////////////////////////////
class CCoreInputBox;
class ICoreInputBoxCallback
{
public:
	enum enPromptType
	{
		ptNone = 0,
		ptType,					// means the user is typing characters in the input box
		ptDropdown,				// means the user is clicking the dropdown button
	};
public:
	// return TRUE to continue showing the list box, otherwise keep the list box invisible
	virtual BOOL OnPrePopuptListBox(CCoreInputBox * pInputBox, DWORD dwPromptType) = 0;
	virtual void OnListBoxSelChange(CCoreInputBox * pInputBox, DWORD dwPromptType) = 0;
	virtual void OnSubmit(CCoreInputBox * pInputBox) = 0;
};
#define WM_HIDELISTBOX			WM_USER + 0x1163
//////////////////////////////////////////////////////////////////////////
// class: CCoreInputBox
//////////////////////////////////////////////////////////////////////////
#pragma optimize("", off)
class CCoreInputBox : public CWindowWithReflectorImplFixed<CCoreInputBox, CEdit>, public ISonicInputFilter
{
public:
	class CCoreInputListBox : public CCoreListBox
	{
	private:
		CCoreInputBox * m_pib;
		DWORD m_dwPromptType;
	public:
		BEGIN_MSG_MAP_EX(CCoreInputListBox)
			MSG_OCM_COMMAND(OnReflectedCommand)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_KILLFOCUS(OnKillFocus)
			CHAIN_MSG_MAP(CCoreListBox)
		END_MSG_MAP()

		CCoreInputListBox(CCoreInputBox * pib) : m_pib(pib)
		{
			m_dwPromptType = 0;
		}
		void SetPromptType(DWORD dwPromptType){ m_dwPromptType = dwPromptType; }
		DWORD GetPromptType(){ return m_dwPromptType; }
	protected:
		void OnReflectedCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
		{
			if(uNotifyCode == LBN_SELCHANGE)
			{
				m_pib->OnLBSelChange();
			}
		}
		void OnLButtonUp(UINT nFlags, CPoint point)
		{
			m_pib->PostMessage(WM_HIDELISTBOX);
		}
		void OnKillFocus(CWindow wndFocus)
		{
			if(wndFocus != *m_pib && wndFocus != m_pib->GetHost())
			{
				m_pib->PostMessage(WM_HIDELISTBOX);
			}
			SetMsgHandled(FALSE);
		}
	};
private:
	HBRUSH m_hBkBrush;
	DWORD m_dwBkColor;
	DWORD m_dwTextColor;
	CString m_strPlaceHolder;
	DWORD m_dwPlaceHolderColor;
	BOOL m_bIsUsingPlaceHolder;
	ISonicString * m_pBtnDropdown;
	CCoreInputListBox m_lb;
	CSize m_szlb;
	BOOL m_bPassword;
	ICoreInputBoxCallback * m_pCallback;
public:
	BEGIN_MSG_MAP_EX(CCoreInputBox)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MESSAGE_HANDLER(WM_HIDELISTBOX, OnHideListBox)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MSG_WM_CHAR(OnChar)
		MSG_OCM_CTLCOLOREDIT(OnCtlColorEdit)
		MSG_OCM_CTLCOLORSTATIC(OnCtlColorEdit)
		CHAIN_MSG_MAP(CWindowWithReflectorImplFixed)
	END_MSG_MAP()
public:
	CCoreInputBox() : m_lb(this)
	{
		m_hBkBrush = NULL;
		m_dwBkColor = 0;
		m_dwTextColor = 0;
		m_dwPlaceHolderColor = 0;
		m_bIsUsingPlaceHolder = FALSE;
		m_pBtnDropdown = NULL;
		m_pCallback = NULL;
		m_bPassword = FALSE;
	}
	HWND Create(HWND hWndParent, DWORD dwStyle, _U_RECT rect, ISonicImage * pImgDropdown = NULL, ICoreInputBoxCallback * pCallback = NULL,
		DWORD dwExStyle = 0, _U_MENUorID MenuOrID = 0U)
	{
		// input box is designed for single-line input only
		dwStyle &= ~(ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN);
		HWND hRet = Create(hWndParent, rect, NULL, dwStyle, dwExStyle, MenuOrID);
		if(hRet)
		{
			if(!m_pOldSetTextColor)
			{
				m_pOldSetTextColor = GetSonicUI()->DetourFunctionEntry(GetProcAddress(GetModuleHandle(_T("gdi32")), "SetTextColor"), 
					&CCoreInputBox::DetourSetTextColor);
			}
			if(dwStyle & ES_PASSWORD)
			{
				m_bPassword = TRUE;
			}
			m_rtMargin.left = m_rtMargin.right = 4;
			if(pImgDropdown)
			{
				m_pBtnDropdown = GetSonicUI()->CreateString();
				m_pBtnDropdown->Format(_T("/a, p=%d, linkc=0, btn_type=1/"), pImgDropdown->GetObjectId());
				m_pBtnDropdown->Delegate(DELEGATE_EVENT_LBUTTONDOWN, NULL, this, &CCoreInputBox::OnClickDropdown);
				m_rtMargin.right += m_pBtnDropdown->GetWidth() + m_rtMargin.left;
				ISonicSkin * pParentSkin = GetSonicUI()->SkinFromHwnd(m_wndReflector);
				pParentSkin->GetPaint()->AddObject(m_pBtnDropdown->GetObjectId(), -m_rtMargin.left, AOA_CENTER);
			}
			m_pCallback = pCallback;
			SetFont(GetSonicUI()->GetDefaultFont());
			m_lb.Create(m_hWnd, CRect(0, 0, 0, 0), NULL, WS_POPUP | WS_BORDER | LBS_HASSTRINGS | LBS_NOTIFY | WS_VSCROLL);
			m_lb.SetHotItemColor(m_lb.GetSelectColor());
			m_lb.SetSelectColor(-1);
			m_lb.SetCoreStyle(m_lb.GetCoreStyle() & ~CCoreListBox::CLBS_FOCUS_TEXT_COLOR_CHANGE);
			m_lb.SetCurSel(-1);
			SetAutoRemoveProp(CORE_INPUT_BOX, this);
		}
		return hRet;
	}
	BOOL OnInputMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if((hWnd == m_hWnd || hWnd == m_lb) && uMsg == WM_KEYDOWN)
		{
			if(wParam == VK_RETURN || wParam == VK_ESCAPE)
			{
				BOOL bHandled;
				OnKeyDown(uMsg, wParam, lParam, bHandled);
				return FALSE;
			}
		}
		if(uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN)
		{
			if(m_lb.IsWindow() && m_lb.IsWindowVisible())
			{
				POINT pt = {(SHORT)LOWORD(lParam), (SHORT)HIWORD(wParam)};
				::ClientToScreen(hWnd, &pt);
				CRect rtWnd;
				m_lb.GetWindowRect(rtWnd);
				if(!rtWnd.PtInRect(pt))
				{
					PostMessage(WM_HIDELISTBOX);
				}

			}
		}
		return TRUE;
	}
	CCoreListBox & GetListBox()
	{
		return m_lb;
	}
	ISonicString * GetDropdownButton()
	{
		return m_pBtnDropdown;
	}
	void SetPlaceHolder(LPCTSTR szText, DWORD dwColor)
	{
		m_dwPlaceHolderColor = dwColor;
		m_strPlaceHolder = szText;
		if(IsWindow() && GetFocus() != m_hWnd)
		{
			OnKillFocus(m_hWnd);
		}
	}
	void SetWindowText(LPCTSTR lpszString)
	{
		CEdit::SetWindowText(lpszString);
		m_bIsUsingPlaceHolder = FALSE;
		CEdit::SetSel(lstrlen(lpszString), -1);
	}
	void SetFont(HFONT hFont, BOOL bRedraw = TRUE)
	{
		CWindow::SetFont(hFont, bRedraw);
		CClientDC dc(m_hWnd);
		SIZE szChar;
		if(GetTextExtentPoint32(dc, _T("1"), 1, &szChar))
		{
			CRect rtWnd;
			GetWindowRect(rtWnd);
			int nTopMargin = (rtWnd.Height() - GetBorderWidth() * 2 - szChar.cy) / 2 + 1;
			nTopMargin = nTopMargin < 0 ? 0 : nTopMargin;
			int nBottomMargin = rtWnd.Height() - GetBorderWidth() * 2 - szChar.cy - nTopMargin;
			nBottomMargin  = nBottomMargin < 0 ? 0 : nBottomMargin;
			SetMargin(m_rtMargin.left, nTopMargin, m_rtMargin.right, nBottomMargin);
		}
	}
	void SetBkColor(DWORD dwBkColor)
	{
		m_dwBkColor = dwBkColor;
		if(m_hBkBrush)
		{
			DeleteObject(m_hBkBrush);
		}
		m_hBkBrush = CreateSolidBrush(m_dwBkColor);
		ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_wndReflector);
		if(pSkin)
		{
			pSkin->SetSkin(_T("bg"), _T("bg_color:%d"), dwBkColor);
		}
	}
	DWORD GetBkColor()
	{
		return m_dwBkColor;
	}
	void SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
	}
	DWORD GetTextColor()
	{
		return m_dwTextColor;
	}
	void SetListBoxSize(int cx, int cy)
	{
		m_szlb.cx = cx;
		m_szlb.cy = cy;
	}
	void HideListBox()
	{
		if(!m_lb.IsWindowVisible())
		{
			return;
		}
		m_lb.ShowWindow(SW_HIDE);
		if(m_lb.GetPromptType() == ICoreInputBoxCallback::ptDropdown)
		{
			m_pBtnDropdown->Push(FALSE);
		}
		m_lb.SetPromptType(0);
	}
	BOOL PopupListBox(DWORD dwPromptType)
	{
		if(m_pCallback)
		{
			if(!m_pCallback->OnPrePopuptListBox(this, dwPromptType))
			{
				return FALSE;
			}
		}
		if(!m_lb.GetCount())
		{
			return FALSE;
		}
		CRect rtWnd;
		GetWindowRect(rtWnd);
		int nWidth = m_szlb.cx <= 0 ? rtWnd.Width() : m_szlb.cx;
		int nHeight = m_lb.GetBorderColor() == -1 ? 0 : 2;
		for(int i = 0; i < m_lb.GetCount(); i++)
		{
			nHeight += m_lb.GetItemHeight(i);
		}
		if(m_szlb.cy > 0)
		{
			nHeight = nHeight < m_szlb.cy ? nHeight : m_szlb.cy;
		}
		m_lb.SetWindowPos(NULL, rtWnd.left, rtWnd.bottom + 1, nWidth, nHeight, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
		m_lb.SetPromptType(dwPromptType);
		m_lb.SetHotItem(m_lb.GetCurSel());
		return TRUE;
	}
	void OnLBSelChange()
	{
		ISonicPaint * pPaint = m_lb.GetItemPaint(m_lb.GetCurSel());
		if(pPaint)
		{
			for(int i = 0; i < pPaint->GetCount(); i++)
			{
				if(pPaint->GetPaintItem(i, TRUE)->pBase->GetType() == BASE_TYPE_STRING)
				{
					ISonicString * pText = (ISonicString *)pPaint->GetPaintItem(i, TRUE)->pBase;
					CString strText = pText->GetTextWithoutCtrl();
					if(strText.GetLength())
					{
						strText.TrimLeft();
						strText.TrimRight();
						SetWindowText(strText);
						m_lb.SetHotItem(m_lb.GetCurSel());
						if(m_pCallback)
						{
							m_pCallback->OnListBoxSelChange(this, m_lb.GetPromptType());
						}
						break;
					}
				}
			}
		}
	}
protected:
	static LPVOID m_pOldSetTextColor;
	static DWORD WINAPI DetourSetTextColor(HDC hdc, DWORD dwColor)
	{
		if(dwColor == GetSysColor(COLOR_GRAYTEXT))
		{
			LPVOID pRetAddr;
			__asm
			{
				mov eax, [ebp + 4]
				mov [pRetAddr], eax
			}
			HMODULE hMod;
			if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)pRetAddr, &hMod))
			{
				if(hMod == GetModuleHandle(_T("comctl32.dll")))
				{
					LPVOID pEbp;
					__asm
					{
						push [ebp]
						pop [pEbp]
					}
					for(int i = 0; i < 6; i++)
					{
						HWND hWnd;
						__asm
						{
							push edx
							mov edx, [pEbp]
							mov eax, [edx + 8]
							mov [hWnd], eax
							mov eax, [edx]
							mov [pEbp], eax
							pop edx
						}
						CCoreInputBox * pInputBox = NULL;
						if(::IsWindow(hWnd) && (pInputBox = (CCoreInputBox *)GetProp(hWnd, CORE_INPUT_BOX)) && !::IsWindowEnabled(hWnd))
						{
							dwColor = pInputBox->GetTextColor();
						}
					}
				}
			}
		}
		__asm
		{
			push [ebp + 0ch]
			push [ebp + 8]
			call [m_pOldSetTextColor]
		}
	}
	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL, DWORD dwStyle = 0, DWORD dwExStyle = 0, 
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return CWindowWithReflectorImplFixed::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);;
	}
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		SetBkColor(RGB(255, 255, 255));
		GetSonicUI()->AddInputFilter(this);
		SetMsgHandled(FALSE);
		return 0;
	}
	LRESULT OnHideListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		HideListBox();
		return 0;
	}
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
	{
		switch(wParam)
		{
		case VK_RETURN:
			HideListBox();
			if(m_pCallback)
			{
				m_pCallback->OnSubmit(this);
			}
			break;
		case VK_UP:
		case VK_DOWN:
			if(m_lb.IsWindowVisible())
			{
				m_lb.SendMessage(uMsg, wParam, lParam);
				m_lb.SetHotItem(m_lb.GetCurSel());
			}
			break;
		case VK_DELETE:
			DefWindowProc();
			if(!PopupListBox(ICoreInputBoxCallback::ptType))
			{
				HideListBox();
			}
			break;
		case VK_ESCAPE:
		case VK_CONTROL:
			HideListBox();
		default:
			bHandled = FALSE;
			break;
		}
		return 0;
	}
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if(GetKeyState(VK_CONTROL) < 0)
		{
			SetMsgHandled(FALSE);
			if(nChar == 1)
			{
				SetSel(0, -1);
			}
		}
		else
		{
			switch (nChar)
			{
			case VK_TAB:
			case VK_CANCEL:
			case 0x16:
				HideListBox();
				SetMsgHandled(FALSE);
				return;
			case VK_RETURN:
			case VK_ESCAPE:
				return;
			default:
				DefWindowProc();
				if(!PopupListBox(ICoreInputBoxCallback::ptType))
				{
					HideListBox();
				}
				break;
			}	
		}	
	}
	void OnClickDropdown(ISonicString * pBtn, LPVOID)
	{
		if(!m_lb.IsWindowVisible())
		{
			if(!PopupListBox(ICoreInputBoxCallback::ptDropdown))
			{
				pBtn->Push(FALSE);
			}
		}
		if(IsWindowEnabled())
		{
			SetFocus();
		}
		else
		{
			m_lb.SetFocus();
		}
	}
	void OnSetFocus(CWindow wndOld)
	{
		if(m_bIsUsingPlaceHolder)
		{
			SetWindowText(_T(""));
		}
		if(m_bPassword)
		{
			SetPasswordChar(_T('*'));
		}
		SetMsgHandled(FALSE);
	}
	void OnKillFocus(CWindow wndFocus)
	{
		if(m_lb.IsWindow() && m_lb.IsWindowVisible())
		{
			if(wndFocus != m_lb && wndFocus != m_lb.GetHost())
			{
				HideListBox();
			}
		}
		if(GetWindowTextLength() == 0)
		{
			m_bIsUsingPlaceHolder = TRUE;
			if(m_bPassword)
			{
				SetPasswordChar(0);
			}
			CEdit::SetWindowText(m_strPlaceHolder);
		}		
		SetMsgHandled(FALSE);
	}
	void OnDestroy()
	{
		GetSonicUI()->RemoveInputFilter(this);
		if(m_hBkBrush)
		{
			DeleteObject(m_hBkBrush);
		}
		SONICUI_DESTROY(m_pBtnDropdown);
		if(m_lb.IsWindow())
		{
			m_lb.DestroyWindow();
		}
		SetMsgHandled(FALSE);
	}
	HBRUSH OnCtlColorEdit(CDCHandle dc, CEdit edit)
	{
		if(m_bIsUsingPlaceHolder)
		{
			dc.SetTextColor(m_dwPlaceHolderColor);
		}
		else
		{
			dc.SetTextColor(m_dwTextColor);
		}
		dc.SetBkMode(OPAQUE);
		dc.SetBkColor(m_dwBkColor);
		return m_hBkBrush;
	}
};
#pragma optimize("", on)
__declspec(selectany) LPVOID CCoreInputBox::m_pOldSetTextColor = NULL;

/////////////////////////////////////////////////////////////////////////
// export some stl containers to keep crt safe
#ifdef CORE_CROSS_MODULE
EXP_STL template class COREWIDGHT_API std::vector<CCoreMenu::MENU_ITEM_DATA>;
EXP_STL template class COREWIDGHT_API std::vector<CString>;
EXP_STL template class COREWIDGHT_API std::vector<ISonicPaint *>;
EXP_STL template class COREWIDGHT_API std::vector<CCoreListBox::ITEM_DATA>;
EXP_STL template class COREWIDGHT_API std::vector<CCoreComboBox::ITEM_DATA>;
#endif
};	// namespace WTL