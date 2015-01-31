#include "StdAfx.h"
#include "UIMainWndframe.h"
#include "OverlayImpl.h"
#include <atltime.h>
#include "data/DataPool.h"
#include "data/Utility.h"
#include "OverlayImImpl.h"
CGameToBackTip::CGameToBackTip()
{

}
CGameToBackTip::~CGameToBackTip()
{

}
int CGameToBackTip::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,(_T("overlay_gaetobacktipwnd")));
	 GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("tip_text"));
	return 0;
}
void CGameToBackTip::SetStrText(_tstring &strText1,_tstring &strTex2)
{
	ISonicString * pText1 = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("tip_text1"));
	if (pText1)
	{
		_tstring str;
		str.Format(2,_T("%s %s"),strText1.c_str(),strTex2.c_str());
		pText1->Format(_T("/c=%x, font, font_height=14,line_width=220,height_limit=40, font_face='Open Sans'/%s"),RGB(223,223,223),GetSonicUI()->HandleRawString(str.c_str(),RSCT_DOUBLE));
	}
}

CUIMainWndframe::CUIMainWndframe(void)
:m_pSystemTime(NULL)
,m_pSystemDate(NULL)
,m_pStatusStr(NULL)
,m_pAvatar(NULL)
,m_pBackToGame(NULL)
,m_pNickName(NULL)
,m_pBgBottom(NULL)
,m_pGameNameTitle(NULL)
,m_pGameName(NULL)
,m_pNewsBtn(NULL)
,m_pWeb(NULL)
,m_pCommunityBtn(NULL)
,m_pMedia(NULL)
,m_pChargeBtn(NULL)
,m_pSupportBtn(NULL)
,m_pPaint(NULL)
,m_szBackToGameTip("")
,m_pBgIcon(NULL)
,m_pBgIcon_big(NULL)
{
}

CUIMainWndframe::~CUIMainWndframe(void)
{
	SONICUI_DESTROY(m_pSystemDate);
	SONICUI_DESTROY(m_pSystemTime);
	SONICUI_DESTROY(m_pAvatar);

}

void CUIMainWndframe::DrawLeftMainBg(ISonicPaint * m_pPaint, LPVOID)
{
	if (!m_pPaint)
	{
		return;
	}
	HDC hdc = m_pPaint->GetCurrentPaint()->hdc;

	CRect rc;
	::GetClientRect(m_hWnd,&rc);

	GetSonicUI()->FillSolidRect(hdc,CRect(1,1,MAINLEFTBGWIDTH, rc.bottom-1),ARGB(255,26,26,31));
	GetSonicUI()->FillSolidRect(hdc,CRect(1,90,480,91),ARGB(255,0,0,0));
	GetSonicUI()->FillSolidRect(hdc,CRect(1,91,480,92),ARGB(255,40,41,45));
	
}

void CUIMainWndframe::Init(HWND hwnd)
{
	m_hWnd = hwnd;
	InitMainUI();
}



