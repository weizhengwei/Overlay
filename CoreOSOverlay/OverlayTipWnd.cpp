#include "StdAfx.h"
#include "OverlayTipWnd.h"
#include "data/DataPool.h"
#include "data/Utility.h"
#include "OverlayImpl.h"

COverlayTipWnd::COverlayTipWnd(void)
:m_pText(NULL)
{
}

COverlayTipWnd::~COverlayTipWnd(void)
{
}
int COverlayTipWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,(_T("overlay_tipwnd")));

	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	ISonicImage* pImageTran = GetCoreUI()->GetImage(_T("BG_TRAN"));
	if(pImageTran)
		pPaint->AddObject(pImageTran->GetObjectId(), 5, 5, FALSE);

	ISonicString *pClose = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("overlay_tipwnd_btn_exit")); \
		pClose->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayTipWnd::OnClickClose);
	/*ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
	pSkin->SetSkin(_T("bg"), _T("close:%d|"), GetCoreUI()->GetImage(_T("BTN_EXIT")));*/

	m_pText = GetSonicUI()->CreateString();
	//pText->Format(_T("/global, align=0x10, c=%x, height_limit= 70,line_width=240,font,font_height=13,font_face='Open Sans'/%s"),RGB(200,200,200),GetSonicUI()->HandleRawString(strTitle.c_str(),RSCT_DOUBLE));
	//ISonicPaint * pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	pPaint->AddObject(m_pText->GetObjectId(),80,25,TRUE);

	SetMsgHandled(FALSE);
	SetTimer(1000,8000);
	return 0;
}
void COverlayTipWnd::OnClickClose(ISonicBase *, LPVOID )
{
	KillTimer(1000);
	if (COverlayImpl::GetInstance()->SetTipNotifyText())
	{
		SetTimer(1000,8000);
	}
	else
	{
		PostMessage(WM_CLOSE);
	}
	
}
void COverlayTipWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (!COverlayImpl::GetInstance()->SetTipNotifyText())
	{
		KillTimer(nIDEvent);
		PostMessage(WM_CLOSE);
	}
}
void COverlayTipWnd::SetTitle(_tstring & strTitle)
{
	///ISonicString*pText = GetSonicUI()->CreateString();
	m_pText->Format(_T("/global, align=0x10, c=%x, height_limit=50,line_width=240,font,font_height=13,font_face='Open Sans'/%s"),RGB(200,200,200),GetSonicUI()->HandleRawString(strTitle.c_str(),RSCT_DOUBLE));
	//ISonicPaint * pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	//pPaint->AddObject(pText->GetObjectId(),80,25,TRUE);
}//