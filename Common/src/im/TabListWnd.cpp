// BrowserListWnd.cpp : implementation file
//

#include "stdafx.h"

#include "im\TabListWnd.h"


CTabListWnd::CTabListWnd(TabListImplBase * pTabListImpl)
:m_pTabListImpl(pTabListImpl)
,m_pListBox(NULL)
,m_nListBoxWidthMargin(1)
,m_nListBoxHeightMargin(1)
{
	m_nListItemHeight = LISTITEMHEIGHT;
	m_nItemTextWidth = 64;
	m_strFontFormat = _T("align=0x1,c=0xaaaaaa,font, font_height=13, font_bold=0, font_face=Open Sans");
	m_TextPoint.x = 12;
	m_TextPoint.y = 0;
}

CTabListWnd::~CTabListWnd()
{
	
}
int CTabListWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("tab_listwnd"));
	m_pListBox = GetCoreUI()->GetObjByID<CCoreListBox>(m_hWnd,_T("tab_list"));
	m_pListBox->SetSelectColor(ARGB(255,21,26,36));
	m_pListBox->SetHotItemColor(-1);
	m_pListBox->SetBGColor(ARGB(255,32,36,44));
	HWND hScroll = m_pListBox->GetCoreScrollWnd(SB_VERT);
	ISonicSkin* pScrollSkin = GetSonicUI()->SkinFromHwnd(hScroll);
	pScrollSkin->GetPaint()->SetBkColor(ARGB(255,32,36,44));
	m_pListBox->SetScrollImage(NULL, NULL, GetCoreUI()->GetImage(_T("SCROLLBAR_CHANNEL")), GetCoreUI()->GetImage(_T("SCROLLBAR_THUMB")));
	///COverlayImpl::GetInstance()->SetListBox(m_pListBox);
	SetMsgHandled(FALSE);
	return 0;
}

void CTabListWnd::OnListSelChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nSel = m_pListBox->GetCurSel() ;
	if (nSel == LB_ERR)
	{
		return ;
	}
	//CBrowserImpl::GetInstance()->SetSelTab(nSel);
	if (m_pTabListImpl)
	{
		m_pTabListImpl->SetSelTab(nSel,m_hWnd);
	}
	ShowWindow(SW_HIDE);
}

void  CTabListWnd::AddListItem(int nIndex,LPCTSTR lpszItem)
{
	if (nIndex == -1)
	{
		nIndex = m_pListBox->GetCount();
	}
	if (m_pListBox)
	{
		ISonicString *pText = GetSonicUI()->CreateString();
	
		pText->Format(_T("/line_width=%d,single_line=2,%s/%s"),m_nItemTextWidth,m_strFontFormat.c_str(),GetSonicUI()->HandleRawString(lpszItem,RSCT_DOUBLE)) ;
		m_pListBox->AddObject(nIndex,pText,m_TextPoint.x,m_TextPoint.y,TRUE,TRUE);

		m_pListBox->SetItemHeight(nIndex,m_nListItemHeight);
		m_pListBox->SetItemDataPtr(nIndex,(void*)pText);
	}
}

void CTabListWnd::DeleteItem(int nIndex)
{
	if (m_pListBox)
	{
		m_pListBox->DeleteString(nIndex);
	}
	
}

void CTabListWnd::Clear()
{
	if (m_pListBox)
	{
		m_pListBox->ResetContent();
	}
}

void CTabListWnd::SelectItem(int nIndex)
{
	if (m_pListBox)
	{
		m_pListBox->SetCurSel(nIndex);
	}	
}
void CTabListWnd::SetItemText(int nIndex,LPCTSTR lpctstr)
{
	if (nIndex == -1 ||nIndex == 65535)
	{
		return;
	}
	ISonicString *pText = (ISonicString *)m_pListBox->GetItemData(nIndex);
	if (pText)
	{
		pText->Format(_T("/line_width=%d,single_line=2,%s/%s"),m_nItemTextWidth,m_strFontFormat.c_str(),GetSonicUI()->HandleRawString(lpctstr,RSCT_DOUBLE));
	}
}
void CTabListWnd::OnSize(UINT nType, WTL::CSize size)
{
	if (m_pListBox)
	{
		CRect rcClient;
		GetClientRect(rcClient);
		::SetWindowPos(m_pListBox->m_hWnd,NULL,m_nListBoxWidthMargin,m_nListBoxHeightMargin,rcClient.Width() - m_nListBoxWidthMargin * 2,rcClient.Height() - m_nListBoxWidthMargin * 2,SWP_NOACTIVATE|SWP_NOZORDER);
	}
	SetMsgHandled(FALSE);
}
void CTabListWnd::OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther)
{
	SetMsgHandled(FALSE);
}
void CTabListWnd::OnKillFocus(CWindow wndFocus)
{
	if (!wndFocus.m_hWnd || (::GetParent(wndFocus.m_hWnd ) != m_hWnd && ::GetParent(::GetParent(wndFocus.m_hWnd )) != m_hWnd))
	{
		ShowWindow(SW_HIDE);
	}
	SetMsgHandled(FALSE);
}
void CTabListWnd::OnDestroy()
{
	if (m_pListBox)
	{
		m_pListBox->ResetContent();
	}
	SetMsgHandled(FALSE);
}
void CTabListWnd::OnSetFocus(CWindow wndOld)
{
	SetMsgHandled(FALSE);
}
void CTabListWnd::OnListKillFocus(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (!wndCtl.m_hWnd || m_hWnd != wndCtl.m_hWnd)
	{
		ShowWindow(SW_HIDE);
	}
	SetMsgHandled(FALSE);
}

// CTabListWnd message handlers
