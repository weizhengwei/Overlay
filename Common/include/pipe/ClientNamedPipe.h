#pragma once
#include <windows.h> 
#include <tchar.h>

class CClientNamedPipe
{
public:
	CClientNamedPipe(LPCTSTR lpszServerName,LPCTSTR lpszPipeName,DWORD dwTimeOut);
	~CClientNamedPipe(void);
public:
	HRESULT PipeSendAndReceive(TCHAR* SendBuf,DWORD cbRequest,TCHAR* RsvBuf, DWORD cbReply);
	HRESULT Initialize();
protected:
	void UnInitialize();

private:
	HANDLE m_hPipe;
	DWORD  m_dwTimeOut;
    TCHAR m_szPipeName[MAX_PATH];
};
