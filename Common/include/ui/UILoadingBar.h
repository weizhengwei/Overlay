/**
 * @file      UILoadingBar.h
   @brief     The header file for loading bar dialog.
   @author    shenhui@pwrd.com
   @data      2012/04/12
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
 * $LastChangedDate: 2012/07/24
 * $Revision: 01.01.00
 * $Id: 
 * $notes: improve ui egine
 */
#include "resource.h"
#pragma once
#include "ui\UIWindow.h"

// CUILoadingBar dialog
class CUILoadingBar : public CUIWindow
{
	DECLARE_DYNAMIC(CUILoadingBar)

public:
	CUILoadingBar(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUILoadingBar();

// Dialog Data
	enum { IDD = IDD_DIALOG_POP };

protected:
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	void OnWindowPosChanging(WINDOWPOS* lpwndpos) ;
	void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	

};
