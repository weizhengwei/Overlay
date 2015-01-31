#pragma once

#include "tstring/tstring.h"
#include "data/WebData.h"
#include "thread/thread.h"
#include "thread/WQueue.h"

class ISNSListener
{
public:
	//number of services retrieved, including friend request, inbox message, connect update, coupon
	virtual bool SNS_ServiceCountUpdated() {return true;} ;

	//callback when achievement comes
	virtual bool SNS_ServiceAchievementUpdated(uint16 nShowOption) {return true ;} ;

	//callback when friend is online
	virtual bool SNS_ServiceCharacterInfoUpdated(_tstring sUserName, _tstring sNickName) {return true ;} ;

	//callback when ems is available.
	virtual bool SNS_ServiceEmsAvailable() {return true ;} ;

	//call back when some web service is returned
	virtual bool SNS_OnResponseWebService(int nType, BOOL bSuccess, int nId, PVOID pBaseDlg, BOOL bAccept){ return true; }
	virtual bool SNS_ServiceShowAlert(int nType){ return true;}
	virtual bool SNS_ReceiveArcMail(CWebData::_WEB_INBOX_MSG& webInbox){ return true; }
} ;

class CSNSManager : public Thread
{
public:

	static const int RESEND_TIMES = 3 ;
	static const int CONUPT_DISPLAY_INIT_COUNT = 3 ;
	static const int CONUPT_DISPLAY_INCREMENT_UNIT = 5 ;

	typedef struct _SNS_INFO{

		int nType ;
		int nRetry ;

		_tstring sAchieveId ; //For achievement use
		int nShowOption;      //For achievement use

		int nFid ;    //For friend request use
		int nRid ;    //For friend request use
		bool bAccept ;//For friend request use

		_tstring sUserName ; //For game avatar use
		_tstring sNickName ; //For game avatar use
		PVOID pBaseNotifyDlg;

		bool    bNotify; //ems content is pushed from server.
		std::string sEmsData;//ems content.

		_SNS_INFO()
		{
			nType=nRetry=0;

			//For achievement use
			sAchieveId = _T("");
			nShowOption=0;

			//For friend request use
			nFid= nRid = bAccept = 0;

			//For game avatar use
			sUserName = sNickName = _T("");

			pBaseNotifyDlg = NULL;

            bNotify = false;
            sEmsData ="";
		}

	}SNS_INFO, *PSNS_INFO ;

	CSNSManager() ;
	virtual ~CSNSManager(){} ;

	bool Init() ;
	void Uninit(int code = 0) ;

	bool RegisterListener(ISNSListener* ) ;
	bool UnregisterListener(ISNSListener* ) ;
	bool ClearListener() ;
	
	bool GetAllChangedService() ;
	bool GetDataCountService() ;
	bool GetInboxMessageService() ;
	bool GetFriendRequestService() ;
	bool GetConnectUpdateService() ;
	bool GetCouponMessageService() ;
	bool GetAchievementService(_tstring snId, uint32 nShowOption) ;
	bool ResponseFriendReq(int fid,int rid,bool bAccept, PVOID pBaseNotifyDlg) ;
	bool ClearConncectUpdateItems() ;
	bool GetGameCharactorService(_tstring sUserName, _tstring sNickName);
	bool GetGameServerService(vector<CWebData::WEB_GAMESERVER_INFO>& vecGameServerInfo) ;
    bool GetEmsDataService(bool bNotify =false,const std::string sEmsData="");

	vector<int> GetDataCount() ;
	vector<CWebData::WEB_INBOX_MSG>  GetInboxData() ;
	vector<CWebData::WEB_FRIEND_REQ> GetFriendReqData() ;
	vector<CWebData::WEB_CONNECT_UPDATE> GetConnectUpdateData() ;
	void GetConnectUpdateMoreFeed() ;
	int  GetMoreFeedCount() ;
	void ResetMoreFeed() ;
	void SetViewedFeed(DWORD id);
	bool IsFeedViewed(DWORD id) ;

	vector<CWebData::WEB_COUPON_MSG> GetCouponData() ;

	void SetNotifyServerIdList(uint32 nServerCount, uint32* pServerIdList) ;
	std::vector<uint32> GetNotifyServerIdList() ;