void CUIMainWndframe::InitLowResolutionUI()
{
    // used for the situation when resolution is less than 1024 [12/10/2014 liuyu]
    m_pNewsBtn = GetSonicUI()->CreateString();
    m_pWeb = GetSonicUI()->CreateString();
    m_pCommunityBtn = GetSonicUI()->CreateString();
    m_pMedia = GetSonicUI()->CreateString();
    m_pChargeBtn = GetSonicUI()->CreateString();
    m_pSupportBtn = GetSonicUI()->CreateString();


    m_pNewsBtn   ->Format(_T("/a='%s', c=%x, linkh=0x5f5f5f, linkc=0,font, font_height=24, font_face='Open Sans'/%s"),m_pMsgCookie->csNewsPage,RGB(247,247,247),m_pMsgCookie->csNews);
    m_pWeb       ->Format(_T("/a='%s', c=%x, linkh=0x5f5f5f, linkc=0,font, font_height=24, font_face='Open Sans'/%s"),m_pMsgCookie->csBillingPage,RGB(247,247,247),m_pMsgCookie->csWeb);
    m_pCommunityBtn  ->Format(_T("/a='%s', c=%x, linkh=0x5f5f5f, linkc=0,font, font_height=24, font_face='Open Sans'/%s"),m_pMsgCookie->csForumPage,RGB(247,247,247),m_pMsgCookie->csForum);
    m_pMedia     ->Format(_T("/a='%s', c=%x, linkh=0x5f5f5f, linkc=0,font, font_height=24, font_face='Open Sans'/%s"),m_pMsgCookie->csForumPage,RGB(247,247,247),m_pMsgCookie->csMedia);
    m_pChargeBtn->Format(_T("/a='%s', c=%x, linkh=0x5f5f5f, linkc=0,font, font_height=24, font_face='Open Sans'/%s"),m_pMsgCookie->csBillingPage,RGB(247,247,247),m_pMsgCookie->csBilling);
    m_pSupportBtn ->Format(_T("/a='%s', c=%x, linkh=0x5f5f5f, linkc=0,font, font_height=24, font_face='Open Sans'/%s"),m_pMsgCookie->csSupportPage,RGB(247,247,247),m_pMsgCookie->csSupport);

    m_pNewsBtn ->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnNewsClick) ;
    m_pWeb->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnWebClick) ;
    m_pCommunityBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnCommunityClick) ;
    m_pMedia->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnMediaClick) ;
    m_pChargeBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnBillingClick) ;
    m_pSupportBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnSupportClick) ;


    m_pPaint->AddObject(m_pNewsBtn->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pWeb->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pCommunityBtn->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pMedia->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pChargeBtn->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pSupportBtn->GetObjectId(),0,0,TRUE);

    m_pBgIcon = GetSonicUI()->CreateString();    
    m_pBgIcon->Format(_T("/a,p=%d/"),GetCoreUI()->GetImage(_T("LOGO_OVERLAYARC"))->GetObjectId());
    m_pBgIcon->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnArcIcon);
    m_pPaint->AddObject(m_pBgIcon->GetObjectId(),0,0,TRUE);

    m_LowResolutionUI.push_back(m_pNewsBtn);
    m_LowResolutionUI.push_back(m_pWeb);
    m_LowResolutionUI.push_back(m_pCommunityBtn);
    m_LowResolutionUI.push_back(m_pMedia);
    m_LowResolutionUI.push_back(m_pChargeBtn);
    m_LowResolutionUI.push_back(m_pSupportBtn);
    m_LowResolutionUI.push_back(m_pBgIcon);
}




void CUIMainWndframe::InitHighResolutionUI()
{
    // used for the situation when resolution is more than 1024 [12/10/2014 liuyu]
    CRect rc;
    ::GetClientRect(m_hWnd,&rc);
    m_pPaintMainBg = GetSonicUI()->CreatePaint();
    m_pPaintMainBg->Create(FALSE,MAINLEFTBGWIDTH,rc.Height());
    m_pPaintMainBg->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&CUIMainWndframe::DrawLeftMainBg);
    m_pPaint->AddObject(m_pPaintMainBg->GetObjectId(),0,0,TRUE);

    m_pStatusStr = GetSonicUI()->CreateString();
    m_pStatusStr->Format(_T("/a,c=%x,font, font_height=12, font_face='Open Sans',align=1/%s"),RGB(223,223,223), _T(""));
    m_pStatusStr->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnClickBtnSetStatus);
    m_pPaint->AddObject(m_pStatusStr->GetObjectId(),86, 51);

    m_pSetting = GetSonicUI()->CreateString();
    m_pPaint->AddObject(m_pSetting->GetObjectId(),0,0);

    m_pBgIcon_big = GetSonicUI()->CreateString();    
    m_pBgIcon_big->Format(_T("/a,p=%d/"),GetCoreUI()->GetImage(_T("LOGO_OVERLAYARC"))->GetObjectId());
    m_pBgIcon_big->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnArcIcon);
    m_pPaint->AddObject(m_pBgIcon_big->GetObjectId(),0,0,TRUE);


    m_pNickName = GetSonicUI()->CreateString();	
    m_pPaint->AddObject(m_pNickName->GetObjectId(),86,23,TRUE);	


    m_pNewsBtn_big = GetSonicUI()->CreateString();
    m_pWeb_big = GetSonicUI()->CreateString();
    m_pCommunityBtn_big = GetSonicUI()->CreateString();
    m_pMedia_big = GetSonicUI()->CreateString();
    m_pChargeBtn_big = GetSonicUI()->CreateString();
    m_pSuportBtn_big = GetSonicUI()->CreateString();


    m_pPaint->AddObject(m_pNewsBtn_big->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pWeb_big->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pCommunityBtn_big->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pMedia_big->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pChargeBtn_big->GetObjectId(),0,0,TRUE);
    m_pPaint->AddObject(m_pSuportBtn_big->GetObjectId(),0,0,TRUE);

    m_pAvatar = GetSonicUI()->CreateImage();
    m_pAvatarPaint = GetSonicUI()->CreatePaint();
    m_pAvatarPaint->Create(
        FALSE,
        GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetWidth(),
        GetCoreUI()->GetImage(_T("CHAT_STATE_BIG"))->GetHeight()/3
        );
    m_pPaint->AddObject(m_pAvatarPaint->GetObjectId(),20,14,TRUE);


    m_HighResolutionUI.push_back(m_pPaintMainBg);
    m_HighResolutionUI.push_back(m_pStatusStr);
    m_HighResolutionUI.push_back(m_pAvatarPaint);
    m_HighResolutionUI.push_back(m_pSetting);
    m_HighResolutionUI.push_back(m_pBgIcon_big);
    m_HighResolutionUI.push_back(m_pNickName);
    m_HighResolutionUI.push_back(m_pNewsBtn_big);
    m_HighResolutionUI.push_back(m_pWeb_big);
    m_HighResolutionUI.push_back(m_pCommunityBtn_big);
    m_HighResolutionUI.push_back(m_pMedia_big);
    m_HighResolutionUI.push_back(m_pChargeBtn_big);
    m_HighResolutionUI.push_back(m_pSuportBtn_big);


}



