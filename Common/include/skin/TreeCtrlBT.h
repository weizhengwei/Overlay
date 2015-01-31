#if !defined(AFX_TREECTRLBT_H__F2142A27_2B46_45A5_B502_3CFED4D1616F__INCLUDED_)
#define AFX_TREECTRLBT_H__F2142A27_2B46_45A5_B502_3CFED4D1616F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeCtrlBT.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTreeCtrlBT window
#include"afxmt.h"
#include <map>
using namespace std;

//宏定义函数：用于释放GDI对象
#define RELEASE_GDI_RESOURCE(gdiResource) if(gdiResource.GetSafeHandle()){gdiResource.DeleteObject();}

//项所包含的相关信息
typedef struct tagItemMsg  
{	
	HTREEITEM hItem;			//项的句柄
	CBitmap   ItemBitmap;		//项的位图
	Bitmap      *pItemHeader;
	COLORREF  crTransparent;	//透明颜色
	tagItemMsg()
	{
		hItem   = NULL;
		
		crTransparent = RGB(255,255,255);
	}
	
	~tagItemMsg()
	{
		if ( ItemBitmap.GetSafeHandle() )
		{
			ItemBitmap.DeleteObject();
		}
	}
	
}ITEM_MSG;
typedef map<HTREEITEM,ITEM_MSG*>	ITEMMSG_MAP;

class CTreeCtrlBT : public CTreeCtrl
{
// Construction
public:
	CTreeCtrlBT();
	
public:
	//设置背景位图
	BOOL SetBackgroudBitmap( UINT nID );
	BOOL SetBackgroudBitmap(LPCTSTR lpszRecourceName);
	void      SetUserSort(bool bSort);
	void      SetNotifyWnd(CWnd  *pWnd);

	HTREEITEM   FindItem(HTREEITEM  item, LPCTSTR lpszStr);   
	
	//设置某一项位图
	void SetItemBitmap(HTREEITEM hItem, UINT nIdBitmap);
	void SetItemBitmap(HTREEITEM hItem, LPCTSTR lpszBitmap);

	//设置展开收缩的图表
	void SetExpandBitmap(UINT nIdExpand, UINT nIdCollapse, COLORREF crTransparent = RGB(255,255,255));
	
	//插入新的项目
	HTREEITEM InsertItemEx(HTREEITEM hParent, LPCTSTR lpszItem, UINT nIdBitmap = 0,Bitmap *pHeader=NULL,COLORREF crTransparent = RGB(255,255,255),HTREEITEM hInsertAfter=TVI_SORT);
	HTREEITEM InsertItemEx(HTREEITEM hParent, LPCTSTR lpszItem, LPCTSTR lpszBitmap,COLORREF crTranspare41nt = RGB(255,255,255),HTREEITEM hInsertAfter=TVI_SORT);
	//开启横线
	void EnableRowLine( BOOL bEnable = TRUE );
	//是否开启横线
	BOOL IsEnableRowLine();
	
	//开启根节点背景
	void EnableRootBk( BOOL bEnable = TRUE );
	//是否开启根节点背景
	BOOL IsEnableRootBk();
	BOOL DeleteAllItems( );
	void SetToolTipCtrlWidth(int nWidth){ m_nToolTipCtrlWidth = nWidth;};

	//获取某一项信息
	ITEM_MSG* GetItemMsg(HTREEITEM hItem);

// Operations
public:

private:
	void Calculate();
	
	void GradientFillRect( CDC *pDC, CRect &rect, COLORREF col_from, COLORREF col_to, bool vert_grad );
	void DrawBackgroudBitmap(CDC* pDc);
	void DrawItem( CDC* pDC );
	void DrawRowLine(CPoint ptBegin, CPoint ptEnd, CDC *pDc = NULL);
	void DrawExpand(CRect rect, int state, CDC *pDc = NULL);
	void DrawItemBitmap(CRect rect, HTREEITEM hItem, CDC *pDc = NULL,bool bSelected=false);
	void DrawItemText(CRect rect, HTREEITEM hItem, bool bselected,CDC *pDc = NULL);	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeCtrlBT)
	protected:
	virtual void PreSubclassWindow();
	
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTreeCtrlBT();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTreeCtrlBT)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual int OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void        OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	ITEMMSG_MAP		m_itmemsg_map;		//保存所有相的相关信息
	ITEM_MSG*       m_pCurDrawItemMsg;	//当前正在绘制的项信息

	CRect			m_rect;				// The client rect when drawing
	int				m_h_offset;			// 0... -x (scroll offset)
	int				m_h_size;			// width of unclipped window
	int				m_v_offset;			// 0... -y (scroll offset)
	int				m_v_size;			// height of unclipped window
	int             m_nToolTipCtrlWidth;    //control the tooltip text appears.


	CBitmap			m_bmpBackgroud;		//背景位图
	CImageList		m_ImageList;		//展开收缩的图标
	int				m_nImageHeight;		//展开收缩图标高度
	int				m_nImageWidth;		//展开收缩图标宽度
	int				m_nItemHeight;		//单元项目的高度

	bool			m_bEnableRowLine;	//单元项分开的横线 true:显示   false:反之
	bool			m_bEnableRootBk;	//根节点项的背景   true:显示   flase:反之
	int             m_nCellWidth;		//单元项图标文本的间距
	
	COLORREF		m_clrTextNormal;	//正常情况下字体颜色
	COLORREF		m_clrTextSelect;	//选择情况下字体颜色
	
	COLORREF		m_clrRoot_from;		//根节点渐变色
	COLORREF		m_clrRoot_to;		//根节点渐变色
	
	CFont			m_FontRoots;		//根节点字体
	CFont			m_FontChild;		//子节点字体
	bool             m_bUserSort;
	CWnd         *m_pNotifyWnd;

	CMutex        m_mapMutex;

	HTREEITEM		m_hItemSelect;		//当前选中的项目句柄
	HTREEITEM       m_hItemMouseMove;	//当前鼠标所在的句柄
	CPoint			m_ptOldMouse;		//上一次鼠标所在的位置

	

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREECTRLBT_H__F2142A27_2B46_45A5_B502_3CFED4D1616F__INCLUDED_)
