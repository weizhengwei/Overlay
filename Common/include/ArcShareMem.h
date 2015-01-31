#pragma once

//////////////////////////////////////////////////////////////////////////
// common components
#define ARC_SM_MAGIC		0x12358
struct arc_sm_header
{
	DWORD dwMagic;
	DWORD dwHeaderSize;
	DWORD dwSize;
	DWORD dwCreatorThreadId;
};

class CArcAutoLock
{
public:
	CArcAutoLock()
	{
		WaitForSingleObject(m_hEvent, INFINITE);
	}
	~CArcAutoLock()
	{
		SetEvent(m_hEvent);
	}
	static void Init(HANDLE hEvent){ m_hEvent = hEvent; }
private:
	static HANDLE m_hEvent;
};
__declspec(selectany) HANDLE CArcAutoLock::m_hEvent = NULL;

template<class THeader = arc_sm_header>
class CArcShareMemMgr
{
private:
	HANDLE m_hFileMapping;
	LPVOID m_pView;
	HANDLE m_hEvent;
public:
	CArcShareMemMgr()
	{
		m_pView = NULL;
		m_hFileMapping = NULL;
		m_hEvent = NULL;
	}
	virtual ~CArcShareMemMgr(){}
	BOOL Create(LPCTSTR szName, DWORD dwSize)
	{
		if(!Init(szName))
		{
			return FALSE;
		}
		m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwSize, szName);
		if(m_hFileMapping)
		{
			m_pView = MapViewOfFile(m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if(m_pView)
			{
				THeader * pHeader = (THeader *)m_pView;
				pHeader->dwSize = dwSize;
				pHeader->dwMagic = ARC_SM_MAGIC;
				pHeader->dwCreatorThreadId = GetCurrentThreadId();
				pHeader->dwHeaderSize = sizeof(THeader);
				return TRUE;
			}
		}
		Close();
		return FALSE;
	}
	BOOL Open(LPCTSTR szName)
	{
		if(!Init(szName))
		{
			return FALSE;
		}
		m_hFileMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
		if(m_hFileMapping)
		{
			m_pView = MapViewOfFile(m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			if(m_pView)
			{
				return TRUE;
			}
		}
		Close();
		return FALSE;
	}
	void Close()
	{
		if(m_pView)
		{
			UnmapViewOfFile(m_pView);
		}
		if(m_hFileMapping)
		{
			CloseHandle(m_hFileMapping);
		}
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
		}
	}
	BOOL IsValid(){ return m_pView != NULL; }
	void Lock(){ WaitForSingleObject(m_hEvent, INFINITE); }
	void Unlock(){ SetEvent(m_hEvent); }
	THeader * GetHeader()
	{
		return (THeader *)m_pView;
	}
	LPBYTE GetBits()
	{
		return (LPBYTE)(m_pView) + sizeof(THeader);
	}
protected:
	BOOL Init(LPCTSTR szName)
	{
		if(IsValid())
		{
			return FALSE;
		}
		TCHAR szEventName[MAX_PATH];
		_sntprintf_s(szEventName, MAX_PATH, _TRUNCATE, _T("%s_event"), szName);
		m_hEvent = CreateEvent(NULL, FALSE, TRUE, szEventName);
		if(!m_hEvent)
		{
			return FALSE;
		}
		CArcAutoLock::Init(m_hEvent);
		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////
// for overlay
struct overlay_sm_header : public arc_sm_header
{
	union
	{
		HWND hGame;
		BYTE byRsv[8];
	};
	union
	{
		HWND hOSPanel;
		BYTE byRsv1[8];
	};
	int nWidth;
	int nHeight;
	RECT rtDirty;
	BOOL bUpdate;
	BOOL bHasImage;
	BOOL bFullScreen;
	DWORD dwOverlayPID;
	UINT uStartTime;
	BOOL bShow;
	char csPage[512];
};
// for arcosbrowser
struct arcbrowser_sm_header:public arc_sm_header
{
	int nWidth;
	int nHeight;
};