// TreeCtrlBT.cpp : implementation file
//

#include "stdafx.h"
#include "skin/TreeCtrlBT.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlBT

CTreeCtrlBT::CTreeCtrlBT()
{
	m_bEnableRowLine  = true;
	m_bEnableRootBk   = true;
	m_nImageHeight    = 0;
	m_nImageWidth     = 0;
	m_nCellWidth	  = 4;
	m_clrTextNormal   = RGB( 0xEE, 0xEE,0xEE );
	m_clrTextSelect	  = RGB( 0xEE, 0xEE,0xEE );
	m_clrRoot_from    = RGB( 0x80, 0xa0, 0xff );
	m_clrRoot_to      = RGB( 0x80, 0xa0, 0xff );

	m_hItemSelect     = NULL;
	m_hItemMouseMove  = NULL;
	m_pCurDrawItemMsg = NULL;

	m_ptOldMouse.x	  = -1;
	m_ptOldMouse.y    = -1;
	m_bUserSort=false;
	m_pNotifyWnd=NULL;

	m_FontRoots.CreateFont(
		12,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,                   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,			   // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		PROOF_QUALITY,             // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Tahoma") );              // lpszFacename

	m_FontChild.CreateFont(
		12,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                   // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,			   // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		PROOF_QUALITY,             // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Tahoma") );              // lpszFacename
}

CTreeCtrlBT::~CTreeCtrlBT()
{
	RELEASE_GDI_RESOURCE(m_bmpBackgroud);
	RELEASE_GDI_RESOURCE(m_FontRoots);
	RELEASE_GDI_RESOURCE(m_FontChild);

	if ( m_ImageList.GetSafeHandle() )		m_ImageList.DeleteImageList();

	ITEMMSG_MAP::iterator item = m_itmemsg_map.begin();
	for ( ; item != m_itmemsg_map.end(); item++)
	{
		delete item->second;
	}
}

BEGIN_MESSAGE_MAP(CTreeCtrlBT, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeCtrlBT)
	ON_WM_PAINT()
	ON_WM_SIZE()
//	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
//	ON_NOTIFY_REFLECT(NM_CLICK, &CTreeCtrlBT::OnNMClick)
//	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDBLCLK( )
//	ON_WM_MOUSEWHEEL()
//	ON_WM_MBUTTONUP()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)

//	ON_NOTIFY_REFLECT(NM_DBLCLK, &CTreeCtrlBT::OnNMDblclk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlBT message handlers

void CTreeCtrlBT::SetUserSort(bool bSort)
{
	m_bUserSort=bSort;
}

void   CTreeCtrlBT::SetNotifyWnd(CWnd  *pWnd)
{
	m_pNotifyWnd=pWnd;
}

void CTreeCtrlBT::PreSubclassWindow() 
{
	SetItemHeight(25);
	m_nItemHeight = 25;	//保存单元项目高度

	CTreeCtrl::PreSubclassWindow();
	EnableToolTips(TRUE);
}


void CTreeCtrlBT::OnSize(UINT nType, int cx, int cy) 
{
	CTreeCtrl::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	Calculate();
}

int CTreeCtrlBT::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
	RECT rect;

	UINT nFlags;
	HTREEITEM hitem = HitTest( point, &nFlags );
	if( nFlags & TVHT_ONITEM )
	{
		GetItemRect( hitem, &rect, false );
		pTI->hwnd = m_hWnd;
		pTI->uId = (UINT)hitem;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		pTI->rect = rect;
		return pTI->uId;
	}
	return -1;
}

//here we supply the text for the item 
BOOL CTreeCtrlBT::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	UINT nID = pNMHDR->idFrom;
	// Do not process the message from built in tooltip 
	if( nID == (UINT)m_hWnd &&
		(( pNMHDR->code == TTN_NEEDTEXTA && pTTTA->uFlags & TTF_IDISHWND ) ||
		( pNMHDR->code == TTN_NEEDTEXTW && pTTTW->uFlags & TTF_IDISHWND ) ) )
	return FALSE;

	// Get the mouse position
	const MSG* pMessage;
	CPoint pt;
	pMessage = GetCurrentMessage(); // get mouse pos 
	ASSERT ( pMessage );
	pt = pMessage->pt;
	ScreenToClient( &pt );

	UINT nFlags;
	HTREEITEM hitem = HitTest( pt, &nFlags ); //Get item pointed by mouse


	strTipText.Format( _T("%s"), GetItemText( (HTREEITEM ) nID));  //get item text
	if (m_bUserSort)
	{
		strTipText.Delete(0);
	}
		
	CClientDC dc(this);
	CSize sz = dc.GetTextExtent(strTipText);
    
	if (sz.cx <= m_rect.Width()-m_nToolTipCtrlWidth)
	{
		//the string not so long ,the tooltip not appeared.
		*pResult = 0;
		return TRUE;  
	}

	DWORD dw = GetItemData((HTREEITEM ) nID); //get item data

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, 80);
	else
		_mbstowcsz(pTTTW->szText, strTipText, 80);
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, 80);
	else
		lstrcpyn(pTTTW->szText, strTipText, 80);
