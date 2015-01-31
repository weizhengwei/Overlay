#include "data/ThirdPartyAppManager.h"
#include "data/ThirdAppBase.h"
//////////////////////////////////////////////////////////////////////////
void DataBase::OnDataChange(DataBase* pData)
{
	g_appManager.SetModified();
}

//////////////////////////////////////////////////////////////////////////
void ThirdAppBase::AddToMyList()
{
	//if(IsHide())
	//{
	//	GetLocalInfo().SetIsHide(false);
	//}
	//if(!IsInMyGameList())
	if(GetLocalStatus() == ThirdAppBase::NOTADDED)
	{
		GetLocalInfo().SetLocalStatus(ThirdAppBase::ADDED);
		SYSTEMTIME t;
		GetLocalTime(&t);
		GetLocalInfo().SetAddedtime(t);
	}
	SetActive(true);
	//g_appManager.SetModified();
}

void ThirdAppBase::RemoveFromMyList()
{
	GetLocalInfo().SetLocalStatus(ThirdAppBase::NOTADDED);
	GetLocalInfo().SetDownTool(-1);
	GetLocalInfo().SetOrder(0);
	GetLocalInfo().SetVersion(_T(""));
	GetLocalInfo().SetEulaVersion(_T(""));

	SYSTEMTIME sysTime = {0};
	GetLocalInfo().SetAddedtime(sysTime);
	GetLocalInfo().SetLastplaytime(sysTime);
	GetLocalInfo().SetInstallDate(_T(""));
	GetLocalInfo().SetDownloadTime(_T(""));
	GetLocalInfo().SetDownloadPath(_T(""));
	GetLocalInfo().SetInstallPath(_T(""));
	GetLocalInfo().SetClientPath(_T(""));
	GetLocalInfo().SetClientCmd(_T(""));
	GetLocalInfo().SetLauncherPath(_T(""));
	GetLocalInfo().SetLauncherCmd(_T(""));
	GetLocalInfo().SetPatcherPath(_T(""));
	GetLocalInfo().SetPatcherCmd(_T(""));
	GetLocalInfo().SetUninstallPath(_T(""));
	GetLocalInfo().SetUninstallCmd(_T(""));

	GetLocalInfo().SetEulaVersion(_T("0"));
	//g_appManager.SetModified();
	GetLocalInfo().SetLocalStatus(NOTADDED);
	GetLocalInfo().SetIsActive(false);
}