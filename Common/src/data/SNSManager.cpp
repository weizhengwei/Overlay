#include <time.h>
#include <stdafx.h>
#include "data/SNSManager.h"
#include "data/DataPool.h"
#include "data/NotifyLog.h"
#include "data/Tracking.h"
#include "data/Utility.h"

#define CONFIG_VIEWFEED_IDLOG	_T("tmp\\ViewFeed.log")
#define FILEPATH_DEFAULT_CHARACTOR	_T("themes\\sonic\\default_charactor.png")
#define ALERT_FILENAME	            (theDataPool.GetUserName() + _T("Alert Item")).c_str()
#define HIDE_ITEM_FILENAME          (theDataPool.GetUserName() + _T("Hide List")).c_str()

CSNSManager g_theSNSManager ;

CSNSManager::CSNSManager()
	:m_bStop(true),
	 m_vecDataCount(6,0),
	 m_uMoreFeedCount(0),
	 m_hThread(0)
{
}

bool CSNSManager::Init()
{
	m_bStop = false ;
	//create a thread for some slow download task
	m_hThread = CreateThread(NULL, NULL, DownloadFileProc, this, NULL, NULL);
	return Thread::start();
}

void CSNSManager::Uninit(int code)
{
	m_bStop = true;
	if (m_hThread != NULL)
	{
		TerminateThread(m_hThread, 0);
		m_hThread = NULL;
	}
	Thread::stop(code);

	ClearListener() ;
}

bool CSNSManager::RegisterListener(ISNSListener* pListener)
{
	if (!pListener)
	{
		return false;
	}
	m_lockListener.lock() ;
	m_mapSNSListener.insert(pair<ISNSListener*, int>(pListener, 1)) ;
	m_lockListener.unlock() ;
	return true ;
}

bool CSNSManager::UnregisterListener(ISNSListener* pListener)
{
	m_lockListener.lock() ;
	if (m_mapSNSListener.find(pListener) == m_mapSNSListener.end())
	{
		m_lockListener.unlock() ;
		return false ;
	}

	bool bRet = m_mapSNSListener.erase(pListener) ;
	m_lockListener.unlock() ;

	return bRet ;
}

bool CSNSManager::ClearListener()
{
	m_lockListener.lock() ;
	map<ISNSListener*, int>::iterator iter = m_mapSNSListener.begin() ;
	while(iter != m_mapSNSListener.end())
	{
		iter = m_mapSNSListener.erase(iter) ;
	}
	m_lockListener.unlock() ;

	return true ;
}

bool CSNSManager::GetDataCountService()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_GET_DATACOUNT ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetAllChangedService()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_GET_ALLCHANGED ;

	m_wTaskQueue.enter(pInfo) ;
	return true ;
}

bool CSNSManager::GetInboxMessageService()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_INBOX_MSG ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetFriendRequestService()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_FRIEND_REQ ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetConnectUpdateService()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_CONNECT_UPDATE ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetCouponMessageService()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_COUPON ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetAchievementService(_tstring snId, uint32 nShowOption)
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_NOTIFY ;
	pInfo->sAchieveId = snId ;
	pInfo->nShowOption = nShowOption ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::ResponseFriendReq(int fid,int rid,bool bAccept, PVOID pBaseNotifyDlg)
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_RESPONSE_FRDREQ ;
	pInfo->nFid = fid ;
	pInfo->nRid = rid ;
	pInfo->bAccept = bAccept ;
	pInfo->pBaseNotifyDlg = pBaseNotifyDlg;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::ClearConncectUpdateItems()
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_CLEAR_CNTUPT ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetGameCharactorService(_tstring sUserName, _tstring sNickName)
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_GET_GAMECHARACTER ;
	pInfo->sUserName = sUserName ;
	pInfo->sNickName = sNickName ;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

bool CSNSManager::GetGameServerService(vector<CWebData::WEB_GAMESERVER_INFO>& vecGameServerInfo)
{
	//PSNS_INFO pInfo = new SNS_INFO ;
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	//pInfo->nType = CWebData::SERVICE_GET_GAMESERVER ;

	//m_wTaskQueue.enter(pInfo) ;

	return m_sWebData.GetGameServerService(m_sSessionId, vecGameServerInfo) ;
}


bool CSNSManager::GetEmsDataService(bool bNotify,const std::string sEmsData)
{
	PSNS_INFO pInfo = new SNS_INFO();
	//memset(pInfo, 0, sizeof(SNS_INFO)) ;
	pInfo->nType = CWebData::SERVICE_EMS_NOTITY ;
    pInfo->bNotify = bNotify;
    pInfo->sEmsData = sEmsData;

	m_wTaskQueue.enter(pInfo) ;

	return true ;
}

