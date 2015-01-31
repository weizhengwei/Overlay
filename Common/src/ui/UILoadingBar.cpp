/**
* @file    UILoadingBar.cpp
@brief     The implementation file for loading bar dialog used when web page is loading.
@author    shenhui@pwrd.com
@data      2011/04/12
@version   01.00.00
@notes     child window only
*/

/* 
*  Copyright(C) 2010. Perfect World Entertainment Inc
*
*  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
*  its licensors and is protected by copyright and other intellectual 
*  property laws and treaties.  We and our licensors own all rights, 
*  title and interest in and to the SOURCE CODE, including all copyright 
*  and other intellectual property rights in the SOURCE CODE.
*/

/*
* $LastChangedBy: shenhui@pwrd.com
* $LastChangedDate: 
* $Revision: 
* $Id: 
* $notes: 
*/

#include "stdafx.h"
#include "ui/UILoadingBar.h"
//#include "toolkit.h"

// CUILoadingBar dialog
IMPLEMENT_DYNAMIC(CUILoadingBar, CUIWindow)

CUILoadingBar::CUILoadingBar(CWnd* pParent /*=NULL*/)
	: CUIWindow(CUILoadingBar::IDD, pParent){}

CUILoadingBar::~CUILoadingBar(){}

BEGIN_MESSAGE_MAP(CUILoadingBar, CUIWindow)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

// CUILoadingBar message handlers
BOOL CUILoadingBar::OnInitDialog()
{
	CUIWindow::OnInitDialog() ;

	GetCoreUI()->AttachWindow(m_hWnd, _T("loadingbar_popup_dialog")) ;

//  	CRect rc ;
//  	GetWindowRect(&rc) ; ScreenToClient(&rc) ;
//  	CRgn rectRgn ;
//  	rectRgn.CreateRectRgn(rc.left+1, rc.top+1, rc.right-1, rc.bottom-1) ;
//  	SetWindowRgn(rectRgn,true) ;
// 	rectRgn.DeleteObject() ;

	return TRUE ;
}

void CUILoadingBar::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanging(lpwndpos) ;
}

void CUILoadingBar::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	if((lpwndpos->flags & SWP_SHOWWINDOW))
	{
		if (GetParent() && GetParent()->GetNextWindow() && GetParent()->GetNextWindow()->GetNextWindow())
		{
			SetWindowPos(GetParent()->GetNextWindow()->GetNextWindow(),0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE|SWP_NOSENDCHANGING);
		}
	}
	CDialog::OnWindowPosChanged(lpwndpos);
}

void CUILoadingBar::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CUIWindow::OnActivate(nState,pWndOther,bMinimized);
}