	int ReadHideList();
	void SaveHideList();
	vector<HIDE_ITEM> GetHideList();
	int GetInBoxHideNum();
	int GetFRHideNum();
	void AddHideItem(HIDE_ITEM& hideItem);

	void ResetWebData(int nIndex) ;
	void RemoveWebDataItem(int nType, int nIndex) ;

	CWebData::WEB_CHARACT_MSG GetCharactDataByNickName(_tstring sNickName) ;
	CWebData::WEB_CHARACT_MSG GetUserCharData() ;

    CWebData::WEB_EMS_DATA   GetEmsData();
    void  SetEmsData(CWebData::WEB_EMS_DATA emsData);

	void SetSessionId(_tstringA sSessionId) ;
	_tstringA GetSessionId() ;

	bool NotifyNotificationUpdated(int nShowOption) ;

	void AddInboxData(CWebData::WEB_INBOX_MSG& webInbox);
	void AddFriendReqData(CWebData::WEB_FRIEND_REQ& webFriendReq);

	void AddAlertItem(ALERT_ITEM& stAlertItem);
	bool RemoveAlertItem(_tstring sId);
	vector<ALERT_ITEM> GetAlertItem();
	int GetAlertItemNum();
	int ReadAlertItem();
	void SaveAlertItem();
	void SetAlertItemReadTag(_tstring sId);
	void ResetAlertItem();
	BOOL IsAlertItemExist(_tstring sId);
	bool IsExpire(_tstring sTimeStamp);
	bool IsInHideList(_tstring sId);

protected:

	virtual long run();
	static DWORD __stdcall DownloadFileProc(LPVOID lpParameter);

	void SetDataCount(vector<int> vecCount) ;
	void SetInboxData(vector<CWebData::WEB_INBOX_MSG>& vecInbox) ;
	void SetFriendReqData(vector<CWebData::WEB_FRIEND_REQ>& vecFriendReq) ;
	void SetConnectUpdateData(vector<CWebData::WEB_CONNECT_UPDATE>& sConUpt) ;
	void SetCouponData(vector<CWebData::WEB_COUPON_MSG>& vecCoupon) ;
	void SetUserCharData(CWebData::WEB_CHARACT_MSG& userCharData) ;

	bool post_process_getgamecharacter(CWebData::WEB_CHARACT_MSG sCharData) ;
	bool post_process_responsefriendreq() ;
	bool post_process_getfriendrequest() ;

protected:
	bool   m_bStop ;
	WQueue<SNS_INFO*> m_wTaskQueue ;
	WQueue<SNS_INFO*> m_wDownloadFile;

private:

	Mutex m_lockWebData ;
	Mutex m_lockInbox ;
	Mutex m_lockFriendReq ;
	Mutex m_lockConUpt ;
	Mutex m_lockCoupon ;
	Mutex m_lockHideList;
	Mutex m_lockAlertItem;
	Mutex m_lockEms;

	vector<int>                          m_vecDataCount ;
	vector<CWebData::WEB_INBOX_MSG>      m_vecInbox ;
	vector<CWebData::WEB_FRIEND_REQ>     m_vecFriendReq;
	vector<CWebData::WEB_CONNECT_UPDATE> m_vecConUpt;
	vector<CWebData::WEB_CONNECT_UPDATE> m_vecMoreFeed;
	vector<DWORD>                        m_vecViewedFeed;
	unsigned int                         m_uMoreFeedCount; 
	vector<CWebData::WEB_COUPON_MSG>     m_vecCoupon ;
	vector<uint32>                       m_vecNotifyServerId ;
	vector<HIDE_ITEM>					 m_vecHideList;
	vector<ALERT_ITEM>					 m_vecAlertItem;
	map<_tstring,CWebData::WEB_CHARACT_MSG>  m_mapCharData ;//std::pair<nickName,WEB_CHARACT_MSG>
	CWebData::WEB_CHARACT_MSG            m_userCharData ;
    CWebData::WEB_EMS_DATA               m_EmsData;

	_tstringA m_sSessionId ;

	Mutex m_lockListener ;
	map<ISNSListener*, int> m_mapSNSListener ;

	CWebData m_sWebData ;
	HANDLE m_hThread;
};


extern CSNSManager g_theSNSManager ;