long CSNSManager::run()
{
	while (!m_bStop)
	{
		SNS_INFO* pInfo = m_wTaskQueue.leave();
		if (pInfo != NULL)
		{
			bool bRet = false ;
			switch(pInfo->nType)
			{
			case CWebData::SERVICE_GET_DATACOUNT:
				{
					vector<int> vecCount ;
					bRet = m_sWebData.GetService(m_sSessionId, vecCount) ;
					if (bRet)
					{
						SetDataCount(vecCount) ;
					}
				}
				break ;
			case CWebData::SERVICE_GET_ALLCHANGED:
				{
                    GetInboxMessageService();
					GetFriendRequestService();
					//update the bottom tip number, no matter GetService fail or succeed
					PSNS_INFO pTmpInfo = new SNS_INFO();
					//memset(pTmpInfo, 0, sizeof(SNS_INFO)) ;
					pTmpInfo->nType = CWebData::SERVICE_UPDATE_NUMBER;
					m_wTaskQueue.enter(pTmpInfo);
					bRet = true;
				}
				break ;
			case CWebData::SERVICE_INBOX_MSG:
				{
					vector<CWebData::WEB_INBOX_MSG> vecInbox ;
					bRet = m_sWebData.GetInboxMessageService(m_sSessionId, vecInbox) ;
					if (bRet)
					{
						//ResetWebData(CWebData::SERVICE_INBOX_MSG) ;
						SetInboxData(vecInbox) ;
					}
				}
				break ;
			case CWebData::SERVICE_FRIEND_REQ:
				{
					vector<CWebData::WEB_FRIEND_REQ> vecFriendReq ;
					bRet = m_sWebData.GetFriendRequestService(m_sSessionId, vecFriendReq) ;
					if (bRet)
					{
						ResetWebData(CWebData::SERVICE_FRIEND_REQ) ;
						SetFriendReqData(vecFriendReq) ;
						//just for download the avatar file
						//download avatar is slow, so move it to the other thread
						SNS_INFO* pInfoNew = new SNS_INFO();
						//memset(pInfoNew, 0, sizeof(SNS_INFO));
						pInfoNew->nType = CWebData::SERVICE_FRIEND_REQ;
						m_wDownloadFile.enter(pInfoNew);
					}
				}
				break ;
			case CWebData::SERVICE_CLEAR_CNTUPT:
				{
					m_sWebData.ClearConncectUpdateItems(m_sSessionId) ;
                    bRet =true;
				}
				break ;
			case CWebData::SERVICE_GET_GAMECHARACTER:
				{
					//this task is slow, so move it to the other thread
					SNS_INFO* pInfoNew = new SNS_INFO();
					//memset(pInfoNew, 0, sizeof(SNS_INFO));
					pInfoNew->nType = CWebData::SERVICE_GET_GAMECHARACTER ;
					pInfoNew->sUserName = pInfo->sUserName ;
					pInfoNew->sNickName = pInfo->sNickName ;
					m_wDownloadFile.enter(pInfoNew);
                    bRet = true;
				}
				break;
			case CWebData::SERVICE_RESPONSE_FRDREQ:
				{
					bRet = m_sWebData.ResponseFriendReq(m_sSessionId, pInfo->nFid, pInfo->nRid, pInfo->bAccept) ;

					BOOL bSuccess = bRet ? 1 : 0;
					m_lockListener.lock() ;
					map<ISNSListener*, int>::iterator iter = m_mapSNSListener.begin() ;
					if (!pInfo->pBaseNotifyDlg)
					{
						//this action is done in the overlay
						
					}
					else
					{
						for (; iter != m_mapSNSListener.end(); ++ iter)
						{
							iter->first->SNS_OnResponseWebService(CWebData::SERVICE_RESPONSE_FRDREQ, bSuccess, pInfo->nFid, pInfo->pBaseNotifyDlg, pInfo->bAccept) ;
						}
					}
					m_lockListener.unlock() ;
				}
				break ;
			case CWebData::SERVICE_EMS_NOTITY:
				{
                   CWebData::WEB_EMS_DATA ems={_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T("")};
                   ems._szIconPath = theDataPool.GetBaseDir()+_T("tmp\\");
                    bRet = m_sWebData.GetEmsService(theDataPool.GetLangAbbrString(),ems,pInfo->bNotify,pInfo->sEmsData);
                    if (bRet)
                    {
						SetEmsData(ems);
                    }
                
                   //bRet =true; //no need retry.
				}
				break;
			case CWebData::SERVICE_UPDATE_NUMBER:
			default:
				{
					bRet = true;//avoid retry.
				}
				break ;
			}

			if (!bRet && (pInfo->nRetry < CSNSManager::RESEND_TIMES))
			{//re-queue the task if failed.
				++ pInfo->nRetry ;
				m_wTaskQueue.enter(pInfo) ;
				continue ;
			}

			switch(pInfo->nType)
			{
			case CWebData::SERVICE_UPDATE_NUMBER:
				{
					m_lockListener.lock() ;
					map<ISNSListener*, int>::iterator iter = m_mapSNSListener.begin() ;
					for (; iter != m_mapSNSListener.end(); ++ iter)
					{
						iter->first->SNS_ServiceCountUpdated() ;
					}
					m_lockListener.unlock() ;
				}
				break ;
            case CWebData::SERVICE_EMS_NOTITY:
				{
					m_lockListener.lock() ;
					map<ISNSListener*, int>::iterator iter = m_mapSNSListener.begin() ;
					for (; iter != m_mapSNSListener.end(); ++ iter)
					{
						iter->first->SNS_ServiceEmsAvailable() ;
					}
					m_lockListener.unlock() ;
				}
				break ;
			default:
				break ;
			}

			delete pInfo ;
			pInfo = NULL ;
		}
	}

	return 0 ;
}

