#pragma once


// CUIEditBorder

#pragma once

class CUIEditBorder : public CEdit
{
	DECLARE_DYNAMIC(CUIEditBorder)

public:
	CUIEditBorder();
	virtual ~CUIEditBorder();
	virtual void OnPaint( );

	void SetHookKeyBoard(BOOL bHookKeyBoard = TRUE)
	{
		m_bHookKeyBoard = bHookKeyBoard ;
	} ;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	BOOL m_bHookKeyBoard ;
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};


