#pragma once

#include"CoreWidget.h"
#include <map>
#include <shellapi.h>
//#include "toolkit.h"
namespace WTL{


#define IDM_RICH_SELALL                 40001
#define IDM_RICH_COPY                   40002
#define IDM_RICH_CUT                    40003
#define IDM_RICH_PASTE                  40004
#define IDM_RICH_CLEAR                  40005
#define IDM_RICH_UNDO                   40006

#define  WM_EDIT_VK_RETURN_KEYDOW          WM_USER + 4101


#define SE_XORPAD_SIZE 32
#define TCH_STDPWCHAR _T('*')

typedef struct tagTextFontInfo
{
	tagTextFontInfo()
		:bQuality(0)
		,lEscapement(0)
		,lOrientation(0)
		,bItalic(0)
		,bUnderline(0)
		,bStrikeOut(0)
		,bCharSet(0)
		,bOutPrecision(0)
		,bClipPrecision(0)
		,bPitchAndFamily(0)
		,dwEffects(0)
		,wWeight(0)
		,lWidth(0)
		,lHeight(12)
		,dwTextColor(RGB(255,255,255))
	{

	}
	CString     strFaceName;
	DWORD		dwTextColor;
	DWORD		dwBackColor;
	LONG		lHeight;
	WORD		wWeight;
	DWORD		dwEffects;
	LONG        lWidth;
	LONG        lEscapement;
	LONG        lOrientation;
	BYTE        bItalic;
	BYTE        bUnderline;
	BYTE        bStrikeOut;
	BYTE        bCharSet;
	BYTE        bOutPrecision;
	BYTE        bClipPrecision;
	BYTE        bQuality;
	BYTE        bPitchAndFamily;
}TEXTFONT_INFO,LPTEXTFONT_INFO;
typedef struct tagMenuItemInfo
{
	CString		strText;
	UINT		uID;
	CString     strIcon;
	CString     strSubItem;
//	tagMenuInfo * pSubMenu;
}MENU_ITEMINFO,*LPMENU_ITEMINFO;
typedef struct tagMenuInfo
{
    tagMenuInfo()
		:dwTextColor((DWORD)-2)
		,dwSelectColor((DWORD)-2)
		,dwBGColor((DWORD)-2)
		,dwBorderColor((DWORD)-2)
	{
		nSideWidth = CCoreMenu::SIDE_WIDTH;
		nItemVMargin = CCoreMenu::ITEM_VMARGIN;
		nFixedWidth = 0;
	}
	DWORD		dwTextColor;
	DWORD		dwBGColor;
	DWORD       dwBorderColor;
	DWORD       dwSelectColor;
	int         nSideWidth;
	int			nItemVMargin;
	int			nFixedWidth;
	CString		strID;
	CString		strType;
	CString		strBGImage;
	CString     strSideImage;
	std::vector<MENU_ITEMINFO> vecItemInfo;
	TEXTFONT_INFO textFontInfo;
}MENU_INFO,*LPMENU_INFO;

template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits> class CWindowImplEdit;
template<class T, class TBase, class TWinTraits>
class CWindowImplEdit : public CWindowImplEx<T, TBase, TWinTraits>
{
public:
	HWND Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		int nOffset = 0;
		CRect rtReflector = rect.m_lpRect;
		if(m_wndReflector.m_dwBorderColor != (DWORD)-1)
		{
			// remove the WS_BORDER, replace the original border with this fake custom one
			nOffset = 1;
			dwStyle = dwStyle & ~WS_BORDER;
			rtReflector.InflateRect(nOffset, nOffset, nOffset, nOffset);
		}
		m_wndReflector.Create(hWndParent, rtReflector, NULL, (dwStyle & (WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP))| WS_CLIPCHILDREN | ((dwStyle & WS_POPUP) ? WS_POPUP : WS_CHILD),
			dwExStyle & WS_EX_TOOLWINDOW);
		ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
		pSkin->Attach(m_wndReflector, FALSE);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_CHILD | WS_VISIBLE;
		dwExStyle &= ~WS_EX_TOOLWINDOW;
		RECT rcPos = { nOffset, nOffset, rect.m_lpRect->right - rect.m_lpRect->left + nOffset, rect.m_lpRect->bottom - rect.m_lpRect->top + nOffset };
		CRect rc = rcPos;
		rc.DeflateRect(m_rcOffset);
		HWND hRet = CWindowImplEx< T, TBase, TWinTraits >::Create(m_wndReflector, rc, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		if(hRet)
		{
			SetAutoRemoveProp(CORE_WINDOW_WITH_REFLECTOR, this);
		}
		SetHost(m_wndReflector);
		//RemoveSysBorder();
      //  SetWindowPos(NULL,rect.m_lpRect->left,rect.m_lpRect->top,rect.m_lpRect->right - rect.m_lpRect->left,rect.m_lpRect->bottom - rect.m_lpRect->top,SWP_NOSENDCHANGING|SWP_NOZORDER | SWP_NOACTIVATE);
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
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(CWindowImplEx)
	END_MSG_MAP()

	void SetBorderColor(DWORD dwColor)
	{
		m_wndReflector.m_dwBorderColor = dwColor;
		RemoveSysBorder();
	}
	void SetBGColor(DWORD dwColor)
	{
		m_wndReflector.m_dwBGColor = dwColor;
	}
	void SetDeflateRect(int l, int t, int r, int b)
	{
		m_rcOffset.left = l;
		m_rcOffset.top = t;
		m_rcOffset.right = r;
		m_rcOffset.bottom = b;
	}
	int GetBorderWidth(){ return m_wndReflector.m_dwBorderColor == (DWORD)-1 ? 0 : 1; }

	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(wParam == 0)
		{
			m_wndReflector.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
		}
		else
		{
			m_wndReflector.SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnNcDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		m_wndReflector.DestroyWindow();
		bHandled = FALSE;
		return 1;
	}
	void OnSize(UINT nType, CSize size)
	{
		m_wndReflector.Invalidate();
		SetMsgHandled(FALSE);
	}
	LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		WINDOWPOS* pWP = (WINDOWPOS*)lParam;
		m_wndReflector.SetWindowPos(pWP->hwndInsertAfter, pWP->x - GetBorderWidth(), pWP->y - GetBorderWidth(), 
			pWP->cx + GetBorderWidth() * 2, pWP->cy + GetBorderWidth() * 2, pWP->flags);

		pWP->x = GetBorderWidth() + m_rcOffset.left;
		pWP->y = GetBorderWidth() + m_rcOffset.top;
		pWP->cx = pWP->cx - m_rcOffset.left - m_rcOffset.right;
		pWP->cy = pWP->cy - m_rcOffset.top - m_rcOffset.bottom;
		pWP->flags &= ~SWP_HIDEWINDOW;
		return DefWindowProc(uMsg, wParam, lParam);
	}
private:
	CRect m_rcOffset;
	// reflector window stuff
	class Reflector : public CWindowImplEx<Reflector>
	{	
	public:
		DWORD m_dwBorderColor;
		DWORD m_dwBGColor;
	public:
		Reflector() : m_dwBorderColor((DWORD)-1)
		{
			m_dwBGColor = RGB(0X25,0X25,0X25);
		}
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
			REFLECT_NOTIFICATIONS()
			MSG_WM_SETFOCUS(OnSetFocus)
			MSG_WM_SIZE(OnSize)
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
					rt.DeflateRect(1,1,1,1);
					GetSonicUI()->FillSolidRect(hdc,rt,m_dwBGColor);
					EndPaint(&ps);
				}
				else
				{
					//bHandled = FALSE;
					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(&ps);
					CRect rt;
					GetClientRect(rt);
					GetSonicUI()->FillSolidRect(hdc,rt,m_dwBGColor);
					EndPaint(&ps);
				}
				return 0;
			}

			LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
			{
				return TRUE;
			}
			void OnSize(UINT nType, CSize size)
			{
				Invalidate();
				SetMsgHandled(FALSE);
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
	void RemoveSysBorder()
	{
		if(IsWindow())
		{
			DWORD dwStyle = (DWORD)GetWindowLong(GWL_STYLE);
			CRect rtWnd;
			GetWindowRect(rtWnd);
			rtWnd.InflateRect(m_rcOffset);
			if(m_wndReflector.GetWindowLong(GWL_STYLE) & WS_CHILD)
			{
				GetParent().ScreenToClient(rtWnd);
			}
			if(dwStyle & WS_BORDER)
			{				
				ModifyStyle(WS_BORDER, 0);
			}
			// repeatedly set window size to notify m_wndReflector to change it's size correspondingly
			SetWindowPos(NULL, rtWnd, SWP_NOZORDER | SWP_FRAMECHANGED);
		}	
	}
protected:
	Reflector m_wndReflector;
};


class CCoreEdit : public CWindowImplEdit<CCoreEdit,CEdit>
	                ,public CEditCommands<CCoreEdit>
				
{
public:
	CCoreEdit()
		:m_bSecMode(FALSE)
		,m_bHookKeyBoard(FALSE)
		,m_dwEditBkColor(RGB(0X25,0X25,0X25))
		,m_dwEditTextColor(0xacacac)
		,m_bMouseIn(FALSE)
		,m_bUpdateText(TRUE)
		,m_bPlaceHolder(FALSE)
		,m_bPlaceHolderSecMode(FALSE)
		,m_nPlaceaHolderFont(1)
		,m_bPassWord(FALSE)
	{
		m_hEditBrush = CreateSolidBrush(RGB(0X25,0X25,0X25));
		m_vecChars.clear();
	    m_vecXorPad.clear();
	}
	virtual ~CCoreEdit()
	{
 		if(m_hEditBrush)
 		{
 			DeleteObject(m_hEditBrush);
 		}
		if (m_bSecMode)
		{
// 			_DeleteAll();
// 			_SetMemoryEx(m_pXorPad, 0, SE_XORPAD_SIZE * sizeof(TCHAR));
// 			_DeleteTPtr(m_pXorPad, TRUE, FALSE);
// 			m_pXorPad = NULL;
			m_vecChars.clear(); 
			m_vecXorPad.clear();
		}	
	}
	BEGIN_MSG_MAP_EX(CCoreEdit)
		MSG_OCM_CTLCOLOREDIT(OnColorEdit)
		MSG_OCM_CTLCOLORSTATIC(OnReflectedCtlColorStatic)
		MSG_WM_CHAR(OnChar)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_KEYUP(OnKeyUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_SYSKEYDOWN(OnSysKeyDown)
		MSG_WM_RBUTTONDBLCLK(OnRButtonDblClk)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MBUTTONDBLCLK(OnMButtonDblClk)
	    MSG_WM_MBUTTONUP(OnMButtonUp)
		MSG_WM_DEADCHAR(OnDeadChar)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_MOUSELEAVE( OnMouseLeave)
		MSG_OCM_COMMAND(OnReflectedCommand)
		MSG_WM_PASTE(OnPaste)
	    CHAIN_MSG_MAP_ALT(CEditCommands<CCoreEdit>, 1)
		CHAIN_MSG_MAP(CWindowImplEdit)
	END_MSG_MAP()
	 void OnDestroy()
	{
		GetSonicUI()->DestroyObject(GetSonicUI()->SkinFromHwnd(m_hWnd));
		SetMsgHandled(FALSE);	
	}
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
		pSkin->Attach(m_hWnd, FALSE);
		pSkin->SetAttr(SATTR_SKIN_GAME_RENDERING, 1);
		pSkin->SetAttr(SATTR_ACCEPT_FOCUS, m_hWnd);
		SetMsgHandled(FALSE);
		return 0;
	}
	
	void OnReflectedCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		switch(uNotifyCode)
		{
		case EN_CHANGE:
			{
				
			//	SetPlaceHolderText(FALSE);
			}
			break;
		case EN_UPDATE:
			{
				if (m_bSecMode)
				{
					OnEnUpdate();
				}
			}

		default:
			return;
		}
	}
	HBRUSH OnReflectedCtlColorStatic(CDCHandle dc, CStatic wndStatic)
	{
		SetTextColor(dc, m_dwEditTextColor);
		SetBkMode(dc, OPAQUE);
		SetBkColor(dc,m_dwEditBkColor);
		return m_hEditBrush;
	}
	void OnPaste()
	{
		if (m_bSecMode)
		{
			PassWordPaste();
		}
		SetMsgHandled(FALSE);
	}
     void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	 {
			OnKeyDown(nChar,nRepCnt,nFlags);
	 }
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_bSecMode)
		{
			OnSecureEditKeyDown(nChar,nRepCnt,nFlags);
		}
		if (m_bHookKeyBoard)
		{
			UINT nFunc = 0;
			if (GetKeyState (VK_SHIFT) < 0)
			{
				nFunc = VK_SHIFT ;
			}
			else if (GetKeyState(VK_MENU) < 0)
			{
				nFunc = VK_MENU ;
			}
			else if (GetKeyState(VK_CONTROL) < 0)
			{
				nFunc = VK_CONTROL ;
			}
			CString strText = GetKeyBoardTextFromVK(nFunc,nChar);
			if (!strText.IsEmpty())
			{
				SetWindowText(strText);
				SetSel(strText.GetLength(),strText.GetLength());
			}
			return ;
		}
		if (GetKeyState(VK_CONTROL) < 0 && nChar == 65)
		{
			SetSel(0,-1);
			return ;
		}
		if (nChar == VK_RETURN )
		{ 
			::SendMessage(GetParent().m_hWnd,WM_EDIT_VK_RETURN_KEYDOW,0,0);
		}
		SetMsgHandled(FALSE);
		
	}
	 void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
	 {
		 SetMsgHandled(FALSE);
		
	 }
	 void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	 {
 		 if (m_bHookKeyBoard)
 		 {
 			  return;
 		 }	
		 if (m_bSecMode)
		 {
			 if (nChar >= 30)
			 {
				 //DWORD dwPos = GetSel() & 0xffff;
				 int nStart,nEnd;
				 GetSel(nStart,nEnd);
				 int nNum = nEnd - nStart;
				 TCHAR ch = (TCHAR)nChar;
				 if (nNum != 0)
				 {
					 _DeleteCharacters(nStart,nNum);
				 }
				 _InsertCharacters(nStart,&ch,1);
			 }

		 }
		 if (nChar == 13)
		 {
			 SetMsgHandled(TRUE);
			 return;
		 }
		 SetMsgHandled(FALSE);
	 }
	  void OnRButtonDown(UINT nFlags, CPoint point)
	  {
		  if (m_bHookKeyBoard || GetSonicUI()->GetOffscreenUICallback())
		  {
			  return;
		  }
		  SetMsgHandled(FALSE);
	  }
	 void OnRButtonDblClk(UINT nFlags, CPoint point)
	 {
		 SetMsgHandled(FALSE);
		
	 }
	  void OnRButtonUp(UINT nFlags, CPoint point)
	  {
		  SetMsgHandled(FALSE);
		
	  }
	  void OnMouseLeave()
	  {
		  if (GetSonicUI()->GetOffscreenUICallback() && m_bMouseIn)
		  {
			  GetSonicUI()->GetOffscreenUICallback()->osChangeCursor(32512);
			  m_bMouseIn = FALSE;
		  }
		  SetMsgHandled(FALSE);
	  }
	  void OnMouseMove(UINT nFlags, CPoint point)
	  {
		  if (GetSonicUI()->GetOffscreenUICallback() && !m_bMouseIn)
		  {
			   GetSonicUI()->GetOffscreenUICallback()->osChangeCursor(32513);
			   m_bMouseIn = TRUE;
		  }
		  
		  SetMsgHandled(FALSE);
		
	  }
	  void OnMButtonDblClk(UINT nFlags, CPoint point)
	  {
		  SetMsgHandled(FALSE);
	
	  }
	   void OnMButtonUp(UINT nFlags, CPoint point)
	   {
		   SetMsgHandled(FALSE);
		 
	   }
	    void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags)
		{
			SetMsgHandled(FALSE);
		
		}
		void OnSetFocus(CWindow wndOld)
		{
			SetPlaceHolderText(FALSE);
			SetMsgHandled(FALSE);
		
		}
	    void OnKillFocus(CWindow wndFocus)
		{
			SetPlaceHolderText(TRUE);
			SetMsgHandled(FALSE);
		}
	HBRUSH OnColorEdit(CDCHandle dc, CEdit edit)
	{
		SetTextColor(dc, m_dwEditTextColor);
		SetBkMode(dc, OPAQUE);
		SetBkColor(dc,m_dwEditBkColor);
		return m_hEditBrush;
	}
 	BOOL OnEraseBkgnd(CDCHandle dc)
 	{
// 		CRect rc;
// 		GetClientRect(rc);
// 		GetSonicUI()->FillSolidRect(dc,&rc,m_dwEditBkColor);
       return TRUE;
 	}
	 void SetHookKeyBoard(BOOL bHookKeyBoard = TRUE)
	{
		m_bHookKeyBoard = bHookKeyBoard ;
	} 
	void SetEditBkColor(DWORD clr)
	{
		m_dwEditBkColor = clr;
		if (m_hEditBrush)
		{
			DeleteObject(m_hEditBrush);
		}
		m_hEditBrush = CreateSolidBrush(m_dwEditBkColor);
	}
	void SetEditTextColor(DWORD clr)
	{
		m_dwEditTextColor = clr;
	}
	void AddTextFontInfo(TEXTFONT_INFO & textFontInfo)
	{
		m_vecFontInfo.push_back(textFontInfo);
	}
	BOOL GetTextFontInfo(int nIndex,TEXTFONT_INFO & textFontInfo)
	{
		if (nIndex >= 0 && nIndex < m_vecFontInfo.size())
		{
			textFontInfo = m_vecFontInfo[nIndex];
			return TRUE;
		}
		return FALSE;
	}
	BOOL SetEditFont(int nIndex)
	{
		TEXTFONT_INFO textFontInfo;
		if (GetTextFontInfo(nIndex,textFontInfo))
		{
			if(m_font.m_hFont)
			{
				m_font.DeleteObject();
			}
            m_font.CreateFont(textFontInfo.lHeight,textFontInfo.lWidth,textFontInfo.lEscapement,textFontInfo.lOrientation,textFontInfo.wWeight,textFontInfo.bItalic,textFontInfo.bUnderline,textFontInfo.bStrikeOut,
				textFontInfo.bCharSet,textFontInfo.bOutPrecision,textFontInfo.bClipPrecision,textFontInfo.bQuality,textFontInfo.bPitchAndFamily,textFontInfo.strFaceName);
			SetFont(m_font);
			SetEditTextColor(textFontInfo.dwTextColor);
			m_nFont = nIndex;
			return TRUE;
		}
		return FALSE;
	}
	CString GetKeyBoardTextFromVK(UINT nFunc, UINT nChar)
	{
		CString sText = _T("") ;

		if (nFunc == VK_SHIFT)
		{
			sText += _T("Shift+") ;
		}
		else if (nFunc == VK_MENU)
		{
			sText += _T("Alt+") ;
		}
		else if (nFunc == VK_CONTROL)
		{
			sText += _T("Ctrl+") ;
		}

		switch(nChar)
		{
		case VK_ESCAPE:
			sText += _T("Escape") ;
			break ;
		case VK_F1:
			sText += _T("F1") ;
			break ;
		case VK_F2:
			sText += _T("F2") ;
			break ;
		case VK_F3:
			sText += _T("F3") ;
			break ;
		case VK_F4:
			sText += _T("F4");
			break ;
		case VK_F5:
			sText += _T("F5") ;
			break ;
		case VK_F6:
			sText += _T("F6") ;
			break ;
		case VK_F7:
			sText += _T("F7") ;
			break ;
		case VK_F8:
			sText += _T("F8") ;
			break ;
		case VK_F9:
			sText += _T("F9") ;
			break ;
		case VK_F10:
			sText += _T("F10") ;
			break ;
		case VK_F11:
			sText += _T("F11") ;
			break ;
		case VK_F12:
			sText += _T("F12") ;
			break ;
		case VK_F13:
			sText += _T("F13") ;
			break ;
		case VK_F14:
			sText += _T("F14") ;
			break ;
		case VK_F15:
			sText += _T("F15") ;
			break ;
		case VK_F16:
			sText += _T("F16");
			break ;
		case VK_F17:
			sText += _T("F17") ;
			break ;
		case VK_F18:
			sText += _T("F18") ;
			break ;
		case VK_F19:
			sText += _T("F19") ;
			break ;
		case VK_F20:
			sText += _T("F20") ;
			break ;
		case VK_F21:
			sText += _T("F21") ;
			break ;
		case VK_F22:
			sText += _T("F22") ;
			break ;
		case VK_F23:
			sText += _T("F23") ;
			break ;
		case VK_F24:
			sText += _T("F24") ;
			break ;
		case VK_SCROLL :
			sText += _T("Scroll Lock") ;
			break ;
		case VK_PAUSE:
			sText += _T("Break") ;
			break ;
		case VK_NUMLOCK:
			sText += _T("Num lock") ;
			break ;
		case VK_DIVIDE:
			sText += _T("Key pad /") ;
			break ;
		case VK_MULTIPLY:
			sText += _T("Key pad *") ;
			break ;
		case VK_ADD :
			sText += _T("Key pad +") ;
			break ;
		case VK_SUBTRACT :
			sText += _T("Key pad -") ;
			break ;
		case VK_DECIMAL:
			sText += _T("Key pad .") ;
			break ;
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
			{
				CString sChar ;
				sChar.Format( _T("NumPad%c"), nChar-48) ;
				sText += sChar ;
			}
			break ;
		case VK_OEM_3:
			sText += _T("`") ;
			break ;
		case VK_OEM_PLUS:
			sText += _T("=") ;
			break ;
		case VK_OEM_MINUS:
			sText += _T("-") ;
			break ;
		case VK_BACK:
			sText += _T("Back") ;
			break ;
		case VK_INSERT:
			sText += _T("Insert") ;
			break ;
		case VK_HOME:
			sText += _T("Home") ;
			break ;
		case VK_PRIOR:
			sText += _T("PageUp") ;
			break ;
		case VK_NEXT:
			sText += _T("PageDown") ;
			break ;
		case VK_END:
			sText += _T("End") ;
			break ;
		case VK_DELETE:
			sText += _T("Delete") ;
			break ;
		case VK_LEFT:
			sText += _T("Left") ;
			break ;
		case VK_UP:
			sText += _T("Up") ;
			break ;
		case VK_RIGHT:
			sText += _T("Right") ;
			break ;
		case VK_DOWN:
			sText += _T("Down") ;
			break ;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				CString sChar ;
				sChar.Format( _T("%c"), nChar) ;
				sText += sChar ;
			}
			break ;
		case VK_TAB:
			sText += _T("Tab") ;
			break ;
		case VK_CAPITAL:
			sText += _T("Caps Lock") ;
			break ;
		case VK_RETURN:
			sText += _T("Enter") ;
			break ;
		case VK_SPACE:
			sText += _T("Space") ;
			break ;
		case VK_APPS:
			sText += _T("App") ;
			break ;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			{
				CString sChar ;
				sChar.Format( _T("%c"), nChar) ;
				sText += sChar ;
			}
			break ;
		case VK_OEM_2:
			sText += _T("/") ;
			break ;
		case VK_OEM_PERIOD:
			sText += _T(".") ;
			break ;
		case VK_OEM_COMMA:
			sText += _T(",") ;
			break ;
		case VK_OEM_5:
			sText += _T("\\") ;
			break ;
		case VK_OEM_7:
			sText += _T("'") ;
			break ;
		case VK_OEM_1:
			sText += _T(";") ;
			break ;
		case VK_OEM_6:
			sText += _T("]") ;
			break ;
		case VK_OEM_4:
			sText += _T("[") ;
			break ;
		default:
			sText = _T("") ;
			break ;
		}

		return sText ;
	}
	
