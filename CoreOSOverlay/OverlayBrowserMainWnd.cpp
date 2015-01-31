
// OverlayBrowserMainWnd.cpp : implementation file
//

#include "stdafx.h"

#include "OverlayBrowserMainWnd.h"
#include "BrowserImpl.h"
#include "OverlayImpl.h"


COverlayBrowserMainWnd::COverlayBrowserMainWnd()
:m_pTab(NULL)
,m_pLoading(NULL)
,m_pPageDown(NULL)
,m_pPageUP(NULL)
,m_BrowserListWnd(this)
{

}

COverlayBrowserMainWnd::~COverlayBrowserMainWnd()
{
	if (m_pListbar)
	{
		SONICUI_DESTROY(m_pListbar);
	}
	if (m_pBtnAdd)
	{
		SONICUI_DESTROY(m_pBtnAdd);
	}
	if (m_pPageDown)
	{
		SONICUI_DESTROY(m_pPageDown);
	}
	if (m_pPageUP)
	{
		SONICUI_DESTROY(m_pPageUP)
	}
}

int COverlayBrowserMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("overlay_browsermainwnd"));

	CBrowserImpl::GetInstance()->SetBrowserMainWnd(m_hWnd);

	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	ISonicPaint *pPaintMainBg = GetSonicUI()->CreatePaint();
	CRect rc;
	::GetClientRect(m_hWnd,&rc);
	pPaintMainBg->Create(FALSE);
	pPaintMainBg->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&COverlayBrowserMainWnd::DrawMainBg);
	pPaint->AddObject(pPaintMainBg->GetObjectId(),0,0,TRUE);

	m_pTab = GetCoreUI()->GetObjByID<ISonicTab>(m_hWnd, _T("overlay_browser_tabs"));
	m_pTab->SetAttr(SATTR_TAB_SIMPLE_BORDER, 1);
	m_pTab->SetAttr(SATTR_TAB_BORDER_COLOR, (LPVOID)(RGB(58, 62, 73)));
	CBrowserImpl::GetInstance()->SetTab(m_pTab);
	m_pTab->SetTabPanelHorMargin(22, 65);
	m_pTab->SetAttr(SATTR_STRING_LEFT_PADDING,6);
	m_pTab->SetAttr(SATTR_STRING_RIGHT_PADDING,22);
	m_pTab->SetAttr(SATTR_STRING_BTNTEXT_ALIGN,1);

	m_pListbar = GetSonicUI()->CreateString();
	m_pBtnAdd = GetSonicUI()->CreateString();
	m_pLoading = GetSonicUI()->CreateString();
	m_pPageUP = GetSonicUI()->CreateString();
	m_pPageDown = GetSonicUI()->CreateString();

	m_pLoading->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("ANI_WEB_LOADING"))->GetObjectId());
	m_pListbar->Format(_T("/a,linkc=0, p4=%d/"), GetCoreUI()->GetImage(_T("TAB_LIST"))->GetObjectId());
	m_pBtnAdd->Format(_T("/a,linkc=0, p4=%d/"), GetCoreUI()->GetImage(_T("TAB_ADD"))->GetObjectId());
	m_pPageUP->Format(_T("/a,linkc=0, p4=%d/"), GetCoreUI()->GetImage(_T("BTN_PAGE_UP"))->GetObjectId());
	m_pPageDown->Format(_T("/a,linkc=0, p4=%d/"), GetCoreUI()->GetImage(_T("BTN_PAGE_DOWN"))->GetObjectId());
	
	
	ISonicPaint * pTabPaint = m_pTab->GetPaint();
	m_pBtnAdd->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&COverlayBrowserMainWnd::OnAddBrwoser);
	m_pListbar->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&COverlayBrowserMainWnd::OnShowList);
	m_pPageDown->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&COverlayBrowserMainWnd::OnPageDown);
	m_pPageUP->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&COverlayBrowserMainWnd::OnPageUp);
	pTabPaint->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&COverlayBrowserMainWnd::DrawTab);


	m_pTab->Delegate(DELEGATE_EVENT_SEL_CHANGED,NULL,this,&COverlayBrowserMainWnd::OnTabChanged);
	m_pTab->Delegate(DELEGATE_EVENT_TAB_DELETING,NULL,this,&COverlayBrowserMainWnd::DeleteTab);
	m_pTab->SetCloseButton(GetCoreUI()->GetImage(_T("TAB_CLOSE")),12,10);
	m_BrowserListWnd.Create(m_hWnd,CRect(0,0,0,0),NULL,WS_POPUP);
	m_BrowserListWnd.SetListBoxMargin(11,12);
	COverlayImpl::GetInstance()->SetListBox(m_BrowserListWnd.GetCoreListBox());

	ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
	pSkin->SetSkin(_T("bg"), _T("close:%d|"), GetCoreUI()->GetImage(_T("BTN_EXIT")));
	pSkin->SetSkin(_T("bg"), _T("max:%d|"), GetCoreUI()->GetImage(_T("BTN_MAX")));
	pSkin->SetSkin(_T("bg"), _T("restore:%d|"), GetCoreUI()->GetImage(_T("BTN_RESTORE")));

	if (COverlayImpl::GetInstance()->GetOverlaySize().cx < 1024)
	{
		SetWindowPos(NULL, 0, 0, COverlayImpl::GetInstance()->GetOverlaySize().cx, COverlayImpl::GetInstance()->GetOverlaySize().cy, 
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos(NULL, 0, 0, 800, 600, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	SetMsgHandled(FALSE);
	return 0;
}
void COverlayBrowserMainWnd::DeleteTab(ISonicBase *,LPVOID pReserve)
{
	CBrowserImpl::GetInstance()->DelBrowserWnd(int(pReserve));
	if (m_pTab && m_pTab->GetTabCount() == 1)
	{
		PostMessage(WM_CLOSE);
	}
}
void COverlayBrowserMainWnd::OnPageUp(ISonicBase *, LPVOID)
{
	if (m_pTab)
	{
		int nFirstVisibleIndex = m_pTab->GetFirstVisibleIndex();
		int nSelIndex = m_pTab->GetSel();
		if (nSelIndex < 0 || nSelIndex >= m_pTab->GetTabCount())
		{
			nSelIndex = nFirstVisibleIndex;
		}
		if (nSelIndex == nFirstVisibleIndex)
		{
			int nIndex = nFirstVisibleIndex - 1;
			if (nIndex >= 0)
			{
				m_pTab->SetTabVisible(nIndex,2);

				m_pTab->SetSel(nIndex);
			}

		}
		else
		{
			m_pTab->SetSel(nSelIndex -1);
		}
		
	
	}
}
void COverlayBrowserMainWnd::OnPageDown(ISonicBase* ,LPVOID)
{
	if (!m_pTab)
	{
		return;
	}
	int nFirstVisibleIndex = m_pTab->GetFirstVisibleIndex();
	int nVisibaleTatol = m_pTab->GetVisibleTabCount();
	int nIndex = nVisibaleTatol + nFirstVisibleIndex;
	int nSelIndex = m_pTab->GetSel();
	if (nSelIndex < 0 || nSelIndex >= m_pTab->GetTabCount())
	{
		nSelIndex = nIndex;
	}
	if (nSelIndex ==  nIndex - 1)
	{
		if (nIndex <= m_pTab->GetTabCount() -1)
		{
			m_pTab->SetTabVisible(nIndex,2);
			m_pTab->SetSel(nIndex);
		}
	}
	else
	{
		m_pTab->SetSel(nSelIndex + 1);
	}

}
void COverlayBrowserMainWnd::OnClose()
{
	CRect rcClient;
	GetWindowRect(&rcClient);
	CRect *rc = CBrowserImpl::GetInstance()->GetMainBrowserWndRect();
	*rc = rcClient;
  SetMsgHandled(FALSE);
}
void COverlayBrowserMainWnd::OnDestroy()
{
	CBrowserImpl::GetInstance()->ClearAllBrowser();
	m_pTab = NULL;
	SetMsgHandled(FALSE);
}
void COverlayBrowserMainWnd::OnTabChanged(ISonicBase *,LPVOID pReserve)
{
	if (m_pTab)
	{
		int nTabIndex = m_pTab->GetSel();
		CBrowserImpl::GetInstance()->SetCurrentBrowser(nTabIndex,TRUE);
		//m_BrowserListWnd.SelectItem(nTabIndex);
	}	
}
LRESULT COverlayBrowserMainWnd::OnCoreBrowserMsg(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled)
{
		
	LPBROWSERMSGDATA pMsgData = (LPBROWSERMSGDATA) lParam;
	if (!pMsgData)
	{
		return 0;
	}
	switch(pMsgData->msgid)
	{
	 case BROWSER_HANDLER_TITLECHANGE:
		{
			m_BrowserListWnd.SetItemText(CBrowserImpl::GetInstance()->GetBrowserWndIndex(pMsgData->pHandler),pMsgData->szTitle);
		}
		break;
	 case BROWSER_HANDLER_LOAD:
		 {
			OnLoading(CBrowserImpl::GetInstance()->GetBrowserWndIndex(pMsgData->pHandler),pMsgData->loading);
		 }
		 return 0;
	 
	 default:
		 break;;
	}	
	//OutputDebugStringA("OnCoreBrowserMsg\n");
	return CBrowserImpl::GetInstance()->OnBrowserMsg(wParam,lParam);
}
LRESULT  COverlayBrowserMainWnd::OnCoreBrowserEvent(UINT msgId,WPARAM wParam,LPARAM lParam,BOOL bHandled)
{
	if (LOWORD(wParam) == 0)
	{
		m_BrowserListWnd.DeleteItem(HIWORD(wParam));
	}
	else if(LOWORD(wParam) == 1)
	{
		m_BrowserListWnd.AddListItem(HIWORD(wParam),(LPCTSTR)lParam);
	}
// 	else if (LOWORD(wParam) == 2)
// 	{
// 		m_BrowserListWnd.SetItemText(HIWORD(wParam),(LPCTSTR)lParam);
// 	}
	return 0;
}
void COverlayBrowserMainWnd::ShowBrowserListWnd(BOOL bShow)
{
	if (bShow)
	{
		//m_BrowserListWnd.GetCoreListBox()->ResetContent();
		int nCount = m_pTab->GetTabCount();
// 		for (int i = 0 ; i < nCount ;i++)
// 		{
// 			m_BrowserListWnd.AddListItem(i,m_pTab->GetTabText(i));
// 		}

		m_BrowserListWnd.SelectItem(m_pTab->GetSel());
		
		CRect rc (*(m_pListbar->GetRect()));
		ClientToScreen(rc);
		int nHeight = nCount * LISTITEMHEIGHT ;
		if (nHeight > 252)
		{
			nHeight = 252;
		}
		int nWidthMargin,nHeightMargin;
		m_BrowserListWnd.GetListBoxMargin(nWidthMargin,nHeightMargin);
		::SetWindowPos(m_BrowserListWnd.m_hWnd,NULL,rc.left - 80,rc.bottom,LISTWIDTH + nWidthMargin * 2,nHeight + nHeightMargin * 2,SWP_NOACTIVATE|SWP_NOZORDER);
 	/*	HRGN hg = CreateRoundRectRgn(3,3,LISTWIDTH - 5,nHeight - 5,10,10);
 		m_BrowserListWnd.SetWindowRgn(hg);
 		DeleteObject(hg);*/
		m_BrowserListWnd.ShowWindow(SW_SHOWNA);
		m_BrowserListWnd.SetListFocus();

	}
	else
	{
		::ShowWindow(m_BrowserListWnd.m_hWnd,SW_HIDE);
	}
}

void COverlayBrowserMainWnd::OnSize(UINT nType, CSize size)
{
	if (m_pTab)
	{
		CRect rcClent;
		GetClientRect(&rcClent);
		m_pTab->Move(6, 36, rcClent.Width() - 12, rcClent.Height() - 42,true);
		m_pTab->SetTabVisible(m_pTab->GetSel());
	}
}
void COverlayBrowserMainWnd::OnAddBrwoser(ISonicBase *,LPVOID)
{
	CBrowserImpl::GetInstance()->CreateBrowser(COverlayImpl::GetInstance()->GetMsgCookie()->csNewsPage,-1);
}

void COverlayBrowserMainWnd::OnShowList(ISonicBase *,LPVOID)
{

	if (m_BrowserListWnd.IsWindowVisible())
	{
		ShowBrowserListWnd(FALSE);
	}
	else
	{
		ShowBrowserListWnd(TRUE);
	}
	
}
void COverlayBrowserMainWnd::OnLoading(int nIndex,BOOL bLoading)
{
	if (nIndex == -1 || nIndex >= m_pTab->GetTabCount())
	{
		return;
	}
	ISonicAnimation *pItem = m_pTab->GetTabItem(nIndex);
	if (bLoading)
	{
		pItem->SetAttr(_T("loading"),1);
		CBrowserImpl::GetInstance()->SetTabText(nIndex);
	}
	else
	{
 		pItem->RemoveAttr(_T("loading"));
 		_tstring strText;
		CBrowserImpl::GetInstance()->GetBrowserTitle(nIndex,strText);
		_tstring strTextFormat;
		strTextFormat.Format(4,_T("/c=%x%s,align=1,single_line=2,linkh=%x/%s"),RGB(170,170,170),TABTILE_FONT,RGB(243,243,243),GetSonicUI()->HandleRawString(strText.c_str(),RSCT_DOUBLE));
		m_pTab->SetTabText(nIndex,strTextFormat.c_str());
	}
	
}
void COverlayBrowserMainWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 260;
	lpMMI->ptMaxTrackSize.x = COverlayImpl::GetInstance()->GetOverlaySize().cx;
	lpMMI->ptMaxTrackSize.y = COverlayImpl::GetInstance()->GetOverlaySize().cy;
//	SetMsgHandled(FALSE);
}

void COverlayBrowserMainWnd::DrawTab(ISonicPaint * pPaint, LPVOID) 
{
	if (pPaint)
	{
		CURRENT_PAINT * pCurrentPaint =  pPaint->GetCurrentPaint();
		int nWidth = m_pTab->GetVisibleTabsTotalWidth();
		static const int nLeftMargin = 22;
		static const int nTop = pCurrentPaint->top + 5;
		if (m_pPageDown)
		{
			m_pPageUP->TextOut(pCurrentPaint->hdc,pCurrentPaint->left - 1 ,nTop,m_hWnd);
		}
		if (m_pPageUP)
		{
			m_pPageDown->TextOut(pCurrentPaint->hdc,pCurrentPaint->left + nWidth +nLeftMargin + 2,nTop,m_hWnd);
		}
		if ( m_pTab->GetTabCount() > m_pTab->GetVisibleTabCount())
		{
			if (!m_pListbar->IsVisible())
			{
				m_pListbar->Show(TRUE,FALSE);
			}
			m_pBtnAdd->TextOut(pCurrentPaint->hdc,pCurrentPaint->left + nWidth + nLeftMargin + 21,nTop,m_hWnd);
			m_pListbar->TextOut(pCurrentPaint->hdc,pCurrentPaint->left + nWidth + nLeftMargin +40,nTop,m_hWnd);
			
		}
		else
		{
			if (m_pListbar->IsVisible())
			{
				m_pListbar->Show(FALSE,FALSE);
			}
			m_pBtnAdd->TextOut(pCurrentPaint->hdc,pCurrentPaint->left + nWidth +nLeftMargin +21 ,nTop,m_hWnd);
		}
	}
}
void COverlayBrowserMainWnd::SetSelTab (int nIndex,HWND hWnd)
{
	CBrowserImpl::GetInstance()->SetSelTab(nIndex);
}
void COverlayBrowserMainWnd::DrawMainBg(ISonicPaint * pPaint, LPVOID)
{
	if (!pPaint)
	{
		return;
	}
	HDC hdc = pPaint->GetCurrentPaint()->hdc;

	CRect rcClient;
	GetClientRect(&rcClient);
	ISonicImage *pIamge = GetCoreUI()->GetImage(_T("BG_BROWSER_WEB"));
	DRAW_PARAM dp;
	memset(&dp, 0, sizeof(dp));
	dp.dwMask = DP_TILE;
	dp.cx = rcClient.Width();
	dp.cy = rcClient.Height() - 110;
	if (pIamge)
	{
		pIamge->Draw(hdc,0, 110, &dp);
	}

	CRect rc;
	//rc.left = 0;
	//rc.top = 0;
	//rc.right = 1;
	//rc.bottom = 44;
	//GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,34,36,41));

	//rc.left = rcClient.right -1;
	//rc.top = 0;
	//rc.right = rcClient.right;
	//rc.bottom = 44;
	//GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,34,36,41));

	rc.left = 6;
	rc.top = rcClient.bottom - 6;
	rc.right = rcClient.right -6;
	rc.bottom = rcClient.bottom - 5;
	GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,34,36,41));

	rc.left = 1;
	rc.top = 67 ;
	rc.right = rcClient.right -1;
	rc.bottom = 68;
	GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,58, 62, 73));


	/*rc.left = 6;
	rc.top = BROWSER_OFFSET_TOP -1;
	rc.right = rcClient.right - 6;
	rc.bottom = BROWSER_OFFSET_TOP;
	GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,34,36,41));*/
};
