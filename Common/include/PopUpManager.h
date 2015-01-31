#pragma once

#include <set>
#include "thread/thread.h"
#include "thread/WQueue.h"
using namespace std;

class IPopUpManagerListener
{
public:
	virtual void Listener_ShowTipWindow(PVOID pTempPopUp){ return ;}
};


class CPopUpManager : public Thread
{
public:
	CPopUpManager();
	virtual ~CPopUpManager();
	bool Init();
	BOOL ShowPopUpWindow(PVOID pTempPopUp);
	bool RegisterListener(IPopUpManagerListener *pListener);
	bool UnRegisterListener(IPopUpManagerListener *pListener);
protected:
	virtual long run();
private:
	WQueue<PVOID > m_PopUpQueue;
	bool m_bStop;
	set<IPopUpManagerListener*> m_Listener;
	Mutex m_LockListener;
	HANDLE m_hEvent;

};