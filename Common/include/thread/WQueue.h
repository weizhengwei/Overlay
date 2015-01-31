#pragma once

#include <windows.h>
#include <queue>

#define MAX_QUEUE_SIZE				(-1)

template <typename T>
class WQueue {
public:
	WQueue(size_t maxSize = MAX_QUEUE_SIZE);
	~WQueue();

	void enter(T& pkt);
	T leave();
	void destroy();
	size_t size();

protected:
	HANDLE			_handles[4];
	const size_t	_maxSize;
	typedef std::queue<T> PacketQueue;
	PacketQueue		_queue;
	bool			_destroyed;
};

//////////////////////////////////////////////////////////////////////////

#define NONFULL_EVENT	0
#define QUEUE_MUTEX		1
#define NONEMPTY_EVENT	2
#define QUEUE_MUTEX2	3

template <typename T>
WQueue<T>::WQueue(size_t maxSize /*  = MAX_QUEUE_SIZE */ ): _maxSize(maxSize)
{
	_handles[NONFULL_EVENT] = ::CreateEvent(NULL, TRUE, TRUE, NULL);
	if (_handles[NONFULL_EVENT] == NULL)
		throw "cannot create event";

	_handles[QUEUE_MUTEX] = ::CreateMutex(NULL, FALSE,NULL);
	if (_handles[NONFULL_EVENT] == NULL)
		throw "cannot create mutex";

	_handles[NONEMPTY_EVENT] = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (_handles[NONEMPTY_EVENT] == NULL)
		throw "cannot create event";

	_handles[QUEUE_MUTEX2] = _handles[QUEUE_MUTEX];

	_destroyed = false;
}

template <typename T>
WQueue<T>::~WQueue()
{
	destroy();
}

template <typename T>
void WQueue<T>::enter(T& pkt)
{
	::WaitForMultipleObjects(2, &_handles[NONFULL_EVENT], TRUE, INFINITE);
	if (_destroyed)
		return;

	_queue.push(pkt);
	if (_queue.size() == 1) {
		::SetEvent(_handles[NONEMPTY_EVENT]);
	}

	if (_queue.size() == _maxSize) {
		::ResetEvent(_handles[NONFULL_EVENT]);
	}

	::ReleaseMutex(_handles[QUEUE_MUTEX]);
}

template <typename T>
T WQueue<T>::leave()
{	
	::WaitForMultipleObjects(2, &_handles[NONEMPTY_EVENT], TRUE, INFINITE);
	if (_destroyed)
		return T();

	T pkt = _queue.front();
	_queue.pop();

	if (_queue.size() == _maxSize - 1) {
		::SetEvent(_handles[NONFULL_EVENT]);
	}

	if (_queue.size() == 0) {
		::ResetEvent(_handles[NONEMPTY_EVENT]);
	}

	::ReleaseMutex(_handles[QUEUE_MUTEX]);
	return pkt;
}

template <typename T>
size_t WQueue<T>::size()
{
	return _queue.size();
}

template <typename T>
void WQueue<T>::destroy()
{
	_destroyed = true;

	if (_handles[NONFULL_EVENT] != NULL) {
		::CloseHandle(_handles[NONFULL_EVENT]);
		_handles[NONFULL_EVENT] = NULL;
	}

	if (_handles[QUEUE_MUTEX] != NULL) {
		::CloseHandle(_handles[QUEUE_MUTEX]);
		_handles[QUEUE_MUTEX] = NULL;
	}

	if (_handles[NONEMPTY_EVENT] != NULL) {
		::CloseHandle(_handles[NONEMPTY_EVENT]);
		_handles[NONEMPTY_EVENT] = NULL;
	}
}