DWORD CSNSManager::DownloadFileProc(LPVOID lpParameter)
{
	if (lpParameter == NULL)
	{
		return 0;
	}
	CSNSManager *pSNSManager = (CSNSManager*)lpParameter;
	while(!pSNSManager->m_bStop)
	{
		SNS_INFO* pInfo = pSNSManager->m_wDownloadFile.leave();
		if (pInfo != NULL)
		{
			bool bRet = false;
			switch(pInfo->nType)
			{
			case CWebData::SERVICE_FRIEND_REQ:
				{
					bRet = pSNSManager->post_process_getfriendrequest();
				}
				break;
			case CWebData::SERVICE_GET_GAMECHARACTER:
				{
					CWebData::WEB_CHARACT_MSG sCharData={_T(""),_T(""),_T(""),_T(""),_T(""),_T("")};
					sCharData._szName = pInfo->sUserName ;
					sCharData._szNick = pInfo->sNickName ;
					bRet = pSNSManager->m_sWebData.GetGameCharactorService(pSNSManager->m_sSessionId, sCharData) ;
					if (bRet)
					{
						pSNSManager->post_process_getgamecharacter(sCharData);
					}
				}
				break;
			default:
				{
                   bRet =true;//avoid retry.
				}
				break;
			}
			if (!bRet && (pInfo->nRetry < CSNSManager::RESEND_TIMES))
			{//re-queue the task if failed.
				++ pInfo->nRetry ;
				pSNSManager->m_wDownloadFile.enter(pInfo) ;
				continue ;
			}
			switch(pInfo->nType)
			{
			case CWebData::SERVICE_GET_GAMECHARACTER:
				{
					pSNSManager->m_lockListener.lock() ;
					map<ISNSListener*, int>::iterator iter = pSNSManager->m_mapSNSListener.begin() ;
					for (; iter != pSNSManager->m_mapSNSListener.end(); ++ iter)
					{
						iter->first->SNS_ServiceCharacterInfoUpdated(pInfo->sUserName, pInfo->sNickName) ;
					}
					pSNSManager->m_lockListener.unlock() ;
				}
				break;
			default:
				break;
			}
			delete pInfo;
			pInfo = NULL;
		}
	}
	return 0;
}

bool CSNSManager::post_process_getgamecharacter(CWebData::WEB_CHARACT_MSG sCharData)
{
	int nPos = sCharData._szAvatar.rfind(_T('/'));
	if(nPos <= 0)
	{
		sCharData._szHeaderPath = FILEPATH_DEFAULT_CHARACTOR ;
	}
	else
	{
		sCharData._szHeaderPath = _T("tmp\\avatar\\") ;
		sCharData._szHeaderPath.append(sCharData._szAvatar.Right(sCharData._szAvatar.length( )-nPos-1)) ;	
		_tstring szPath = theDataPool.GetBaseDir() + sCharData._szHeaderPath ;
		m_sWebData.DownloadFile(sCharData._szAvatar.c_str( ), szPath.c_str()) ;
	}

	m_lockWebData.lock() ;
	if (m_mapCharData.find(sCharData._szNick) != m_mapCharData.end())
	{
		m_mapCharData[sCharData._szNick]._szHeaderPath = sCharData._szHeaderPath;
		m_mapCharData[sCharData._szNick]._szAvatar = sCharData._szAvatar;
		m_mapCharData[sCharData._szNick]._szName = sCharData._szName;
        m_mapCharData[sCharData._szNick]._szCharater = sCharData._szCharater;
		m_mapCharData[sCharData._szNick]._szServer = sCharData._szServer;
	}
	else
	{
		m_mapCharData.insert(pair<_tstring,CWebData::WEB_CHARACT_MSG>(sCharData._szNick,sCharData));
	}
	m_lockWebData.unlock() ;

	return true ;
}

bool CSNSManager::post_process_responsefriendreq()
{//when GetFriendRequest function returns, we should update the count vector and friend request vector.
	return GetDataCountService()&&GetFriendRequestService() ;
}