void CUIMainWndframe::InitTextUI()
{
    m_szBackToGameTip.Format(2,_T("%s %s"),GetToolTipText().c_str(),m_pMsgCookie->csClose);    

    m_szNickName.Format(2,_T("%s, %s"),m_pMsgCookie->csHi, COverlayImImpl::GetUIChatMgr()->GetNickName());
}

_tstring CUIMainWndframe::GetToolTipText()
{
    int nFunction = GetPrivateProfileInt(INI_CFG_CLIENT_INGAME, INI_CFG_CLIENT_INGAME_FUNCTION, 16, theDataPool.GetUserProfilePath().c_str());
    int nShortcut = GetPrivateProfileInt(INI_CFG_CLIENT_INGAME ,INI_CFG_CLIENT_INGAME_SHORTCUT, 9, theDataPool.GetUserProfilePath().c_str());
    return CUtility::GetKeyBoardTextFromVK(nFunction, nShortcut);
}



void CUIMainWndframe::InitCommonUI()
{
    m_pBgBottom = GetSonicUI()->CreateString();
    m_pBgBottom->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_LIGHT"))->GetObjectId());
    m_pPaint->AddObject(m_pBgBottom->GetObjectId(),0,0,TRUE);


    m_pBackToGame = GetSonicUI()->CreateString();
    m_pBackToGame->Format(_T("/a,p=%d/"),GetCoreUI()->GetImage(_T("BTN_OVERLAYMAIN_BACKTOGAME"))->GetObjectId());

    m_pBackToGame->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnBacktoGame);
    m_pBackToGame->Delegate(DELEGATE_EVENT_MOUSEENTER,NULL,this,&CUIMainWndframe::OnBacktoGameMouseEnter);
    m_pBackToGame->Delegate(DELEGATE_EVENT_MOUSELEAVE,NULL,this,&CUIMainWndframe::OnBacktoGameMouseLeave);

    m_pSystemTime = GetSonicUI()->CreateString();
    m_pSystemDate = GetSonicUI()->CreateString();
    SetSystemDate();
    SetSystemTime();


    CRect rcClient;
    ::GetClientRect(m_hWnd,&rcClient);
    m_pPaint->AddObject(m_pBackToGame->GetObjectId(),rcClient.right - 46,22,TRUE);
    m_pPaint->AddObject(m_pSystemTime->GetObjectId(),rcClient.right - m_pSystemTime->GetWidth() - 16,rcClient.bottom - 80,TRUE);
    m_pPaint->AddObject(m_pSystemDate->GetObjectId(),rcClient.right - m_pSystemDate->GetWidth() - 16 , rcClient.bottom - 35,TRUE);

    m_pGameNameTitle = GetSonicUI()->CreateString();
    m_pGameName = GetSonicUI()->CreateString();
    m_pPaint->AddObject(m_pGameNameTitle->GetObjectId(),18,22,TRUE);
    m_pPaint->AddObject(m_pGameName->GetObjectId(),18,48,TRUE);


}

