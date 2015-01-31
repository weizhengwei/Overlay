#include "StdAfx.h"
#include "OverlayFileWnd.h"
#include "OverlayImpl.h"

COverlayFileWnd::COverlayFileWnd(void)
:m_pFileEdit(NULL)
,m_ListWnd(this)
,m_pFolderBtn(NULL)
,m_pSelectListItem(NULL)
,m_pChatPath(NULL)
,m_pFolderTypesBtn(NULL)
,m_FileTypesListWnd(this)
,m_bFolderTypes(FALSE)
{
}

COverlayFileWnd::~COverlayFileWnd(void)
{
}
int COverlayFileWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	GetCoreUI()->AttachWindow(m_hWnd,(_T("overlay_fileWnd")));
	ISonicSkin * pSkin = GetSonicUI()->SkinFromHwnd(m_hWnd);
	pSkin->SetSkin(_T("bg"), _T("close:%d|;btn_top_margin:8;btn_right_margin:15"), GetCoreUI()->GetImage(_T("BTN_EXIT")));
	ISonicPaint *pPaint = GetSonicUI()->SkinFromHwnd(m_hWnd)->GetPaint();
	ISonicPaint *pPaintMainBg = GetSonicUI()->CreatePaint();
	CRect rcClient;
	GetClientRect(rcClient);
	pPaintMainBg->Create(FALSE,rcClient.Width(),rcClient.Height());
	pPaintMainBg->Delegate(DELEGATE_EVENT_PAINT,NULL,this,&COverlayFileWnd::DrawListBg);
	pPaint->AddObject(pPaintMainBg->GetObjectId(),0,0,TRUE);

	m_pFileEdit = GetCoreUI()->GetObjByID<WTL::CCoreEdit>(m_hWnd,_T("overlay_filelist_edit"));
	m_pFileEdit->SetReadOnly(TRUE);
	m_pUpOnlevelBtn = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("overlay_UpOneLevel_btn"));
	m_pUpOnlevelBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayFileWnd::OnBackFolderClick);
	m_pUndoBtn = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("overlay_Undo_btn"));
	m_pUndoBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayFileWnd::OnUndoFolderClick);


	m_pFileScrollView = GetSonicUI()->CreateObject<ISonicScrollView>();
	m_pFileScrollView->Create(m_hWnd,0,0,436,202,GetCoreUI()->GetImage(_T("SCROLLBAR_THUMB")),GetCoreUI()->GetImage(_T("SCROLLBAR_CHANNEL")));
	pPaint->AddObject(m_pFileScrollView->GetObjectId(),22,87,TRUE);

	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("overlay_open_btn"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this, &COverlayFileWnd::OnOpenFileClick);
	GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("overlay_cancel_btn"))->Delegate(DELEGATE_EVENT_CLICK,NULL,this, &COverlayFileWnd::OnCancelClick);


	m_pFolderBtn = GetCoreUI()->GetObjByID<ISonicString>(m_hWnd,_T("folderlist_btn"));;
	m_pFolderBtn->SetAttr(SATTR_STRING_BTNTEXTSTATIC,1);
	m_pFolderBtn->SetAttr(SATTR_STRING_BTNTEXT_ALIGN,0);
	m_pFolderBtn->Format(_T("/a, p=%d, btn_height=30,btn_width=380,linkc=0,btn_text='/c=%x, font, font_height=14, single_line=2,font_face='Open Sans',align=1/  %s'/"),GetCoreUI()->GetImage(_T("BTN_FOLDERBOX_BG"))->GetObjectId(),RGB(223,223,223),_T(""));
	m_pFolderBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayFileWnd::OnShowFolderList);
	//pPaint->AddObject(m_pFolderBtn->GetObjectId(),20,45,TRUE);
	m_ListWnd.Create(m_hWnd,CRect(0,0,0,0),NULL,WS_POPUP);
	m_ListWnd.SetListBoxMargin(1,1);
	m_ListWnd.SetListItemHeight(30);
	m_ListWnd.SetFontFormat(_tstring(_T("align=0x1,c=0xaaaaaa,font, font_height=14, font_bold=0, font_face=Open Sans")));
	
	POINT p;
	p.x = 12;
	p.y = 4;
	m_ListWnd.SetTextPoint(p);

	m_pFolderTypesBtn = GetSonicUI()->CreateString();
	m_pFolderTypesBtn->SetAttr(SATTR_STRING_BTNTEXTSTATIC,1);
	m_pFolderTypesBtn->SetAttr(SATTR_STRING_BTNTEXT_ALIGN,0);
	m_pFolderTypesBtn->Format(_T("/a, p=%d, btn_height=30,btn_width=340,linkc=0,btn_text='/c=%x, font, font_height=14, single_line=2,font_face='Open Sans',align=1/  %s'/"),GetCoreUI()->GetImage(_T("BTN_FOLDERBOX_BG"))->GetObjectId(),RGB(223,223,223),_T(""));
	m_pFolderTypesBtn->Delegate(DELEGATE_EVENT_CLICK, NULL, this, &COverlayFileWnd::OnShowFolderTypesList);
	pPaint->AddObject(m_pFolderTypesBtn->GetObjectId(),120,341,TRUE);
	m_FileTypesListWnd.Create(m_hWnd,CRect(0,0,0,0),NULL,WS_POPUP);
	m_FileTypesListWnd.SetListBoxMargin(1,1);
	m_FileTypesListWnd.SetListItemHeight(30);
	m_FileTypesListWnd.SetFontFormat(_tstring(_T("align=0x1,c=0xaaaaaa,font, font_height=14, font_bold=0, font_face=Open Sans")));
	m_FileTypesListWnd.SetListItemTextWidth(320);
	
	m_FileTypesListWnd.SetTextPoint(p);





	LoadDriveList();
	InitList();
	return 0L;
}
void COverlayFileWnd::OnDestroy()
{
	SetMsgHandled(FALSE);
}
void COverlayFileWnd::OnShowFolderList(ISonicBase *, LPVOID pReserve)
{
	m_ListWnd.SetListFocus();
	int nCount = m_ListWnd.GetCoreListBox()->GetCount();
	int nHeight = nCount * m_ListWnd.GetListItemHeight() ;
	CRect rc (*(m_pFolderBtn->GetRect()));
	ClientToScreen(rc);
	int nWidthMargin,nHeightMargin;
	m_ListWnd.GetListBoxMargin(nWidthMargin,nHeightMargin);
	::SetWindowPos(m_ListWnd.m_hWnd,NULL,rc.left,rc.bottom - 1,378 + nWidthMargin * 2,nHeight + nHeightMargin * 2,SWP_NOACTIVATE|SWP_NOZORDER);
	m_ListWnd.ShowWindow(SW_SHOWNA);
}
void COverlayFileWnd::OnShowFolderTypesList(ISonicBase *,LPVOID pReserve)
{
	int nCount = m_FileTypesListWnd.GetCoreListBox()->GetCount();
	if (nCount == 0)
	{
		return;
	}
	m_FileTypesListWnd.SetListFocus();
	int nHeight = nCount * m_FileTypesListWnd.GetListItemHeight() ;
	CRect rc (*(m_pFolderTypesBtn->GetRect()));
	ClientToScreen(rc);
	int nWidthMargin,nHeightMargin;
	m_FileTypesListWnd.GetListBoxMargin(nWidthMargin,nHeightMargin);
	::SetWindowPos(m_FileTypesListWnd.m_hWnd,NULL,rc.left,rc.bottom - 1,338+ nWidthMargin * 2,nHeight + nHeightMargin * 2,SWP_NOACTIVATE|SWP_NOZORDER);
	m_FileTypesListWnd.ShowWindow(SW_SHOWNA);
}