bool CSNSManager::post_process_getfriendrequest()
{
	vector<CWebData::WEB_FRIEND_REQ> vecFriendReq ;
	m_lockFriendReq.lock() ;
	vecFriendReq = m_vecFriendReq ;
	m_lockFriendReq.unlock() ;

	//download avatar of users in friend requests
	for (int i = 0 ; i < vecFriendReq.size(); ++ i)
	{
		if (!vecFriendReq[i]._szAvatar.empty())
		{
			size_t   nCount      = vecFriendReq[i]._szAvatar.size() - vecFriendReq[i]._szAvatar.rfind('/') - 1 ;
			_tstring sAvatarPath = _T("tmp\\") + vecFriendReq[i]._szAvatar.Right(nCount) ;
			_tstring sPath       = theDataPool.GetBaseDir() + sAvatarPath ;
			if(m_sWebData.DownloadFile(vecFriendReq[i]._szAvatar.c_str(),sPath.c_str( )))
			{
				vecFriendReq[i]._szHeaderPath = sAvatarPath;
			}
		}
	}

	m_lockFriendReq.lock() ;
	m_vecFriendReq = vecFriendReq ;
	m_lockFriendReq.unlock() ;

	return true ;
}

vector<int> CSNSManager::GetDataCount()
{
	vector<int> vecCount ;

	m_lockWebData.lock() ;
	vecCount = m_vecDataCount ;
	m_lockWebData.unlock() ;

	return vecCount ;
}

vector<CWebData::WEB_INBOX_MSG> CSNSManager::GetInboxData()
{
	vector<CWebData::WEB_INBOX_MSG> vecInbox ;

	m_lockInbox.lock() ;
	for (vector<CWebData::_WEB_INBOX_MSG>::iterator it = m_vecInbox.begin(); it != m_vecInbox.end(); it ++)
	{
		bool bIsHide = IsInHideList(it->_gid);
		bool bIsExpire = false/*IsExpire(it->_szDateLine)*/;
		
		if (!bIsHide && !bIsExpire)
		{
			vecInbox.push_back(*it);
		}
	}
	m_lockInbox.unlock() ;

	return vecInbox ;
}

vector<CWebData::WEB_FRIEND_REQ> CSNSManager::GetFriendReqData()
{
	vector<CWebData::WEB_FRIEND_REQ> vecFriendReq ;

	m_lockFriendReq.lock() ;
	for (vector<CWebData::_WEB_FRIEND_REQ>::iterator it = m_vecFriendReq.begin(); it != m_vecFriendReq.end(); it ++)
	{
		bool bIsHide = IsInHideList(it->_id);
		bool bIsExpire = false/*IsExpire(it->_szDateLine)*/;

		if (!bIsHide && !bIsExpire)
		{
			vecFriendReq.push_back(*it);
		}
	}
	m_lockFriendReq.unlock() ;

	return vecFriendReq ;
}

bool CSNSManager::IsExpire(_tstring sTimeStamp)
{
	//30 days expire time
	time_t nExpireTime = 60 * 60 * 24 * 30;
	time_t nTime = _ttoi64(sTimeStamp.c_str());
	time_t nCurtime = 0;
	time(&nCurtime);
	time_t nTimeSpan = nCurtime - nTime;
	return nTimeSpan < nExpireTime ? false : true;  
}

bool CSNSManager::IsInHideList(_tstring sId)
{
	bool bRet = false;
	m_lockHideList.lock();
	for (vector<HIDE_ITEM>::iterator hideIt = m_vecHideList.begin(); hideIt != m_vecHideList.end(); hideIt ++)
	{
		if (!hideIt->m_sId.CompareNoCase(sId))
		{
			bRet = true;
			break;
		}
	}
	m_lockHideList.unlock();
	return bRet;
}

vector<CWebData::WEB_CONNECT_UPDATE> CSNSManager::GetConnectUpdateData()
{
	vector<CWebData::WEB_CONNECT_UPDATE> vecConUpt ;

	m_lockConUpt.lock() ;
	vecConUpt = m_vecConUpt ;
	m_lockConUpt.unlock() ;

	return vecConUpt ;
}

void CSNSManager::GetConnectUpdateMoreFeed()
{
	if(m_uMoreFeedCount >= CONUPT_DISPLAY_INCREMENT_UNIT)
	{
		m_uMoreFeedCount -= CONUPT_DISPLAY_INCREMENT_UNIT ;
	}
	else
	{
		m_uMoreFeedCount = 0 ;
	}

	int nIndex = 0 ;
	vector<CWebData::WEB_CONNECT_UPDATE>::iterator iter = m_vecMoreFeed.begin() ;
	while(iter != m_vecMoreFeed.end( ))
	{
		iter = m_vecMoreFeed.erase(iter) ;

		nIndex ++ ;
		if(nIndex >= CONUPT_DISPLAY_INCREMENT_UNIT)
		{
			break;
		}
	}
}

int CSNSManager::GetMoreFeedCount()
{
	return m_uMoreFeedCount ;
}

