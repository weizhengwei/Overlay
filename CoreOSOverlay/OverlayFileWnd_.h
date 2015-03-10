#pragma once
#include "ui/CoreWidget.h"
#include "ui/ICoreUI.h"
#include "tstring/tstring.h"
#include "FolderListWnd.h"
#include <stack>
#include <vector>
using namespace std ;
using namespace String ;


typedef struct tagFileDataInfo
{
	_tstring  strPath;
	_tstring  strFileName;
	int      strType;

}FILEDATAINFO,*LPFILEDATAINFO;

class COverlayFileWnd : public CWindowImpl<COverlayFileWnd> ,public TabListImplBase
{
public:
	COverlayFileWnd(void);
	~COverlayFileWnd(void);

	DECLARE_WND_CLASS(_T("COverlayFileWnd"))

	BEGIN_MSG_MAP_EX(COverlayFileWnd)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_PAINT(OnPaint)		
	END_MSG_MAP()
public:
	
	BOOL OpenFileWnd(WPARAM wParam,LPARAM lParam);

protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);

	void OnListSelChange(UINT uNotifyCode, int nID, CWindow wndCtl); 
	void OnDestroy();
	void SelectFile(_tstring &strFile);
	void OnPaint(HDC);
	void Undo();



	void LoadDriveList();
	void LoadFileList(_tstring strPath,BOOL bUndo = TRUE);
	void InitList();
	void BackHigherFolder();
	void OnBackFolderClick(ISonicBase *, LPVOID pReserve);
	void OnUndoFolderClick(ISonicBase *, LPVOID pReserve);
	void OnShowFolderList(ISonicBase *, LPVOID pReserve);
	void OnShowFolderTypesList(ISonicBase *,LPVOID pReserve);
	void OnFileClick(ISonicBase *pISonicBase, LPVOID pReserve);
	void OnFileBtnDown(ISonicBase *pISonicBase, LPVOID pReserve);
	void OnOpenFileClick(ISonicBase *pISonicBase, LPVOID pReserve);
	void OnCancelClick(ISonicBase *pISonicBase, LPVOID pReserve);
	void SetSelTab (int nIndex,HWND hWnd = NULL);
	void DrawListBg(ISonicPaint * pPaint, LPVOID);
	
	
private: 
	//WTL::CCoreListCtrl m_FileList;
	// WTL::CCoreComboBox *m_pFolderBox;
	 WTL::CCoreEdit *m_pFileEdit;
	
	_tstring  m_strCurrentPath;
	_tstring  m_strFolderTypes;
	CFolderListWnd m_ListWnd;
	CFolderListWnd m_FileTypesListWnd;
	ISonicString *m_pFolderBtn;
	ISonicString *m_pFolderTypesBtn;
	ISonicScrollView *m_pFileScrollView;
	ISonicString *m_pSelectListItem;
	ISonicString *m_pUpOnlevelBtn;
	ISonicString *m_pUndoBtn;
	vector<ISonicImage*> m_vecpImage;
	wchar_t * m_pChatPath;
	stack<_tstring> m_stackUndostrPath;
	BOOL m_bFolderTypes;
};
