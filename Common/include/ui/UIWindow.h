/**
 * @file      UIWindow.h
   @brief     The header file for ui base dialog.
   @author    shenhui@pwrd.com
   @data      2012/07/30
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

#pragma once

#include "ui/ICoreUI.h"
#include "ui/CoreWidget.h"
#include "ui/CoreEdit.h"

// CUIWindow dialog

class CUIWindow : public CDialog
{
	DECLARE_DYNAMIC(CUIWindow)

public:
	CUIWindow(UINT uId, CWnd* pParent = NULL);
	virtual ~CUIWindow();

	BOOL ShowWindow(int nCmdShow) ;

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog() ;
	void OnWindowPosChanging(WINDOWPOS* lpwndpos) ;
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK(){/*do nothing to avoid close of application.*/};
};