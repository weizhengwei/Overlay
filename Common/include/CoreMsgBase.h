#pragma  once

//////////////////////////////////////////////////////////////////////////
// file:	CoreMsgComm.h
// brief:	This file is a global definition and implementation for communication between CoreClient
//			and In-Game Overlay based on C/S framework.
// author:	Sonic Guan
//////////////////////////////////////////////////////////////////////////
#include "atlbase.h"
#include "atlwin.h"
#include <list>

//////////////////////////////////////////////////////////////////////////
// macros
// class name of server message window
#define MSG_SERVER_CLSNAME		_T("ArcMsgServer")
// class name of client message window
#define MSG_CLIENT_CLSNAME			_T("ArcMsgClient")
#define SET_CLASS_NAME(clsname)	static const TCHAR * GetClsName(){ return clsname; }

// the host window should responds this message to handle core messages
// wParam: hwnd of client or server who sent this message
// lParam: a pointer to a buffer with a core_msg_header
#define WM_COREMESSAGE			WM_USER + 0x7316

// magic number to verify if the packet is a valid core message packet
#define CORE_MSG_MAGIC			0x83777	

enum
{
	CORE_MSG_CONNECT = 1,
};
//////////////////////////////////////////////////////////////////////////
// structures
struct core_msg_header
{
	DWORD dwCmdId;		// command id of the message packet
	DWORD dwSize;		// the size of the whole message packet, including this message header
	DWORD dwRet;		// result returned by this sending operation
	BOOL bGet;			// if this parameter is set to TRUE, it means the sender wanna the receiver to set the value of this packet for it.
	// after the sender called SendCoreMessage and returned, the packet passed to the function is already set with data from receiver
};

//////////////////////////////////////////////////////////////////////////
// this packet is used for a client to connect a message server and register itself to the server
// it is implemented internally during the initialization of a client
// dwRet: 0 for OK, 1 for renew failed
struct core_msg_connect : public core_msg_header
{
	DWORD dwClientId;	// a client id is generally an user-define identity number which should be unique.
	// in CoreClient this value should the game id.
	BOOL bAlterable;	// to tell the server if the hwnd can be altered if a new message client with the same dwClientId is created
	core_msg_connect()
	{
		memset(this, 0, sizeof(core_msg_connect));
		dwSize = sizeof(core_msg_connect);
		dwCmdId = CORE_MSG_CONNECT;
		bAlterable = TRUE;
	}
};

class ICoreMsgCallback
{
public:
	virtual void OnCoreMessage(HWND hFrom, core_msg_header * pHeader) = 0;
};

//////////////////////////////////////////////////////////////////////////
// CCoreMsgBase
//////////////////////////////////////////////////////////////////////////
template<class T>
class CCoreMsgBase
{
public:
	// waiting data structure
	enum
	{
		waitGet = 0,
		waitMsg = 1,
		waitOK = 0x10
	};
	struct tagWaitingItem
	{
		DWORD dwCmdId;
		DWORD dwFlag;
		core_msg_header * pItem;
	};
	typedef std::list<tagWaitingItem> LIST_WAITING;
private:
	class CMsgWnd : public CWindowImpl<CMsgWnd>
	{
	public:
		typedef CCoreMsgBase<T> CCoreMsgBaseT;
		CMsgWnd(CCoreMsgBaseT * pBase) : m_pBase(pBase){}
		~CMsgWnd(){}
		DECLARE_WND_CLASS(T::GetClsName())
		BEGIN_MSG_MAP(CMsgWnd)
			MESSAGE_HANDLER(WM_COPYDATA, OnCopyData)
		END_MSG_MAP()

