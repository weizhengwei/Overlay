/**
 * @file      UIWindow.cpp
   @brief     The implementation file for ui base dialog.
   @author    shenhui@pwrd.com
   @data      2012/07/18
   @version   01.00.00
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
 * $Id: 2
 * $notes:
 */

#include "stdafx.h"
#include "ui/UIWindow.h"

// CUIWindow dialog

IMPLEMENT_DYNAMIC(CUIWindow, CDialog)

CUIWindow::CUIWindow(UINT uId, CWnd* pParent /*=NULL*/)
	: CDialog(uId, pParent){}

CUIWindow::~CUIWindow(){}

BEGIN_MESSAGE_MAP(CUIWindow, CDialog)
	//ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

BOOL CUIWindow::ShowWindow(int nCmdShow)
{
	if (nCmdShow == SW_SHOW || nCmdShow == SW_SHOWNORMAL)
	{
		ISonicWndEffect* pEffect = GetSonicUI()->EffectFromHwnd(m_hWnd) ;
		if (pEffect)
		{
			pEffect->ShowGently(TRUE, TRUE) ;
			return TRUE;
		}
	}

	return CDialog::ShowWindow(nCmdShow) ; 
}

BOOL CUIWindow::OnInitDialog()
{
	CDialog::OnInitDialog() ;

	//ISonicWndEffect* pEffect = GetSonicUI()->EffectFromHwnd(m_hWnd) ;
	//if (pEffect)
	//{
	//	pEffect->ShowGently(TRUE, TRUE) ;
	//}

	return TRUE ;
}

void CUIWindow::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanging(lpwndpos) ;

	if (lpwndpos && (lpwndpos->flags & SWP_SHOWWINDOW))
	{//all of windows can be shown gently. note:do not do this in OnShowWindow,
	 //it is not a right place to do this job.
	 //do not do this job here to avoid stack overflow in CoreUI, especially 
	 //when skintype > 1
		//ISonicWndEffect* pEffect = GetSonicUI()->EffectFromHwnd(m_hWnd) ;
		//if (pEffect)
		//{
		//	pEffect->ShowGently(TRUE, TRUE) ;
		//}
	}
}

void CUIWindow::WinHelp(DWORD dwData, UINT nCmd)
{
	// TODO: Add your specialized code here and/or call the base class
	return ; //diable F1 to popup help

	CDialog::WinHelp(dwData, nCmd);
}
BOOL CUIWindow::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN )
	{
		//contrls focus change for tab
		if (pMsg->wParam == VK_TAB && ::GetWindowLong(pMsg->hwnd,GWL_STYLE) & WS_TABSTOP)
		{
			HWND hDlg = m_hWnd;
			if ( GetSonicUI()->EffectFromHwnd(hDlg) && GetSonicUI()->EffectFromHwnd(hDlg)->IsAlphaPerPixel())
			{
				hDlg = ::GetParent(::GetParent(pMsg->hwnd));
			}
			HWND hwnd = ::GetNextDlgTabItem(hDlg,pMsg->hwnd,FALSE); 
			::SetFocus(hwnd);
			return TRUE;
		}
// 		else if (pMsg->wParam == VK_ESCAPE)
// 		{
// 			return TRUE;
// 		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}