#endif
	*pResult = 0;

	return TRUE;    // message was handled
}

HTREEITEM   CTreeCtrlBT::FindItem(HTREEITEM  item, LPCTSTR lpszStr)
{
	HTREEITEM  hfind=NULL; 
	HTREEITEM  hFirst=NULL; 
	HTREEITEM  hDel=NULL;
	
	if(item ==  NULL)
		return  NULL;  

	while(item!=NULL)  
	{  
		if(0==GetItemText(item).CompareNoCase(lpszStr)) 
		{
			if(NULL==hFirst)
				hFirst=item;	
			item = GetNextSiblingItem(item); 
			continue;
		}
		if(ItemHasChildren(item)) 
		{  
			item   =   GetChildItem(item); 
			hfind   =   FindItem(item,lpszStr);  
			if(hfind)
			{  
				return  hfind;  
			}  
			else  
				item = GetNextSiblingItem(GetParentItem(item));  
		}  
		else
		{
			hDel=item;
			item = GetNextSiblingItem(item);  
		}  
	}  
	return hFirst;  
}



void CTreeCtrlBT::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	//Calculate();

	CBitmap bitmap;
	CDC MemeDc;
	MemeDc.CreateCompatibleDC(&dc);
	bitmap.CreateCompatibleBitmap(&dc, m_rect.Width(), m_rect.Height());
	CBitmap *pOldBitmap = MemeDc.SelectObject(&bitmap);
	
	DrawBackgroudBitmap(&MemeDc);
	DrawItem(&MemeDc);

	dc.BitBlt( m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), &MemeDc, 0, 0, SRCAND);
	dc.BitBlt( m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), &MemeDc, 0, 0,SRCCOPY);
	//dc.BitBlt( m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(), &MemeDc, 0, 0, SRCCOPY);
	
	MemeDc.SelectObject(pOldBitmap);
	MemeDc.DeleteDC();
	bitmap.DeleteObject() ;
}

void CTreeCtrlBT::Calculate()
{
	GetClientRect(&m_rect);
	SCROLLINFO scroll_info;
	// Determine window portal to draw into taking into account
	// scrolling position
	if ( GetScrollInfo( SB_HORZ, &scroll_info, SIF_POS | SIF_RANGE ) )
	{
		m_h_offset = -scroll_info.nPos;
		m_h_size = max( scroll_info.nMax+1, m_rect.Width());
	}
	else
	{
		m_h_offset = m_rect.left;
		m_h_size = m_rect.Width();
	}
	if ( GetScrollInfo( SB_VERT, &scroll_info, SIF_POS | SIF_RANGE ) )
	{
		if ( scroll_info.nMin == 0 && scroll_info.nMax == 100) 
			scroll_info.nMax = 0;
		m_v_offset = -scroll_info.nPos * GetItemHeight();
		m_v_size = max( (scroll_info.nMax+2)*((int)GetItemHeight()+1), m_rect.Height() );
	}
	else
	{
		m_v_offset = m_rect.top;
		m_v_size = m_rect.Height();
	}
}

