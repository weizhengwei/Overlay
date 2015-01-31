#include "data/GameSearch.h"



CGameSearch::CGameSearch(void)
{
  m_bStopSearch = FALSE;
  m_lpfnDealSearchResultCallBack = NULL;
  
  TCHAR winDirs[MAX_PATH] = {0};
  if (GetWindowsDirectory(winDirs,MAX_PATH))
  {
	  m_vecFilterDirs.push_back(winDirs);
  }
}
CGameSearch::~CGameSearch(void)
{

}
void CGameSearch::StartSearch()
{
	do 
	{
		if (m_vecChlidDirs.empty())
		{
			break;
		}
		m_clsCriticalSection.Lock();
		TCHAR strFile[MAX_PATH+2] = {0} ;
		_tcscpy_s(strFile,MAX_PATH,m_vecChlidDirs.front().c_str());
		m_vecChlidDirs.pop_front();
		m_clsCriticalSection.Unlock();
		DoSearch(strFile);
	} while (!m_bStopSearch);

}

void CGameSearch::DoSearch(LPCTSTR lpDirectory)
{
	if (m_bStopSearch)
	{
		return;
	}

	if (lpDirectory == NULL /*|| !PathIsDirectory(lpDirectory) || !PathFileExists(lpDirectory)*/)
	{
		return;
	}
	if (CheckFilterDir(lpDirectory))
	{
		return;
	}
	TCHAR strPath[MAX_PATH], strFile[MAX_PATH];
	HANDLE hFile;
	WIN32_FIND_DATA FindFileData;

	CString szTmp(lpDirectory);
	if (szTmp.Right(1)!= _T("\\"))
	{
		szTmp += _T("\\");
	}
	wsprintf(strPath, _T("%s*.*"), szTmp.GetBuffer());
	szTmp.ReleaseBuffer();

	hFile = FindFirstFile(strPath, &FindFileData);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do 
		{
			if (lstrcmp(FindFileData.cFileName, _T(".")) == 0 || lstrcmp(FindFileData.cFileName, _T("..")) == 0)
				continue;

			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wsprintf(strFile,_T("%s%s"), szTmp, FindFileData.cFileName);
				if (!CheckFilterDir(strFile))
				{
					m_clsCriticalSection.Lock();
					m_vecChlidDirs.push_back(strFile);
					m_clsCriticalSection.Unlock();
				}
			}
			else
			{
				if (CheckFileName(FindFileData.cFileName)) //find the file 
				{
					wsprintf(strFile,_T("%s%s"), szTmp, FindFileData.cFileName);
					m_lpfnDealSearchResultCallBack(strFile,m_lpUserData);
				}
			}

		} while( !m_bStopSearch && FindNextFile(hFile, &FindFileData));
		FindClose(hFile);
	}
}
BOOL CGameSearch::CheckFilterDir(LPCTSTR lpDirectory)
{
	BOOL bRet = FALSE;
	_tstring strDir(lpDirectory);
	do 
	{
		if (lpDirectory == NULL || m_vecFilterDirs.empty())
		{
			break;
		}
		std::vector<_tstring>::iterator iter = m_vecFilterDirs.begin();
		for (;iter!= m_vecFilterDirs.end();iter++)
		{
			if (_tcslen(lpDirectory) < _tcslen((*iter).c_str()))
			{
				continue;
			}
			_tstring tsTmp = *iter;
			if (tsTmp.CompareNoCase(strDir.Left(tsTmp.length())) == 0)
			{
				bRet = TRUE;
				break;
			}
		}
	} while (FALSE);
	return bRet;
}

BOOL CGameSearch::CheckFileName(LPCTSTR lpFileName)
{
	_tstring sFilename(lpFileName);
   BOOL bRet = FALSE;
   do 
   {
	   if (lpFileName == NULL || m_vecFileNames.empty())
	   {
		   break;
	   }
	   std::vector<_tstring>::iterator iter = m_vecFileNames.begin();
	   for (;iter != m_vecFileNames.end();iter++)
	   {
		   if (sFilename.CompareNoCase(*iter) == 0)
		   {
			   bRet = TRUE;
			   break;
		   }
	   }
   } while (FALSE);
   return bRet;
}