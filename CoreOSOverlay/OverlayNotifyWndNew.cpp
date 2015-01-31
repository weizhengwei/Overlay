#include <stdafx.h>
#include "OverlayNotifyWndNew.h"
#include "OverlayImpl.h"
#include "data/UIString.h"
#include "im/im_constant.h"
#include "OverlayImImpl.h"

#define TIMER1 1
#define TIMER2 2
#define LINK_COLOR RGB(78,126,219)

////////////////////////////////////////////////////////////////////////
//Inbox messages
ListItemInboxNew::ListItemInboxNew()
{

	m_pIcon    = GetSonicUI()->CreateString();
	m_pTitle   = GetSonicUI()->CreateString();
	m_pContent = GetSonicUI()->CreateString();
}

ListItemInboxNew::~ListItemInboxNew()
{
	GetSonicUI()->DestroyObject(m_pIcon);
	GetSonicUI()->DestroyObject(m_pTitle);
	GetSonicUI()->DestroyObject(m_pContent);
}

BOOL ListItemInboxNew::InsertItem(CTempPopUp* pDlg, PVOID psData)
{
	if (!pDlg)
	{
		return FALSE;
	}
	InitItems(pDlg, psData);
	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(pDlg->m_hWnd)->GetPaint();
	if (pPaint != NULL)
	{
		pPaint->AddObject(m_pIcon->GetObjectId(), 25, 30, FALSE);
		pPaint->AddObject(m_pTitle->GetObjectId(), 60, 25, FALSE);
		pPaint->AddObject(m_pContent->GetObjectId(), 60, 25 + m_pTitle->GetHeight() + 10, FALSE);
	}
	return TRUE;
}

void ListItemInboxNew::InitItems(CTempPopUp* pDlg, PVOID psData)
{ 
	m_sData = *(core_msg_inbox*)psData;
	_tstring sNickName = m_sData.sNickName;
	_tstring sContent = m_sData.sContent;
	sNickName.Replace(_T("/"), _T("//"));
	sContent.Replace(_T("/"), _T("//"));
	_tstring sTitle;
	sTitle.Format(1, m_sData.sTitle, m_sData.sNickName);
	if(!lstrcmpi(m_sData.sType, _T("user")))
	{
		m_pTitle->Format(_T("/a, c=0xffffff, single_line=2, line_width=%d, font, font_height=14, \
							font_face='Open Sans'/%s"), CONTENT_WIDTH, sTitle.c_str()) ;
		m_pTitle->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemInboxNew::OnClickTop) ;
	}
	else if (!lstrcmpi(m_sData.sType, _T("admin")))
	{
		if (!lstrcmpi(m_sData.sArcMailSubject, _T("")))
		{
			m_pTitle->Format(_T("/c=0xffffff, single_line=2, line_width=%d, font, font_height=14, \
								font_face='Open Sans'/%s"), CONTENT_WIDTH, sTitle.c_str()) ;
		}
		else
		{
			m_pTitle->Format(_T("/c=0xffffff, single_line=2, line_width=%d, font, font_height=14, \
								font_face='Open Sans'/%s"), CONTENT_WIDTH, m_sData.sArcMailSubject) ;
		}
		
	}
	

	m_pContent->Format(_T("/a, c=0xa7a7a7, line_width=%d, height_limit=%d, font, font_height=13, \
							  font_face='Open Sans'/%s"),CONTENT_WIDTH, CONTENT_HEIGHT, sContent.c_str()) ;

	m_pIcon->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("NOTIFY_ICON_INBOX"))->GetObjectId());

	
	m_pContent->Delegate(DELEGATE_EVENT_CLICK,  pDlg, this, &ListItemInboxNew::OnClickBottom) ;
}

void ListItemInboxNew::OnClickTop(ISonicBase *, LPVOID pReserve)
{
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg == NULL)
	{
		return;
	}
	_tstring sUrl;
	sUrl = m_sData.sFriendLink;
	if (!sUrl.empty())
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sUrl.c_str());
	}	
}

