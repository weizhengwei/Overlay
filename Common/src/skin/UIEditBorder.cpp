// D:\Develop\Client\PWPClient\PWPClient\src\ui\UIEditBorder.cpp : implementation file
//

#include "stdafx.h"
#include "skin\UIEditBorder.h"
#include "data\Utility.h"

// CUIEditBorder

IMPLEMENT_DYNAMIC(CUIEditBorder, CEdit)

CUIEditBorder::CUIEditBorder()
{
	m_bHookKeyBoard = FALSE ;
}

CUIEditBorder::~CUIEditBorder()
{
}


BEGIN_MESSAGE_MAP(CUIEditBorder, CEdit)
	ON_WM_PAINT()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
END_MESSAGE_MAP()



// CUIEditBorder message handlers

void CUIEditBorder::OnPaint()
{
  CPaintDC dc(this); // device context for painting
    
         // TODO: Add your message handler code here
    CRect rect;
    GetWindowRect(rect);
    rect.left -= 1;
	rect.top -= 1;
    rect.right += 2;    
    rect.bottom += 2;
    
    ScreenToClient(rect);
    dc.Draw3dRect(rect, RGB(88, 88, 88), RGB(88, 88,88));
    Invalidate();
    Default();

}



void CUIEditBorder::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	Invalidate() ; //text will not overlap when inputting in editbox

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CUIEditBorder::PreTranslateMessage(MSG* pMsg)
{
	if (m_bHookKeyBoard && ((pMsg->message == WM_KEYDOWN) || pMsg->message == WM_SYSKEYDOWN))
    {
		if (pMsg->wParam == VK_TAB || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || \
			pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_APPS || pMsg->wParam == VK_MENU || pMsg->wParam == VK_F10)
			{
				::TranslateMessage (pMsg);
				::DispatchMessage (pMsg);
				return TRUE;		    	// DO NOT process further
			}
    }

    return CEdit::PreTranslateMessage (pMsg);
}

void CUIEditBorder::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (!m_bHookKeyBoard)
	{
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		return ;
	}

	UINT nFunc = 0;
	if (GetKeyState (VK_SHIFT) < 0)
	{
		nFunc = VK_SHIFT ;
	}
	else if (GetKeyState(VK_MENU) < 0)
	{
		nFunc = VK_MENU ;
	}
	else if (GetKeyState(VK_CONTROL) < 0)
	{
		nFunc = VK_CONTROL ;
	}

	_tstring sText = CUtility::GetKeyBoardTextFromVK(nFunc, nChar) ;
	if (!sText.empty())
	{
		SetWindowText(sText.c_str()) ;
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CUIEditBorder::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (!m_bHookKeyBoard)
	{
		CEdit::OnSysKeyDown(nChar, nRepCnt, nFlags);
		return ;
	}

	UINT nFunc = 0;
	if (GetKeyState (VK_SHIFT) < 0)
	{
		nFunc = VK_SHIFT ;
	}
	else if (GetKeyState(VK_MENU) < 0)
	{
		nFunc = VK_MENU ;
	}
	else if (GetKeyState(VK_CONTROL) < 0)
	{
		nFunc = VK_CONTROL ;
	}

	_tstring sText = CUtility::GetKeyBoardTextFromVK(nFunc, nChar) ;
	if (!sText.empty())
	{
		SetWindowText(sText.c_str()) ;
	}

	CEdit::OnSysKeyDown(nChar, nRepCnt, nFlags);
}