private:
	BOOL m_bHookKeyBoard;
	HBRUSH m_hEditBrush;
	DWORD  m_dwEditBkColor;
	DWORD  m_dwEditTextColor;
 	typedef std::vector<TEXTFONT_INFO> vecTextFontInfo;
 	vecTextFontInfo m_vecFontInfo;
	CFont m_font;
	BOOL m_bMouseIn;
	BOOL m_bPlaceHolder;
	CString m_strPlaceHolder;
	BOOL m_bPlaceHolderSecMode;
	int m_nPlaceaHolderFont;
	int m_nFont;
	BOOL m_bPassWord;
//SecureEdit 
public:
	void EnableSecureMode(BOOL bEnable = TRUE)
	{
		SetPlaceHolderText(FALSE);
		if (!(GetWindowLong(GWL_STYLE) & ES_PASSWORD))
		{
			return;
		}
		_DeleteAll();
		m_bSecMode = bEnable;
		if (!m_bSecMode )
		{
			m_vecXorPad.clear();
		}
		else if (m_vecXorPad.empty())
		{
			for(int i = 0; i < SE_XORPAD_SIZE; i++) 
			{
				m_vecXorPad.push_back((TCHAR)rand());
			}
		}
	}
	// Retrieve the currently entered password
	CString  GetPassword()
	{
		int i, nNumChars;
		TCHAR tchp;
		CString strPassword;

		ATLASSERT(GetWindowTextLength() == m_vecChars.size()); // Sync failed?

		if(m_bSecMode == FALSE)
		{
			nNumChars = GetWindowTextLength();
			
			::GetWindowText(m_hWnd,strPassword.GetBuffer(nNumChars + 1),nNumChars + 1);
			strPassword.ReleaseBuffer();
			return strPassword;
		}

		nNumChars = m_vecChars.size();

		for(i = 0; i < nNumChars; i++)
		{
			// Get one character
			tchp = m_vecChars[i];
			//TCHAR ch = (TCHAR)((tchp) ^ m_vecXorPad[i % SE_XORPAD_SIZE]);
			strPassword += (TCHAR)((tchp) ^ m_vecXorPad[i % SE_XORPAD_SIZE]); // Decrypt
		}
		return strPassword;
	}


	// Set the currently entered password, may be NULL
	void SetPassword(LPCTSTR lpPassword)
	{
		SetPlaceHolderText(FALSE);
		if(m_bSecMode == FALSE)
		{
			if(lpPassword != NULL)
			{
				
				SetWindowText(lpPassword);
				SetSel(0,-1,TRUE);
			}
			else
			{
				
				SetWindowText(_T(""));
			}

			return;
		}
		_DeleteAll();

		if(lpPassword != NULL)
		{
			_InsertCharacters(0, lpPassword, _tcslen(lpPassword));
            
			CString strtext = lpPassword;
			SetWindowText(strtext);
			SetSel(0,-1,TRUE);
		}
		else 
		{
			SetWindowText(_T(""));
		}
	}

	void SetPlaceHolderText(BOOL bIn)
	{
		if (m_strPlaceHolder.IsEmpty() || m_bPlaceHolder == bIn)
		{
			return;
		}

		CString rString;
		int nLen = GetWindowTextLength();
		::GetWindowText(m_hWnd, rString.GetBufferSetLength(nLen), nLen+1);
		rString.ReleaseBuffer();
		if (bIn)
		{
			if (rString.IsEmpty())
			{
				if(m_nPlaceaHolderFont != m_nFont)
				{
					int nTemp = m_nPlaceaHolderFont;
					m_nPlaceaHolderFont = m_nFont;
					SetEditFont(nTemp);
					
				}
				if (GetWindowLong(GWL_STYLE) & ES_PASSWORD)
				{
					m_bPassWord = TRUE;
					SetPasswordChar(0);
				}
				BOOL bSecModeTemp = m_bSecMode;
				if (m_bSecMode)
				{
					m_bSecMode = FALSE;
				}
				SetWindowText(m_strPlaceHolder);
				m_bPlaceHolder = TRUE;
				m_bSecMode = bSecModeTemp;
			}
		}
		else
		{
			if (m_nPlaceaHolderFont != m_nFont)
			{
				int nTemp = m_nPlaceaHolderFont;
				m_nPlaceaHolderFont = m_nFont;
				SetEditFont(nTemp);
				
			}
			if (m_bPassWord)
			{
				SetPasswordChar(_T('*'));	
			}

			m_bPlaceHolder = FALSE;
			if (m_strPlaceHolder.CompareNoCase(rString) == 0)
			{
				BOOL bSecModeTemp = m_bSecMode;
				if (m_bSecMode)
				{
					m_bSecMode = FALSE;
				}
				SetWindowText(_T(""));
				m_bSecMode = bSecModeTemp;
			}
			
		}
	}
	void SetPlaceHolderStr(CString & strText ,int nFont = 1 ,BOOL bUpDate = TRUE)
	{
		m_strPlaceHolder = strText;
		m_nPlaceaHolderFont = nFont;
		if (bUpDate)
		{
			SetPlaceHolderText(TRUE);
		}
	}
	void SetWindowText(LPCTSTR lpszString)
	{
		SetPlaceHolderText(FALSE);
		CEdit::SetWindowText(lpszString);
	}
	void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
	{
		SetPlaceHolderText(FALSE);
		CEdit::ReplaceSel(lpszNewText,bCanUndo);	 
	}