void CUIMainWndframe::InitMainUI()
{
	m_pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	m_pMsgCookie = COverlayImpl::GetInstance()->GetMsgCookie();
	if (!m_pMsgCookie || !m_pPaint)
		return;


    InitCommonUI();
    InitLowResolutionUI();
    InitHighResolutionUI();
    InitIMUI();
    InitTextUI();
}

void CUIMainWndframe::InitIMUI()
{
    COverlayImImpl::GetInstance()->ShowFriendDlg(CRect(0,0,0,0)); //create friend dialog here

	int nPresence = COverlayImImpl::GetUIChatMgr()->GetSefPresence();
	if (nPresence != ArcPresenceType::Unavailable)
	{
		COverlayImImpl::GetUIChatMgr()->RequestFriendList();
	}
	
	COverlayImImpl::GetInstance()->ShowStatusDlg(CRect(0,140,0,108),TRUE);
	IntiSatusStr();
	SetStatus();
	UpdateStatus(nPresence);
}

void CUIMainWndframe::OnClickBtnSetStatus(ISonicString *pStatus, LPVOID pReserve)
 {
	//CRect rc;
	CUIStatusDlgSecondary* pStausDlg = (CUIStatusDlgSecondary* )COverlayImImpl::GetUIChatMgr()->GetStatusDlg();
	if (!pStausDlg) return ;
	if(::IsWindowVisible(pStausDlg->m_hWnd))
	{
	 ::ShowWindow(pStausDlg->m_hWnd,SW_HIDE);
	 return;
	}
	im_dialog_params params;
	params.pParentWnd = COverlayImpl::GetInstance()->GetMainWnd();
	params.nShowCmd = SW_SHOWNA;
	CRect rc;
	rc = (*m_pStatusStr->GetRect());
	POINT pt;
	pt.x =  rc.right;
	pt.y = rc.bottom;
	::ClientToScreen(m_hWnd, &pt);
	 
	params.rt.right = pt.x +115;
	params.rt.left = params.rt.right - 140;
	params.rt.top = pt.y ;
	params.rt.bottom = params.rt.top + 108;
	// ::MoveWindow(pStausDlg->m_hWnd,100,100,400,400,true);
	//::ShowWindow(pStausDlg->m_hWnd,SW_SHOW);
	 
	 
	COverlayImImpl::GetUIChatMgr()->ShowStatusDlg(params);
	// SetWindowPos(pStausDlg->m_hWnd,NULL,rc.left - 10,rc.bottom + 10,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
	SetStatus();
	pStausDlg->m_LBSetting.SendMessage(WM_SETFOCUS);
	
//
///	 COverlayImImpl::GetInstance()->ShowStatusDlg(IsWindowVisible(COverlayImImpl::GetUIChatMgr()->GetStatusDlg()->m_hWnd)); 	 
 }
void CUIMainWndframe::SetStatus()
{
	//set selected item.
	CUIStatusDlgSecondary* pStatusDlg = (CUIStatusDlgSecondary*)COverlayImImpl::GetUIChatMgr()->GetStatusDlg();
	CUIFriendDlgSecondary* pFriendDlg = (CUIFriendDlgSecondary*)COverlayImImpl::GetUIChatMgr()->GetFriendDlg();
	if (!pStatusDlg || !pFriendDlg) 
		return ;

	int i = 0;
	for (;i < pStatusDlg->m_LBSetting.GetCount();i++)
	{
		if (pFriendDlg->GetCurPresence() == pStatusDlg->m_LBSetting.GetItemData(i))
		{
			pStatusDlg->m_LBSetting.SetCurSel(i);
			break;
		}
	}
	if (i == pStatusDlg->m_LBSetting.GetCount())
	{
		pStatusDlg->m_LBSetting.SetCurSel(pStatusDlg->m_LBSetting.GetCount()-1);
	}
}