ITEM_MSG* CTreeCtrlBT::GetItemMsg(HTREEITEM hItem)
{
	if ( !hItem )
	{
		return NULL;
	}

	ITEM_MSG* pItemMsg = NULL;

	ITEMMSG_MAP::iterator item = m_itmemsg_map.find(hItem);
	if ( item != m_itmemsg_map.end() )
	{
		pItemMsg = item->second;
	}

	return pItemMsg;
}

void CTreeCtrlBT::GradientFillRect( CDC *pDC, CRect &rect, COLORREF col_from, COLORREF col_to, bool vert_grad )
{
	TRIVERTEX        vert[2];
	GRADIENT_RECT    mesh;
	
	vert[0].x      = rect.left;
	vert[0].y      = rect.top;
	vert[0].Alpha  = 0x0000;
	vert[0].Blue   = GetBValue(col_from) << 8;
	vert[0].Green  = GetGValue(col_from) << 8;
	vert[0].Red    = GetRValue(col_from) << 8;
	
	vert[1].x      = rect.right;
	vert[1].y      = rect.bottom; 
	vert[1].Alpha  = 0x0000;
	vert[1].Blue   = GetBValue(col_to) << 8;
	vert[1].Green  = GetGValue(col_to) << 8;
	vert[1].Red    = GetRValue(col_to) << 8;
	
	mesh.UpperLeft  = 0;
	mesh.LowerRight = 1;
#if _MSC_VER >= 1300  // only VS7 and above has GradientFill as a pDC member
	pDC->GradientFill( vert, 2, &mesh, 1, vert_grad ? GRADIENT_FILL_RECT_V : 
	GRADIENT_FILL_RECT_H );
#else
	GradientFill( pDC->m_hDC, vert, 2, &mesh, 1, vert_grad ? GRADIENT_FILL_RECT_V : 
	GRADIENT_FILL_RECT_H );
#endif	
}

void CTreeCtrlBT::DrawBackgroudBitmap(CDC* pDc)
{
	if ( m_bmpBackgroud.GetSafeHandle() )
	{
		BITMAP bitmap;
		m_bmpBackgroud.GetBitmap(&bitmap);
		CDC dctmp;
		
		dctmp.CreateCompatibleDC(NULL);
		CBitmap* pOldBitmap = dctmp.SelectObject( &m_bmpBackgroud ); 
		
		CRect rc;
		this->GetClientRect(&rc);
		pDc->StretchBlt(rc.left, rc.top, rc.Width(), rc.Height(), &dctmp, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY );

		dctmp.SelectObject(pOldBitmap);
	}
	else
	{
		pDc->FillSolidRect( m_rect, GetBkColor() ); 
	}
}