private:
	void _DeleteAll()
	{
		m_vecChars.clear();
	}

	void _InsertCharacters(unsigned int uPos, LPCTSTR lpSource, unsigned int uNumChars)
	{
		ATLASSERT(lpSource != NULL); if(lpSource == NULL) return;
		ATLASSERT(uNumChars != 0); if(uNumChars == 0) return;

		//ATLASSERT(uPos <= (unsigned int)(m_vecChars.size() + 1));
		if(uPos > (unsigned int)(m_vecChars.size() )) return;

		_EncryptBuffer(FALSE);

//		LPTSTR lp;
		unsigned int i;
		BOOL bAppend = (uPos == (unsigned int)m_vecChars.size()) ? TRUE : FALSE;

		for(i = 0; i < uNumChars; i++)
		{
			if(bAppend == FALSE) 
			{
				m_vecChars.insert(m_vecChars.begin() + (int)(uPos + i), 1, lpSource[i]);
			}
			else 
			{
				m_vecChars.push_back(lpSource[i]);
			}
		}

		_EncryptBuffer(TRUE);
	}
	void _DeleteCharacters(unsigned int uPos, unsigned int uCount)
	{
		ATLASSERT((uPos + uCount) <= (unsigned int)m_vecChars.size());
		if((uPos + uCount) > (unsigned int)m_vecChars.size()) return;

		_EncryptBuffer(FALSE); // Unlock buffer

		m_vecChars.erase(m_vecChars.begin() + uPos,m_vecChars.begin() + uPos + uCount);

		_EncryptBuffer(TRUE); // Lock buffer again
	}
	void _EncryptBuffer(BOOL bEncrypt = TRUE)
	{
		TCHAR tchp;
		// XOR encryption is self-inverting; no separation of encryption
		// and decryption code needed
		UNREFERENCED_PARAMETER(bEncrypt);

		for(int i = 0; i < m_vecChars.size(); i++)
		{
			tchp = m_vecChars[i];
			ATLASSERT(tchp != NULL); if(tchp == NULL) continue;

			tchp ^= m_vecXorPad[i % SE_XORPAD_SIZE];
			m_vecChars.erase(m_vecChars.begin() + i);
			m_vecChars.insert(m_vecChars.begin() + i,tchp);
		}
	}
	void OnSecureEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		int nStart,nEnd,nLen;
		if (nChar == VK_BACK)
		{
			GetSel(nStart,nEnd);
			nLen = nEnd - nStart;
			if (nLen == GetWindowTextLength())
			{
				_DeleteAll();
			}
			else if (nLen != 0 )
			{
				_DeleteCharacters(nStart,nLen);	
			}
			else if(nStart != 0)
			{
				_DeleteCharacters(nStart - 1,1);
			}

		}
		else if(nChar == VK_DELETE)
		{
			GetSel(nStart,nEnd);
			nLen = nEnd - nStart;
			if (nLen == GetWindowTextLength())
			{
				_DeleteAll();
			}
			else if (nLen != 0)
			{
				_DeleteCharacters(nStart,nLen);
			}
			else 
			{
				_DeleteCharacters(nStart,1);
			}
		}
	}

	void PassWordPaste()
	{
		int nStart,nEnd,nLen;
		GetSel(nStart,nEnd);
		nLen = nEnd - nStart;
		if (nLen != 0)
		{
			_DeleteCharacters(nStart,nLen);
		}
		if (OpenClipboard())
		{
			HANDLE hclip;
			char *buf;
			hclip=GetClipboardData(CF_TEXT);
			buf=(char*)GlobalLock(hclip);
			GlobalUnlock(hclip);
			//nLen = strlen(buf);
			USES_CONVERSION;
			//nLen = wcslen(A2T(buf));
			CString str(buf);
			nLen = str.GetLength();
			int nPos = str.Find('\n');
			if (nPos != -1)
			{
				nLen = nPos;
			}
			nPos = str.Find('\r');
			if (nPos != -1)
			{
				nLen = nPos;
			}
			_InsertCharacters(nStart,A2T(buf),nLen);

			CloseClipboard();
		}
	}
	void OnEnUpdate() 
	{
		//LPTSTR lpWnd;
		if (m_bSecMode)
		{
			if( m_bUpdateText == FALSE)
			{
				m_bUpdateText = TRUE;
				return;
			}
			CString strText;
			int nLen = GetWindowTextLength();
			DWORD dwPos = GetSel() & 0xffff;
			::GetWindowText(m_hWnd,strText.GetBuffer(nLen + 1),nLen + 1);
			strText.ReleaseBuffer();
			_tcsset(strText.GetBuffer(strText.GetLength() + 1), GetPasswordChar());
			strText.ReleaseBuffer();
			m_bUpdateText = FALSE;
			SetWindowText(strText);
			SetSel((int)dwPos, (int)dwPos, FALSE);
		}
	}
	BOOL m_bSecMode;
	std::vector<TCHAR> m_vecXorPad;
	std::vector<TCHAR> m_vecChars;
	BOOL m_bUpdateText;


};

