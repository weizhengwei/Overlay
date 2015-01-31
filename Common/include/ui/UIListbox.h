/**
 * @file      UIListbox.h
   @brief     The header file for listbox base dialog.
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

#include "ui/UIWindow.h"

// CUIListbox dialog

class CUIListbox : public CUIWindow
{
	DECLARE_DYNAMIC(CUIListbox)

public:
	CUIListbox(UINT uId, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUIListbox();

	DECLARE_MESSAGE_MAP()

protected:
	WTL::CCoreListBox m_listBox;
};
