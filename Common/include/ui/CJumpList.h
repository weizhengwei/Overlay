#pragma once

#include <atlcomcli.h>
#include <ShlObj.h>
#include <propvarutil.h>
#include <propkey.h>
#include <vector>
using namespace std;

typedef struct _SHELLLINK
{
	TCHAR szExeLocation[MAX_PATH];
	TCHAR szTitle[MAX_PATH];
	TCHAR szCommandLine[MAX_PATH];
	TCHAR szIconLocation[MAX_PATH];
	UINT     uIconIndex;
	_SHELLLINK()
	{
		memset(this, 0, sizeof(_SHELLLINK));
	}
}SHELLLINK, *PSHELLLINK;

class CJumpList
{
public:
	CJumpList();
	~CJumpList();
	//This method must be called when use the CJumplist other functions
	void SetAppID(LPCTSTR szAppID);
	//Check the data that has been removed by the user from the jump list;
	void CheckForRemovedItems();
	//when calling this function, you should first call SetAppID and CheckForRemovedItems
	UINT AddTasks(const vector<SHELLLINK> &vecShellLink, BOOL bNewCategory = FALSE, LPCTSTR szCateName = NULL);
	//This method must be called when after calling the AddTasks function for the commit to take effect
	BOOL CommitList();

	
private:
	BOOL Initialize();
	
	//This function is internally called by AddTasks
	BOOL BeginList();
	BOOL AddTask(const SHELLLINK &stShellLink);
private:
	CComPtr<ICustomDestinationList> m_ICDL;
	CComPtr<IObjectArray>           m_IRemovedOA;
	CComPtr<IObjectCollection>		m_IOC;
	UINT							m_uMinSlots;
	BOOL                            m_bInitialized;
	BOOL							m_bHasBegin;
	LPTSTR                          m_szAppID;	
};

extern CJumpList jumpList;