class CCoreRichEdit : public CWindowImplWithScrollBar<CCoreRichEdit,CRichEditCtrl>
	                , public CRichEditCommands<CCoreRichEdit>
{
public:
	CCoreRichEdit()
	{
		m_bMenuCmd = FALSE;
		m_bMenuUsed = FALSE;
		m_nMaxLength = 1000000000;
		m_dEditBkColor = RGB(255,255,255);
		m_dEditTextColor = RGB(0,0,1);
		m_bMouseIn = FALSE;
		m_bPlaceHolder = FALSE;
		m_nPlaceHolderFontInfo = 1;
	}
	virtual ~CCoreRichEdit()
	{
		// ClearMenuInfo();
		//m_vecMenuInfo.clear();
	}
	BEGIN_MSG_MAP_EX(CCoreRichEdit)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_SYSKEYDOWN(OnKeyDown)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CHAR(OnChar)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		COMMAND_ID_HANDLER(IDM_RICH_UNDO, OnUndo)
		COMMAND_ID_HANDLER(IDM_RICH_CUT,  OnCutString)
		COMMAND_ID_HANDLER(IDM_RICH_COPY, OnCopyString)
		COMMAND_ID_HANDLER(IDM_RICH_PASTE,  OnPasteString)
		COMMAND_ID_HANDLER(IDM_RICH_CLEAR,  OnClearAll)	
		COMMAND_ID_HANDLER(IDM_RICH_SELALL,  OnSelectAll)
		MSG_OCM_COMMAND(OnReflectedCommand)
		MSG_OCM_NOTIFY(OnReflectedNotify)

		CHAIN_MSG_MAP_ALT(CEditCommands<CCoreRichEdit>, 1)
		CHAIN_MSG_MAP(CWindowImplWithScrollBar)
	END_MSG_MAP()
	 void OnSetFocus(CWindow wndOld)
	{
		SetPlaceHolderText(FALSE);
		SetMsgHandled(FALSE);
	}
	 void OnKillFocus(CWindow wndFocus)
	 {
		SetPlaceHolderText(TRUE);
		SetMsgHandled(FALSE);
	 }
	LRESULT OnReflectedNotify(int idCtrl, LPNMHDR pnmh)
	{
		switch(pnmh->code)
		{
		case EN_LINK:
			{
				ENLINK *pEnLink = reinterpret_cast<ENLINK *>(pnmh);
				int i = 0;
				if (pEnLink->msg == WM_LBUTTONUP)
				{
					//CHARFORMAT2 cf2;
					//GetSelectionCharFormat(cf2);
					CString strURL ;
					CHARRANGE crCharRange ;
					GetSel( crCharRange ) ;
					SetSel( pEnLink->chrg ) ;
					GetSelText(strURL) ;
					SetSel( crCharRange ) ;
					::SetCursor(::LoadCursor(0, IDC_WAIT));
					::ShellExecute( m_hWnd, _T( "open" ),strURL, NULL, NULL, SW_SHOWNORMAL ) ;
					::SetCursor(::LoadCursor(0, IDC_NO));
				}
			}
		default:
			return 0;
		}
		return 0;

	}
    void OnReflectedCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		switch(uNotifyCode)
		{
		case EN_CHANGE:
			{
				int nLen = GetTextLength();
				if (nLen > m_nMaxLength)
				{
					LimitText(m_nMaxLength);
					SetSelAll();
					CString strtext;
					GetSelText(strtext);
					ReplaceSel(strtext.Left(m_nMaxLength));
					SetSel(-1,-1);
				}
			}
			break;
		
		default:
			break;
		}
		 SetMsgHandled(FALSE);
	}
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
      if (nChar == VK_TAB)
      {
		  return;
      }
	  SetMsgHandled(FALSE);
	}
	void OnMouseLeave()
	{
		if (GetSonicUI()->GetOffscreenUICallback() && m_bMouseIn)
		{
			GetSonicUI()->GetOffscreenUICallback()->osChangeCursor(32512);
			m_bMouseIn = FALSE;
		}
		SetMsgHandled(FALSE);
	}
	 void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (GetSonicUI()->GetOffscreenUICallback() && !m_bMouseIn)
		{
			GetSonicUI()->GetOffscreenUICallback()->osChangeCursor(32513);
			m_bMouseIn = TRUE;
		}
		SetMsgHandled(FALSE);
	}
	void OnDestroy()
	{
		GetSonicUI()->DestroyObject(GetSonicUI()->SkinFromHwnd(m_hWnd));
		SetMsgHandled(FALSE);	
	}
	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		ISonicSkin * pSkin = GetSonicUI()->CreateSkin();
		pSkin->Attach(m_hWnd, FALSE);
		pSkin->SetAttr(SATTR_SKIN_GAME_RENDERING, 1);
		pSkin->SetAttr(SATTR_ACCEPT_FOCUS, m_hWnd);
		SetMsgHandled(FALSE);
		return 0;
	}
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (nChar == VK_ESCAPE)
		{
			return;
		}
		SetMsgHandled(FALSE);
	}
	void InitMenu(BOOL bUseMenu = TRUE)
	{
		m_bMenuUsed = bUseMenu;
		//CCoreMenu::Initialize(m_hWnd);
	}
	void SetMenuCmd(BOOL bCmd)
	{
		m_bMenuCmd = bCmd;
	}
	void OnRButtonDown(UINT nFlags, CPoint point)
	{
		if (m_bMenuUsed)
		{
			ClientToScreen(&point);
			if (GetWindowLong(GWL_STYLE) & ES_READONLY)
			{
				MENU_INFO menuInfo;
				GetCoreMenuInfo(_T("readonly"),menuInfo);
				GetCoreUI()->CreateCoreMenu(m_hWnd,menuInfo.strID,point);
			}
			else
			{
				MENU_INFO menuInfo;
				GetCoreMenuInfo(_T("readandwrite"),menuInfo);
				GetCoreUI()->CreateCoreMenu(m_hWnd,menuInfo.strID,point);	
			}
// 			ClientToScreen(&point);
// 			GetCoreUI()->CreateMenu(m_hWnd,(_T("")),point);
		}
	}
    LRESULT OnUndo(UINT uMsg, WPARAM wParam, HWND lParam, BOOL & bHandled)
	{
		if (m_bMenuCmd && CanUndo())
		{
			Undo();	
		}
		return 0;
	}
	LRESULT OnCutString(UINT uMsg, WPARAM wParam, HWND lParam, BOOL & bHandled)
	{
		if(!m_bMenuCmd|| (ECO_READONLY&GetOptions()))
		{
			return 0;
		}
	    Cut();
		return 0;
    }
	LRESULT OnCopyString(UINT uMsg, WPARAM wParam, HWND lParam, BOOL & bHandled)
	{
		if (m_bMenuCmd)
		{
			Copy();
		}
		return 0;
	}
	LRESULT OnPasteString(UINT uMsg, WPARAM wParam, HWND lParam, BOOL & bHandled)
	{
		if(!m_bMenuCmd ||(ECO_READONLY&GetOptions()))
		{
			return 0;
		}
		Paste( );
		return 0;

	}
	LRESULT OnClearAll(UINT uMsg, WPARAM wParam, HWND lParam, BOOL & bHandled)
	{
		if(!m_bMenuCmd || (ECO_READONLY&GetOptions()))
		{
			return 0;
		}
        UINT uTextLength = GetTextLength();
		if (uTextLength > 0)
		{
			SetSelAll();
			Clear();
		}	
		return 0;
	}
	LRESULT OnSelectAll(UINT uMsg, WPARAM wParam, HWND lParam, BOOL & bHandled)
	{
		if (m_bMenuCmd)
		{
			int nVPos = ::GetScrollPos(m_hWnd,SB_VERT);
			int nHPos = ::GetScrollPos(m_hWnd,SB_HORZ);
			SetSelAll();
			SendMessage(WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,nVPos),(LPARAM)m_hWnd);
			SendMessage(WM_HSCROLL,MAKEWPARAM(SB_THUMBPOSITION,nHPos),(LPARAM)m_hWnd);
		}
		return 0;
	}
	void SetLinkMask()
	{
		DWORD Mask = GetEventMask();
		Mask = Mask|ENM_LINK|ENM_MOUSEEVENTS|ENM_SCROLLEVENTS|ENM_KEYEVENTS;
		SetEventMask(Mask); 
	  // ::SendMessage(m_hWnd, EM_AUTOURLDETECT, true, 0);
		SetAutoURLDetect(TRUE);
	}
	void SetEditTextColor(DWORD clr)
	{
		m_dEditTextColor = clr;
		SetEditDefaultCharFormat();
	}
	void SetEditBkColor(DWORD clr)
	{
		m_dEditBkColor = clr;
		SetBackgroundColor(clr);
	}
	void SetEditDefaultCharFormat()
	{
		CHARFORMAT2 chFormat;
		GetTextFontFormat(0,chFormat);
		
		SetDefaultCharFormat( chFormat );

		DWORD dwEventMask = GetEventMask();
		dwEventMask |= ENM_CHANGE;
		SetEventMask(dwEventMask);	
	}
	void AddMenuInfo(MENU_INFO  menuInfo)
	{
		vecMenuInfo::iterator it = m_vecMenuInfo.begin();
		vecMenuInfo::iterator end = m_vecMenuInfo.end();
		while(it != end)
		{
			if (menuInfo.strID.CompareNoCase(it->strID) == 0)
			{
				return;
			}
			it++;
		}
		m_vecMenuInfo.push_back(menuInfo);	
	}
	void GetCoreMenuInfo(LPCTSTR szType,MENU_INFO & info)
	{
		vecMenuInfo::iterator it = m_vecMenuInfo.begin();
		vecMenuInfo::iterator end = m_vecMenuInfo.end();
		while(it != end)
		{
			if ((*it).strType.CompareNoCase(szType) == 0)
			{
				info = (*it);
			}
			it++;
		}
	}
	void ClearMenuInfo()
	{
		vecMenuInfo::iterator it = m_vecMenuInfo.begin();
		vecMenuInfo::iterator end = m_vecMenuInfo.end();
		while(it != end)
		{
			(*it).vecItemInfo.clear();
			it++;
		}
		m_vecMenuInfo.clear();
	}
	void AddTextFontInfo(TEXTFONT_INFO & textFontInfo)
	{
		m_vecTextFontInfo.push_back(textFontInfo);
	}
	BOOL GetTextFontInfo(int nIndex,TEXTFONT_INFO & textFontInfo)
	{
		if (nIndex >= 0 && nIndex < m_vecTextFontInfo.size())
		{
			textFontInfo = m_vecTextFontInfo[nIndex];
			return TRUE;
		}
		return FALSE;
	}
	void SetMaxLength(int nMaxLength)
	{
		m_nMaxLength = nMaxLength;
	}
	void InsertBitmap(LPCTSTR lpszFilePath)
 	{
		FORMATETC fm;
		fm.cfFormat = CF_BITMAP;    // Clipboard format = CF_BITMAP
		fm.ptd = NULL;       // Target Device = Screen
		fm.dwAspect = DVASPECT_CONTENT;   // Level of detail = Full content
		fm.lindex = -1;       // Index = Not applicaple
		fm.tymed = TYMED_GDI;  

		////创建输入数据源
		IStorage *pStorage; 

		///分配内存
		LPLOCKBYTES lpLockBytes = NULL;
		SCODE sc = ::CreateILockBytesOnHGlobal(NULL, TRUE, &lpLockBytes);
// 		if (sc != S_OK)
// 		AfxThrowOleException(sc);	
		ATLASSERT(lpLockBytes != NULL);
		if (lpLockBytes == NULL)
		{
			return ;
		}
		sc = ::StgCreateDocfileOnILockBytes(lpLockBytes,
			STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_READWRITE, 0, &pStorage);
		if (sc != S_OK)
		{
		///	VERIFY(lpLockBytes->Release( ) == 0);
			lpLockBytes->Release();
			lpLockBytes = NULL;
		//	AfxThrowOleException(sc);
		}
		ATLASSERT(pStorage != NULL);

		///获取RichEdit的OLEClientSite
		LPOLECLIENTSITE lpClientSite;
		GetOleInterface()->GetClientSite( &lpClientSite );
		


		///创建OLE对象
 		IOleObject *pOleObject;

		sc = OleCreateFromFile(CLSID_NULL,lpszFilePath,IID_IOleObject,OLERENDER_FORMAT,
		&fm,lpClientSite,pStorage,(void **)&pOleObject);
		if (sc != S_OK)
		{
			return;
		}

		///插入OLE对象

		REOBJECT reobject;
		ZeroMemory(&reobject, sizeof(REOBJECT));
		reobject.cbStruct = sizeof(REOBJECT);

		CLSID clsid;
		sc = pOleObject->GetUserClassID(&clsid);
		if (sc != S_OK)
			//AfxThrowOleException(sc);

		reobject.clsid = clsid;
		reobject.cp = REO_CP_SELECTION;
		reobject.dvaspect = DVASPECT_CONTENT;
		reobject.poleobj = pOleObject;
		reobject.polesite = lpClientSite;
		reobject.pstg = pStorage;

		HRESULT hr = GetOleInterface()->InsertObject( &reobject );

		pOleObject->Release();
		lpClientSite->Release();
		pStorage->Release();
	}


