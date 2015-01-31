// BitmapSlider.cpp : implementation file

#include "stdafx.h"
#include <atlimage.h>
#include "skin/UIBitmapSlider.h"
#include "tstring/tstring.h"

CUIBitmapSlider::CUIBitmapSlider()
{
	m_nPos = m_nMin = 0;
	m_nMax = 100;

	m_pbmChannel = NULL ; //progress background image
	m_pbmActive = NULL ; //progress active image
}

CUIBitmapSlider::~CUIBitmapSlider()
{
	if (m_pbmChannel)
	{
		delete m_pbmChannel ;
	}

	if (m_pbmActive)
	{
		delete m_pbmActive ;
	}
}


BEGIN_MESSAGE_MAP(CUIBitmapSlider, CStatic)
	//{{AFX_MSG_MAP(CUIBitmapSlider)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUIBitmapSlider message handlers

BOOL CUIBitmapSlider::OnEraseBkgnd(CDC* pDC) 
{	
	// Do not erase background for the transparency effect
	return TRUE;
}

// Draw channel and thumb
//
void CUIBitmapSlider::OnPaint() 
{
	CPaintDC dcOrigin(this);

	HRESULT  hResult;
	IStream       *pIStream=NULL;
	Graphics     gDc(dcOrigin.m_hDC);

	if ((m_pbmChannel != NULL) && (m_pbmActive != NULL))
	{
		///set progress background
		gDc.DrawImage(m_pbmChannel,0, 0, m_pbmChannel->GetWidth(), m_pbmChannel->GetHeight());

		///compute progress active posistion
		int nPos = (int)(m_pbmActive->GetWidth()*((double)(m_nPos - m_nMin)/(m_nMax - m_nMin)));

		///set active position image
		gDc.DrawImage(m_pbmActive, 2, 2, nPos, m_pbmActive->GetHeight());
	}
}

void CUIBitmapSlider::SetRangeMax(int nMax, BOOL bRedraw)
{
	m_nMax = nMax;
	if( bRedraw )
		Invalidate();
}

void CUIBitmapSlider::SetRangeMin(int nMin, BOOL bRedraw)
{
	m_nMin = nMin;
	if( bRedraw )
		Invalidate();
}

void CUIBitmapSlider::SetRange(int nMin, int nMax, BOOL bRedraw)
{
	SetRangeMin( nMin, FALSE );
	SetRangeMax( nMax, bRedraw );
}

void CUIBitmapSlider::SetPos(int nPos)
{
	if (m_nPos == nPos)
	{
		return ;
	}

	m_nPos = nPos;

	// Boundary check
	if( m_nPos > m_nMax )
		m_nPos = m_nMax;
	if( m_nPos < m_nMin )
		m_nPos = m_nMin;

	Invalidate();
}

BOOL CUIBitmapSlider::SetBitmapChannel(LPCTSTR lpszChannelFilePath, LPCTSTR lpszActiveFilePath)
{
	HRESULT  hResult;
	IStream  *pIStream=NULL;
	String::_tstring   tsPath=lpszChannelFilePath;

	///create progress background image
	
	m_pbmChannel =Bitmap::FromFile(tsPath.toWideString().c_str());

	///create progress image
	tsPath=lpszActiveFilePath;
	m_pbmActive = Bitmap::FromFile(tsPath.toWideString().c_str());

	return TRUE;
}

BOOL CUIBitmapSlider::SetToolTipText(LPCTSTR lpszText, BOOL bActivate)
{
	InitToolTip(); 
	
	// If there is no tooltip defined then add it
	if (m_ttTooltip.GetToolCount() == 0)
	{
		CRect rect; 
		GetClientRect(rect);
		m_ttTooltip.AddTool(this, lpszText, rect, 1);
	}

	try
	{
		m_ttTooltip.UpdateTipText(lpszText, this, 1);
		m_ttTooltip.Activate(bActivate);
	}
	catch (CException* e)
	{
		return FALSE;
	}
	return TRUE ;
}

BOOL CUIBitmapSlider::InitToolTip()
{
	if (m_ttTooltip.m_hWnd == NULL) {
		m_ttTooltip.Create(this);
		m_ttTooltip.Activate(TRUE);
		m_ttTooltip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
	}
	
	return TRUE;
}

BOOL CUIBitmapSlider::ActivateToolTip(BOOL bActivate)
{
	if (m_ttTooltip.GetToolCount() == 0)
		return false;
	
	// Activate tooltip
	m_ttTooltip.Activate(bActivate);
	
	return true;
}

BOOL CUIBitmapSlider::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	InitToolTip();
	m_ttTooltip.RelayEvent(pMsg);

	return CStatic::PreTranslateMessage(pMsg);
}
