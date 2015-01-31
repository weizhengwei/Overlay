#include "StdAfx.h"
#include "OverlayNotifyWnd.h"
#include "data/DataPool.h"
#include "data/Utility.h"
#include "OverlayImpl.h"

COverlayNotifyWnd::COverlayNotifyWnd(void)
:m_nNotifyID(0)
{

}

COverlayNotifyWnd::~COverlayNotifyWnd(void)
{

}

int COverlayNotifyWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,(_T("overlay_notifywnd")));

	ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
	pSkin->SetSkin(_T("bg"), _T("close:%d|;btn_top_margin:1"), GetCoreUI()->GetImage(_T("BTN_EXIT")));
	SetMsgHandled(FALSE);
	m_pTitle = GetSonicUI()->CreateString();
	m_pText = GetSonicUI()->CreateString();
	ISonicPaint * pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	pPaint->AddObject(m_pTitle->GetObjectId(),3,5,TRUE);
	pPaint->AddObject(m_pText->GetObjectId(),30,35,TRUE);
	SetTimer(1000,8000);
	return 0;
}
void COverlayNotifyWnd::OnLink(ISonicBase*,LPVOID pReserve)
{
	if (!m_strLink.empty())
	{

		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(m_strLink.c_str());
		PostMessage(WM_CLOSE);
	}
	// send core_msg_track message to server
	core_msg_track msgTrack;
	msgTrack.dwClientId = m_nNotifyID;
	lstrcpyn(msgTrack.csTrack,_T("notifyclick"),256);
	COverlayImpl::GetInstance()->SendCoreMsg(&msgTrack);

}
void COverlayNotifyWnd::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(nIDEvent);
	PostMessage(WM_CLOSE);
}
void COverlayNotifyWnd:: SetText(LPCTSTR csText,LPCTSTR csTitle,LPCTSTR csLink,int nNotifyID)
{
	m_strLink = csLink;
	m_nNotifyID = nNotifyID;
	m_pTitle->Format(_T("/c=%x,font,font_height=12,single_line=2,line_width=300,font_face='Open Sans'/%s"),RGB(170,170,170),GetSonicUI()->HandleRawString(csTitle,RSCT_DOUBLE));
	if (m_strLink.empty())
	{
		m_pText->Format(_T("/a,linkc=0,c=%x,font,font_height=11,height_limit=55,line_width=290,font_face='Open Sans'/%s"),RGB(170,170,170),GetSonicUI()->HandleRawString(csText,RSCT_DOUBLE));
	}
	else
	{
		m_pText->Format(_T("/a,linkc=0,c=%x,font,linkl=1,font_height=11,height_limit=55,line_width=290,font_face='Open Sans'/%s"),RGB(170,170,170),GetSonicUI()->HandleRawString(csText,RSCT_DOUBLE));
	}

	m_pText->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&COverlayNotifyWnd::OnLink);	
}

