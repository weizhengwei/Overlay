// OverlayWnd.cpp : implementation file
//

#include "stdafx.h"

#include "OverlaySettingWnd.h"
#include "data/DataPool.h"
#include "data/Utility.h"
#include "OverlayImpl.h"


COverlaySettingWnd::COverlaySettingWnd()
:m_pIngameEditKey(NULL)
{

}

COverlaySettingWnd::~COverlaySettingWnd()
{
}

int COverlaySettingWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("overlay_settingwnd"));

	InitEvent();
	SetMsgHandled(FALSE);

	return 0;
}
void COverlaySettingWnd::InitEvent()
{
	core_msg_cookie *pMsgCookie = COverlayImpl::GetInstance()->GetMsgCookie();

	m_pIngameEditKey = GetCoreUI()->GetObjByID<CCoreEdit>(m_hWnd,_T("overlay_setting_shortcut"));
	int nFunction = GetPrivateProfileInt(INI_CFG_CLIENT_INGAME, INI_CFG_CLIENT_INGAME_FUNCTION, 16, theDataPool.GetUserProfilePath().c_str());
	int nShortcut = GetPrivateProfileInt(INI_CFG_CLIENT_INGAME ,INI_CFG_CLIENT_INGAME_SHORTCUT, 9, theDataPool.GetUserProfilePath().c_str());
	_tstring szKey = CUtility::GetKeyBoardTextFromVK(nFunction, nShortcut);
	m_pIngameEditKey->SetWindowText(szKey.c_str());
	m_pImgTran = GetCoreUI()->GetImage(_T("BG_TRAN"));
	//m_pText1 = GetSonicUI()->CreateString();
	//m_pText1->Format(_T("/c=%x, font, font_height=14,height_limit= 100,line_width=420,font_face='Open Sans'/%s"),RGB(167,167,167),pMsgCookie->csText1);
	//m_pText2 = GetSonicUI()->CreateString();
	//m_pText2->Format(_T("/c=%x, font, font_height=14,height_limit= 100,line_width=420,font_face='Open Sans'/%s"),RGB(167,167,167),pMsgCookie->csText2);
	ISonicPaint * pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();

    pPaint->AddObject(m_pImgTran->GetObjectId(), 5, 5, FALSE);

	ISonicString *pTitleText = GetSonicUI()->CreateString();
	pTitleText->Format(_T("/c=%x, font, font_bold=0,font_height=14,font_face='Open Sans'/%s %s"),RGB(255,255,255),pMsgCookie->csInGameTitle,pMsgCookie->csDialogTitle);
	//pPaint->AddObject(m_pText1->GetObjectId(),35,60,TRUE);
	//pPaint->AddObject(m_pText2->GetObjectId(),35,125,TRUE);
	pPaint->AddObject(pTitleText->GetObjectId(),(pPaint->GetWidth() - pTitleText->GetWidth())/2,9,TRUE);
	ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
	pSkin->SetSkin(_T("bg"), _T("close:%d|;btn_top_margin:11;btn_right_margin:15"), GetCoreUI()->GetImage(_T("BTN_CLOSE")));

	CRect rcClient;
	GetWindowRect(&rcClient);

	ISonicString *pOkBtn = GetSonicUI()->CreateString();
	_tstring strText;
	strText.Format(1,_T("/c=0xa7a7a7, font,font_bold=0, font_height=14,font_face='Open Sans', single_line=2/%s"),pMsgCookie->csOk);
	pOkBtn->Format(_T("/a,p4=%d,linkc=0,btn_text='%s',btn_width=90, btn_height=30/"),GetCoreUI()->GetImage(_T("BTN_POPUP"))->GetObjectId(),strText.c_str());
	strText.Format(1,_T("/c=0xa7a7a7, font,font_bold=0, font_height=14,font_face='Open Sans', single_line=2/%s"),pMsgCookie->csCancel);
	ISonicString *pCancel = GetSonicUI()->CreateString();
	pCancel->Format(_T("/a,p4=%d,linkc=0,btn_text='%s',btn_width=90, btn_height=30/"),GetCoreUI()->GetImage(_T("BTN_POPUP"))->GetObjectId(),strText.c_str());
	pPaint->AddObject(pCancel->GetObjectId(),rcClient.Width() - 225,rcClient.Height() - 60,TRUE);
	pPaint->AddObject(pOkBtn->GetObjectId(),rcClient.Width() - 125,rcClient.Height() - 60,TRUE);
	pCancel->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlaySettingWnd::OnCancel) ;
	pOkBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlaySettingWnd::OnSave) ;
}
void COverlaySettingWnd::OnCloseClick(ISonicBase *, LPVOID pReserve)
{
	PostMessage(WM_CLOSE);
}
void COverlaySettingWnd::OnCancel(ISonicBase*,LPVOID pReserve)
{
	PostMessage(WM_CLOSE);
}
void COverlaySettingWnd::OnSave(ISonicBase*,LPVOID pReserve)
{
	SaveKey();
	PostMessage(WM_CLOSE);
}
void COverlaySettingWnd::SaveKey()
{

 	if(!m_pIngameEditKey)	return;
 
 	CString szShortcut ;
	int nLen = ::GetWindowTextLength(m_pIngameEditKey->m_hWnd);
	::GetWindowText(m_pIngameEditKey->m_hWnd,szShortcut.GetBuffer(nLen + 1),nLen + 1);

 	UINT nFunc, nChar;
 	CUtility::GetVKFromKeyBoardText(szShortcut.GetBuffer(szShortcut.GetLength()), nFunc, nChar);
	szShortcut.ReleaseBuffer();
 	_tstring sFunc, sChar;
 	sFunc.Format(1, _T("%d"), nFunc);
 	sChar.Format(1, _T("%d"), nChar);
 	WritePrivateProfileString(INI_CFG_CLIENT_INGAME, INI_CFG_CLIENT_INGAME_FUNCTION, sFunc.c_str(), theDataPool.GetUserProfilePath().c_str());
 	WritePrivateProfileString(INI_CFG_CLIENT_INGAME, INI_CFG_CLIENT_INGAME_SHORTCUT, sChar.c_str(), theDataPool.GetUserProfilePath().c_str());
	COverlayImpl::GetInstance()->SetBackBtnText();
}

void COverlaySettingWnd::OnPaint(CDC hDc)
{
	CRect rect;
	GetClientRect(&rect);
	CPaintDC dc(m_hWnd);
	DRAW_PARAM dp = {0};
	dp.dwMask = DP_TILE;
	dp.cx = 420;
	ISonicImage *pDivideLine = GetCoreUI()->GetImage(_T("POPUP_DIVIDE_LINE"));
	if (pDivideLine)
		pDivideLine->Draw(dc.m_hDC, 30, rect.Height() - 70, &dp);
	dp.cx = 478;
	pDivideLine = GetCoreUI()->GetImage(_T("POPUP_DIVIDE_LINE1"));
	if (pDivideLine)
		pDivideLine->Draw(dc.m_hDC, 6, 35, &dp);
}