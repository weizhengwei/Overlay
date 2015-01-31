#include "thread/thread.h"


Thread::Thread(void)
{
	_threadHandle = NULL;
	_threadId = 0;
}

Thread::~Thread(void)
{
	if (_threadHandle != NULL) {

		stop();
		// CloseHandle(_threadHandle);
		_threadHandle = NULL;
		_threadId = 0;
	}
}

bool Thread::start(size_t stackSize /* = 0 */)
{
	if (!this->init())
		return false;

	_threadHandle = ::CreateThread(NULL, stackSize, threadProc, this, 0, &_threadId);
	if (_threadHandle == NULL)
		return false;

	return true;
}

void Thread::stop(int exitCode)
{
	::TerminateThread(_threadHandle, (DWORD )exitCode);
	CloseHandle(_threadHandle);
	_threadHandle = NULL;
	_threadId = 0;
}

static DWORD exceptionFilter(EXCEPTION_POINTERS* excepInfo)
{
	//glog->log(LogError, "%s:%d -  exception code: %d, exception address: %p", 
	//	__FILE__, __LINE__, excepInfo->ExceptionRecord->ExceptionCode, 
	//	excepInfo->ExceptionRecord->ExceptionAddress);

	return EXCEPTION_EXECUTE_HANDLER;
}

DWORD __stdcall Thread::threadProc(void* param)
{

#define PAGE_SIZE		4096

	//BYTE guardPages[PAGE_SIZE * 2 + 1];
	//DWORD oldProt;
	//VirtualProtect(&guardPages[PAGE_SIZE + 1], 1, PAGE_READONLY, &oldProt);
	
	Thread* thisPtr = (Thread* )param;
	DWORD result;
	
	if (IsDebuggerPresent()) {

		result = (DWORD )thisPtr->run();

	} else {

		__try {

			result = (DWORD )thisPtr->run();

		} __except(exceptionFilter(GetExceptionInformation())) {

		}
	}

	__try {

		thisPtr->final(result);

	} __except(exceptionFilter(GetExceptionInformation())) {

	}
	
	return result;
}

long Thread::wait()
{
	WaitForSingleObject(_threadHandle, INFINITE);
	long r;
	GetExitCodeThread(_threadHandle, (LPDWORD )&r);
	return r;
}

//----------------------------------------------

Mutex::Mutex()
{
	InitializeCriticalSection(&_cs);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&_cs);
}

void Mutex::lock()
{
	EnterCriticalSection(&_cs);
}

bool Mutex::trylock()
{
	return TryEnterCriticalSection(&_cs) == TRUE;
}

void Mutex::unlock()
{
	LeaveCriticalSection(&_cs);
}