void CTreeCtrlBT::DrawItem( CDC *pDC )
{
	// draw items
	HTREEITEM show_item,parent; //当前项句柄，以及它的父窗口句柄
	CRect rc_item;				//项的区域
	DWORD tree_style;			//树的类型
	int state;					//某一项的状态
	bool selected;				//True:表示是需要高亮	
	bool has_children;			//True:表示是不可以展开或者收缩
	
	show_item = GetFirstVisibleItem();//获取第一个可见项的句柄
	if ( show_item == NULL )
		return;
	
	tree_style = ::GetWindowLong( m_hWnd, GWL_STYLE ); //获取TREE的类型
	
	do
	{
		state = GetItemState( show_item, TVIF_STATE );//获取某一项的状态
		parent = GetParentItem( show_item );		  //获取某一项的父窗口句柄
		
		has_children = ItemHasChildren( show_item ) ;
		
		selected = (state & TVIS_SELECTED) && ((this == GetFocus()) || 
			(tree_style & TVS_SHOWSELALWAYS));
		
		if ( GetItemRect( show_item, rc_item, false ) )//获取某一项的矩形区域
		{
			CRect rcclient;
			GetClientRect(rcclient);
			if (rc_item.top >= rcclient.bottom)
			{
				break;	//不可见的时候,就不需要再绘制
			}

//			rc_item.left=rc_item.left-10;
//			rc_item.right=rcclient.right-5;
		
			m_mapMutex.Lock( );
			m_pCurDrawItemMsg = GetItemMsg(show_item);
			m_mapMutex.Unlock( );

			CRect rect;
			rect.top = rc_item.top;
			rect.bottom = rc_item.bottom-1;
			rect.right = m_h_size + m_h_offset;
			rect.left = m_h_offset;
					
			if ( has_children  )
			{	
				//绘制根节点背景
				if ( m_bEnableRootBk )
				{
					//GradientFillRect( pDC, rect, m_clrRoot_from, RGB(164,154,191), FALSE );
				}
				else if ( m_hItemMouseMove == show_item ) //热跟踪
				{
					GradientFillRect( pDC, rect, RGB(43,43,45), RGB(43,43,45), FALSE );
				}

				//绘制展开收缩
				DrawExpand(rc_item,state,pDC);
					
				//绘制项位图
				DrawItemBitmap(rc_item,show_item,pDC,selected);
			}
			else
			{
				if ( selected )	//选中时候的子项背景
				{
					GradientFillRect( pDC, rect, RGB(43,43,45), RGB(43,43,45), FALSE );
				}
				else if ( m_hItemMouseMove == show_item )	//热跟踪
				{
	//				GradientFillRect( pDC, rect, RGB(43,43,45), RGB(43,43,45), FALSE );
				}
					
				//绘制项位图
				DrawItemBitmap(rc_item,show_item,pDC,selected);	
			}
			
			//绘制文本
			DrawItemText(rc_item,show_item,selected,pDC);
		}
		
	} while ( (show_item = GetNextVisibleItem( show_item )) != NULL );
}

void CTreeCtrlBT::DrawRowLine(CPoint ptBegin,CPoint ptEnd,CDC *pDc /* = NULL */)
{
/*	if ( !IsEnableRowLine() )
	{
		return;
	}
	
	if ( NULL == pDc)
	{
		CClientDC dc(this);
		pDc = &dc;
	}*/
	
	//CPen pen(PS_SOLID, 1, RGB(194,194,191));//RGB(235,235,235)
	//CPen *pOldPen = pDc->SelectObject(&pen);
	//pDc->MoveTo(ptBegin);
	//pDc->LineTo(ptEnd);
	//pDc->SelectObject(pOldPen);
	//pen.DeleteObject();
}

void CTreeCtrlBT::DrawExpand(CRect rect, int state, CDC *pDc /* = NULL */)
{
	if ( !m_ImageList.GetImageCount())
	{
		return;
	}
	
	if ( NULL == pDc)
	{
		CClientDC dc(this);
		pDc = &dc;
	}
	
	int nImage = 0;
	if ( state & TVIS_EXPANDED )	//展开
	{
		nImage = 1;
	}
	else	//收缩
	{
		nImage = 0;
	}
	
	CPoint point;
	point.x = rect.left - m_nImageWidth-m_nCellWidth;
	point.y = rect.top+(rect.Height()-m_nImageHeight)/2;
	
	m_ImageList.Draw(pDc,nImage, point, ILD_TRANSPARENT);
}

void CTreeCtrlBT::DrawItemBitmap(CRect rect, HTREEITEM hItem, CDC *pDc /* = NULL */,bool bSelected)
{
	if ( NULL == pDc)
	{
		CClientDC dc(this);
		pDc = &dc;
	}

	if(GetImageList(TVSIL_NORMAL))
	{
		///draw status image
		int nImage=0,nSelImage=0;
		GetItemImage(hItem,nImage,nSelImage);		
		GetImageList(TVSIL_NORMAL )->Draw(pDc,nImage,CPoint(rect.left+4,rect.top+6),ILD_TRANSPARENT);
	}

	if(GetImageList(TVSIL_STATE))
	{
		if(bSelected)
			GetImageList(TVSIL_STATE )->Draw(pDc,0,CPoint(rect.right-15,rect.top+4),ILD_TRANSPARENT);
		else
			GetImageList(TVSIL_STATE )->Draw(pDc,1,CPoint(rect.right-15,rect.top+4),ILD_TRANSPARENT);
	}

	else if ( m_pCurDrawItemMsg && m_pCurDrawItemMsg->pItemHeader )
	{
		///Draw header image
		Graphics    gDC(pDc->m_hDC);
		int   nHeight=rect.Height()-2;
		CRect        rectHeader(rect.right-3-nHeight,rect.top+1,rect.right-3,rect.bottom-1);

		gDC.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		gDC.DrawImage(m_pCurDrawItemMsg->pItemHeader, rectHeader.left, rectHeader.top, rectHeader.Width() ,rectHeader.Height());
	}
	
	return;
}

