// CoreOSOverlay.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "CoreOSOverlayDlg.h"

// CCoreOSOverlayApp:
// See CoreOSOverlay.cpp for the implementation of this class
//

class CCoreOSOverlayApp : public CWinApp
{
public:
	CCoreOSOverlayApp();

// Overrides
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	CCoreOSOverlayDlg m_dlg;

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCoreOSOverlayApp theApp;