		// handle WM_COPYDATA and transmit core message to the host window
		LRESULT OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
		{
			COPYDATASTRUCT * pCopyData = (COPYDATASTRUCT *)lParam;
			if(pCopyData)
			{
				if(pCopyData->dwData == CORE_MSG_MAGIC)
				{
					// valid core messages, go on
					core_msg_header * pHeader = (core_msg_header *)(pCopyData->lpData);
					if(pHeader->dwCmdId == CORE_MSG_CONNECT)
					{
						// only server needs to handle this message, a client doesn't care
						m_pBase->OnConnect((HWND)wParam, (core_msg_connect *)pHeader);
					}
					if(m_pBase->m_WaitingList.size())
					{
						// there is someone waiting for messages
						EnterCriticalSection(&m_pBase->m_csWaitingLock);
						for(CCoreMsgBaseT::LIST_WAITING::iterator it = m_pBase->m_WaitingList.begin(); it != m_pBase->m_WaitingList.end(); it++)
						{
							if(it->dwCmdId == pHeader->dwCmdId)
							{
								if(pHeader->bGet == 2 && it->pItem && it->dwFlag == waitGet)
								{
									memcpy(it->pItem, pHeader, pHeader->dwSize);
									it->dwFlag |= CCoreMsgBaseT::waitOK;
									break;
								}
								if(pHeader->bGet <= 1 && it->dwFlag == waitMsg)
								{
									it->dwFlag |= CCoreMsgBaseT::waitOK;
									break;
								}
							}
						}
						LeaveCriticalSection(&m_pBase->m_csWaitingLock);
					}
					if(pHeader->bGet > 1)
					{
						// now in the sender, receiving the packet reflected from the receiver
						return 0;
					}
					// relay messages
					if(m_pBase->m_hHost)
					{
						::SendMessage(m_pBase->m_hHost, WM_COREMESSAGE, wParam, (LPARAM)pHeader);
					}
					else if(m_pBase->m_pCallback)
					{
						m_pBase->m_pCallback->OnCoreMessage((HWND)wParam, pHeader);
					}

					if(pHeader->bGet == TRUE)
					{
						// I am the receiver, the packet is already filled with data by receiver, reflect it to the sender
						pHeader->bGet++;
						m_pBase->SendCoreMsg((HWND)wParam, pHeader);
					}
					return (LRESULT)pHeader->dwRet;
				}
			}
			return 0;
		}
	private:
		CCoreMsgBaseT * m_pBase;
	};
public:
	CCoreMsgBase() : m_MsgWnd(this)
	{
		m_hHost = NULL;
		m_pCallback = NULL;
	}
	virtual ~CCoreMsgBase(){}
	friend class CMsgWnd;

	//brief: necessary initialization of the message server, should be global unique in a process.
	//hHost[in]: a core message server or client will transmit core message packets to the host window by sending method
	//dwUserData[in]: user-define data, this param is only significant in CCoreMsgClient
	//szServerName[in]: this parameter has different meanings in a server or in a client. 
	//					when initiate a server, this means the window name, by using which the client could find the server.
	BOOL Initialize(HWND hHost, DWORD dwUserData = 0, LPCTSTR szServerName = NULL)
	{
		if(!IsWindow(hHost))
		{
			return FALSE;
		}
		m_hHost = hHost;
		return InternalInit(szServerName, dwUserData);
	}

	//brief: necessary initialization of the message server, should be global unique in a process.
	//pCallback[in]: the message handler who will actually process customer messages
	//dwUserData[in]: user-define data, this param is only significant in CCoreMsgClient
	//szServerName[in]: this parameter has different meanings in a server or in a client. 
	//					when initiate a server, this means the window name, by using which the client could find the server.
	BOOL Initialize(ICoreMsgCallback * pCallback, DWORD dwUserData = 0, LPCTSTR szServerName = NULL)
	{
		if(!pCallback)
		{
			return FALSE;
		}
		m_pCallback = pCallback;
		return InternalInit(szServerName, dwUserData);
	}

	// brief: should be called at the end of a process
	virtual void Uninitialize()
	{
		if(m_MsgWnd.IsWindow())
		{
			m_MsgWnd.DestroyWindow();
		}
		m_hHost = NULL;
		DeleteCriticalSection(&m_csWaitingLock);
		m_WaitingList.clear();
	}
	// brief: test if the server or client is now working
	virtual BOOL IsValid()
	{
		return m_MsgWnd.IsWindow();
	}