void CTreeCtrlBT::DrawItemText(CRect rect, HTREEITEM hItem, bool bselected,CDC *pDc /* = NULL */)
{
	if ( NULL == pDc)
	{
		CClientDC dc(this);
		pDc = &dc;
	}

	int nwidth = 20;//bitmap.bmWidth+m_nCellWidth;
	rect.left += nwidth;
	rect.right+= nwidth;
	
	CString strText = GetItemText( hItem );
	if(m_bUserSort)
		strText.Delete(0);
	
	//这个方法太别扭.因为发现DrawText DT_VCENTER在vc6下无效
	CSize sz = pDc->GetTextExtent(strText);
	rect.DeflateRect( 0,1,0,1 );
	rect.top += (rect.Height()-sz.cy)/2;
	rect.right= m_rect.right-20;
	
	//设置字体
	CFont* pOldFont = NULL;
	HTREEITEM hParentItem = GetParentItem(hItem);
	if ( hParentItem )
	{
		pOldFont = pDc->SelectObject(GetFont());
	}
	else
	{
		pOldFont = pDc->SelectObject(GetFont());
	}
	
	pDc->SetBkMode(TRANSPARENT);
	if ( bselected )
	{
		if ( IsEnableRootBk() )
		{
			pDc->SetTextColor( m_clrTextSelect );
		}
	}
	else
	{
		pDc->SetTextColor( m_clrTextNormal );
	}

	pDc->DrawText(strText, rect, DT_LEFT|DT_VCENTER|DT_WORD_ELLIPSIS);
	
	pDc->SelectObject(pOldFont);
	pOldFont = NULL;
}

void CTreeCtrlBT::EnableRowLine(BOOL bEnable /* = TRUE  */)
{
	m_bEnableRowLine = bEnable;
}

BOOL CTreeCtrlBT::IsEnableRowLine()
{
	return m_bEnableRowLine;
}

void CTreeCtrlBT::EnableRootBk(BOOL bEnable /* = TRUE  */)
{
	m_bEnableRootBk = bEnable;
}

BOOL CTreeCtrlBT::IsEnableRootBk()
{
	return m_bEnableRootBk;
}

BOOL CTreeCtrlBT::SetBackgroudBitmap(UINT nID )
{
	return m_bmpBackgroud.LoadBitmap(nID);
}

BOOL CTreeCtrlBT::SetBackgroudBitmap(LPCTSTR lpszRecourceName)
{
	return m_bmpBackgroud.LoadBitmap(lpszRecourceName);
}

void CTreeCtrlBT::SetItemBitmap(HTREEITEM hItem,UINT nIdBitmap)
{
	if ( !hItem )
	{
		return;
	}
	ITEM_MSG* pItemMsg = GetItemMsg(hItem);	
	if ( !pItemMsg )
	{
		return;
	}

	if ( pItemMsg->ItemBitmap.GetSafeHandle() )
	{
		pItemMsg->ItemBitmap.DeleteObject();
	}
	
	if ( 0 != nIdBitmap )
	{
		pItemMsg->ItemBitmap.LoadBitmap(nIdBitmap);
	}

}