void CSNSManager::ResetMoreFeed()
{
	m_lockConUpt.lock() ;
	
	if(m_vecConUpt.size() <= CONUPT_DISPLAY_INIT_COUNT)
	{
		m_vecMoreFeed.clear();
	}
	else
	{
		m_vecMoreFeed.assign(m_vecConUpt.begin() + CONUPT_DISPLAY_INIT_COUNT, m_vecConUpt.end()) ;
	}

	m_uMoreFeedCount = m_vecMoreFeed.size() ;

	m_lockConUpt.unlock() ;
}

void CSNSManager::SetViewedFeed(DWORD id)
{
	bool bRes=false;
	_tstring szPath = theDataPool.GetBaseDir()+CONFIG_VIEWFEED_IDLOG ;
	szPath.append(theDataPool.GetUserName().c_str()) ;
	
	m_lockConUpt.lock() ;

	_tstring szItemName, szItem ;
	for(int i=0;i<m_vecViewedFeed.size();i++)
	{
		if(id==m_vecViewedFeed[i])	
		{
			bRes=true;
			break;
		}
		else
			continue;
	}

	if(!bRes)
	{
		m_vecViewedFeed.push_back(id);

		if(m_vecViewedFeed.size( )<=100)
		{
			szItem.Format(1,_T("%d"),id);
			szItemName.Format(1,_T("item_%d"),m_vecViewedFeed.size( ));
			WritePrivateProfileString(_T("FeedID"),szItemName.c_str( ),szItem.c_str( ),szPath.c_str() );
		}
		else
		{
			DeleteFile(szPath.c_str( ));
			for(int j=1;j<=m_vecViewedFeed.size()-20;j++)
			{
				szItem.Format(1,_T("%d"),m_vecViewedFeed[j+19]);
				szItemName.Format(1,_T("item_%d"),j);
				WritePrivateProfileString(_T("FeedID"),szItemName.c_str( ),szItem.c_str( ),szPath.c_str() );
			}
			m_vecViewedFeed.clear();
		}
	}

	m_lockConUpt.unlock() ;
}

bool CSNSManager::IsFeedViewed(DWORD id)
{
	_tstring szPath=theDataPool.GetBaseDir()+CONFIG_VIEWFEED_IDLOG;
	szPath.append(theDataPool.GetUserName().c_str());
	
	m_lockConUpt.lock() ;

	if(m_vecViewedFeed.empty())
	{
		for(int i=1;i<=100;i++)
		{
			_tstring szItemName ;
			szItemName.Format(1,_T("item_%d"),i) ;
			DWORD dwRes = GetPrivateProfileInt(_T("FeedID"),szItemName.c_str( ),0,szPath.c_str());
			if(0==dwRes)
				break;
			else
				m_vecViewedFeed.push_back(dwRes);
		}
	}

	bool bRes=false;
	for(int i=0;i<m_vecViewedFeed.size();i++)
	{
		if(id==m_vecViewedFeed[i])	
		{
			bRes=true;
			break;
		}
		else
			continue;
	}

	m_lockConUpt.unlock() ;

	return bRes;
}

vector<CWebData::WEB_COUPON_MSG> CSNSManager::GetCouponData()
{
	vector<CWebData::WEB_COUPON_MSG> vecCoupon ;

	m_lockCoupon.lock() ;
	vecCoupon = m_vecCoupon ;
	m_lockCoupon.unlock() ;

	return vecCoupon;
}


void CSNSManager::AddAlertItem(ALERT_ITEM& stAlertItem)
{
	m_lockAlertItem.lock();
	m_vecAlertItem.insert(m_vecAlertItem.begin(), stAlertItem);
	SaveAlertItem();
	m_lockAlertItem.unlock();
}

void CSNSManager::AddHideItem(HIDE_ITEM& hideItem)
{
	m_lockHideList.lock();
	m_vecHideList.push_back(hideItem);
	/*SaveHideList();*/
	m_lockHideList.unlock();
}

bool CSNSManager::RemoveAlertItem(_tstring sId)
{
	bool bDelete = false ;
	m_lockAlertItem.lock() ;
	vector<ALERT_ITEM>::iterator iter = m_vecAlertItem.begin() ;
	for (; iter != m_vecAlertItem.end(); ++ iter)
	{
		if (!iter->m_sId.CompareNoCase(sId))
		{
			bDelete = true ;
			m_vecAlertItem.erase(iter) ;
			break ;
		}
	}
	m_lockAlertItem.unlock() ;

	if (!bDelete)
	{
		return false ;
	}
	return true;
}

BOOL CSNSManager::IsAlertItemExist(_tstring sId)
{
	BOOL bExist = FALSE;
	m_lockAlertItem.lock() ;
	vector<ALERT_ITEM>::iterator iter = m_vecAlertItem.begin() ;
	for (; iter != m_vecAlertItem.end(); ++ iter)
	{
		if (!iter->m_sId.CompareNoCase(sId))
		{
			bExist = TRUE;
			break ;
		}
	}
	m_lockAlertItem.unlock() ;
	return bExist;
}

void CSNSManager::SetAlertItemReadTag(_tstring sId)
{
}