void CUIMainWndframe::OnNewsClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csNewsPage);
	COverlayImpl::GetInstance()->SendTrack(_T("news"));
	
}
void CUIMainWndframe::OnBillingClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csBillingPage);
	COverlayImpl::GetInstance()->SendTrack(_T("billing"));
}
void CUIMainWndframe::OnSupportClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csSupportPage);	
	COverlayImpl::GetInstance()->SendTrack(_T("support"));

}
void CUIMainWndframe::OnWebClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csWebPage);
	COverlayImpl::GetInstance()->SendTrack(_T("achievement"));
}
void CUIMainWndframe::OnCommunityClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csForumPage);
	COverlayImpl::GetInstance()->SendTrack(_T("forum"));
}
void CUIMainWndframe::OnMediaClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csMediaPage);
	COverlayImpl::GetInstance()->SendTrack(_T("coreconnect"));
}
void CUIMainWndframe::OnSettingClick(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowSetting();
}
void CUIMainWndframe::OnBacktoGame(ISonicBase*,LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowOverlay(FALSE);
}
void CUIMainWndframe::OnArcIcon(ISonicBase*,LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csWebPage);
}

void CUIMainWndframe:: CalMainBtn(int & nHeight,int & nGap,int &nTopPoint)
{
	CRect rc;
	::GetClientRect(m_hWnd,&rc);
	float fTotalHieght = rc.Height() - DEFAULTLISTTOP -DEFAULLISTBOTTOM + 1;
	float fGap = fTotalHieght/38;
	float fBtnHieght = fGap * 5.5;
	nHeight  = (int)(fBtnHieght + 0.5);
	nGap = (int)(fGap + 0.5);
	int nTemp = fTotalHieght - nHeight * 6 - nGap * 5;
	if (nTemp >= 0)
	{
		int  i = nTemp/2;
		nTopPoint = DEFAULTLISTTOP + i;
		//nBottomPoint = rc.Height() - DEFAULLISTBOTTOM  - (nTemp - i);
	}
	else
	{
		int nNum = abs(nTemp);
		int  i = nTemp/2;
		nTopPoint = DEFAULTLISTTOP - i;
		//nBottomPoint = rc.Height() - DEFAULLISTBOTTOM  + (nTemp - i);
	}
}