void CTreeCtrlBT::SetItemBitmap(HTREEITEM hItem, LPCTSTR lpszBitmap)
{
	if ( !hItem )
	{
		return;
	}
	ITEM_MSG* pItemMsg = GetItemMsg(hItem);	
	if ( !pItemMsg )
	{
		return;
	}
	
	if ( pItemMsg->ItemBitmap.GetSafeHandle() )
	{
		pItemMsg->ItemBitmap.DeleteObject();
	}
	
	CFileFind fileFind;
	if ( fileFind.FindFile(lpszBitmap) )
	{
		pItemMsg->ItemBitmap.LoadBitmap(lpszBitmap);
	}
}

BOOL CTreeCtrlBT::DeleteAllItems( )
{
	CTreeCtrl::DeleteAllItems();
	m_mapMutex.Lock( );
	m_itmemsg_map.clear();
	delete m_pCurDrawItemMsg;
	m_pCurDrawItemMsg=NULL;
	m_mapMutex.Unlock( );
	return true;
}




/************************************************************************/

/* 函数功能: 设置展开收缩的图表

  /* 函数参数: 
		UINT	  nIdExpand			展开时候的图标,比如 " - "
		UINT      nIdCollapse       收缩时候的图标,比如 " + "
		COLORREF  crTransparent		位图透明色
		
		  /* 函数说明:
		  两个位图的透明色必须相同,大小必须相同
		  
/************************************************************************/
void CTreeCtrlBT::SetExpandBitmap(UINT nIdExpand, UINT nIdCollapse, COLORREF crTransparent /* = RGB */)
{
	CBitmap bmpExpand,bmpCollapse;
	bmpExpand.LoadBitmap(nIdExpand);
	bmpCollapse.LoadBitmap(nIdCollapse);
	if ( !bmpExpand.GetSafeHandle() || !bmpCollapse.GetSafeHandle() )
	{
		return;
	}
	
	//保存位图高度//宽度,在绘制的时候居中
	BITMAP	bitmapExpand,bitmapCollapse;
	bmpExpand.GetBitmap(&bitmapExpand);
	bmpCollapse.GetBitmap(&bitmapCollapse);
	if ( (bitmapExpand.bmWidth  != bitmapCollapse.bmWidth) || 
		(bitmapExpand.bmHeight != bitmapCollapse.bmHeight) )
	{
		return;
	}
	m_nImageWidth  = bitmapExpand.bmWidth;
	m_nImageHeight = bitmapExpand.bmHeight;
	
	//创建CImageList
	m_ImageList.Create(11,11,ILC_COLOR24|ILC_MASK,0,2);
	m_ImageList.Add(&bmpExpand,crTransparent);
	m_ImageList.Add(&bmpCollapse,crTransparent);
	
	
	bmpExpand.DeleteObject();
	bmpCollapse.DeleteObject();
	return;
}

/************************************************************************/

/* 函数功能: 插入新的一项

  /* 函数参数: 
		HTREEITEM hParent			父节点句柄
		LPCTSTR	  lpszItem			插入项文本
		UINT      nIdBitmap			插入项位图
		COLORREF  crTransparent		位图透明色
		
		  /* 函数说明:
		  返回插入项的句柄
		  
/************************************************************************/
HTREEITEM CTreeCtrlBT::InsertItemEx(HTREEITEM hParent, LPCTSTR lpszItem, UINT nIdBitmap /* = 0 */,Bitmap *pHeader,COLORREF crTransparent /* = RGB */,HTREEITEM hInsertAfter)
{	
	HTREEITEM hItem = InsertItem(lpszItem,nIdBitmap,nIdBitmap,hParent,hInsertAfter); 
	
	ITEM_MSG *pItemMsg = new ITEM_MSG;
	
	pItemMsg->hItem = hItem;
	pItemMsg->pItemHeader=pHeader;

	if(pHeader)
	{
		///Get frame count for .gif file
		UINT count=0;  
		count=pItemMsg->pItemHeader->GetFrameDimensionsCount();  
		GUID *pDimensionIDs=(GUID*)new GUID[count];  
		pItemMsg->pItemHeader->GetFrameDimensionsList(pDimensionIDs,count);  
		WCHAR strGuid[39];  
		StringFromGUID2(pDimensionIDs[0],strGuid,39);  
		int nFrameCount=pItemMsg->pItemHeader->GetFrameCount(&pDimensionIDs[0]);  
		delete []pDimensionIDs;  

		if(nFrameCount>1)    ///file is a .gif file
		{
			const GUID   Guid=FrameDimensionTime;
			pItemMsg->pItemHeader->SelectActiveFrame(&Guid,0);  
		}
	}
	
	if ( 0 != nIdBitmap)
	{
		pItemMsg->ItemBitmap.LoadBitmap(nIdBitmap);
	}

	if (pItemMsg->ItemBitmap.GetSafeHandle())
	{	
		pItemMsg->crTransparent = crTransparent;
	}
	
	ITEMMSG_MAP::iterator item = m_itmemsg_map.find(hItem);
	if ( item == m_itmemsg_map.end() )
	{
		m_mapMutex.Lock( );
		m_itmemsg_map[hItem] = pItemMsg;
		m_mapMutex.Unlock( );
	}
	else
	{
		item->second = pItemMsg;
	}
	
	return hItem;
}