void ListItemInboxNew::OnClickBottom(ISonicBase *, LPVOID pReserve)
{
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg == NULL)
	{
		return;
	}
	_tstring sUrl;
	sUrl = m_sData.sContentLink;
	if (!sUrl.empty())
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sUrl.c_str());
		/*core_msg_track stTrack;
		lstrcpyn(stTrack.csTrack, INBOX_CLICK, 256);
		stTrack.dwId = _ttoi64(m_sData.sId);
		COverlayImpl::GetInstance()->SendCoreMsg(&stTrack);*/
	}
	pDlg->PostMessage(WM_CLOSE);
}

/////////////////////////////////////////////////////////////////////
///Friend requests
ListItemFriReqNew::ListItemFriReqNew()
{ 
	m_pTitle         = GetSonicUI()->CreateString();
	m_pTextContent   = GetSonicUI()->CreateString();
	m_pBtnAccept     = GetSonicUI()->CreateString();
	m_pBtnCancel     = GetSonicUI()->CreateString();
	m_pIcon          = GetSonicUI()->CreateString();
}

ListItemFriReqNew::~ListItemFriReqNew()
{
	GetSonicUI()->DestroyObject(m_pBtnCancel);
	GetSonicUI()->DestroyObject(m_pBtnAccept);
	GetSonicUI()->DestroyObject(m_pTextContent);
	GetSonicUI()->DestroyObject(m_pTitle);
	GetSonicUI()->DestroyObject(m_pIcon);
}

BOOL ListItemFriReqNew::InsertItem(CTempPopUp* pDlg, PVOID psData)
{
	if (NULL == pDlg)
	{
		return FALSE ;
	}
	
	InitItems(pDlg, psData);
	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(pDlg->m_hWnd)->GetPaint();
	if (pPaint != NULL)
	{
		pPaint->AddObject(m_pIcon->GetObjectId(), 25, 30, FALSE);
		pPaint->AddObject(m_pTitle->GetObjectId(), 60, 25, FALSE) ;
		pPaint->AddObject(m_pTextContent->GetObjectId(), 60, 25 + m_pTitle->GetHeight() + 10, FALSE) ; 
		pPaint->AddObject(m_pBtnAccept->GetObjectId(), 60, 25 + m_pTitle->GetHeight() + m_pTextContent->GetHeight() + 20, FALSE) ;
		pPaint->AddObject(m_pBtnCancel->GetObjectId(), 60 + m_pBtnAccept->GetWidth() + 10, \
			25 + m_pTitle->GetHeight() + m_pTextContent->GetHeight() + 20, FALSE) ;
	}
	
	return TRUE ; 
}
void ListItemFriReqNew::InitItems(CTempPopUp* pDlg, PVOID psData)
{
	m_sData = *(core_msg_friendreq*)psData;
	m_pTitle->Format(_T("/a, single_line=2, line_width=%d, c=0xffffff, font, font_height=14, \
						font_bold=1, font_face='Open Sans'/%s"), TITLE_WIDTH, m_sData.sTitle);
	
	m_pTextContent->Format(_T("/a, linkl=1, single_line=2, line_width=%d, c=%x, font, font_height=13, font_face='Open Sans'/%s ")
		_T("/c=0xacacac, font, font_height=13, font_face='Open Sans'/%s"), 
		CONTENT_WIDTH, LINK_COLOR, m_sData.sNickName, m_sData.sContent);
	m_pTextContent->SetAttr(CORE_ATTR_NOHILIGHT, 1);

	m_pBtnAccept->Format(_T("/a, p=%d, btn_width=%d, btn_height=%d, btn_text='/c=0xffffff, font, font_height=13, font_bold=0, \
							font_face='Open Sans', single_line=2, align=1/%s'/"), GetCoreUI()->GetImage(_T("BOTTOM_POPUP_ACCEPT"))->GetObjectId(), 75, 25,  \
							COverlayImpl::GetInstance()->GetMsgCookie()->csAccept) ;
	m_pBtnCancel->Format(_T("/a, p=%d, btn_width=%d, btn_height=%d, btn_text='/c=0xffffff, font, font_height=13, font_bold=0, \
							font_face='Open Sans', single_line=2, align=1/%s'/"), GetCoreUI()->GetImage(_T("BTN_POPUP"))->GetObjectId(), 75, 25,\
							COverlayImpl::GetInstance()->GetMsgCookie()->csIgnore) ;

	m_pIcon->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("NOTIFY_ICON_FRIREQ"))->GetObjectId());

	m_pTitle->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemFriReqNew::OnClickTop) ;
	m_pTextContent->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemFriReqNew::OnClickContent);
	m_pBtnAccept->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemFriReqNew::OnClickAccept) ;
	m_pBtnCancel->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemFriReqNew::OnClickIgnore) ;

}

