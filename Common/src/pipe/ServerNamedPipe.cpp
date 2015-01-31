#include "pipe/ServerNamedPipe.h"
#include "thread/ThreadPool.h"

CServerNamedPipe::CServerNamedPipe(LPCTSTR lpszServerName,LPCTSTR lpszPipeName,IPipeBufferListener* pBufferListener,DWORD dwMaxInstances,DWORD dwDefaultTimeOut):m_dwTimeout(dwDefaultTimeOut),m_bStopping(false)
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
	m_nMaxInstance = dwMaxInstances>0 ? dwMaxInstances:1;
	if (m_nMaxInstance > PIPE_UNLIMITED_INSTANCES )
	{
		m_nMaxInstance = PIPE_UNLIMITED_INSTANCES;
	}
	m_hStoppedEvent = NULL;
	m_pBufferListener = pBufferListener;
    m_hEvents  =NULL;
    m_Pipe = NULL;
    InitializeCriticalSection(&m_cs);
}

CServerNamedPipe::~CServerNamedPipe(void)
{
	DeleteCriticalSection(&m_cs);
}
bool CServerNamedPipe::Initialize()
{
	//The initial loop creates several instances of a named pipe along with an event object for each instance.An overlapped ConnectNamedPipe operation is started for each instance. 
	m_hEvents = new HANDLE[m_nMaxInstance];
	m_Pipe    = new PIPEINST[m_nMaxInstance];

    for (int i=0;i<m_nMaxInstance;i++)
    {
		m_hEvents[i] = NULL;
		m_Pipe[i].hPipeInst = INVALID_HANDLE_VALUE;
    }

	for (int i = 0; i < m_nMaxInstance; i++) 
	{ 
		// Create an event object for this instance. 
		m_hEvents[i] = CreateEvent(NULL,TRUE,TRUE, NULL);   
		if (m_hEvents[i] == NULL) 
		{
			return false;
		}

		m_Pipe[i].oOverlap.hEvent = m_hEvents[i]; 
 
		//create security attributes
		SECURITY_ATTRIBUTES sa;
		SECURITY_DESCRIPTOR sd;
		BYTE aclBuffer[1024], sidBuffer[100];
		PACL pacl = (PACL)&aclBuffer;
		PSID psid = (PSID) &sidBuffer;
		SID_NAME_USE snu;
		DWORD sidBufferSize = 100, domainBufferSize = 80;
		TCHAR domainBuffer[80];
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		InitializeAcl(pacl, 1024, ACL_REVISION);
		BOOL bRet= LookupAccountName(0, _T("everyone"), psid, &sidBufferSize, domainBuffer,&domainBufferSize, &snu);
        if (!bRet)
        {
           TCHAR szUserName[MAX_PATH]={0};
           DWORD  dwLen=MAX_PATH;
           ::GetUserName(szUserName,&dwLen);
           bRet = LookupAccountName(0, szUserName, psid, &sidBufferSize, domainBuffer,&domainBufferSize, &snu);
           if (!bRet)
           {
              return false;
           }
        }
		AddAccessAllowedAce(pacl, ACL_REVISION, GENERIC_ALL, psid);
		SetSecurityDescriptorDacl(&sd, TRUE, pacl, FALSE);
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = FALSE;

		m_Pipe[i].hPipeInst = CreateNamedPipe( m_szPipeName,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,\
			PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,m_nMaxInstance,INBUFSIZE*sizeof(TCHAR),OUTBUFSIZE*sizeof(TCHAR),m_dwTimeout,&sa);
		if (m_Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) 
		{
			return false;
		}

		// Call the subroutine to connect to the new client
		m_Pipe[i].fPendingIO = ConnectToNewClient( m_Pipe[i].hPipeInst, &m_Pipe[i].oOverlap); 
		m_Pipe[i].dwState = m_Pipe[i].fPendingIO ? CONNECTING_STATE :READING_STATE;
	} 
	return true;
}
void CServerNamedPipe::Release()
{
	// Disconnect the pipe instances. 
	for (int i = 0; i < m_nMaxInstance; i++) 
	{
		DisconnectNamedPipe(m_Pipe[i].hPipeInst);
	}
}
bool CServerNamedPipe::ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{ 
	BOOL fConnected, fPendingIO = FALSE; 

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo); 

	// Overlapped ConnectNamedPipe should return false. 
	if (fConnected) 
	{
		return false;
	}

	switch (GetLastError()) 
	{ 
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING: 
		fPendingIO = TRUE; 
		break; 

		// Client is already connected, so signal an event. 
	case ERROR_PIPE_CONNECTED: 
		if (SetEvent(lpo->hEvent)) 
			break; 

		// If an error occurs during the connect operation... 
	default: 
		{
			return false;
		}
	} 

	return fPendingIO; 
}
void CServerNamedPipe::DisconnectAndReconnect(DWORD i) 
{ 
	// Disconnect the pipe instance. 
	DisconnectNamedPipe(m_Pipe[i].hPipeInst);

	// Call a subroutine to connect to the new client. 
	m_Pipe[i].fPendingIO = ConnectToNewClient(m_Pipe[i].hPipeInst, &m_Pipe[i].oOverlap); 

	m_Pipe[i].dwState = m_Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
} 

