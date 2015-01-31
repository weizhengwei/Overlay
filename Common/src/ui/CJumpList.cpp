
#include "ui\CJumpList.h"
#include "data\RecentPlayGameLog.h"
#include "data\DataPool.h"

#pragma comment(lib, "shlwapi.lib")

CJumpList jumpList;

CJumpList::CJumpList()
:m_bInitialized(FALSE),
m_bHasBegin(FALSE)
{
	m_szAppID = new TCHAR[128];
	CoInitialize(NULL);
}

CJumpList::~CJumpList()
{
	delete m_szAppID;
	CoUninitialize();
}


void CJumpList::SetAppID(LPCTSTR szAppID)
{
	if (szAppID != NULL)
	{
		_tcscpy_s(m_szAppID, 128, szAppID);
	}
}

BOOL CJumpList::Initialize()
{
	HRESULT hr = E_FAIL;
	OSVERSIONINFOEX stOsVersion = {0};
	stOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	BOOL bRet = GetVersionEx((OSVERSIONINFO*)&stOsVersion);
	if (bRet)
	{
		if (stOsVersion.dwMajorVersion < 6 || (stOsVersion.dwMajorVersion == 6 && stOsVersion.dwMinorVersion < 1))
		{
			return FALSE;
		}
	}

	if (!m_ICDL)
	{
		hr = m_ICDL.CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER);
	}
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("CustomDestinationList Create failed"));
		return FALSE;
	}

	if (m_szAppID != NULL)
	{
		hr = m_ICDL->SetAppID(m_szAppID);
	}
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("SetAppID Failed"));
		return FALSE;
	}

	if (!m_IOC)
	{
		hr = m_IOC.CoCreateInstance(CLSID_EnumerableObjectCollection,NULL, CLSCTX_INPROC_SERVER);
	}
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("IObjectCollection Create Failed"));
		return FALSE;
	}
	m_bInitialized = TRUE;
	return TRUE;
}

void CJumpList::CheckForRemovedItems()
{
	if (m_bInitialized == FALSE)
	{
		if(Initialize() == FALSE)
			return;
	}
	//First delete the log that the user has choosen to delete from the jump list;
	m_ICDL->GetRemovedDestinations(IID_IObjectArray, (void **)&m_IRemovedOA);
	if (!m_IRemovedOA)
	{
		return;
	}

	UINT uCount;
	if (FAILED(m_IRemovedOA->GetCount(&uCount)))
	{
		return;
	}

	CComPtr<IShellLink> pRemovedShellLink;
	for (int i = 0; i < uCount; i ++)
	{
		m_IRemovedOA->GetAt(i, IID_IShellLink, (void**)&pRemovedShellLink);
		if (pRemovedShellLink)
		{
			TCHAR szArgument[256];
			if(SUCCEEDED(pRemovedShellLink->GetArguments(szArgument, 256)))
			{
				recentPlayGameLog.RemoveLog(szArgument);
			}

		}
	}
	
	

}

BOOL CJumpList::BeginList()
{
	if (m_bHasBegin)
	{
		return TRUE;
	}

	if (m_bInitialized == FALSE)
	{
		if (Initialize() == FALSE)
		{
			return FALSE;
		}
	}

	if(FAILED(m_ICDL->DeleteList(m_szAppID)))
	{
		return FALSE;
	}
	
	CComPtr<IObjectArray> removedOA;
	if (FAILED(m_ICDL->BeginList(&m_uMinSlots, IID_IObjectArray, (void**)&removedOA)))
	{
		OutputDebugString(TEXT("BeginList failed"));
		return FALSE;
	}
	m_bHasBegin = TRUE;
	return TRUE;
}