///nTextFontInfo   xml font item index
void AddLine( int nTextFontInfo, LPCTSTR lpszLine)
{
	 SetPlaceHolderText(FALSE);
	CHARFORMAT2 cf2; 
	GetTextFontFormat(nTextFontInfo,cf2);

	UINT uTextLength = GetTextLength();
	if (uTextLength != 0)
	{
		this->SetSel(uTextLength ,uTextLength );
		this->ReplaceSel(_T("\r\n"));
		uTextLength = GetTextLength();
	}
	this->SetSel(uTextLength-1 ,uTextLength );
	this->SetSelectionCharFormat(cf2);
	
	this->ReplaceSel(lpszLine);	
}
void GetTextFontFormat(int nTextFontInfo,CHARFORMAT2 &cf2)
{
	ZeroMemory(&cf2, sizeof(CHARFORMAT2));
	cf2.cbSize = sizeof(CHARFORMAT2);
	cf2.dwMask = CFM_BACKCOLOR|CFM_BOLD | CFM_COLOR | CFM_FACE |
		CFM_ITALIC | CFM_SIZE | CFM_UNDERLINE;;
	cf2.crBackColor = m_dEditBkColor; 
	cf2.dwEffects = 0;
	cf2.yHeight = 14*14;
	cf2.crBackColor = m_dEditBkColor;
	cf2.crTextColor = m_dEditTextColor ; 
	_tcscpy(cf2.szFaceName ,_T("Tahoma"));

	TEXTFONT_INFO textFontInfo;
	if (GetTextFontInfo(nTextFontInfo,textFontInfo))
	{
		cf2.yHeight = textFontInfo.lHeight;
		cf2.dwEffects = textFontInfo.dwEffects;
		if (textFontInfo.dwTextColor != 0)
		{
			cf2.crTextColor = textFontInfo.dwTextColor;
		}
		if (textFontInfo.dwBackColor != 0)
		{
			cf2.crBackColor = textFontInfo.dwBackColor;
		}
		if (textFontInfo.wWeight != 0)
		{
			cf2.wWeight = textFontInfo.wWeight;
		}
		_tcscpy(cf2.szFaceName ,textFontInfo.strFaceName);
	}
}
void AddText(int nTextFontInfo,LPCTSTR lpszText)
{
	 SetPlaceHolderText(FALSE);
	CHARFORMAT2 cf2; 
	GetTextFontFormat(nTextFontInfo,cf2);
	UINT uTextLength = GetTextLength();
	this->SetSel(uTextLength ,uTextLength );
	this->SetSelectionCharFormat(cf2);
	this->ReplaceSel(lpszText);	
}
BOOL SetLineSpacing(LONG lLineSpacing,BYTE bLineSpacingRule = 3)
{
	PARAFORMAT2 pf2;
	ZeroMemory(&pf2,sizeof(PARAFORMAT2));
	pf2.cbSize = sizeof(PARAFORMAT2);
	pf2.dyLineSpacing = lLineSpacing;
	pf2.bLineSpacingRule = bLineSpacingRule;
	pf2.dwMask = PFM_LINESPACING;
	return SetParaFormat(pf2);
}
void InsertLink(CString strText, DWORD dwLinkId)
{
	 SetPlaceHolderText(FALSE);
	int nLen = strText.GetLength();
	long lBegin, lEnd;
	GetSel(lBegin,lEnd);
	ReplaceSel(strText);
	SetSel(lEnd,lEnd + nLen);

	CHARFORMAT2 cf2;
	GetSelectionCharFormat(cf2);
	cf2.dwMask = CFM_LINK | CFM_LCID;
	cf2.dwEffects = CFE_LINK | CFM_LCID;
	cf2.lcid = dwLinkId;
	cf2.yHeight ;
	cf2.dwMask |= CFM_SIZE;
	SetSelectionCharFormat(cf2);
	SetSel(lBegin+nLen, lEnd+nLen);
}
void SetDefaultEditParaFormat(PARAFORMAT2 &pf2)
{
	//m_Paraformat = pf2;
	SetParaFormat(pf2);
}
 void SetPlaceHolderText(BOOL bIn)
{
	if (m_strPlaceHolder.IsEmpty() || m_bPlaceHolder == bIn)
	{
		return;
	}

	CString rString;
	int nLen = GetWindowTextLength();
	::GetWindowText(m_hWnd, rString.GetBufferSetLength(nLen), nLen+1);
	rString.ReleaseBuffer();
	if (bIn)
	{
		if (rString.IsEmpty())
		{
			AddText(m_nPlaceHolderFontInfo,m_strPlaceHolder);
			m_bPlaceHolder = TRUE;
		}
	}
	else
	{
		m_bPlaceHolder = FALSE;
		if (m_strPlaceHolder.CompareNoCase(rString) == 0)
		{
			SetWindowText(_T(""));
		}	
	}
}
 void SetWindowText(LPCTSTR lpszString)
 {
	 CRichEditCtrl::SetWindowText(lpszString);
	 m_bPlaceHolder = FALSE;
 }
 void ReplaceSel(LPCTSTR lpszNewText, BOOL bCanUndo = FALSE)
 {
	 SetPlaceHolderText(FALSE);
	 CRichEditCtrl::ReplaceSel(lpszNewText,bCanUndo);	 
 }
 void SetPlaceHolderStr(CString & strText,int nTextFontInfo = 1,BOOL bUpDate = TRUE)
 {
	 m_strPlaceHolder = strText;
	 m_nPlaceHolderFontInfo = nTextFontInfo;
	 if (bUpDate)
	 {
		 SetPlaceHolderText(TRUE);
	 }
 }