bool CServerNamedPipe::StartPipeServer()
{
	__try
	{
		EnterCriticalSection(&m_cs);
		if (m_hStoppedEvent != NULL)
		{
			CloseHandle(m_hStoppedEvent);
			m_hStoppedEvent =NULL;
		}
		m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hStoppedEvent == NULL)
		{
			__leave ;
		}
	}
	__finally
	{
		LeaveCriticalSection(&m_cs);
	}

	// Queue the main service function for execution in a worker thread.
	CThreadPool::QueueUserWorkItem(&CServerNamedPipe::WorkerThreadProc, this);
	return true;
}
bool CServerNamedPipe::StopPipeServer()
{
	//for (int i=0;i<100000;i++)
	//{
        //OutputDebugString(L"StopPipeServer\n");
        EnterCriticalSection(&m_cs);
		m_bStopping = TRUE;
		//should pay attention to avoid block caller thread when the server thread is sleeping(waiting some things).
		if (m_hStoppedEvent != NULL) 
		{
			WaitForSingleObject(m_hStoppedEvent, INFINITE);
			CloseHandle(m_hStoppedEvent);
			m_hStoppedEvent = NULL;
		}
        LeaveCriticalSection(&m_cs);
        //Sleep(1000);
	//}
	return true;
}

bool CServerNamedPipe::UnInitialize()
{
	m_bStopping = false;
	for (int i=0;i<m_nMaxInstance;i++)
	{
		if (m_Pipe[i].hPipeInst != INVALID_HANDLE_VALUE)
		{
			DisconnectNamedPipe(m_Pipe[i].hPipeInst);
            CloseHandle(m_Pipe[i].hPipeInst );
            m_Pipe[i].hPipeInst =NULL;
		}
		CloseHandle((HANDLE)m_hEvents[i]);
		m_hEvents[i] = NULL;
	}
	delete []m_hEvents;
	delete []m_Pipe;
	return true;
}

void CServerNamedPipe::WorkerThreadProc(void)
{
	//trace the server work thread.
	bool bRet= false;
	__try
	{
		bRet = PipeRunLoop();
	}
	__finally
	{
        UnInitialize();

		//should pay attention to avoid block caller thread when the server thread is sleeping(waiting some things).
		if (m_hStoppedEvent != NULL) 
		{
			::SetEvent(m_hStoppedEvent);
		}
		if (!bRet)
		{
            //OutputDebugString(L"StartPipeServer\n");
            StartPipeServer(); // restart pipe server when an exception encounters.
		}
	
	}
}
 //bool bTest = false;