void ListItemFriReqNew::OnClickTop(ISonicBase *, LPVOID pReserve)
{
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg == NULL)
	{
		return;
	}
	_tstring sUrl;
	sUrl = m_sData.sFriendLink;
	if (!sUrl.empty())
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sUrl.c_str());
	}			
}

void ListItemFriReqNew::OnClickContent(ISonicBase *, LPVOID pReserve)
{
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg == NULL)
	{
		return;
	}
	_tstring sUrl;
	sUrl = COverlayImpl::GetInstance()->GetMsgCookie()->csMyFriendUrl;
	if (!sUrl.empty())
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sUrl.c_str());
	}			
}

void ListItemFriReqNew::OnClickAccept(ISonicBase *, LPVOID pReserve)
{
	m_pBtnAccept->Enable(FALSE);
	m_pBtnCancel->Enable(FALSE);
	core_msg_track stTrack;
	lstrcpyn(stTrack.csTrack, FR_ACCEPT, 256);
	stTrack.dwSenderId = m_sData.nSenderId;
	stTrack.dwReceiverId = _ttoi64(m_sData.sId);
	COverlayImpl::GetInstance()->SendCoreMsg(&stTrack);

	_tstring sText;
	sText.Format(1, COverlayImpl::GetInstance()->GetMsgCookie()->csAcceptText , m_sData.sNickName);
	m_pTextContent->Format(_T("/c=0xa7a7a7, line_width=%d, height_limit=%d, font, font_height=13, font_face='Open Sans'/%s"),\
		CONTENT_WIDTH + 50, CONTENT_HEIGHT, sText.c_str());
	m_pBtnAccept->Show(FALSE);
	m_pBtnCancel->Show(FALSE);
}

void ListItemFriReqNew::OnClickIgnore(ISonicBase*, LPVOID pReserve)
{
	m_pBtnAccept->Enable(FALSE);
	m_pBtnCancel->Enable(FALSE);
	core_msg_track stTrack;
	lstrcpyn(stTrack.csTrack, FR_IGNORE, 256);
	stTrack.dwSenderId = m_sData.nSenderId;
	stTrack.dwReceiverId = _ttoi64(m_sData.sId);
	COverlayImpl::GetInstance()->SendCoreMsg(&stTrack);

	_tstring sText;
	sText.Format(2, _T("%s %s"), COverlayImpl::GetInstance()->GetMsgCookie()->csIgnortext, m_sData.sNickName);
	m_pTextContent->Format(_T("/c=0xa7a7a7, line_width=%d, height_limit=%d, font, font_height=13, font_face='Open Sans'/%s"),\
		CONTENT_WIDTH + 50, CONTENT_HEIGHT, sText.c_str());
	m_pBtnAccept->Show(FALSE);
	m_pBtnCancel->Show(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
///Notifications
ListItemNotifyNew::ListItemNotifyNew()
{
	m_pIcon        = GetSonicUI()->CreateString();
	m_pTextTitle   = GetSonicUI()->CreateString();
	m_pTextContent = GetSonicUI()->CreateString();
}

ListItemNotifyNew::~ListItemNotifyNew()
{
	GetSonicUI()->DestroyObject(m_pIcon);
	GetSonicUI()->DestroyObject(m_pTextContent) ;
	GetSonicUI()->DestroyObject(m_pTextTitle) ;
}

BOOL ListItemNotifyNew::InsertItem(CTempPopUp* pDlg, PVOID psData)
{
	BOOL bRet = TRUE ;

	if (NULL == pDlg)
	{
		return FALSE ;
	}

	InitItems(pDlg, psData);
	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(pDlg->m_hWnd)->GetPaint();
	if (pPaint != NULL)
	{
		pPaint->AddObject(m_pIcon->GetObjectId(), 25, 30, FALSE);
		pPaint->AddObject(m_pTextTitle->GetObjectId(), 60, 25, FALSE);
		pPaint->AddObject(m_pTextContent->GetObjectId(), 60, 25 + m_pTextTitle->GetHeight() + 10, FALSE);
	}
	return bRet ;
}

void ListItemNotifyNew::InitItems(CTempPopUp* pDlg, PVOID psData)
{
	core_msg_notification* pData = (core_msg_notification*)psData ;
	m_sData = *pData;
	_tstring sTitle = m_sData.sTitle;
	_tstring sContent = m_sData.sText;
	sTitle.Replace(_T("/"), _T("//"));
	sContent.Replace(_T("/"), _T("//"));
	int nPos = sContent.Find(_T("^link:")) ;
	if(nPos >= 0)
	{
		int nLinkend=sContent.Find(_T(";"));
		_tstring sLink = sContent.substr(nPos+_tcslen(_T("^link:")), nLinkend-nPos-_tcslen(_T("^link:")));
		//m_sData.m_sContent.erase(nIndex,6);
		sContent.erase(0, sLink.length() + 1);
		lstrcpyn(m_sData.sText, sContent.c_str(), 1024);
		lstrcpyn(m_sData.sLink, sLink.c_str(), 1024);
	}

	m_pTextTitle->Format(_T("/c=0xffffff, line_width=%d, single_line=2, font, font_height=14, \
							font_bold=1, font_face='Open Sans'/%s"), TITLE_WIDTH, sTitle.c_str()) ;

	m_pIcon->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("NOTIFY_ICON_ALERT"))->GetObjectId());

	if (*m_sData.sLink == 0)
	{
		m_pTextContent->Format(_T("/c=0xa7a7a7, linkl=0, line_width=%d, height_limit=%d, font, font_height=13, \
								  font_face='Open Sans'/%s"), CONTENT_WIDTH, CONTENT_HEIGHT, sContent.c_str()) ;
	}
	else
	{
		m_pTextContent->Format(_T("/a, c=0xa7a7a7, line_width=%d, height_limit=%d, font, font_height=13, \
								  font_face='Open Sans'/%s"), CONTENT_WIDTH, CONTENT_HEIGHT, sContent.c_str()) ;
	}

	if (*m_sData.sLink != 0)
	{
		m_pTextContent->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemNotifyNew::OnClickContent) ;
	}
}