void CUIMainWndframe::SetSystemDate()
{
	if(m_pSystemDate)
	{
		CTime ct = CTime::GetCurrentTime();
		core_msg_cookie * pCookie = COverlayImpl::GetInstance()->GetMsgCookie();
		_tstring strdate;
		if (pCookie)
		{
			strdate.Format(4,_T("%s, %s %d %d"),pCookie->csWeek[ct.GetDayOfWeek() - 1],pCookie->csMonth[ct.GetMonth() - 1],ct.GetDay(),ct.GetYear());
		}
		m_pSystemDate->Format(_T("/c=%x, font, font_height=14, font_face='Open Sans'/%s"),RGB(170,170,170),strdate.c_str());
	}

	// CSimpleStringT
	return ;
}
void CUIMainWndframe::SetSystemTime()
{
	if (m_pSystemTime)
	{
		CTime ct = CTime::GetCurrentTime();
		_tstring strSystemTime = ct.Format(_T("%H:%M"));
		m_pSystemTime->Format(_T("/c=%x, font, font_height=34, font_face='Open Sans'/%s"),RGB(247,247,247),strSystemTime.c_str());
	}	
}
void CUIMainWndframe::HandleSelfAvatar(LPCTSTR lpPath)
{
	CUIFriendDlgSecondary* pFriendDlg = (CUIFriendDlgSecondary*)COverlayImImpl::GetUIChatMgr()->GetFriendDlg();
	if (!pFriendDlg) 
		return ;

	if (::GetFileAttributes(lpPath) != INVALID_FILE_ATTRIBUTES)
		m_pAvatar->Load(lpPath);
		
	UpdateAvatarPaint(m_pAvatar, pFriendDlg->GetCurPresence());
}
void CUIMainWndframe::HandleSelfPresence(UINT iStatus)
{
	_tstring szPath = COverlayImImpl::GetUIChatMgr()->GetBaseDir() + COverlayImImpl::GetUIChatMgr()->GetSelfAvatarPath();
	if (::GetFileAttributes(szPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		m_pAvatar->Load(szPath.c_str());

	UpdateAvatarPaint(m_pAvatar, iStatus);
	UpdateStatus(iStatus);
}
void CUIMainWndframe::UpdateAvatarPaint(ISonicImage* pImg,UINT iStatus)
{
	if (m_pAvatarPaint == NULL || pImg == NULL)
	{
		return;
	}
	ISonicString* pAvatarStr = GetSonicUI()->CreateString();
	pAvatarStr->Format(_T("/a,p=%d, pstretch=43|43/"),pImg->GetObjectId());
	pAvatarStr->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnClickedBtnAvatar);

	m_pAvatarPaint->DelAllObject();
	m_pAvatarPaint->AddObject(COverlayImImpl::GetUIChatMgr()->GetStatusBkImg(iStatus,true)->GetObjectId(),0,0,TRUE);
	m_pAvatarPaint->AddObject(pAvatarStr->GetObjectId(),(m_pAvatarPaint->GetWidth()-pAvatarStr->GetWidth())/2,(m_pAvatarPaint->GetHeight()-pAvatarStr->GetHeight())/2,TRUE);		
	m_pAvatarPaint->Redraw(FALSE);
}

void CUIMainWndframe::OnClickedBtnAvatar(ISonicString *pStatus, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowBrowserMainWnd(COverlayImpl::GetInstance()->GetMsgCookie()->csMyAccountPage);	
}

void CUIMainWndframe::UpdateStatus(UINT iStatus)
{
	if (m_pStatusStr)
	{
		m_pStatusStr->Format(_T("/a,c=%x, font, font_height=12, font_face='Open Sans',align=1/%s  /a,p=%d,align=1/"),RGB(223,223,223),m_szStatus[iStatus].c_str(),GetCoreUI()->GetImage(_T("BTN_OVERLAY_STATUS"))->GetObjectId());
	}
}

void CUIMainWndframe::IntiSatusStr()
{
	m_szStatus[0] = COverlayImImpl::GetUIChatMgr()->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_ONLINE"));
	m_szStatus[1] = _T("");
	m_szStatus[2] = _T("");
	m_szStatus[3] = COverlayImImpl::GetUIChatMgr()->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_BUSY"));
	m_szStatus[4] = _T("");
	m_szStatus[5] = COverlayImImpl::GetUIChatMgr()->GetStringFromId(_T("IDS_DIALOG_FRIENDLIST_OFFLINE"));
}

void CUIMainWndframe::ShowBacktoGameTip()
{
	CRect rc = (*m_pBackToGame->GetRect());
	::ClientToScreen(m_hWnd, (LPPOINT)&rc);
	
    m_GameBackTip.SetWindowPos(HWND_TOPMOST,(rc.left - 270),(rc.top - 20),270,96,SWP_NOACTIVATE|SWP_SHOWWINDOW);
}

void CUIMainWndframe::OnBacktoGameMouseEnter(ISonicBase*,LPVOID pReserve)
{
	SetBackToGameTip();
	ShowBacktoGameTip();
}

void CUIMainWndframe::OnBacktoGameMouseLeave(ISonicBase*,LPVOID pReserve)
{
	m_GameBackTip.ShowWindow(SW_HIDE);
}

void CUIMainWndframe::SetBackToGameTip()
{
	if (!m_GameBackTip.m_hWnd)
	{
		m_GameBackTip.Create(m_hWnd,WTL::CRect(0,0,100,100),NULL,WS_POPUP);
	}

    m_szBackToGameTip.Format(2,_T("%s %s"),GetToolTipText().c_str(),m_pMsgCookie->csClose);    
	m_GameBackTip.SetStrText(_tstring(m_pMsgCookie->csBackToGame), m_szBackToGameTip);

}

void CUIMainWndframe::Resize()
{
    CRect rcClient;
    ::GetClientRect(m_hWnd,&rcClient);

    if (m_pBackToGame)
        m_pBackToGame->Move(rcClient.right - 46, 22);

    if (m_pSystemTime)
        m_pSystemTime->Move(rcClient.right - m_pSystemTime->GetWidth() - 16,rcClient.bottom - 80);

    if (m_pSystemDate)
        m_pSystemDate->Move(rcClient.right - m_pSystemDate->GetWidth() - 16 , rcClient.bottom - 35);




	CSize szOverlaySize = COverlayImpl::GetInstance()->GetOverlaySize();
    if (szOverlaySize.cx < RESOLUTION_SIGN)
    {
        // hide [12/10/2014 liuyu]

        if (m_pGameNameTitle)
        {
            m_pGameNameTitle->Move(18, 22);
            m_pGameNameTitle->Format(_T("/c=%x, font, font_height=16,line_width=%d,single_line=2, font_face='Open Sans'/%s"),RGB(147,147,147),150,GetSonicUI()->HandleRawString(m_pMsgCookie->csCurrentPlay,RSCT_DOUBLE));
        }

        if (m_pGameName)
        {
            m_pGameName->Move(18, 48);
            m_pGameName->Format(_T("/c=%x, font, font_height=26,line_width=%d,single_line=2, font_face='Open Sans'/%s"),RGB(237,237,237),300,GetSonicUI()->HandleRawString(m_pMsgCookie->csGameName,RSCT_DOUBLE));
        }

        if (m_pBackToGame)
            m_pBackToGame->Move(rcClient.right - 46, 22);

        if (m_pBgIcon)
            m_pBgIcon->Move(20,rcClient.bottom - 53);

        int nTop = 90;
        int nTemp = 58;
        if (m_pNewsBtn)
            m_pNewsBtn->Move(20, nTop);        
        nTop += nTemp;

        if (m_pWeb)
            m_pWeb->Move(20, nTop);        
        nTop += nTemp;

        if (m_pCommunityBtn)
            m_pCommunityBtn->Move(20, nTop);        
        nTop += nTemp;

        if (m_pMedia)
            m_pMedia->Move(20, nTop);        
        nTop += nTemp;

        if (m_pChargeBtn)
            m_pChargeBtn->Move(20, nTop);        
        nTop += nTemp;

        if (m_pSupportBtn)
            m_pSupportBtn->Move(20, nTop);        
        
    }
    else
    {

        if (m_pGameNameTitle)
        {
            m_pGameNameTitle->Format(_T("/c=%x, font, font_height=14,line_width=%d,single_line=2, font_face='Open Sans'/%s"),RGB(147,147,147),150,GetSonicUI()->HandleRawString(m_pMsgCookie->csCurrentPlay,RSCT_DOUBLE));
            m_pGameNameTitle->Move(498, 22);
        }

        if (m_pGameName)
        {
            m_pGameName->Format(_T("/c=%x, font, font_height=18,line_width=%d,single_line=2, font_face='Open Sans'/%s"),RGB(237,237,237),300,GetSonicUI()->HandleRawString(m_pMsgCookie->csGameName,RSCT_DOUBLE));
            m_pGameName->Move(498, 45);
        }

        CRect rcClient;
        ::GetClientRect(m_hWnd,&rcClient);

        if (m_pPaintMainBg)
            m_pPaintMainBg->Move(0, 0,MAINLEFTBGWIDTH, rcClient.Height());

        if (m_pNickName)
        {
            m_pNickName->Format(_T("/a,c=0xffffff,font,font_bold=0,font_height=18,single_line=2,line_width=350,font_face='Open Sans'/%s"), m_szNickName.c_str());
            m_pNickName->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnClickedBtnAvatar);
            m_pNickName->Move(86, 23);
        }

        if (m_pStatusStr) 
            m_pStatusStr->Move(86, 51);



        int nHeight,nGap,nTopPoint;
        CalMainBtn(nHeight,nGap,nTopPoint);
        CRect rcFriendList;
        rcFriendList.top = nTopPoint;


        if (m_pNewsBtn_big)
        {    
            m_pNewsBtn_big->Format(_T("/a, p=%d, btn_height=%d,btn_width=140, linkc=0, btn_text='/p=%d,align=0x10/\n/c=%x, font, font_height=14,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_BTN"))->GetObjectId(),nHeight,GetCoreUI()->GetImage(_T("OVERLAY_NEWS"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),m_pMsgCookie->csNews);
            m_pNewsBtn_big->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnNewsClick) ;
            m_pNewsBtn_big->Move(MAINBTN_X,nTopPoint);
            nTopPoint += (nHeight + nGap);
        }

        if (m_pWeb_big)
        {            
            m_pWeb_big->Format(_T("/a, p=%d, btn_height=%d,btn_width=140, linkc=0, btn_text='/p=%d, align=0x10/\n/c=%x,font, font_height=14,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_BTN"))->GetObjectId(),nHeight,GetCoreUI()->GetImage(_T("OVERLAY_WEB"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),m_pMsgCookie->csWeb);
            m_pWeb_big->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnWebClick) ;
            m_pWeb_big->Move(MAINBTN_X,nTopPoint);
            nTopPoint += (nHeight + nGap);
        }


        if (m_pCommunityBtn_big)
        {
            m_pCommunityBtn_big->Format(_T("/a, p=%d, btn_height=%d,btn_width=140, linkc=0, btn_text='/p=%d, align=0x10/\n/c=%x,font, font_height=14,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_BTN"))->GetObjectId(),nHeight,GetCoreUI()->GetImage(_T("OVERLAY_COMMUNITY"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),m_pMsgCookie->csForum);
            m_pCommunityBtn_big->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnCommunityClick) ;
            m_pCommunityBtn_big->Move(MAINBTN_X,nTopPoint);
            nTopPoint += (nHeight + nGap);
        }

        if (m_pMedia_big)
        {            
            m_pMedia_big->Format(_T("/a, p=%d, btn_height=%d,btn_width=140, linkc=0, btn_text='/p=%d, align=0x10/\n/c=%x, font, font_height=14,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_BTN"))->GetObjectId(),nHeight,GetCoreUI()->GetImage(_T("OVERLAY_MEDIA"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),m_pMsgCookie->csMedia);
            m_pMedia_big->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnMediaClick) ;
            m_pMedia_big->Move(MAINBTN_X,nTopPoint);
            nTopPoint += (nHeight + nGap);
        }


        if (m_pChargeBtn_big)
        {
            m_pChargeBtn_big->Format(_T("/a, p=%d, btn_height=%d,btn_width=140, linkc=0, btn_text='/p=%d, align=0x10/\n/c=%x,font, font_height=14,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_BTN"))->GetObjectId(),nHeight,GetCoreUI()->GetImage(_T("OVERLAY_BILLING"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),m_pMsgCookie->csBilling);
            m_pChargeBtn_big->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnBillingClick) ;
            m_pChargeBtn_big->Move(MAINBTN_X,nTopPoint);
            nTopPoint += (nHeight + nGap);
        }

        if (m_pSuportBtn_big)
        {
            m_pSuportBtn_big->Format(_T("/a, p=%d, btn_height=%d,btn_width=140, linkc=0, btn_text='/p=%d, align=0x10/\n/c=%x,font, font_height=14,font_face='Open Sans',linkh=%x/%s'/"),GetCoreUI()->GetImage(_T("BG_OVERLAYMAIN_BTN"))->GetObjectId(),nHeight,GetCoreUI()->GetImage(_T("OVERLAY_SUPPORT"))->GetObjectId(),RGB(170,170,170),RGB(243,243,243),m_pMsgCookie->csSupport);
            m_pSuportBtn_big->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CUIMainWndframe::OnSupportClick) ;
            m_pSuportBtn_big->Move(MAINBTN_X,nTopPoint);
        }


        if (m_pSetting)
        {
            m_pSetting->Format(_T("/a,p=%d/"), GetCoreUI()->GetImage(_T("BTN_OVERLAY_SETTING"))->GetObjectId());
            m_pSetting->Delegate(DELEGATE_EVENT_CLICK,NULL,this,&CUIMainWndframe::OnSettingClick);
            m_pSetting->Move(443,rcClient.bottom - 45);
        }

        if (m_pBgIcon_big)
            m_pBgIcon_big->Move(20,rcClient.bottom - 53);


        if (m_pAvatarPaint)
            m_pAvatarPaint->Move(20,14);

        rcFriendList.left = FRIENDLISTLEFT;
        rcFriendList.bottom = nTopPoint + nHeight;
        rcFriendList.right = FRIENDLISTRIGHT;

        if(COverlayImImpl::GetUIChatMgr()->GetFriendDlg())
            COverlayImImpl::GetUIChatMgr()->GetFriendDlg()->MoveWindow(rcFriendList);
    }
    
    ShowByResolution(szOverlaySize.cx < RESOLUTION_SIGN);
}

void CUIMainWndframe::ShowByResolution( BOOL bLowResolution )
{
    for each(ISonicBaseWnd* ui in m_LowResolutionUI)
    {
        ui->Show(bLowResolution);
    }

    for each(ISonicBaseWnd* ui in m_HighResolutionUI)
    {
        ui->Show(!bLowResolution);
    }

    if(COverlayImImpl::GetUIChatMgr()->GetFriendDlg())
        COverlayImImpl::GetUIChatMgr()->GetFriendDlg()->ShowWindow(bLowResolution?SW_HIDE:SW_SHOW);
}

