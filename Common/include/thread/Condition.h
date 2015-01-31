#pragma once

#include "tstring/tstring.h"
using namespace String ;

class Condition {
public:
	Condition(bool sig, bool manual = false, _tstring sName = _tstring())
	{
		_event = CreateEvent(NULL, manual, sig, sName.c_str());
	}

	virtual ~Condition()
	{
		CloseHandle(_event);
	}

	bool wait()
	{
		return WaitForSingleObject(_event, INFINITE) == WAIT_OBJECT_0;	
	}

	bool wait(DWORD dwInterval)
	{
		return WaitForSingleObject(_event, dwInterval) ;
	}

	bool signal()
	{
		return SetEvent(_event) == TRUE;
	}

	bool pulse()
	{
		return PulseEvent(_event) == TRUE;
	}

	void reset()
	{
		ResetEvent(_event);
	}

private:
	Condition(const Condition&)
	{

	}

private:
	HANDLE		_event;
};