	// send a block of buffer with core_msg_header to a specified window
	// hWnd[in]: the window will receive this buffer
	// pHeader[in]: a pointer to a block of buffer with core_msg_header
	virtual BOOL SendCoreMsg(HWND hWnd, core_msg_header * pHeader)
	{
		if(!hWnd || !IsWindow(hWnd))
		{
			return FALSE;
		}
		COPYDATASTRUCT cds;
		cds.cbData = pHeader->dwSize;
		cds.dwData = CORE_MSG_MAGIC;
		cds.lpData = (LPVOID)pHeader;
		if(pHeader->bGet == TRUE)
		{
			// wanna directly get data from receiver
			AddWaitingItem(pHeader->dwCmdId, pHeader);
		}
		DWORD dwRet = 0;
		BOOL bOK = FALSE;
		if(SendMessageTimeout(hWnd, WM_COPYDATA, (WPARAM)(HWND)m_MsgWnd, (LPARAM)&cds, SMTO_ABORTIFHUNG, 50000, (PDWORD_PTR)&dwRet))
		{
			pHeader->dwRet = dwRet;
			bOK = TRUE;
		}
		if(pHeader->bGet)
		{
			RemoveWaitingItem(pHeader->dwCmdId, pHeader);
		}
		return bOK;
	}
	// wait for a specified sent core message
	virtual BOOL WaitForCoreMsg(DWORD dwCmdId, UINT uMilliseconds)
	{
		BOOL bOK = FALSE;
		AddWaitingItem(dwCmdId);
		const UINT uInterval = 100;
		for(UINT uElapse = 0; ;uElapse += uInterval)
		{
			MSG msg;
			PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
			Sleep(uInterval);
			if(uMilliseconds != -1 && uElapse >=uMilliseconds)
			{
				// overtime
				break;
			}
			EnterCriticalSection(&m_csWaitingLock);
			for(LIST_WAITING::iterator it = m_WaitingList.begin(); it != m_WaitingList.end(); it++)
			{
				if(it->dwCmdId == dwCmdId && (it->dwFlag & waitOK))
				{
					// the waiting item has been removed from the waiting list, which means we have received the packet we wanted
					bOK = TRUE;
					m_WaitingList.erase(it);
					break;
				}
			}
			LeaveCriticalSection(&m_csWaitingLock);
			if(bOK)
			{
				break;
			}
		}
		return bOK;
	}
	// get the host window who will actually handle messages
	HWND GetHostWindow(){ return m_hHost; }

	// get the window of this message base used for relaying messages
	HWND GetSelfWindow(){ return m_MsgWnd.m_hWnd; }
protected:
	virtual void OnConnect(HWND hClientSender, core_msg_connect * pConnect){}
	virtual BOOL InternalInit(LPCTSTR szServerName, DWORD dwUserData)
	{ 
		InitializeCriticalSection(&m_csWaitingLock);
		return m_MsgWnd.Create(HWND_MESSAGE, NULL, szServerName) != NULL;
	}
private:
	void AddWaitingItem(DWORD dwCmdId, core_msg_header * pWaitingHeader = NULL)
	{
		tagWaitingItem item = {dwCmdId, pWaitingHeader ? waitGet : waitMsg, pWaitingHeader};
		EnterCriticalSection(&m_csWaitingLock);
		m_WaitingList.push_back(item);
		LeaveCriticalSection(&m_csWaitingLock);
	}
	BOOL RemoveWaitingItem(DWORD dwCmdId, core_msg_header * pWaitingHeader = NULL)
	{
		BOOL bRet = FALSE;
		EnterCriticalSection(&m_csWaitingLock);
		for(LIST_WAITING::iterator it = m_WaitingList.begin(); it != m_WaitingList.end(); it++)
		{
			if(it->dwCmdId == dwCmdId)
			{
				if(!pWaitingHeader || it->pItem == pWaitingHeader)
				{
					m_WaitingList.erase(it);
					bRet = TRUE;
					break;
				}
			}
		}
		LeaveCriticalSection(&m_csWaitingLock);
		return bRet;
	}
private:
	HWND m_hHost;
	ICoreMsgCallback * m_pCallback;
	CMsgWnd m_MsgWnd;
	LIST_WAITING m_WaitingList;
	CRITICAL_SECTION m_csWaitingLock;
};

//////////////////////////////////////////////////////////////////////////
// CCoreMsgServer
//////////////////////////////////////////////////////////////////////////
class CCoreMsgServer : public CCoreMsgBase<CCoreMsgServer>
{
public:
	typedef struct tagClientData
	{
		HWND hClientWnd;
		DWORD dwClientId;
		BOOL bAlterable;
	}CLIENT_DATA;
	typedef std::list<CLIENT_DATA> LIST_CLIENT_DATA;
private:
	LIST_CLIENT_DATA m_listClientData;
public:
	SET_CLASS_NAME(MSG_SERVER_CLSNAME)

