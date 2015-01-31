#include "StdAfx.h"
#include "OverlayImImpl.h"
#include "OverlayImpl.h"

COverlayImImpl::COverlayImImpl(void)
{
}

COverlayImImpl::~COverlayImImpl(void)
{
	m_ImMessage.clear();
}
BOOL COverlayImImpl::InsertImMessage(_IMMESSAGE_INFO & ImMessage)
{
	list<COverlayImImpl::_IMMESSAGE_INFO>::iterator it = m_ImMessage.begin();
	list<COverlayImImpl::_IMMESSAGE_INFO>::iterator end = m_ImMessage.end();
	while(it != end)
	{
		if ((*it)._szUserName.CompareNoCase(ImMessage._szUserName) == 0)
		{
			//	m_ImMessage.remove((*it));
			//m_ImMessage.erase(it);
			if (ImMessage.nTimeCur - (*it).nTimeCur > 600000)
			{
				(*it).nTimeCur = ImMessage.nTimeCur;
				return TRUE;
			}
			return FALSE;	
		}
		it++;
		
	}
	m_ImMessage.push_back(ImMessage);
	return TRUE;
}
void COverlayImImpl::RemoveImMessage(LPCTSTR lpUserName)
{
	list<COverlayImImpl::_IMMESSAGE_INFO>::iterator it = m_ImMessage.begin();
	while(it !=  m_ImMessage.end())
	{
		if ((*it)._szUserName.CompareNoCase(lpUserName) == 0)
		{
		//	m_ImMessage.remove((*it));
			m_ImMessage.erase(it);
			return;	
		}
		it++;
	}
}

void COverlayImImpl::ShowFriendDlg(CRect &rc)
{
	im_dialog_params dlgparms;
	dlgparms.nShowCmd = SW_SHOWNA;
	dlgparms.pParentWnd = COverlayImpl::GetInstance()->GetMainWnd();
	dlgparms.rt.left = rc.left;
	dlgparms.rt.top = rc.top;
	dlgparms.rt.right = rc.right;
	dlgparms.rt.bottom = rc.bottom;
	dlgparms.bCreated = true;
	GetUIChatMgr()->ShowFriendDlg(dlgparms);
}

void COverlayImImpl::ShowStatusDlg(CRect &rc,BOOL bShow)
{
	im_dialog_params dlgparms;
	if (bShow)
	{
		dlgparms.nShowCmd = SW_SHOWNA;
	}
	dlgparms.pParentWnd = COverlayImpl::GetInstance()->GetMainWnd();
	dlgparms.rt.left = rc.left;
	dlgparms.rt.top = rc.top;
	dlgparms.rt.right = rc.right;
	dlgparms.rt.bottom = rc.bottom;
	dlgparms.bCreated = true;
	GetUIChatMgr()->ShowStatusDlg(dlgparms);
}
bool COverlayImImpl::InitIM()
{
	GetUIChatMgr()->InitChatSystem(COverlayImpl::GetInstance()->GetMainWnd()->m_hWnd);

	ITEM_BASE_INFO info;
	if (GetUIChatMgr()->GetSelfUserInfo(&info))
	{
		GetUIChatMgr()->SetUserName(info._szUser);
		GetUIChatMgr()->SetNickName(info._szNick);
		GetUIChatMgr()->SetSelfAvatarPath(info._szAvatarPath);
		_tstring StrPath = GetUIChatMgr()->GetBaseDir() + info._szAvatarPath;
		COverlayImpl::GetInstance()->HandleSelfAvatar(StrPath);
	}

	return true;
}
im_dialog_params COverlayImImpl::GetDefaultTabMgrParams()
{
	im_dialog_params params;
	params.pParentWnd = COverlayImpl::GetInstance()->GetMainWnd();
	if (::IsWindowVisible(params.pParentWnd->m_hWnd))
	{
		params.nShowCmd = SW_SHOWNA;
	}
	else
	{
		params.nShowCmd = SW_HIDE;
	}
	CRect rcWindow;
	::GetWindowRect(params.pParentWnd->m_hWnd,rcWindow);
    params.rt.left = rcWindow.left + 480;
    params.rt.top = rcWindow.top + 110;
    params.rt.bottom = params.rt.top + 522;
    params.rt.right = params.rt.left + 542;
    return params;
}
void COverlayImImpl::ShowOverlayChatDlg(BOOL bShow)
{
	if (bShow)
	{
		GetUIChatMgr()->ShowTabMgrChatDlg(SW_SHOWNA);
	}
	else
	{
		GetUIChatMgr()->ShowTabMgrChatDlg(SW_HIDE);
		if (GetUIChatMgr()->GetStatusDlg())
		{
			::ShowWindow(GetUIChatMgr()->GetStatusDlg()->m_hWnd,SW_HIDE);
		}
		
		
	}
}
void COverlayImImpl::ShowChatDlgForMsg(LPCTSTR lpUserName)
{
	ITEM_BASE_INFO info;
	if (GetUIChatMgr()->GetFriendInfo(lpUserName, &info))
	{
		GetUIChatMgr()->ShowChatDlg(info, FALSE, false);
	}
	
}
