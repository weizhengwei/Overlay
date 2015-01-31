#pragma once

#include "stdafx.h"
#include <deque>
#include <vector>
#include <tstring/tstring.h>
#include <afxmt.h>
using namespace String;

typedef void (CALLBACK* DealSearchResultCallback)(LPCTSTR szFileName,LPVOID lpUserData);

class CGameSearch
{
public:
	CGameSearch(void);
	~CGameSearch(void);
public:
	void AddFilterDirectory(LPCTSTR lpDirectory)
	{
		if (lpDirectory)
		{
			m_vecFilterDirs.push_back(lpDirectory);
		}
	};
	BOOL CheckFilterDir(LPCTSTR lpDirectory);

	void AddSearchFileName(LPCTSTR lpFileName)
	{
       if (lpFileName)
       {
		   m_vecFileNames.push_back(lpFileName);
       }
	};
	void InitSearchDir(LPCTSTR lpDirName)
	{
		m_clsCriticalSection.Lock();
		if (lpDirName)
		{
			m_vecChlidDirs.push_back(lpDirName);
		}
		m_clsCriticalSection.Unlock();
	};
    BOOL CheckFileName(LPCTSTR lpFileName);

	void StartSearch();
	void DoSearch(LPCTSTR lpDirectory);
	void SetSearchResultCallBack(DealSearchResultCallback lpfnDealSearchResultCallBack,LPVOID lpUserData)
	{
		m_lpfnDealSearchResultCallBack = lpfnDealSearchResultCallBack;
		m_lpUserData = lpUserData;
	};
	void StopSearch(void)
	{
		m_bStopSearch = TRUE;
	};
	
private:
	std::deque<_tstring> m_vecChlidDirs;//the vector of search directory path;
	std::vector<_tstring> m_vecFilterDirs;//the vector of filter dirctory.
	std::vector<_tstring> m_vecFileNames;  // the vector of file name which will be searched.
	BOOL m_bStopSearch; //the control variable for stoping search.
	DealSearchResultCallback m_lpfnDealSearchResultCallBack;
	LPVOID m_lpUserData;
	CCriticalSection  m_clsCriticalSection;
};
