#include "stdafx.h"
#include "skin/UIIconButton.h"

#define TIME_NID_SHOW_GIFFRAME    50

CUIIconButton::CUIIconButton(void)
{
	m_bDisable			= FALSE;
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;
	m_bSetCursor		= FALSE;
	m_bHandOnMove       = TRUE ;
	m_colorText         = 0x1f1f1f ;
	m_pBitmap           = NULL ;
	m_nFrameCount   =0;
	m_nCurFrame      =0;
	m_pPropertyItem =NULL;
}

CUIIconButton::~CUIIconButton(void)
{
	if(NULL!=m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL ;
	}
	if(NULL!=m_pPropertyItem)
	{
		delete  m_pPropertyItem;
		m_pPropertyItem=NULL;
	}
	KillTimer(TIME_NID_SHOW_GIFFRAME);
}

BEGIN_MESSAGE_MAP(CUIIconButton, CButton)
	//{{AFX_MSG_MAP(CKbcButton)
	ON_WM_TIMER( )
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_DESTROY( )
	ON_MESSAGE(WM_MOUSEHOVER,OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE,OnMouseLeave)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKbcButton message handlers

void CUIIconButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{	
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	Graphics    gDC(pDC->m_hDC);

	GetClientRect(&m_rectButton);

	if (m_pBitmap != NULL)
	{
		//gDC.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		gDC.DrawImage(m_pBitmap,m_rectButton.left,m_rectButton.top,m_rectButton.right,m_rectButton.bottom);
	}
	//ReleaseDC(pDC);

	return;
}

void CUIIconButton::SetButtonImage(LPCTSTR lpszFileName,UINT nMask)
{
	HRESULT  hResult;

	String::_tstring         szPath=lpszFileName;
	m_nMask = nMask;

	if(NULL!=m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap=NULL;
	}

	m_pBitmap=Bitmap::FromFile(szPath.toWideString().c_str());
	if(NULL==m_pBitmap)
	{
		TRACE(_T("Load icon file error,invalid image object"));
	}

	///Get frame count for .gif file
	UINT count=0;  
	count=m_pBitmap->GetFrameDimensionsCount();  
	GUID *pDimensionIDs=(GUID*)new GUID[count];  
	m_pBitmap->GetFrameDimensionsList(pDimensionIDs,count);  
	WCHAR strGuid[39];  
	StringFromGUID2(pDimensionIDs[0],strGuid,39);  
	m_nFrameCount=m_pBitmap->GetFrameCount(&pDimensionIDs[0]);  
	delete []pDimensionIDs;  
	if(m_nFrameCount>1)    ///file is a .gif file
	{
		///get frame delay
		int nSize = m_pBitmap->GetPropertyItemSize(PropertyTagFrameDelay);  
		m_pPropertyItem = (PropertyItem*) malloc(nSize); 
		m_pBitmap->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);   

		SetTimer(TIME_NID_SHOW_GIFFRAME,100,NULL);
	}

	CWnd *pWnd = this->GetParent();
	GetWindowRect(&m_rectButtonPos);
	pWnd->ScreenToClient(m_rectButtonPos);

	MoveWindow(m_rectButtonPos);

	return;
}

void CUIIconButton::SetButtonImage(Bitmap* pBitmap,UINT nMask)
{
	HRESULT  hResult;

	m_nMask = nMask;

	if(NULL!=m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap=NULL;
	}

	m_pBitmap=pBitmap;
	if(NULL==m_pBitmap)
	{
		TRACE(_T("Load icon file error,invalid image object"));
	}

	///Get frame count for .gif file
	UINT count=0;  
	count=m_pBitmap->GetFrameDimensionsCount();  
	GUID *pDimensionIDs=(GUID*)new GUID[count];  
	m_pBitmap->GetFrameDimensionsList(pDimensionIDs,count);  
	WCHAR strGuid[39];  
	StringFromGUID2(pDimensionIDs[0],strGuid,39);  
	m_nFrameCount=m_pBitmap->GetFrameCount(&pDimensionIDs[0]);  
	delete []pDimensionIDs;  
	if(m_nFrameCount>1)    ///file is a .gif file
	{
		///get frame delay
		int nSize = m_pBitmap->GetPropertyItemSize(PropertyTagFrameDelay);  
		m_pPropertyItem = (PropertyItem*) malloc(nSize); 
		m_pBitmap->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);   

		SetTimer(TIME_NID_SHOW_GIFFRAME,100,NULL);
	}

	CWnd *pWnd = this->GetParent();
	GetWindowRect(&m_rectButtonPos);
	pWnd->ScreenToClient(m_rectButtonPos);

	MoveWindow(m_rectButtonPos);

	return;

}