void ListItemNotifyNew::OnClickContent(ISonicBase *, LPVOID pReserve)
{
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg == NULL)
	{
		return;
	}
	_tstring sUrl;
	sUrl = m_sData.sLink;
	if (!sUrl.empty())
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sUrl.c_str());
		core_msg_track stTrack;
		lstrcpyn(stTrack.csTrack, NOTIFY_CLICK, 256);
		stTrack.dwId = _ttoi64(m_sData.sNotifId);
		COverlayImpl::GetInstance()->SendCoreMsg(&stTrack);
		pDlg->PostMessage(WM_CLOSE);
	}
	
}

///////////////////////////////////////////////////////////////////////////
//friend status: login status and play game status

ListItemFRStatusNew::ListItemFRStatusNew()
{
	m_pText = GetSonicUI()->CreateString();
}

ListItemFRStatusNew::~ListItemFRStatusNew()
{
	GetSonicUI()->DestroyObject(m_pText);
}



BOOL ListItemFRStatusNew::InsertItem(CTempPopUp* pDlg, PVOID psData)
{
	BOOL bRet = TRUE ;

	if (NULL == pDlg)
	{
		return FALSE ;
	}

	InitItems(pDlg, psData); 
		
	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(pDlg->m_hWnd)->GetPaint();
	if (pPaint != NULL)
	{
		CRect rect;
		pDlg->GetClientRect(rect);
		int x = (rect.Width() - m_pText->GetWidth()) / 2;
		int y = (rect.Height() - m_pText->GetHeight()) / 2;
		pPaint->AddObject(m_pText->GetObjectId(), x, y, FALSE);
	}
	return bRet ;
}

