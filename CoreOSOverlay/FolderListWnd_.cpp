#include "FolderListWnd.h"

CFolderListWnd::CFolderListWnd(TabListImplBase * pTabListImpl)
:CTabListWnd(pTabListImpl)
{
}

CFolderListWnd::~CFolderListWnd(void)
{
}
int CFolderListWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,_T("tab_folderlistwnd"));
	m_pListBox = GetCoreUI()->GetObjByID<CCoreListBox>(m_hWnd,_T("tab_list"));
	m_pListBox->SetSelectColor(ARGB(255,31,32,36));
	m_pListBox->SetHotItemColor(-1);
	m_pListBox->SetBGColor(ARGB(255,21,21,23));
	HWND hScroll = m_pListBox->GetCoreScrollWnd(SB_VERT);
	ISonicSkin* pScrollSkin = GetSonicUI()->SkinFromHwnd(hScroll);
	pScrollSkin->GetPaint()->SetBkColor(ARGB(255,32,36,44));
	m_pListBox->SetScrollImage(NULL, NULL, GetCoreUI()->GetImage(_T("SCROLLBAR_CHANNEL")), GetCoreUI()->GetImage(_T("SCROLLBAR_THUMB")));
	///COverlayImpl::GetInstance()->SetListBox(m_pListBox);
//	SetMsgHandled(FALSE);
	return 0;
}
