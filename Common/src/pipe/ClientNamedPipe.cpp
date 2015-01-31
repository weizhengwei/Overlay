#include "pipe/ClientNamedPipe.h"

CClientNamedPipe::CClientNamedPipe(LPCTSTR lpszServerName,LPCTSTR lpszPipeName,DWORD dwTimeOut)
{
	//Construct the canonical pipe name
	if (NULL == lpszServerName)
	{
		_stprintf_s(m_szPipeName,MAX_PATH,_T("\\\\.\\PIPE\\%s"),lpszPipeName);
	}
	else
	{
		_stprintf_s(m_szPipeName,MAX_PATH,_T("\\\\%s\\PIPE\\%s"), lpszServerName, lpszPipeName);
	}
	m_dwTimeOut = dwTimeOut;
	m_hPipe = INVALID_HANDLE_VALUE;
}

CClientNamedPipe::~CClientNamedPipe(void)
{
	UnInitialize();
}
HRESULT CClientNamedPipe::PipeSendAndReceive(TCHAR* SendBuf,DWORD cbRequest,TCHAR* RsvBuf, DWORD cbReply)
{
	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		return HRESULT_FROM_WIN32(ERROR_PIPE_NOT_CONNECTED);
	}
	
	HRESULT status = S_OK;
	DWORD dwError = ERROR_SUCCESS;
	DWORD cbWritten=0,cbRead=0;
	
	do{
		if (!SendBuf || cbRequest==0)
		{
			break;
		}
		//call writefile(pipe handle,SendBuf)
		if (!WriteFile(m_hPipe,SendBuf,cbRequest,&cbWritten,NULL))
		{
			dwError = GetLastError();
			break;
		}
        if (!RsvBuf || cbReply==0)
        {
			break;
        }
		//call readfile (pipe handle,RecvBuf) 
		BOOL fFinishRead = ReadFile(m_hPipe,RsvBuf,cbReply,&cbRead,NULL);
		if (!fFinishRead )
		{
			dwError = GetLastError();
		}
	}while(FALSE);

	status = HRESULT_FROM_WIN32(dwError);
	return status;
}
HRESULT CClientNamedPipe::Initialize()
{
	DWORD dwError = ERROR_SUCCESS;
	HRESULT status = S_OK;

	// Try to open the named pipe identified by the pipe name.
	while (TRUE) 
	{
		if (m_hPipe != INVALID_HANDLE_VALUE)
		{
			break;
		}
		m_hPipe = CreateFile( m_szPipeName,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);

		// If the pipe handle is opened successfully.
		if (m_hPipe != INVALID_HANDLE_VALUE)
		{
			break;
		}

		dwError = GetLastError();
		// Exit if an error other than ERROR_PIPE_BUSY occurs.
		if (ERROR_PIPE_BUSY != dwError)
		{
			// can not find the pipe name!
			if( ERROR_FILE_NOT_FOUND == dwError )
			{
				status= HRESULT_FROM_WIN32(ERROR_PIPE_NOT_CONNECTED);
			}
			else
			{
				status= HRESULT_FROM_WIN32(dwError);
			}
			break;
		}

		// All pipe instances are busy, so wait for it.
		if (!WaitNamedPipe(m_szPipeName, m_dwTimeOut))
		{
			dwError = GetLastError();
			status= HRESULT_FROM_WIN32(dwError);
			break;
		}
	}

	//Set the read mode and the blocking mode of the named pipe.we set data to be read from the pipe as a stream of messages.
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	if (!SetNamedPipeHandleState(m_hPipe, &dwMode, NULL, NULL))
	{
		dwError = GetLastError();
		status = HRESULT_FROM_WIN32(dwError);
	}	
	if (status != S_OK)
	{
		UnInitialize();// if initialized failure,close the connection.
	}
	return status;
}
void CClientNamedPipe::UnInitialize()
{
	if (m_hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;
	}
}