void ListItemFRStatusNew::InitItems(CTempPopUp* pDlg, PVOID psData)
{
	core_msg_FRStatus* pData = (core_msg_FRStatus*)psData ;
	m_sData = *pData;
	if (!pData->nType)
	{
		m_pText->Format(__T("/align=0x10, c=0xa7a7a7, line_width=%d, font, font_height=14,")
						   _T("font_face='Open Sans'/%s /a, c=%x, linkl=1, linkc=1, font, font_height=14,")
						   _T("font_face='Open Sans'/%s/c=0xa7a7a7, font, font_height=14,")
						   _T("font_face='Open Sans'/ %s"), 300, COverlayImpl::GetInstance()->GetMsgCookie()->csYourFriend, LINK_COLOR, m_sData.sNickName, \
						   COverlayImpl::GetInstance()->GetMsgCookie()->csOnLine) ;
		SetType(LOGIN_STATUS);
	}
	else if (pData->nType== 1)
	{
		_tstring szContent = _tstring(COverlayImpl::GetInstance()->GetMsgCookie()->csYourFriend) + _T(" ") +  m_sData.sNickName + _T(" ") + \
			_tstring(COverlayImpl::GetInstance()->GetMsgCookie()->csPlayGame); 
 		m_pText->Format(_T("/align=0x10, c=0xa7a7a7, line_width=%d, font, font_height=14, font_face='Open Sans'/%s/br/")
			_T(" /a, c=%x, linkl=1, linkc=1, font, font_height=14, font_face='Open Sans'/%s"), 
 						   300, szContent.c_str(), LINK_COLOR, pData->sGameName) ;
		SetType(PLAYGAME_STATUS);
	}

	m_pText->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemFRStatusNew::OnClickContent);


}

void ListItemFRStatusNew::OnClickContent(ISonicBase*, LPVOID pReserve)
{
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg == NULL)
	{
		return;
	}
	_tstring sUrl = m_sData.sGameUrl;
	if (!sUrl.empty())
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sUrl.c_str());
		pDlg->PostMessage(WM_CLOSE);
	}	
}

////////////////////////////////////////////////////////////////////////////////////////
///game ready to play
ListItemGameReadyNew::ListItemGameReadyNew()
{
	m_pIcon      = GetSonicUI()->CreateString();
	m_pTitle     = GetSonicUI()->CreateString();
	m_pText      = GetSonicUI()->CreateString();
}

ListItemGameReadyNew::~ListItemGameReadyNew()
{
	GetSonicUI()->DestroyObject(m_pIcon);
	GetSonicUI()->DestroyObject(m_pTitle);
	GetSonicUI()->DestroyObject(m_pText);
}


BOOL ListItemGameReadyNew::InsertItem(CTempPopUp* pDlg, PVOID psData)
{
	BOOL bRet = TRUE ;

	if (NULL == pDlg)
	{
		return FALSE ;
	}

	InitItems(pDlg, psData); 

	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(pDlg->m_hWnd)->GetPaint();
	if (pPaint != NULL)
	{
		pPaint->AddObject(m_pIcon->GetObjectId(), 25, 30, FALSE);
		pPaint->AddObject(m_pTitle->GetObjectId(), 75, 25, FALSE);
		pPaint->AddObject(m_pText->GetObjectId(), 75, 25 + m_pTitle->GetHeight() + 5, FALSE);
	}
	return bRet ;
}

void ListItemGameReadyNew::InitItems(CTempPopUp* pDlg, PVOID psData)
{
	core_msg_ready_play* pData = (core_msg_ready_play*)psData ;
	m_sData = *pData;
	m_pTitle->Format(_T("/a, c=0xffffff, font, font_bold=1, font_height=14, font_face='Open Sans'/%s"),
		m_sData.szGameName);

	m_pText->Format(_T("/c=0xacacac, font, font_height=13, font_face='Open Sans'/%s"),
		m_sData.szContent);

	ISonicImage *pImage = NULL;
	_tstring sGameLogoPath = m_sData.szGameLogoPath;
	if (sGameLogoPath.empty())
	{
		_tstring sIconFile;
		_tstring sAbbr = m_sData.szGameAbbr;
		sIconFile.Format(1, _T("NOTIFY_ICON_%s"), sAbbr.MakeUpper().c_str());
		pImage = GetCoreUI()->GetImage(sIconFile.c_str());
	}
	else
	{
		pImage = GetSonicUI()->CreateImage();
		pImage->Load(sGameLogoPath.c_str());
	}
	
	if (pImage)
	{
		m_pIcon->Format(_T("/p=%d/"), pImage->GetObjectId());
	}

	m_pTitle->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemGameReadyNew::OnClickContent);
}