vector<ALERT_ITEM> CSNSManager::GetAlertItem()
{
	vector<ALERT_ITEM> vecAlertItem ;

	m_lockAlertItem.lock() ;
	for (vector<ALERT_ITEM>::iterator it = m_vecAlertItem.begin(); it != m_vecAlertItem.end(); it ++)
	{
		bool bIsExpire = IsExpire(it->m_sTimeStamp);
		if (!bIsExpire)
		{
			if (!it->m_sType.CompareNoCase(ALERT_ITEM_NOTIFICATION) && it->m_nShowCmd == NOTIFY_GAME)
			{
				continue;
			}
			vecAlertItem.push_back(*it);
		}
	}
	m_lockAlertItem.unlock() ;

	return vecAlertItem ;
}

vector<HIDE_ITEM> CSNSManager::GetHideList()
{
	vector<HIDE_ITEM> vecHideList;
	m_lockHideList.lock();
	vecHideList = m_vecHideList;
	m_lockHideList.unlock();

	return vecHideList;
}

int CSNSManager::GetAlertItemNum()
{
	int nCount = 0;
	vector<ALERT_ITEM> vecAlert = GetAlertItem();
	m_lockAlertItem.lock();
	for (int i = 0; i < vecAlert.size(); i ++)
	{
		if((NOTIFY_GAME == vecAlert[i].m_nShowCmd && !m_vecAlertItem[i].m_sType.CompareNoCase(ALERT_ITEM_NOTIFICATION)) || IsExpire(vecAlert[i].m_sTimeStamp))
		{
			continue;
		}
		nCount++;
		
	}
	m_lockAlertItem.unlock();
	return nCount;
}

int CSNSManager::GetInBoxHideNum()
{
	int nCount = 0;
	for (int i = 0; i < m_vecHideList.size(); i ++)
	{
		if (!m_vecHideList[i].m_sType.CompareNoCase(_T("inbox")))
		{
			nCount ++;
		}
	}
	return nCount;
}

int CSNSManager::GetFRHideNum()
{
	int nCount = 0;
	for (int i = 0; i < m_vecHideList.size(); i ++)
	{
		if (!m_vecHideList[i].m_sType.CompareNoCase(_T("friend request")))
		{
			nCount ++;
		}
	}
	return nCount;
}

void CSNSManager::SetNotifyServerIdList(uint32 nServerCount, uint32* pServerIdList)
{
	m_lockAlertItem.lock() ;
	
	m_vecNotifyServerId.clear() ;

	for (int i=0; i< nServerCount; ++i)
	{
		m_vecNotifyServerId.push_back(pServerIdList[i]) ;
	}

	m_lockAlertItem.unlock() ;
}

vector<uint32> CSNSManager::GetNotifyServerIdList()
{
	vector<uint32> vecNotifySvrId ;

	m_lockAlertItem.lock() ;
	vecNotifySvrId = m_vecNotifyServerId ;
	m_lockAlertItem.unlock() ;

	return vecNotifySvrId ;
}

void CSNSManager::ResetAlertItem()
{
	CAlertLog alertLog(theDataPool.GetBaseDir()) ;
	alertLog.SetUserJid(ALERT_FILENAME) ;
	alertLog.RemoveLog() ;
	
	m_lockAlertItem.lock() ;
	m_vecAlertItem.clear() ;
	m_lockAlertItem.unlock() ;
}

int CSNSManager::ReadAlertItem()
{
	CAlertLog alertLog(theDataPool.GetBaseDir());
	alertLog.SetUserJid(ALERT_FILENAME);
	alertLog.InitialLog();
	
	m_lockAlertItem.lock() ;
	m_vecAlertItem = alertLog.ReadAlertItem();
	int nSize = m_vecAlertItem.size();
	m_lockAlertItem.unlock() ;

	return nSize ;
}

int CSNSManager::ReadHideList()
{
	CAlertLog hideListLog(theDataPool.GetBaseDir());
	hideListLog.SetUserJid(HIDE_ITEM_FILENAME);
	hideListLog.InitialLog();

	m_lockHideList.lock();
	m_vecHideList = hideListLog.ReadHideListLog();
	int nSize = m_vecHideList.size();
	m_lockHideList.unlock();

	return nSize;
}

void CSNSManager::SaveAlertItem()
{
	CAlertLog alertLog(theDataPool.GetBaseDir()) ;
	alertLog.SetUserJid(ALERT_FILENAME) ;
	alertLog.RemoveLog() ;
	alertLog.InitialLog() ;
	
	m_lockAlertItem.lock() ;
	for(int i=0; i<m_vecAlertItem.size(); ++i)
	{
		alertLog.AppendLog(m_vecAlertItem[i]) ;
	}
	m_lockAlertItem.unlock() ;
}

