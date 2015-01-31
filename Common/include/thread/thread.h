#pragma once

#include <windows.h>

class Thread
{
public:
	Thread(void);
	virtual ~Thread(void);

	virtual bool init()
	{
		return true;
	}
	
	virtual long run() = 0;

	virtual void final(long result)
	{

	}

	bool start(size_t stackSize = 0);
	void stop(int exitCode = 0);

	long wait();

protected:
	static DWORD __stdcall threadProc(void* param);

protected:
	DWORD		_threadId;
	HANDLE		_threadHandle;
	bool		_bExit;
};

//--------------------------------
class Mutex 
{
public:
	Mutex();
	virtual ~Mutex();

	void lock();
	bool trylock();
	void unlock();

private:
	Mutex(const Mutex& )
	{

	}

private:
	CRITICAL_SECTION	_cs;
};

//-------------------------------

template<class LOCK_TYPE>
class LockGuard {
public:
	LockGuard (LOCK_TYPE* lock): _lock(lock)
	{
		lock->lock();
	}

	~LockGuard ()
	{
		_lock->unlock();
	}

protected:
	LOCK_TYPE*	_lock;
};

typedef LockGuard<Mutex> MutexGuard;