void ListItemGameReadyNew::OnClickContent(ISonicBase*, LPVOID pReserve)
{
	_tstring sProductPage = m_sData.szGameProPage;
	if (!sProductPage.empty() && pReserve)
	{
		COverlayImpl::GetInstance()->ShowOverlay(TRUE);
		COverlayImpl::GetInstance()->ShowBrowserMainWnd(sProductPage.c_str());
		CTempPopUp* pTempPop = (CTempPopUp*)pReserve;
		pTempPop->PostMessage(WM_CLOSE);
	}
	
}
///////////////////////////////////////////////////////
///ListItemImMessage

ListItemImMessage::ListItemImMessage()
{
	m_pIcon   = GetSonicUI()->CreateString();
	m_pTitle  = GetSonicUI()->CreateString();
	m_pContent = GetSonicUI()->CreateString();
}
ListItemImMessage::~ListItemImMessage()
{
	//SONICUI_DESTROY(m_pText);
	GetSonicUI()->DestroyObject(m_pIcon);
	GetSonicUI()->DestroyObject(m_pTitle);
	GetSonicUI()->DestroyObject(m_pContent);
}
BOOL ListItemImMessage::InsertItem(CTempPopUp* pDlg, PVOID psData)
{
	BOOL bRet = TRUE ;

	if (NULL == pDlg)
	{
		return FALSE ;
	}

	InitItems(pDlg, psData); 

	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(pDlg->m_hWnd)->GetPaint();
	if (pPaint != NULL)
	{
		pPaint->AddObject(m_pIcon->GetObjectId(), 25, 30, FALSE);
		pPaint->AddObject(m_pTitle->GetObjectId(), 75, 25, FALSE);
		pPaint->AddObject(m_pContent->GetObjectId(), 75, 25 + m_pTitle->GetHeight() + 5, FALSE);
	}
	return bRet ;
}
void ListItemImMessage::InitItems(CTempPopUp* pDlg, PVOID psData)
{
	im_msg_rcv_msg *pData = (im_msg_rcv_msg*)psData;
	m_strSendUserName = pData->msg._szUserName; 

	m_pIcon->Format(_T("/p=%d/"), GetCoreUI()->GetImage(_T("NOTIFY_ICON_CHAT"))->GetObjectId());
	m_pTitle->Format(__T("/a, c=0xffffff, single_line=2, line_width=%d, font, font_bold=1, font_height=14,")
		_T("font_face='Open Sans'/%s"), TITLE_WIDTH,pData->msg._szSenderNick) ;
	m_pContent->Format(_T("/a, c=0xa7a7a7, line_width=%d, height_limit=%d, font, font_height=13, font_face='Open Sans'/%s"),
		               CONTENT_WIDTH, CONTENT_HEIGHT, pData->msg._szContent);

	m_pContent->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemImMessage::OnClickContent);
	m_pTitle->Delegate(DELEGATE_EVENT_CLICK, pDlg, this, &ListItemImMessage::OnClickContent);

}
void ListItemImMessage::OnClickContent(ISonicBase *, LPVOID pReserve)
{
	COverlayImpl::GetInstance()->ShowOverlay(TRUE);
	COverlayImImpl::GetInstance()->ShowChatDlgForMsg(m_strSendUserName.c_str());
	COverlayImImpl::GetInstance()->RemoveImMessage(m_strSendUserName.c_str());
	CTempPopUp* pDlg = (CTempPopUp*)pReserve;
	if (pDlg)
	{
		pDlg->PostMessage(WM_CLOSE);
	}
 //showchat;

}



///////////////////////////////////////////////////////////////////////////
///CTempPopUp

CTempPopUp::CTempPopUp():
m_bMouseTrack(TRUE),
m_pDataPtr(NULL),
m_nWidth(0),
m_nHeight(0)
{

}

CTempPopUp::~CTempPopUp()
{
}

int CTempPopUp::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(GetCoreUI()->AttachWindow(m_hWnd, _T("notify_temp_popup")) == FALSE)
	{
		return 0;
	}

	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	ISonicImage* pImageTran = GetCoreUI()->GetImage(_T("BG_TRAN"));
	if(pImageTran)
		pPaint->AddObject(pImageTran->GetObjectId(), 5, 5, FALSE);

	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd, _T("notify_temp_popup_btn_close"))-> \
		Delegate(DELEGATE_EVENT_CLICK, NULL, this, &CTempPopUp::OnClickClose);
	return 0;
}