void CSNSManager::SaveHideList()
{
	CAlertLog hideListLog(theDataPool.GetBaseDir()) ;
	hideListLog.SetUserJid(HIDE_ITEM_FILENAME) ;
	hideListLog.RemoveLog() ;
	hideListLog.InitialLog() ;

	m_lockHideList.lock();
	for (int i = 0; i < m_vecHideList.size(); i++)
	{
		hideListLog.AppendLog(m_vecHideList[i]);
	}
	m_lockHideList.unlock();
}

void CSNSManager::ResetWebData(int nIndex)
{
	switch(nIndex)
	{
	case CWebData::SERVICE_INBOX_MSG:
		m_lockInbox.lock() ;
		m_vecInbox.clear();
		m_lockInbox.unlock() ;
		break;
	case CWebData::SERVICE_FRIEND_REQ:
		m_lockFriendReq.lock() ;
		m_vecFriendReq.clear();
		m_lockFriendReq.unlock() ;
		break;
	case CWebData::SERVICE_CONNECT_UPDATE:
		m_lockConUpt.lock() ;
		m_vecConUpt.clear();
		m_vecMoreFeed.clear();
		m_lockConUpt.unlock() ;
		break;
	case CWebData::SERVICE_COUPON:
		m_lockCoupon.lock() ;
		m_vecCoupon.clear();
		m_lockCoupon.unlock() ;
		break;
	case CWebData::SERVICE_NOTIFY:
		break;
	}
}

void CSNSManager::RemoveWebDataItem(int nType, int nId)
{
	switch(nType)
	{
	/*case CWebData::SERVICE_INBOX_MSG:
		m_lockInbox.lock() ;
		for(vector<CWebData::_WEB_INBOX_MSG>::iterator it = m_vecInbox.begin(); it != m_vecInbox.end(); it++)
		{
			if (it->_id == nId)
			{
				m_vecInbox.erase(it);
				break;
			}
		}
		m_lockInbox.unlock() ;
		break;*/
	case CWebData::SERVICE_FRIEND_REQ:
		m_lockFriendReq.lock() ;
		for(vector<CWebData::_WEB_FRIEND_REQ>::iterator it = m_vecFriendReq.begin(); it != m_vecFriendReq.end(); it++)
		{
			if (it->_senderId == nId)
			{
				m_vecFriendReq.erase(it);
				break;
			}
		}
		m_lockFriendReq.unlock() ;
		break;
	case CWebData::SERVICE_NOTIFY:
		m_lockAlertItem.lock();
		for (vector<ALERT_ITEM>::iterator it = m_vecAlertItem.begin(); it != m_vecAlertItem.end(); it++)
		{
			if (nId == _ttoi64(it->m_sId.c_str()))
			{
				m_vecAlertItem.erase(it);
				break;
			}
		}
		m_lockAlertItem.unlock();
		break;
	default:
		break;
	}
}

CWebData::WEB_CHARACT_MSG CSNSManager::GetCharactDataByNickName(_tstring sNickName)
{
	CWebData::WEB_CHARACT_MSG charData ={_T(""),_T(""),_T(""),_T(""),_T(""),_T("")};
	charData._szNick = sNickName;

	m_lockWebData.lock() ;
	if (m_mapCharData.find(sNickName) != m_mapCharData.end())
	{
		charData._szName=m_mapCharData[sNickName]._szName;
		charData._szCharater=m_mapCharData[sNickName]._szCharater;
		charData._szServer=m_mapCharData[sNickName]._szServer;
		charData._szHeaderPath=m_mapCharData[sNickName]._szHeaderPath;
		charData._szAvatar = m_mapCharData[sNickName]._szAvatar;
	}
	if(charData._szHeaderPath.empty())
	{
		charData._szHeaderPath=FILEPATH_DEFAULT_CHARACTOR;
	}
	m_lockWebData.unlock() ;
	return charData;
}

CWebData::WEB_CHARACT_MSG CSNSManager::GetUserCharData()
{
	CWebData::WEB_CHARACT_MSG userCharData={_T(""),_T(""),_T(""),_T(""),_T(""),_T("")};
	userCharData._szNick = theDataPool.GetUserNick();

	m_lockWebData.lock() ;
	if (m_mapCharData.find(theDataPool.GetUserNick()) != m_mapCharData.end())
	{
		userCharData._szName=m_mapCharData[theDataPool.GetUserNick()]._szName;
		userCharData._szServer=m_mapCharData[theDataPool.GetUserNick()]._szServer;
		userCharData._szHeaderPath=m_mapCharData[theDataPool.GetUserNick()]._szHeaderPath;
		userCharData._szAvatar = m_mapCharData[theDataPool.GetUserNick()]._szAvatar;
        userCharData._szCharater = m_mapCharData[theDataPool.GetUserNick()]._szCharater;
	}
	if(userCharData._szHeaderPath.empty())
	{
		userCharData._szHeaderPath=FILEPATH_DEFAULT_CHARACTOR;
	}
	m_lockWebData.unlock() ;
	return userCharData ;
}

