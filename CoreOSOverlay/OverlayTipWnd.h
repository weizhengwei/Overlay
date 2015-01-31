#pragma once
#include "tstring/tstring.h"
using String::_tstring;
class COverlayTipWnd : public CWindowImpl<COverlayTipWnd>
{
public:
	COverlayTipWnd(void);
	~COverlayTipWnd(void);
	DECLARE_WND_CLASS(_T("COverlayTipWnd"))

	BEGIN_MSG_MAP_EX(COverlayTipWnd)
		MSG_WM_CREATE(OnCreate)
		//	MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)
		END_MSG_MAP()
public:
	void SetTitle(_tstring & strTitle);
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnClickClose(ISonicBase *, LPVOID );
	void OnTimer(UINT_PTR nIDEvent);
	ISonicString* m_pText;
	
};
