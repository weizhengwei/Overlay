#pragma once
#include <windows.h> 
#include <tchar.h>

#define INSTANCES 1 
#define PIPE_TIMEOUT 200
#define INBUFSIZE 1024
#define OUTBUFSIZE 8192
typedef struct 
{ 
	OVERLAPPED oOverlap; 
	HANDLE hPipeInst; 
	TCHAR chRequest[INBUFSIZE]; 
	DWORD cbRead;
	TCHAR chReply[OUTBUFSIZE];
	DWORD cbToWrite; 
	DWORD dwState; 
	BOOL fPendingIO; 
} PIPEINST, *LPPIPEINST; 

class IPipeBufferListener
{
public:
   virtual HRESULT HandleReadBuf(PIPEINST& Pipe) = 0;
};

class CServerNamedPipe
{
public:
	CServerNamedPipe(LPCTSTR lpszServerName,LPCTSTR lpszPipeName,IPipeBufferListener* pBufferListener,DWORD dwMaxInstances=INSTANCES,DWORD dwDefaultTimeOut=PIPE_TIMEOUT);
	~CServerNamedPipe(void);
public:
	bool StartPipeServer();
	bool StopPipeServer();
protected:
	bool Initialize();
    bool UnInitialize();
	void Release();
	void WorkerThreadProc(void);
    bool PipeRunLoop();
	bool ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) ;
	void DisconnectAndReconnect(DWORD i); 
private:
	enum {
		CONNECTING_STATE=0,
		READING_STATE,
		WRITING_STATE
	};
	TCHAR m_szPipeName[MAX_PATH];
	DWORD m_nMaxInstance;
    DWORD m_dwTimeout;
	PIPEINST* m_Pipe; 
	HANDLE* m_hEvents; 
	volatile bool m_bStopping;
	HANDLE m_hStoppedEvent;
	IPipeBufferListener* m_pBufferListener;
    CRITICAL_SECTION	m_cs;
};