bool CServerNamedPipe::PipeRunLoop(void)
{

	DWORD  cbRet, dwErr;
	BOOL bSuccess; 
	
	if (!Initialize())
	{
		return false;
	}

	//Periodically check if the service is stopping.
	while (!m_bStopping)
	{
		// Wait for the event object to be signaled, indicating completion of an overlapped read, write, or connect operation. 
		DWORD dwWait = WaitForMultipleObjects(m_nMaxInstance,m_hEvents,FALSE,m_dwTimeout);
		if(dwWait == WAIT_TIMEOUT) 
			continue;

		//dwWait shows which pipe completed the operation.
		DWORD i = dwWait - WAIT_OBJECT_0;
		if (i < 0 || i > (m_nMaxInstance - 1)) 
		{
			return false;
		}
        
		// Get the result if the operation was pending. 
		if (m_Pipe[i].fPendingIO) 
		{ 
			bSuccess = GetOverlappedResult(m_Pipe[i].hPipeInst,&m_Pipe[i].oOverlap, &cbRet,FALSE);
			switch (m_Pipe[i].dwState) 
			{ 
			//Pending connect operation 
			case CONNECTING_STATE: 
				if (!bSuccess) 
				{
					return false;
				}
				m_Pipe[i].dwState = READING_STATE; 
				break; 

				// Pending read operation 
			case READING_STATE: 
				if (! bSuccess || cbRet == 0) 
				{ 
					DisconnectAndReconnect(i); 
					continue; 
				} 
				m_Pipe[i].dwState = WRITING_STATE; 
				break; 

				// Pending write operation 
			case WRITING_STATE: 
				if (! bSuccess || cbRet != m_Pipe[i].cbToWrite) 
				{ 
					DisconnectAndReconnect(i); 
					continue; 
				} 
				m_Pipe[i].dwState = READING_STATE; 
				break; 

			default: 
				{
					return false;
				}
			} 
		}  

		//if (!bTest)
		//{ 
			//bTest =true;
			//m_Pipe[i].dwState=6;
		//}
		// The pipe state determines which operation to do next. 
		switch (m_Pipe[i].dwState) 
		{ 		
		case READING_STATE: 
			{
				// READING_STATE:The pipe instance is connected to the client and is ready to read a request from the client. 
				m_Pipe[i].oOverlap.Offset=0;
				m_Pipe[i].oOverlap.OffsetHigh=0;
                memset(m_Pipe[i].chRequest,0,sizeof(TCHAR)*INBUFSIZE);//clear the dirty data.
				bSuccess = ReadFile(m_Pipe[i].hPipeInst, m_Pipe[i].chRequest, sizeof(TCHAR)*INBUFSIZE, &m_Pipe[i].cbRead, &m_Pipe[i].oOverlap); 

				// The read operation completed successfully. 
				if (bSuccess && m_Pipe[i].cbRead != 0) 
				{ 
					m_Pipe[i].fPendingIO = FALSE; 
					m_Pipe[i].dwState = WRITING_STATE; 
					continue; 
				} 

				// The read operation is still pending. 
				dwErr = GetLastError(); 
				if (! bSuccess && (dwErr == ERROR_IO_PENDING)) 
				{ 
					m_Pipe[i].fPendingIO = TRUE; 
					continue; 
				} 

				// An error occurred; disconnect from the client. 
				DisconnectAndReconnect(i); 
			}
			break; 

		case WRITING_STATE: 
			{
				// WRITING_STATE: The request was successfully read from the client. Get the reply data and write it to the client. 
				m_pBufferListener->HandleReadBuf(m_Pipe[i]); 

				bSuccess = WriteFile( m_Pipe[i].hPipeInst, m_Pipe[i].chReply, m_Pipe[i].cbToWrite, &cbRet, &m_Pipe[i].oOverlap); 

				// The write operation completed successfully. 
				if (bSuccess && cbRet == m_Pipe[i].cbToWrite) 
				{ 
					m_Pipe[i].fPendingIO = FALSE; 
					m_Pipe[i].dwState = READING_STATE; 
					continue; 
				} 

				// The write operation is still pending. 
				dwErr = GetLastError(); 
				if (!bSuccess && (dwErr ==ERROR_IO_PENDING)) 
				{ 
					m_Pipe[i].fPendingIO = TRUE; 
					continue; 
				} 

				// An error occurred; disconnect from the client. 
				DisconnectAndReconnect(i); 
			}
			break; 

		default: 
			{
				return false;
			}
		} 
	}

	//Release();
	//::Sleep(2000);//Simulate some lengthy operations.
    //SetEvent(m_hStoppedEvent);// Signal the stopped event.
   return true;
}