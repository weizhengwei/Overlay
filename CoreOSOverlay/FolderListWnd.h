#pragma once
#include "im/TabListWnd.h"
class CFolderListWnd : public  CTabListWnd
{
public:
	CFolderListWnd(TabListImplBase * pTabListImpl);
	~CFolderListWnd(void);
	DECLARE_WND_CLASS(_T("CFolderListWnd"))
	BEGIN_MSG_MAP_EX(CFolderListWnd)
		MSG_WM_CREATE(OnCreate)
		CHAIN_MSG_MAP(CTabListWnd)
	END_MSG_MAP()
protected:
		int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