void CUIIconButton::OnTimer(UINT_PTR nIDEvent)
{
	long    lPause=0;
	const GUID   Guid=FrameDimensionTime;
	switch(nIDEvent)
	{
	case TIME_NID_SHOW_GIFFRAME:
		m_pBitmap->SelectActiveFrame(&Guid,m_nCurFrame++);  
		if(m_nCurFrame==m_nFrameCount)
			m_nCurFrame=0;  

		lPause=((long*)m_pPropertyItem->value)[m_nCurFrame]*10;  
		KillTimer(TIME_NID_SHOW_GIFFRAME);  
		SetTimer(TIME_NID_SHOW_GIFFRAME,lPause,NULL);  
		InvalidateRect (NULL, FALSE) ;  
		break;
	default:
		break;
	}
	return;
}


void CUIIconButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	// we wanna get WM_MOUSEHOVER,WM_MOUSELEAVE message, so we call _TrackMouseEvent
	if( m_bCursorOnButton == FALSE )
	{
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme,sizeof(TRACKMOUSEEVENT));
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE|TME_HOVER;
		tme.dwHoverTime = 1;
		m_bCursorOnButton = _TrackMouseEvent(&tme);
	}

	if (m_bHandOnMove)
	{
		::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_HAND)));
	}

	return CButton::OnMouseMove(nFlags,point);
}


LRESULT CUIIconButton::OnMouseHover(WPARAM wparam, LPARAM lparam)
{
	m_bHover = TRUE;

	Invalidate();
	return 0L;
}

LRESULT CUIIconButton::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	
	m_bCursorOnButton	= FALSE;
	m_bHover			= FALSE;
	CRect    rect;

	GetClientRect(&rect);
	ClientToScreen(&rect);

	if (m_bHandOnMove)
	{
		::SetCursor(::LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW)));
	}

//	Invalidate();
	GetParent()->ScreenToClient(&rect);
	GetParent()->InvalidateRect(rect);
	return 0L;
}

void CUIIconButton::SetButtonEnable()
{
	m_bDisable = FALSE;	
	Invalidate();
}

void CUIIconButton::SetButtonDisable()
{
	m_bDisable = TRUE;	
	Invalidate();
}

void CUIIconButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// if button state is disable we do not call CButton::OnLButtonDown funtion..
	if( !m_bDisable )
		CButton::OnLButtonDown(nFlags, point);
}

BOOL CUIIconButton::PreTranslateMessage(MSG* pMsg) 
{
	// to showing a tooltip message, call RelayEvent if m_ToolTip has created.
	if( m_ToolTip.GetSafeHwnd() != NULL )
		m_ToolTip.RelayEvent(pMsg);

	return CButton::PreTranslateMessage(pMsg);
}

void CUIIconButton::SetToolTipText(LPCTSTR lpszText)
{
	if(NULL==m_ToolTip.GetSafeHwnd( ))
	{
		m_ToolTip.Create(this);
	}
	m_ToolTip.Activate(TRUE);
	m_ToolTip.AddTool(this,lpszText);
}

BOOL CUIIconButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if( m_bSetCursor )
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	else
		return CButton::OnSetCursor(pWnd,nHitTest,message);
}

void CUIIconButton::SetCursor(HCURSOR hCursor)
{
	m_bSetCursor	= TRUE;
	m_hCursor		= hCursor;
}

BOOL CUIIconButton::OnEraseBkgnd(CDC* pDC) 
{
	// do not draw back ground. just return here, so we can see the background.
	// if you have curiosity about this, then remove ON_WM_ERASEBKGND() and test..
	return TRUE;
}

void CUIIconButton::OnDestroy()
{
//	delete m_pBitmap;
	return CButton::OnDestroy();
}