HTREEITEM CTreeCtrlBT::InsertItemEx(HTREEITEM hParent, LPCTSTR lpszItem, LPCTSTR lpszBitmap,COLORREF crTransparent ,/* = RGB */HTREEITEM hInsertAfter)
{
	HTREEITEM hItem = InsertItem(lpszItem,0,1,hParent,hInsertAfter);
	
	CFileFind fileFind;
	if ( fileFind.FindFile(lpszBitmap) )
	{
		ITEM_MSG *pItemMsg = new ITEM_MSG;
		
		pItemMsg->hItem = hItem;
		
		pItemMsg->ItemBitmap.LoadBitmap(lpszBitmap);
		if (pItemMsg->ItemBitmap.GetSafeHandle())
		{	
			pItemMsg->crTransparent = crTransparent;
		}
		
		ITEMMSG_MAP::iterator item = m_itmemsg_map.find(hItem);
		if ( item == m_itmemsg_map.end() )
		{
			m_mapMutex.Lock( );
			m_itmemsg_map[hItem] = pItemMsg;
			m_mapMutex.Unlock( );
		}
		else
		{
			item->second = pItemMsg;
		}
	}
	
	fileFind.Close();
	
	return hItem;	
}

void CTreeCtrlBT::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CTreeCtrl::OnMouseMove(nFlags, point);

	m_ptOldMouse = point;
	HTREEITEM hItem = HitTest(point);
	if ( hItem != NULL && hItem != m_hItemMouseMove )
	{
		m_hItemMouseMove = hItem;
		Invalidate(FALSE);
	}
}


BOOL CTreeCtrlBT::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return true;//CTreeCtrl::OnEraseBkgnd(pDC);
}

void CTreeCtrlBT::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	this->Invalidate();
	*pResult = 0;
}

void CTreeCtrlBT::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CTreeCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTreeCtrlBT::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nSBCode==4||nSBCode==5)
	{
		SetScrollPos( SB_VERT, nPos);		
	}	
	CTreeCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
	Invalidate();
	

}

BOOL CTreeCtrlBT::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	this->Invalidate();
	return false;//CTreeCtrl::OnMouseHWheel(nFlags, zDelta, pt);
}

void CTreeCtrlBT::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	this->Invalidate();
	CTreeCtrl::OnMButtonUp(nFlags, point);
}

void CTreeCtrlBT::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	this->Invalidate();
	*pResult = 0;
}

void CTreeCtrlBT::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd* pParent = GetParent();
	// Check if valid parent
	ASSERT(pParent);

	// Send notification to parent
	NMHDR nmhdr;

	nmhdr.code = NM_DBLCLK;
	nmhdr.hwndFrom = m_hWnd;
	nmhdr.idFrom = GetDlgCtrlID();

	if(m_pNotifyWnd)
		m_pNotifyWnd->PostMessage(WM_TREE_LBUTTON_DBCLICK, (WPARAM)GetDlgCtrlID(), (LPARAM)&nmhdr);
	else
	{
		CTreeCtrl::OnLButtonDblClk(nFlags, point);
	}
}

