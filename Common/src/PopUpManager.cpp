#include "stdafx.h"
#include "PopUpManager.h"

CPopUpManager::CPopUpManager()
:m_bStop(false)
{

}

CPopUpManager::~CPopUpManager()
{
	if (m_hEvent)
	{
		CloseHandle(m_hEvent);
	}
}

bool CPopUpManager::Init()
{
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, POPUP_CLOSE_EVENT);
	return Thread::start();
}

BOOL CPopUpManager::ShowPopUpWindow(PVOID pTempPopUp)
{
	m_PopUpQueue.enter(pTempPopUp);
	return TRUE;
}

bool CPopUpManager::RegisterListener(IPopUpManagerListener *pListener)
{
	m_LockListener.lock();
	if (pListener != NULL)
	{
		m_Listener.insert(pListener);
	}
	m_LockListener.unlock();
	return true;
}

bool CPopUpManager::UnRegisterListener(IPopUpManagerListener *pListener)
{
	m_LockListener.lock();
	if (m_Listener.find(pListener) != m_Listener.end())
	{
		m_Listener.erase(pListener);
	}
	m_LockListener.unlock();
	return true;
}

long CPopUpManager::run()
{
	while (!m_bStop)
	{
		PVOID pTempPopUp = m_PopUpQueue.leave();
		if (pTempPopUp)
		{
			set<IPopUpManagerListener*>::iterator iter = m_Listener.begin();
			while (iter != m_Listener.end())
			{
				(*iter)->Listener_ShowTipWindow(pTempPopUp);
				iter++;
			}
			WaitForSingleObject(m_hEvent, INFINITE);
			Sleep(500);
		}
	}
	return 0;
}