	virtual void Uninitialize()
	{
		m_listClientData.clear();
		CCoreMsgBase::Uninitialize();
	}
	// remark: a server could set hWnd to HWND_BROADCAST to broadcast a message to all clients
	virtual BOOL SendCoreMsg(HWND hWnd, core_msg_header * pHeader)
	{
		if(hWnd == HWND_BROADCAST)
		{
			// broadcast the message to all clients
			for(LIST_CLIENT_DATA::iterator it = m_listClientData.begin(); it != m_listClientData.end(); it++)
			{
				CCoreMsgBase::SendCoreMsg(it->hClientWnd, pHeader);
			}
			return TRUE;
		}
		else if(!IsWindow(hWnd))
		{
			// the destination client is unavailable, remove it from the client list
			for(LIST_CLIENT_DATA::iterator it = m_listClientData.begin(); it != m_listClientData.end(); it++)
			{
				if(hWnd == it->hClientWnd)
				{
					m_listClientData.erase(it);
					break;
				}
			}
			return FALSE;
		}
		else
		{
			return CCoreMsgBase::SendCoreMsg(hWnd, pHeader);
		}	
	}
	// remark: a server could send messages to a client with a user-define client id
	virtual BOOL SendCoreMsg(DWORD dwClientID, core_msg_header * pHeader)
	{
		for(LIST_CLIENT_DATA::iterator it = m_listClientData.begin(); it != m_listClientData.end(); it++)
		{
			if(it->dwClientId == dwClientID)
			{
				return SendCoreMsg(it->hClientWnd, pHeader);
			}
		}
		return FALSE;
	}
	// to test if a client is still valid according to the validity of the binding hwnd
	BOOL IsClientValid(DWORD dwClientId)
	{
		for(LIST_CLIENT_DATA::iterator it = m_listClientData.begin(); it != m_listClientData.end(); it++)
		{
			if(it->dwClientId == dwClientId)
			{
				return IsWindow(it->hClientWnd);
			}
		}
		return FALSE;
	}
	// return the client id corresponding to a specified hWnd
	DWORD GetClientIdByHwnd(HWND hWnd)
	{
		for(LIST_CLIENT_DATA::iterator it = m_listClientData.begin(); it != m_listClientData.end(); it++)
		{
			if(it->hClientWnd == hWnd)
			{
				return it->dwClientId;
			}
		}
		return 0;
	}
	BOOL RemoveClientById(DWORD dwClientId)
	{
		for (LIST_CLIENT_DATA::iterator it = m_listClientData.begin();it != m_listClientData.end(); it++)
		{
			if (it->dwClientId == dwClientId)
			{
				m_listClientData.erase(it);
				return TRUE;
			}
		}
		return FALSE;
	}
protected:
	virtual void OnConnect(HWND hClientSender, core_msg_connect * pConnect)
	{
		for(LIST_CLIENT_DATA::iterator it = m_listClientData.begin(); it != m_listClientData.end(); it++)
		{
			if(it->dwClientId == pConnect->dwClientId)
			{
				// already exists, renew the message hwnd of the client
				if(it->bAlterable || !IsWindow(it->hClientWnd))
				{
					it->hClientWnd = hClientSender;
					it->bAlterable = pConnect->bAlterable;
				}
				else
				{
					// a game has been binded to the client id, can not renew
					pConnect->dwRet = 1;
				}
				return;
			}
		}
		// a new client is connecting
		CLIENT_DATA data = {hClientSender, pConnect->dwClientId, TRUE};
		m_listClientData.push_back(data);
	}
};

//////////////////////////////////////////////////////////////////////////
// CCoreMsgClient
//////////////////////////////////////////////////////////////////////////
class CCoreMsgClient : public CCoreMsgBase<CCoreMsgClient>
{
private:
	HWND m_hSvr;
	DWORD m_dwClientID;
protected:
	virtual BOOL InternalInit(LPCTSTR szServerName, DWORD dwUserData)
	{
		if(!CCoreMsgBase::InternalInit(szServerName, dwUserData))
		{
			return FALSE;
		}
		::SetWindowText(GetSelfWindow(), NULL);
		m_hSvr = FindWindow(MSG_SERVER_CLSNAME, szServerName);
		if(!m_hSvr)
		{
			return FALSE;
		}
		m_dwClientID = dwUserData;
		return TRUE;
	}
public:
	SET_CLASS_NAME(MSG_CLIENT_CLSNAME)

	CCoreMsgClient()
	{
		m_hSvr = NULL;
		m_dwClientID = 0;
	}
	~CCoreMsgClient(){}
	virtual BOOL SendCoreMsg(HWND hWnd, core_msg_header * pHeader)
	{
		return CCoreMsgBase::SendCoreMsg(hWnd, pHeader);
	}
	virtual BOOL SendCoreMsg(core_msg_header * pHeader)
	{
		return CCoreMsgBase::SendCoreMsg(m_hSvr, pHeader);
	}
	virtual void Uninitialize()
	{
		m_hSvr = NULL;
		CCoreMsgBase::Uninitialize();
	}
	DWORD GetClientId(){ return m_dwClientID; }

	BOOL Connect(BOOL bAlterable = FALSE)
	{
		core_msg_connect msg;
		msg.bAlterable = bAlterable;
		msg.dwClientId = m_dwClientID;
		if(!SendCoreMsg(&msg))
		{
			return FALSE;
		}
		if(msg.dwRet)
		{
			return FALSE;
		}
		return TRUE;
	}
};