void COverlayFileWnd::OnFileClick(ISonicBase *pISonicBase, LPVOID pReserve)
{
	ISonicString * pText = (ISonicString *)pISonicBase;
	_tstring strText = pText->GetBtnText()->GetTextWithoutCtrl();
	_tstring strPath = m_strCurrentPath ;
	strPath +=  strText;
	WIN32_FIND_DATA FindFileData;
	FindFirstFile(strPath.c_str(), &FindFileData);

	if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
		&& FindFileData.cFileName != CString (".")
		&& FindFileData.cFileName != CString (".."))
	{
		strPath +=  _T("\\");
		LoadFileList(strPath);
	}
	else if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		OnOpenFileClick(NULL,NULL);
	}

}

void COverlayFileWnd::LoadDriveList()
{
	// 	if (!m_pFolderBox)
	// 	{
	// 		return;
	// 	}
	DWORD dwSize = ::GetLogicalDriveStrings(0, NULL);
	CString strDrive;

	if (dwSize > 0)
	{
		GetLogicalDriveStrings(dwSize,strDrive.GetBuffer(dwSize +1));
		strDrive.ReleaseBuffer();
	}
	else
	{
		return;
	}
	LPTSTR lp = strDrive.GetBuffer(strDrive.GetLength() + 1);
	strDrive.ReleaseBuffer();
	int i = 0;
	while(*lp != 0)
	{
		strDrive = lp;
		if (strDrive.CompareNoCase(_T("A:\\")) == 0 || strDrive.CompareNoCase(_T("B:\\")) == 0)
		{
			lp = _tcschr(lp,0)+1;
			continue;
		}
		if(DRIVE_CDROM == GetDriveType(lp))
		{
			lp = _tcschr(lp,0)+1;
			continue;
		}

		m_ListWnd.AddListItem(i,lp);
		lp = _tcschr(lp,0)+1;
		++i;
	}
	
	if (m_ListWnd.GetCoreListBox()->GetCount() > 0)
	{
		m_ListWnd.SelectItem(0);
	}
}
void COverlayFileWnd::InitList()
{

}
void COverlayFileWnd::OnFileBtnDown(ISonicBase *pISonicBase, LPVOID pReserve)
{
	if (m_pSelectListItem)
	{
		m_pSelectListItem->Push(FALSE);
	}
	m_pSelectListItem = (ISonicString *)pISonicBase;
	if (m_pFileEdit)
	{
		_tstring strText = m_pSelectListItem->GetBtnText()->GetTextWithoutCtrl();
		_tstring strPath = m_strCurrentPath ;
		strPath +=  strText;
		WIN32_FIND_DATA FindFileData;
		FindFirstFile(strPath.c_str(), &FindFileData);

		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			m_pFileEdit->SetWindowText(strText.c_str());
		}

	}
}
void COverlayFileWnd::LoadFileList(_tstring strPath ,BOOL bUndo)
{
	m_pSelectListItem = NULL;
	m_pFileScrollView->GetCanvas()->DelAllObject();
	m_pFileEdit->SetWindowText(_T(""));
	m_pFileScrollView->ScrollTo(0, 0, FALSE, SATYPE_NONE);
	vector<ISonicImage*>::iterator it = m_vecpImage.begin();
	while(it != m_vecpImage.end())
	{
		SONICUI_DESTROY(*it);
		++it;
	}
	m_vecpImage.clear();
	//m_FileList.DeleteAllItems();
	_tstring strFilePath = strPath + _T("\\*.*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(strFilePath.c_str(), &FindFileData);	
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}
	if (bUndo && !m_strCurrentPath.empty())
	{
		m_stackUndostrPath.push(m_strCurrentPath);
		m_pUndoBtn->Enable(TRUE);
	}
	m_strCurrentPath = strPath;
	_tstring strData;
	//m_FileList.InsertColumn(0,NULL,LVCFMT_LEFT,100);
	int nLeft = 0,nTop = 0, i = 0;
	do 
	{
		// 	 LPFILEDATAINFO pInfo = new tagFileDataInfo();
		// 	 pInfo->strFileName = FindFileData.cFileName;
		// 	 pInfo->strPath = strPath;
		// 	 m_FileList.SetItemData()
		strFilePath = strPath + FindFileData.cFileName;


		SHFILEINFO shfi;
		ZeroMemory(&shfi, sizeof(shfi));
		SHGetFileInfo(strFilePath.c_str(), 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON|SHGFI_TYPENAME|SHGFI_DISPLAYNAME);
		if (FindFileData.cFileName != CString (".")
			&& FindFileData.cFileName != CString (".."))
		{
			if (m_bFolderTypes == TRUE && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		 {
			 _tstring strName = FindFileData.cFileName;
			 int nLen = strName.rfind('.');
			 if (nLen != -1)
			 {
				 strName = strName.Right(strName.length() - nLen );
				 if ( m_strFolderTypes.Find(strName) == -1)
				 {
					 continue;
				 }
			 }
		 }
			ICONINFO iconinfo = { 0 }; 
			GetIconInfo(shfi.hIcon, &iconinfo);

			ISonicImage* pImag = GetSonicUI()->CreateImage();
			pImag->Load(iconinfo.hbmColor);
			DeleteObject(iconinfo.hbmMask);
			DeleteObject(iconinfo.hbmColor);
			m_vecpImage.push_back(pImag);
			//m_FileList.InsertItem(i++,FindFileData.cFileName,shfi.iIcon);
			ISonicString * pText = GetSonicUI()->CreateObject<ISonicString>();
			// pText->Format(_T("/a,btn_type=1, p=%d//a,c=%x, font, font_height=12, single_line=2,line_width=180,font_face='Open Sans'/%s"),pImag->GetObjectId(),RGB(223,223,223),FindFileData.cFileName);
			pText->Format(_T("/a, p=%d,btn_type=1,btn_width=180, linkc=0, btn_text='/p=%d//c=%x, font, font_height=13, single_line=2,font_face=Open Sans/%s'/"),GetCoreUI()->GetImage(_T("LIST_TOP")),pImag->GetObjectId(),RGB(223,223,223),FindFileData.cFileName);
			pText->SetAttr(SATTR_STRING_BTNTEXT_ALIGN, 0);
			pText->SetAttr(SATTR_STRING_BTNTEXTSTATIC,1);
			m_pFileScrollView->GetCanvas()->AddObject(pText->GetObjectId(),nLeft,nTop,TRUE);
			pText->Delegate(DELEGATE_EVENT_DBLCLICK, NULL, this, &COverlayFileWnd::OnFileClick);
			pText->Delegate(DELEGATE_EVENT_LBUTTONDOWN, NULL, this, &COverlayFileWnd::OnFileBtnDown);

			if (++i > 5)
			{
				i = 0 ;
				nLeft += 200; nTop = 0;
			}
			else
			{
				nTop += 30;
			}
		}
		::DestroyIcon(shfi.hIcon);

	} while (::FindNextFile(hFind,&FindFileData));

	::FindClose(hFind);
	m_pFileScrollView->Redraw();	

	strPath = m_strCurrentPath;

	int nlen = strPath.rfind(_T('\\'));
	if (nlen == -1)
	{
		m_pUpOnlevelBtn->Enable(FALSE);
		return;
	}
	strPath = strPath.Left(nlen);
	nlen = strPath.rfind(_T('\\'));
	if (nlen == -1)
	{
		m_pUpOnlevelBtn->Enable(FALSE);
		return;
	}
	m_pUpOnlevelBtn->Enable(TRUE);
}
void COverlayFileWnd::OnOpenFileClick(ISonicBase *pISonicBase, LPVOID pReserve)
{
	int nLen = m_pFileEdit->GetWindowTextLength();
	if (nLen > 0 && m_pChatPath)
	{
		CString strText;
		::GetWindowText(m_pFileEdit->m_hWnd,strText.GetBuffer(nLen + 1),nLen + 1);
		strText.ReleaseBuffer();
		_tstring strPath = m_strCurrentPath;
		strPath += strText;
		lstrcpyn(m_pChatPath,strPath.c_str(),strPath.length() + 1);
		SendMessage(WM_CLOSE);
	}
}
void COverlayFileWnd::OnCancelClick(ISonicBase *pISonicBase, LPVOID pReserve)
{
	
	SendMessage(WM_CLOSE);
	//DestroyWindow();
}
void COverlayFileWnd::SelectFile(_tstring &strFile)
{

}
void COverlayFileWnd::OnBackFolderClick(ISonicBase *, LPVOID pReserve)
{
	BackHigherFolder();
}
void COverlayFileWnd::BackHigherFolder()
{
	_tstring strPath = m_strCurrentPath;
	int nlen = strPath.rfind(_T('\\'));
	if (nlen == -1)
	{
		return;
	}
	strPath = strPath.Left(nlen);
	nlen = strPath.rfind(_T('\\'));
	if (nlen == -1)
	{
		return;
	}
	strPath = strPath.Left(nlen + 1);
	//	strPath += _T("..\\");

	LoadFileList(strPath);

}
void COverlayFileWnd::SetSelTab (int nIndex,HWND hWnd)
{
	//_tstring strText = m_ListWnd.GetCoreListBox()->GetText(nIndex);
	if (m_ListWnd.m_hWnd == hWnd)
	{
		ISonicString *pText = (ISonicString *)m_ListWnd.GetCoreListBox()->GetItemData(nIndex);
		_tstring strText = pText->GetTextWithoutCtrl();
		if (m_pFolderBtn)
		{
			m_pFolderBtn->Format(_T("/a, p=%d, btn_height=30,btn_width=380,linkc=0,btn_text='/c=%x, font, font_height=14, single_line=2,font_face='Open Sans',align=1/  %s'/"),GetCoreUI()->GetImage(_T("BTN_FOLDERBOX_BG"))->GetObjectId(),RGB(223,223,223),strText.c_str());
		}
		if (strText.CompareNoCase(m_strCurrentPath) != 0)
		{
			LoadFileList(strText);
		}
	}
	else if (m_FileTypesListWnd.m_hWnd == hWnd)
	{
		ISonicString *pText = (ISonicString *)m_FileTypesListWnd.GetCoreListBox()->GetItemData(nIndex);
		_tstring strText = pText->GetTextWithoutCtrl();
		if (m_pFolderBtn)
		{
			m_pFolderTypesBtn->Format(_T("/a, p=%d, btn_height=30,btn_width=340,linkc=0,btn_text='/c=%x, font, font_height=14, single_line=2,font_face='Open Sans',align=1/  %s'/"),GetCoreUI()->GetImage(_T("BTN_FOLDERBOX_BG"))->GetObjectId(),RGB(223,223,223),strText.c_str());
		}
		if(strText.rfind(('*')) - strText.Find('*') == 2)
		{
			m_bFolderTypes = FALSE;
		}
		else
		{
			m_bFolderTypes = TRUE;
		}

		m_strFolderTypes = strText;

		LoadFileList(m_strCurrentPath);
	}
}
void COverlayFileWnd::OnPaint(HDC)
{
	
}
void COverlayFileWnd::DrawListBg(ISonicPaint * pPaint, LPVOID)
{
	if (!pPaint)
	{
		return;
	}

	HDC hdc = pPaint->GetCurrentPaint()->hdc;
	CRect rcClient ,rc;
	GetClientRect(rcClient);
	rc.top = 34;
	rc.left = 1;
	rc.bottom = 35;
	rc.right = rcClient.right -1;
	GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,44,49,55));
	//	::FillRect(hdc,rc,brush);
	//	::DeleteObject(brush);

	rc.left = 1;
	rc.top = 35;
	rc.right = rcClient.right -1;
	rc.bottom = 36;
	GetSonicUI()->FillSolidRect(hdc,rc,ARGB(255,9,9,12));


	ISonicImage * pImg = GetCoreUI()->GetImage(_T("BTN_FOLDERBOXLIST_BG"));
	if (pImg)
	{

		DRAW_PARAM dp = { 0 };
		dp.dwMask = DP_TILE;
		dp.cyDiv = 10;
		dp.cy = 206;
		dp.cx = 440;
		pImg->Draw(hdc,rcClient.left + 20,rcClient.top + 85, &dp);

		dp.cy = 30;
		dp.cx = 340;
		pImg->Draw(hdc,rcClient.left + 120,rcClient.top + 301,&dp);
	}
}
BOOL COverlayFileWnd::OpenFileWnd(WPARAM wParam,LPARAM lParam)
{


	//	::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);

	m_pChatPath = (wchar_t*)lParam;
	m_pChatPath[0] = '1';
	_tstring strFileType = (wchar_t*)wParam;
	/*strFileType = _T("All Files(*.*) | Movie Files (*.avi;*.mpg;*.mpeg;*.png) | \
	*.avi;*.mpg;*.mpeg | \
	Audio Files (*.wav;*mp3;*.mpa;*.mpu;*.au) |\
	*.wav;*.mp3;*.png;*.mpu;*.au | \
	Midi Files (*.mid;*.midi;*.rmi) |\
	*.mid;*.midi;*.rmi");*/
	int nlen = 0;
	do 
	{
		nlen = strFileType.Find('|');
		_tstring strText ;
		if (nlen != -1)
		{
			strText = strFileType.Left(nlen);
		}
		else
		{
			strText = strFileType;
		}
		m_FileTypesListWnd.AddListItem(m_FileTypesListWnd.GetCoreListBox()->GetCount(),strText.c_str());
		strFileType = strFileType.Right(strFileType.length() - nlen -1);
	} while (nlen != -1);

	if (m_FileTypesListWnd.GetCoreListBox()->GetCount() > 0)
	{
		m_FileTypesListWnd.SelectItem(0);
	}

	return GetSonicUI()->DoModalLoop(m_hWnd);
}
void COverlayFileWnd::Undo()
{
	if (m_stackUndostrPath.empty())
	{
		return;
	}

	_tstring strPath = m_stackUndostrPath.top();
	m_stackUndostrPath.pop();
	if (m_stackUndostrPath.empty())
	{
		m_pUndoBtn->Enable(FALSE);
	}
	//	strPath = m_strCurrentPath;
	LoadFileList(strPath,FALSE);
	_tstring strTempPath = strPath;
	int nlen = strTempPath.rfind(_T('\\'));
	if (nlen == -1)
	{
		return;
	}
	strTempPath = strTempPath.Left(nlen);
	nlen = strTempPath.rfind(_T('\\'));
	if (nlen == -1)
	{
		CCoreListBox*pListBox =   m_ListWnd.GetCoreListBox();
		if (pListBox)
		{
			int nCount = pListBox->GetCount();
			for (int nIndex = 0 ; nIndex < nCount ; nIndex++)
			{
				ISonicString *pText = (ISonicString *)m_ListWnd.GetCoreListBox()->GetItemData(nIndex);
				_tstring strText = pText->GetTextWithoutCtrl();
				if (strText.CompareNoCase(strPath) == 0)
				{
					m_ListWnd.SelectItem(nIndex);
				}
			}
		}
	}

}
void COverlayFileWnd::OnUndoFolderClick(ISonicBase *, LPVOID pReserve)
{
	Undo();
}