UINT CJumpList::AddTasks(const vector<SHELLLINK> &vecShellLink, BOOL bNewCategory, LPCTSTR szCateName)
{
	UINT uRet = 0;
	BOOL bRet = TRUE;
	HRESULT hr = S_FALSE;
	if (BeginList() == FALSE)
	{
		return 0;
	}
	if (vecShellLink.empty())
	{
		return 0;
	}
	m_IOC->Clear();
	vector<SHELLLINK>::const_iterator iter = vecShellLink.begin();
	for ( ; iter != vecShellLink.end(); iter ++)
	{
		if (AddTask(*iter) == FALSE)
		{
			bRet = FALSE;
		}
	}
	if (bRet)
	{
		if (bNewCategory)
		{
			if (szCateName == NULL)
			{
				return FALSE;
			}
			hr = m_ICDL->AppendCategory(szCateName, m_IOC);
			if (FAILED(hr))
			{
				return 0;
			}
			m_IOC->GetCount(&uRet);
		}
		else
		{
			hr = m_ICDL->AddUserTasks(m_IOC);
			if (FAILED(hr))
			{
				return 0;
			}
			m_IOC->GetCount(&uRet);
		}
	}
	return uRet;
}

BOOL CJumpList::AddTask(const SHELLLINK &stShellLink)
{
	LPCTSTR sExeLocation = stShellLink.szExeLocation;
	LPCTSTR sTitle       = stShellLink.szTitle;
	LPCTSTR sCommandLine = stShellLink.szCommandLine;
	LPCTSTR sIconLoca    = stShellLink.szIconLocation;
	UINT     uIconIndex   = stShellLink.uIconIndex;

	if (sTitle == NULL || sCommandLine == NULL)
	{
		return FALSE;
	}
	CComPtr<IShellLink> ifShellLink;
	HRESULT hr = ifShellLink.CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("ShellLink Create Failed"));
		return FALSE;
	}

	hr = ifShellLink->SetPath(sExeLocation);
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("ShellLink SetPath Failed"));
		return FALSE;
	}

	hr = ifShellLink->SetArguments(sCommandLine);
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("ShellLink SetArguments Failed"));
		return FALSE;
	}

	hr = ifShellLink->SetIconLocation(sIconLoca, uIconIndex);
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("ShellLink SetIconLocation Failed"));
		return FALSE;
	}

	CComQIPtr<IPropertyStore> ifPS = ifShellLink;
	if (ifPS != NULL)
	{
		PROPVARIANT pvTitle;
		hr = InitPropVariantFromString(sTitle, &pvTitle);
		if (FAILED(hr))
		{
			OutputDebugString(TEXT("InitPropVariantFromString Failed"));
			return FALSE;
		}

		hr = ifPS->SetValue(PKEY_Title, pvTitle);
		if (FAILED(hr))
		{
			OutputDebugString(TEXT("IPropertyStore SetValue Failed"));
			return FALSE;
		}
		ifPS->Commit();
		PropVariantClear(&pvTitle);
	}

	hr = m_IOC->AddObject(ifShellLink);
	if (FAILED(hr))
	{
		OutputDebugString(TEXT("IObjectCollection Addobject Failed"));
		return FALSE;
	}
	return TRUE;
}

BOOL CJumpList::CommitList()
{
	UINT uTaskCount;
	HRESULT hr = S_FALSE;
	if (m_bInitialized)
	{
		hr = m_ICDL->CommitList();
		if (FAILED(hr))
		{
			OutputDebugString(_T("CommitList Failed"));
			return FALSE;
		}
		//Set the state to unintialized
		ICustomDestinationList* pCDL = m_ICDL.Detach();
		if (pCDL)
		{
			pCDL->Release();
		}
		IObjectCollection* pOC = m_IOC.Detach();
		if (pOC)
		{
			pOC->Release();
		}
		IObjectArray* pOA = m_IRemovedOA.Detach();
		if (pOA)
		{
			pOA->Release();
		}
		m_uMinSlots = 0;
		m_bInitialized = FALSE;
		m_bHasBegin = FALSE;
		return TRUE;
	}
	return FALSE;
}