BOOL CTempPopUp::InitInboxTipWnd(core_msg_inbox& stInboxData)
{
	ListItemInboxNew* pInboxData = new ListItemInboxNew;
	if(pInboxData->InsertItem(this, &stInboxData))
	{
		m_pDataPtr = pInboxData;
		m_nType = INBOX_MESSAGE;
		return TRUE;
	}
	return FALSE;

}

BOOL CTempPopUp::InitFriTipWnd(core_msg_friendreq& stFriReqData)
{
	ListItemFriReqNew* pFriendReqData = new ListItemFriReqNew;
	if(pFriendReqData->InsertItem(this, &stFriReqData))
	{
		m_pDataPtr = pFriendReqData;
		m_nType = FRIEND_REQUEST;
		return TRUE;
	}
	return FALSE;

} 

BOOL CTempPopUp::InitNotifyTipWnd(core_msg_notification& stNotifyData)
{
	ListItemNotifyNew* pNotifyData = new ListItemNotifyNew;
	if (pNotifyData->InsertItem(this, &stNotifyData))
	{
		m_pDataPtr = pNotifyData;
		m_nType = NOTIFICATION;
		return TRUE;
	}
	return FALSE;

}

BOOL CTempPopUp::InitFRStatusWnd(core_msg_FRStatus& stFRStatus)
{
	ListItemFRStatusNew* pFRStatusData = new ListItemFRStatusNew;
	if(pFRStatusData->InsertItem(this, &stFRStatus))
	{	
		m_pDataPtr = pFRStatusData;
		m_nType = FRIEND_STATUS;
		return TRUE;
	}
	return FALSE;
}

BOOL CTempPopUp::InitGameReadyPlayWnd(core_msg_ready_play& stGameReady)
{
	ListItemGameReadyNew* pGameReady = new ListItemGameReadyNew;
	if (pGameReady->InsertItem(this, &stGameReady))
	{
		m_pDataPtr = pGameReady;
		m_nType = GAME_READY_PLAY;
		return TRUE;
	}
	return FALSE;
}
BOOL CTempPopUp::InitImMessageWnd(im_msg_rcv_msg& stImMsg)
{
	ListItemImMessage* pImMessage = new ListItemImMessage;
	if (pImMessage->InsertItem(this, &stImMsg))
	{
		m_pDataPtr = pImMessage;
		m_nType = CHAT_MESSAGE;
		return TRUE;
	}
	return FALSE;
}

void CTempPopUp::OnClickClose(ISonicBase* pBase, LPVOID pReserve)
{
	DestroyWindow();
}


void CTempPopUp::OnTimer(UINT_PTR nIDEvent)
{
	CPoint ptCurPos;
	GetSonicUI()->GetCursorPosInWindow(m_hWnd, &ptCurPos);
	if (ptCurPos.x > 0 && ptCurPos.y > 0)
	{
		if (m_bMouseTrack)
		{
			KillTimer(TIMER2);
			m_bMouseTrack = FALSE;
		}	
	}
	else
	{
		if (m_bMouseTrack == FALSE)
		{
			SetTimer(TIMER2, 4000, NULL);
			m_bMouseTrack = TRUE;
		}

	}
	if (nIDEvent == TIMER2)
	{
		DestroyWindow();
	}
}

void CTempPopUp::OnMouseMove(UINT nFlags, CPoint point)
{
	
}

void CTempPopUp::OnFinalMessage(HWND)    
{
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, POPUP_CLOSE_EVENT);
	if (hEvent)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
	delete m_pDataPtr;
	delete this;
}

BOOL CTempPopUp::ShowWindow(int nCmdShow)
{
	SetTimer(TIMER1, 100, NULL);
	SetTimer(TIMER2, 4000, NULL);

	return CWindow::ShowWindow(nCmdShow);
}

void CTempPopUp::OnResponseFriendReq(BOOL bAccept, BOOL bResult)
{

}

void CTempPopUp::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_pDataPtr == NULL)
	{
		return;
	}
	if (m_pDataPtr->GetClassType() == INBOX_MESSAGE || 
		m_pDataPtr->GetClassType() == NOTIFICATION ||
		m_pDataPtr->GetClassType() == CHAT_MESSAGE)
	{
		m_pDataPtr->OnClick(NULL, this);
	}

}