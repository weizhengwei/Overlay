#pragma once
#include "tstring/tstring.h"
using String::_tstring;
class COverlayNotifyWnd : public CWindowImpl<COverlayNotifyWnd>
{
public:
	COverlayNotifyWnd(void);
	~COverlayNotifyWnd(void);

	DECLARE_WND_CLASS(_T("COverlayNotifyWnd"))

	BEGIN_MSG_MAP_EX(COverlayNotifyWnd)
		MSG_WM_CREATE(OnCreate)
	//	MSG_WM_SIZE(OnSize)
	    MSG_WM_TIMER(OnTimer)
		END_MSG_MAP()
public:
	void SetText(LPCTSTR csText,LPCTSTR csTitle,LPCTSTR csLink,int nNotifyID);
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnTimer(UINT_PTR nIDEvent);
	void OnLink(ISonicBase*,LPVOID pReserve);
	ISonicString *m_pTitle;
	ISonicString *m_pText;
	_tstring m_strLink;
	int m_nNotifyID;

};