CWebData::WEB_EMS_DATA  CSNSManager::GetEmsData()
{
	CWebData::WEB_EMS_DATA ems;
	m_lockEms.lock();
	//ems._szUrl = m_EmsData._szUrl;
	ems._szId = m_EmsData._szId;
	ems._szTimeBegin = m_EmsData._szTimeBegin;
	ems._szTimeEnd = m_EmsData._szTimeEnd;
	ems._szLink = m_EmsData._szLink;
	ems._szIconPath = m_EmsData._szIconPath;
	ems._szContent =m_EmsData._szContent;
    ems._szTitle = m_EmsData._szTitle;
	m_lockEms.unlock();
	return ems;
}

void  CSNSManager::SetEmsData( CWebData::WEB_EMS_DATA emsData)
{
   m_lockEms.lock();
   //m_EmsData._szUrl = emsData._szUrl;
   m_EmsData._szId = emsData._szId;
   m_EmsData._szTimeBegin = emsData._szTimeBegin;
   m_EmsData._szTimeEnd = emsData._szTimeEnd;
   m_EmsData._szLink = emsData._szLink;
   m_EmsData._szIconPath = emsData._szIconPath;
   m_EmsData._szContent = emsData._szContent;
   m_EmsData._szTitle = emsData._szTitle;
   m_lockEms.unlock();
}

void CSNSManager::SetSessionId(_tstringA sSessionId)
{
	m_sSessionId = sSessionId ;
}

_tstringA CSNSManager::GetSessionId()
{
	return m_sSessionId ;
}

bool CSNSManager::NotifyNotificationUpdated(int nShowOption)
{
	m_lockListener.lock() ;
	map<ISNSListener*, int>::iterator iter = m_mapSNSListener.begin() ;
	for (; iter != m_mapSNSListener.end(); ++ iter)
	{
		iter->first->SNS_ServiceAchievementUpdated(nShowOption) ;
	}
	m_lockListener.unlock() ;

	return true ;
}

void CSNSManager::SetDataCount(vector<int> vecCount)
{
	m_lockWebData.lock() ;
	m_vecDataCount = vecCount ;
	m_lockWebData.unlock() ;
}

void CSNSManager::SetInboxData(vector<CWebData::WEB_INBOX_MSG>& vecInbox)
{
	m_lockInbox.lock() ;
	//support arc mail
	int nPreSize = m_vecInbox.size();
	int nCount = vecInbox.size() - nPreSize;
	if (nCount == 1 && nPreSize != 0)
	{
		m_lockListener.lock() ;
		map<ISNSListener*, int>::iterator iter = m_mapSNSListener.begin() ;
		for (; iter != m_mapSNSListener.end(); ++ iter)
		{
			iter->first->SNS_ReceiveArcMail(vecInbox[0]) ;
		}
		m_lockListener.unlock() ;
	}
	
	m_vecInbox = vecInbox ;
	m_lockInbox.unlock() ;
}

void CSNSManager::AddInboxData(CWebData::WEB_INBOX_MSG& webInbox)
{
	m_lockInbox.lock();
	m_vecInbox.insert(m_vecInbox.begin(), webInbox);
	m_lockInbox.unlock();
}

void CSNSManager::SetFriendReqData(vector<CWebData::WEB_FRIEND_REQ>& vecFriendReq)
{
	m_lockFriendReq.lock() ;
	m_vecFriendReq = vecFriendReq ;
	m_lockFriendReq.unlock() ;
}

void CSNSManager::AddFriendReqData(CWebData::WEB_FRIEND_REQ& webFriendReq)
{
	m_lockFriendReq.lock();
	m_vecFriendReq.insert(m_vecFriendReq.begin(), webFriendReq);
	m_lockFriendReq.unlock();
}

void CSNSManager::SetConnectUpdateData(vector<CWebData::WEB_CONNECT_UPDATE>& sConUpt)
{
	for (int i = 0; i < sConUpt.size(); ++ i)
	{
		if(IsFeedViewed(sConUpt[i]._id))
		{
			continue ;
		}

		time_t ltime;
		time(&ltime);
		if(691200 < ltime - _ttoi(sConUpt[i]._szDateLine.c_str()))
		{
			continue ;
		}

		m_lockConUpt.lock() ;
		m_vecConUpt.push_back(sConUpt[i]);
		if(m_vecConUpt.size() > CONUPT_DISPLAY_INIT_COUNT)
		{
			m_vecMoreFeed.push_back(sConUpt[i]) ;
		}
		m_uMoreFeedCount = m_vecMoreFeed.size() ;
		m_lockConUpt.unlock() ;
	}
}

void CSNSManager::SetCouponData(vector<CWebData::WEB_COUPON_MSG>& vecCoupon)
{
	m_lockCoupon.lock() ;
	m_vecCoupon = vecCoupon ;
	m_lockCoupon.unlock() ;
}

void CSNSManager::SetUserCharData(CWebData::WEB_CHARACT_MSG& userCharData)
{
	m_lockWebData.lock() ;
	m_userCharData = userCharData ;
	m_lockWebData.unlock() ;
}