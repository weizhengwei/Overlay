/**
 * @file      UIWindowBase.h
   @brief     The header file for base class of list.
   @author    xushenghui@pwrd.com
   @data      2012/07/12
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
 * $LastChangedBy: 
 * $LastChangedDate: 
 * $Revision: 
 * $Id: 2
 * $notes:
 */


#pragma once

#include "ui/UIWindowBase.h"

class CUICoreList : public CUIWindowBase
{
public:
	CUICoreList(void) ;
	virtual ~CUICoreList(void);

protected:

	WTL::CCoreListBox m_listBox;
};