private:
	DWORD m_dEditBkColor;
	DWORD m_dEditTextColor;
	BOOL m_bMenuCmd;
	typedef std::vector<MENU_INFO> vecMenuInfo;
	vecMenuInfo m_vecMenuInfo;
	typedef std::vector<TEXTFONT_INFO> vecTextFontInfo;
	vecTextFontInfo m_vecTextFontInfo;
	BOOL m_bMenuUsed;
	int m_nMaxLength;
	BOOL m_bMouseIn;
	BOOL m_bPlaceHolder; // tiptext in ctrl
	CString m_strPlaceHolder;
	int m_nPlaceHolderFontInfo;
	//PARAFORMAT2 m_Paraformat;
};
class CCoreDriveInfoList : public CCoreListBox
{
public:
	CCoreDriveInfoList(int nItemTopMargin = ITEM_TOP_MARGIN)
		:CCoreListBox(nItemTopMargin)
		,m_nLeftItemWidth(20)
		,m_nRightItemWidth(200)
		,m_nItemGap(5)
		,m_nTopBottomGap(10)
		,m_nPrecision(2)
		,m_nItemheight(30)
		,m_nLeftTextItem_x(2)
		,m_nTextItem_y(12)
		,m_nDriveViewWidth(100)
	{
		m_dwTextColor = RGB(255,255,255);
		m_dwUsedColor = RGB(28,27,30);
		m_dwAvaliableColor = RGB(88,88,88);
	}
	virtual ~CCoreDriveInfoList()
	{
		if(m_font.m_hFont)
		{
			m_font.DeleteObject();
		}
	}
	BEGIN_MSG_MAP_EX(CCoreDriveInfoList)
		MSG_OCM_DRAWITEM(OnDrawItem)
		CHAIN_MSG_MAP(CCoreListBox)	
		END_MSG_MAP()

