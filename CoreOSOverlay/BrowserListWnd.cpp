// BrowserListWnd.cpp : implementation file
//

#include "stdafx.h"

#include "BrowserListWnd.h"
#include "BrowserImpl.h"
#include "OverlayImpl.h"
#include "OverlayImpl.h"

CBrowserListWnd::CBrowserListWnd()
:m_pListBox(NULL)
,m_pSelect(NULL)
{
	
}

CBrowserListWnd::~CBrowserListWnd()
{
	
}
int CBrowserListWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("overlay_browserlist"));
	m_pListBox = GetCoreUI()->GetObjByID<CCoreListBox>(m_hWnd,_T("page_list"));
	m_pListBox->SetSelectColor(-1);
	m_pListBox->SetHotItemColor(RGB(10,10,10));
	m_pSelect = GetSonicUI()->CreateString();
	m_pSelect->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("LIST_TICK"))->GetObjectId());
	//m_pListBox->SetCoreStyle(CCoreListBox::CLBS_QUICK_SEL | m_pListBox->GetCoreStyle());
	COverlayImpl::GetInstance()->SetListBox(m_pListBox);
	SetMsgHandled(FALSE);
	

	return 0;
}

void CBrowserListWnd::OnListSelChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nSel = m_pListBox->GetCurSel() ;
	if (nSel == LB_ERR)
	{
		return ;
	}

	CBrowserImpl::GetInstance()->SetSelTab(nSel);
	ShowWindow(SW_HIDE);
}

void  CBrowserListWnd::AddListItem(int nIndex,LPCTSTR lpszItem)
{
	if (m_pListBox)
	{
		ISonicString *pText = GetSonicUI()->CreateString();
	
		pText->Format(_T("/c=0xaaaaaa, align=0x1,line_width=110, font, font_height=12, font_bold=0, font_face=Open Sans/%s"),GetSonicUI()->HandleRawString(lpszItem,RSCT_DOUBLE)) ;
		m_pListBox->AddObject(nIndex,pText,20,0,TRUE,TRUE);

		m_pListBox->SetItemHeight(nIndex,LISTITEMHEIGHT);
		m_pListBox->SetItemDataPtr(nIndex,(void*)pText);
		ISonicPaint *pPaint = m_pListBox->GetItemPaint(nIndex);
		pPaint->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&CBrowserListWnd::DrawItem);

	//	m_pListBox->InsertString(nIndex,lpszItem);
	}
}
void CBrowserListWnd::DrawItem(ISonicPaint * pPaint, LPVOID) 
{
	if (pPaint)
	{
		//m_pListBox->GetIndexByPaint(pPaint);
		int nSel = m_pListBox->GetCurSel();
 		if (nSel == m_pListBox->GetIndexByPaint(pPaint))
 		{
			//GLOG(_T("m_pListBox->GetCurSel() = %d"),nSel);
 			CURRENT_PAINT * pCurrentPaint =  pPaint->GetCurrentPaint();			
			CRect rc;
 			m_pListBox->GetClientRect(rc);
 			m_pSelect->TextOut(pCurrentPaint->hdc,pCurrentPaint->rtMem.right -25,pCurrentPaint->top + 3);
 		}
	}
}
void CBrowserListWnd::DeleteItem(int nIndex)
{
	if (m_pListBox)
	{
		m_pListBox->DeleteString(nIndex);
	}
	
}
void CBrowserListWnd::SelectItem(int nIndex)
{
	if (m_pListBox)
	{
		m_pListBox->SetCurSel(nIndex);
	}	
}
void CBrowserListWnd::SetItemText(int nIndex,LPCTSTR lpctstr)
{
	if (nIndex == -1 ||nIndex == 65535)
	{
		return;
	}
	ISonicString *pText = (ISonicString *)m_pListBox->GetItemData(nIndex);
	if (pText)
	{
		pText->Format(_T("/c=0xaaaaaa,align=0x1,line_width=110, single_line=2, font, font_height=12, font_bold=0, font_face='Open Sans', align=1/%s"),GetSonicUI()->HandleRawString(lpctstr,RSCT_DOUBLE));
	}
}
void CBrowserListWnd::OnSize(UINT nType, CSize size)
{
	if (m_pListBox)
	{
		CRect rcClient;
		GetClientRect(rcClient);
		::SetWindowPos(m_pListBox->m_hWnd,NULL,2,2,rcClient.Width() - 4,rcClient.Height() - 4,SWP_NOACTIVATE|SWP_NOZORDER);
		//m_pListBox->MoveWindow(2,2,rcClient.Width() - 4,rcClient.Height() - 4);
	}
	SetMsgHandled(FALSE);
}
void CBrowserListWnd::OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther)
 {
	 SetMsgHandled(FALSE);
 }
void CBrowserListWnd::OnKillFocus(CWindow wndFocus)
{
	SetMsgHandled(FALSE);
}
void CBrowserListWnd::OnDestroy()
{
	if (m_pSelect)
	{
		GetSonicUI()->DestroyObject(m_pSelect);
	}
	if (m_pListBox)
	{
		m_pListBox->ResetContent();
	}
	SetMsgHandled(FALSE);
}

// CBrowserListWnd message handlers
