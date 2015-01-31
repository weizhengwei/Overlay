#pragma once

using WTL::CCoreEdit;
// COverlaySettingWnd dialog

class COverlaySettingWnd : public CWindowImpl<COverlaySettingWnd>
{
	

public:
	COverlaySettingWnd();   // standard constructor
	virtual ~COverlaySettingWnd();

	DECLARE_WND_CLASS(_T("COverlaySettingWnd"))

	BEGIN_MSG_MAP_EX(COverlaySettingWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT(OnPaint)
		END_MSG_MAP()
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnPaint(CDC hDc);
	void SaveKey();
	void InitEvent();
	void OnCloseClick(ISonicBase *, LPVOID pReserve);
	void OnSave(ISonicBase*,LPVOID pReserve);
	void OnCancel(ISonicBase*,LPVOID pReserve);
private:
	CCoreEdit* m_pIngameEditKey;
	//ISonicString *m_pText1;
	//ISonicString *m_pText2;
	ISonicImage* m_pImgTran;
};