		typedef struct tagDriveInfo 
	{
		//CString strDriveName;
		float     fTatal;
		float     fAvaliable;
	}DRIVE_INFO,*LPDRIV_INFO;

	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
	{

		if(lpDrawItemStruct->CtlType == ODT_SELFDRAW)
		{
			if(lpDrawItemStruct->itemID == (UINT)-1)
			{
				return;
			}
			ISonicPaint * pPaint = m_vecItemData[lpDrawItemStruct->itemID].pPaint;
			CRect rtItem = lpDrawItemStruct->rcItem;
			pPaint->Resize(rtItem.Width(), rtItem.Height() - 2 * m_nItemTopMargin);
			HilightPaintString(pPaint, FALSE);

			int x, y;
			x = rtItem.left;
			y = rtItem.top + m_nItemTopMargin;
			pPaint->Move(x, y);
			pPaint->Render(lpDrawItemStruct->hDC, m_hWnd);

			DRIVE_INFO info = m_vecDriveInfo[lpDrawItemStruct->itemID];
			CString strText = _T("");
			CString strTemp;
			CString strUnit ;

			TCHAR ch[32] = {0};
			if (info.fAvaliable >= 1)
			{
				strUnit = _T("GB");
			}
			else 
			{
				info.fAvaliable = info.fAvaliable * 1024;
				strUnit = _T("MB");
			}

			swprintf_s(ch,32,_T("%f"),info.fAvaliable);
			strTemp = ch;
			int nPos = strTemp.Find(_T('.'));
			strTemp = strTemp.Left(nPos + m_nPrecision + 1< strTemp.GetLength() ? nPos + m_nPrecision + 1 : strTemp.GetLength());
			strText = strTemp;
			strText += strUnit;
			strText += m_strAvaliableText;
			strText += _T(", ");


			if (info.fTatal >= 1)
			{
				strUnit = _T("GB");
			}
			else
			{
				info.fTatal = info.fTatal *1024;
				strUnit = _T("MB");
			}

			memset(ch,0,sizeof(TCHAR) *32);
			swprintf_s(ch,32,_T("%f"),info.fTatal);
			strTemp = ch;
			nPos = strTemp.Find(_T('.'));
			strTemp = strTemp.Left(nPos + m_nPrecision + 1 < strTemp.GetLength() ? nPos + m_nPrecision + 1: strTemp.GetLength());
			strText += strTemp;
			strText += strUnit;
			strText += m_strTatalText;
			GetSonicUI()->DrawText(lpDrawItemStruct->hDC,m_nLeftItemWidth + m_nItemGap + m_nDriveViewWidth +m_nItemGap,rtItem.top += m_nTextItem_y,strText,m_dwTextColor,m_font.m_hFont);
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
	void AddDriveInfo(int nIndex,LPCTSTR szName,float fTotal,float fAvaliable)
	{
		if (nIndex < 0 || nIndex >= m_vecDriveInfo.size())
		{
			DRIVE_INFO info;
			info.fAvaliable = fAvaliable;
			info.fTatal = fTotal;
			//info.strDriveName = szName;
			m_vecDriveInfo.push_back(info);
		}
		else
		{
			m_vecDriveInfo[nIndex].fAvaliable = fAvaliable;
			m_vecDriveInfo[nIndex].fTatal = fTotal;
			//			m_vecDriveInfo[nIndex].strDriveName = szName;
		}		
	}
	void SetAvaliableText(LPCTSTR szAvaliable)
	{
		m_strAvaliableText = szAvaliable;
	}
	void SetTatalText(LPCTSTR szTatal)
	{
		m_strTatalText = szTatal;
	}
	void SetDriveInfoTextColor(DWORD dwColor)
	{
		m_dwTextColor = dwColor;
	}
	void SetItemGap(int nItemGap)
	{
		m_nItemGap = nItemGap;
	}
	void SetTopBottomGap(int nTopBttomGap)
	{
		m_nTopBottomGap  = nTopBttomGap;
	}
	void SetLeftItemWidth(int nLeftItemWidth)
	{
		m_nLeftItemWidth = nLeftItemWidth;
	}
	void SetRightItemWidth(int nRightItemWidth)
	{
		m_nRightItemWidth = nRightItemWidth;
	}
	void SetPrecision(int nPrecision)
	{
		m_nPrecision = nPrecision;
	}
	void SetDriveTextFont(TEXTFONT_INFO & fontTextInfo)
	{
		if(m_font.m_hFont)
		{
			m_font.DeleteObject();
		}
		m_dwTextColor = fontTextInfo.dwTextColor;
		m_textFontInfo = fontTextInfo;
		m_font.CreateFont(fontTextInfo.lHeight,fontTextInfo.lWidth,fontTextInfo.lEscapement,fontTextInfo.lOrientation,fontTextInfo.wWeight
			,fontTextInfo.bItalic,fontTextInfo.bUnderline,fontTextInfo.bStrikeOut,fontTextInfo.bCharSet,fontTextInfo.bOutPrecision,fontTextInfo.bClipPrecision,fontTextInfo.bQuality,fontTextInfo.bPitchAndFamily,fontTextInfo.strFaceName);
	}
	void GetDriveInfo(int nIndex,float &fAvalialbe,float &fTatal)
	{
		if (nIndex >=0 && nIndex < m_vecDriveInfo.size())
		{
			fAvalialbe = m_vecDriveInfo[nIndex].fAvaliable;
			fTatal = m_vecDriveInfo[nIndex].fTatal;
		}
	}
	virtual void SwapItem(int i, int j)
	{
		DRIVE_INFO info = m_vecDriveInfo[i];
		m_vecDriveInfo[i] = m_vecDriveInfo[j];
		m_vecDriveInfo[j] = info;
		CCoreListBox::SwapItem(i,j);
	}
	BOOL LoadLogicalDriveInfo()
	{
		RemoveAllItems();
		DWORD dwSize = ::GetLogicalDriveStrings(0, NULL);
		CString strDrive;

		if (dwSize > 0)
		{
			GetLogicalDriveStrings(dwSize,strDrive.GetBuffer(dwSize +1));
			strDrive.ReleaseBuffer();
		}
		else
		{
			return FALSE;
		}
		LPTSTR lp = strDrive.GetBuffer(strDrive.GetLength() + 1);
		strDrive.ReleaseBuffer();

		ULARGE_INTEGER nFreeBytesAvailable;
		ULARGE_INTEGER nTotalNumberOfBytes;
		ULARGE_INTEGER nTotalNumberOfFreeBytes;

		int  i = 0;
		while(*lp != 0)
		{
			UINT res=::GetDriveType(lp);
			strDrive = lp;
			if (strDrive.CompareNoCase(_T("A:\\")) == 0 || strDrive.CompareNoCase(_T("B:\\")) == 0)
			{
				lp = _tcschr(lp,0)+1;
				continue;
			}
			if (PathFileExists(lp) && (DRIVE_FIXED  == res || DRIVE_REMOVABLE == res))
			{
				//::GetVolumeInformation();

				if(!GetDiskFreeSpaceEx(lp,&nFreeBytesAvailable,&nTotalNumberOfBytes,&nTotalNumberOfFreeBytes))
				{
					lp = _tcschr(lp,0)+1;
					continue;
				}
				double nTotal = (double)nTotalNumberOfBytes.QuadPart/(1024*1024*1024);
				double nFree = (double)nFreeBytesAvailable.QuadPart/(1024*1024*1024);
				//double nTotalNumberOfFree = (double)nTotalNumberOfFreeBytes.QuadPart/(1024*1024*1024);

				ISonicString *pText1 = GetSonicUI()->CreateString();
				CString strText = lp;
				int nPos = strText.Find('\\');
				if (nPos)
				{
					strText = strText.Left(nPos);
				}
				pText1->Format(_T("/line_width=%d//c=%x, font, font_height=%d, font_face='%s',align=1/%s"),m_nLeftItemWidth,m_dwTextColor,m_textFontInfo.lHeight,m_textFontInfo.strFaceName,strText);
				AddObject(i,pText1,m_nLeftTextItem_x,m_nTextItem_y - 1);
				AddDriveInfo(i,lp,nTotal,nFree);
				SetItemHeight(i,m_nItemheight);

				ISonicProgressBar * progressbar = GetSonicUI()->CreateProgressBar();
				progressbar ->Create(*this, 0, 0, m_nDriveViewWidth, GetCoreUI()->GetImage(m_strIamgeChannel),
					GetCoreUI()->GetImage(m_strImageActive));
				progressbar->SetPos(((nTotal - nFree) * 100)/nTotal);
				AddObject(i,progressbar,m_nLeftItemWidth + m_nItemGap,m_nTopBottomGap,TRUE);
				i++;
			}
			lp = _tcschr(lp,0)+1;
		}
		if (i < 3)
		{
			CRect rect;
			GetWindowRect(rect);
			SetWindowPos(NULL,0,0,rect.Width(),GetCount() *m_nItemheight + m_nTopBottomGap,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
		}
		return TRUE;
	}

	void SetDefaultItemHeight(int nItemHeight)
	{
		m_nItemheight = nItemHeight;
	}
	void SetLeftTextItem_x(int nPos)
	{
		m_nLeftTextItem_x = nPos;
	}
	void SetTextItem_y(int nPos)
	{
		m_nTextItem_y = nPos;
	}
	void SetAvaliableColor(DWORD dwColor)
	{
		m_dwAvaliableColor = dwColor;
	}
	void SertUsedColor(DWORD dwColor)
	{
		m_dwUsedColor = dwColor;
	}
	void SetDriveViewWidth (int nDriveViewWidth)
	{
		m_nDriveViewWidth = nDriveViewWidth;
	}
	void RemoveAllItems()
	{
		DeleteAllItems();
		m_vecDriveInfo.clear();
	}
	void SetDriveViewImage(LPCTSTR strChannel,LPCTSTR strActive)
	{
		m_strIamgeChannel = strChannel;
		m_strImageActive = strActive;
	}

private:
	std::vector<DRIVE_INFO> m_vecDriveInfo;
	int m_nLeftItemWidth;
	int m_nRightItemWidth;
	CString m_strAvaliableText;
	CString m_strTatalText;
	CString m_strIamgeChannel;
	CString m_strImageActive;
	int m_nItemGap;
	int m_nTopBottomGap;
	int m_nPrecision;
	int m_nItemheight;
	int m_nLeftTextItem_x;
	int m_nTextItem_y;
	int m_nDriveViewWidth;
	DWORD m_dwTextColor;
	DWORD m_dwAvaliableColor;
	DWORD m_dwUsedColor;
	TEXTFONT_INFO m_textFontInfo;
	CFont m_font;
};
/////////////////////////////////////////////////////////////////////////
// export some stl containers to keep crt safe
#ifdef CORE_CROSS_MODULE
EXP_STL template class COREWIDGHT_API std::vector<MENU_ITEMINFO>;
EXP_STL template class COREWIDGHT_API std::vector<MENU_INFO>;
//EXP_STL template class COREWIDGHT_API std::map<int ,TEXTFONT_INFO>;
EXP_STL template class COREWIDGHT_API std::vector<CCoreDriveInfoList::DRIVE_INFO>;
EXP_STL template class COREWIDGHT_API std::vector<TCHAR>;
EXP_STL template class COREWIDGHT_API std::vector<TEXTFONT_INFO>;
#endif
};