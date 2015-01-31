/**
* @file      WebData.cpp
  @brief     The implementation file for web service.
  @author    shenhui@pwrd.com
  @data      2011/09/16
  @version   01.00.00
*/

/* 
*  Copyright(C) 2010. Perfect World Entertainment Inc
*
*  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
*  its licensors and is protected by copyright and other intellectual 
*  property laws and treaties.  We and our licensors own all rights, 
*  title and interest in and to the SOURCE CODE, including all copyright 
*  and other intellectual property rights in the SOURCE CODE.
*/

/*
* $LastChangedBy: shenhui@pwrd.com
* $LastChangedDate: 
* $Revision: 01.00.01
* $Id: 
* $notes:
*/

#include <time.h>
#include "crypt/md5.h"
#include "data/WebData.h"
#include "data/NotifyLog.h"
#include "data/DataPool.h"
#include "data/UIString.h"
#include "data/AsynWebServiceHelper.h"
#include "curl/curl.h"

CWebData g_theWebData ;

CWebData::CWebData(void)
{
}

CWebData::~CWebData(void)
{
}

bool CWebData::GetService(_tstringA sSessionId, vector<int>& vecCount)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_UPDATE")) ;
	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	//////////////////////////////////////////////////////////////////////////////////
	//-tstring sGetFqNumUrl = theUIString._GetStringFromId(_T(" "));
	//sSvrRes = {0, "", "", NULL};
	//sWebSvr.GetFieldService(sGetFqNumUrl, _tstring(sCookie.c_str()), vecCount);
	//bool bFq = process_fqnum_data(sSvrRes.sResult.c_str(), vecCount);

	return process_service_data(sSvrRes.sResult.c_str(), vecCount) ;
}

bool CWebData::process_service_data(std::string sData, vector<int>& vecCount)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;

	do {

		vecCount.resize(SERVICE_MAX) ;

		xmlDoc.Parse(sData.c_str()) ;
		
		TiXmlElement* pElement=xmlDoc.FirstChildElement("data");
		if(!pElement)
		{
			break ;
		}

		pElement=pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pElement->NextSiblingElement();
		if (pElement)
		{
			pElement=pElement->FirstChildElement("newmsg");
		}

		const char* p = NULL ;
		if (pElement)
		{
			p = pElement->GetText( ) ;
			if (p != NULL)
			{
				vecCount[CWebData::SERVICE_INBOX_MSG]=atoi(p) ;
			}
			pElement=pElement->NextSiblingElement() ;
		}
		if (pElement)
		{
			p = pElement->GetText( ) ;
			if (p != NULL)
			{
				vecCount[CWebData::SERVICE_FRIEND_REQ]=atoi(p);
			}
			pElement=pElement->NextSiblingElement(); 
		}
		if (pElement)
		{

			p = pElement->GetText( ) ;
			if (p != NULL)
			{
				vecCount[CWebData::SERVICE_COUPON]=atoi(p);
			}

			pElement=pElement->NextSiblingElement();
		}
		if (pElement)
		{

			p = pElement->GetText() ;
			if (p != NULL)
			{
				vecCount[CWebData::SERVICE_CONNECT_UPDATE]=atoi(p);
			}

			pElement=pElement->NextSiblingElement();
		}
		if (pElement)
		{
			p = pElement->GetText() ;
			if (p != NULL)
			{	
				vecCount[CWebData::SERVICE_NOTIFY]=atoi(p);
			}
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();
	return bRet ;
}

bool CWebData::GetInboxMessageService(_tstringA sSessionId, vector<WEB_INBOX_MSG>& vecInbox)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_MESSAGE")) ;
	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_inbox_data(sSvrRes.sResult.c_str(), vecInbox) ;
}

bool CWebData::process_inbox_data(std::string sData, vector<WEB_INBOX_MSG>& vecInbox)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pElement = xmlDoc.FirstChildElement("data") ;
		if(!pElement)
		{
			break ;
		}

		pElement = pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult ;
		szResult.fromUTF8(pElement->GetText()) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement = pElement->NextSiblingElement() ;

		while(pElement)
		{
			CWebData::_WEB_INBOX_MSG webData ;

			TiXmlElement *pSubElement = NULL ;
			pSubElement = pElement->FirstChildElement("type");
			if (pSubElement && pSubElement->GetText())
			{
				webData._szType.fromUTF8(pSubElement->GetText());
			}
			if (!webData._szType.CompareNoCase(_T("user")))
			{
				pSubElement=pElement->FirstChildElement("id");
				if(pSubElement && pSubElement->GetText())
					webData._id.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("groupid");
				if(pSubElement && pSubElement->GetText())
					webData._gid.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("senderid");
				if(pSubElement && pSubElement->GetText())
					webData._senderId.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("sender");
				if(pSubElement && pSubElement->GetText())
					webData._szSender.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("title");
				if(pSubElement && pSubElement->GetText())
					webData._szTitle.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("datetime");
				if(pSubElement && pSubElement->GetText())
					webData._szDateLine.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("link");
				if(pSubElement && pSubElement->GetText())
					webData._szLink.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("profile");
				if(pSubElement && pSubElement->GetText())
					webData._szFriendLink.fromUTF8(pSubElement->GetText());

				vecInbox.push_back(webData);

			}
			else if (!webData._szType.CompareNoCase(_T("admin")))
			{
				pSubElement=pElement->FirstChildElement("id");
				if(pSubElement && pSubElement->GetText())
				{
					webData._id.fromUTF8(pSubElement->GetText());
					webData._gid.fromUTF8(pSubElement->GetText());
				}

				pSubElement=pElement->FirstChildElement("sender");
				if(pSubElement && pSubElement->GetText())
					webData._szSender.fromUTF8(pSubElement->GetText());

				pSubElement = pElement->FirstChildElement("title");
				if (pSubElement && pSubElement->GetText())
					webData._szArcMailSubject.fromUTF8(pSubElement->GetText());
				
				pSubElement=pElement->FirstChildElement("excerpt");
				if(pSubElement && pSubElement->GetText())
					webData._szTitle.fromUTF8(pSubElement->GetText());
			
				pSubElement=pElement->FirstChildElement("datetime");
				if(pSubElement && pSubElement->GetText())
					webData._szDateLine.fromUTF8(pSubElement->GetText());

				pSubElement=pElement->FirstChildElement("link");
				if(pSubElement && pSubElement->GetText())
					webData._szLink.fromUTF8(pSubElement->GetText());

				vecInbox.push_back(webData);

			}
			
			pElement=pElement->NextSiblingElement();
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::GetPlayNowRequestService(_tstringA sSessionId, _tstring sLang, vector<AUTOPLAYNOW_INFO>& vecPlayNowReq)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_PLAYNOW"));
	_tstringA sCookie;
	//sSessionId = _T("79a949j56maunt4t1f3dfkeui2");
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL};
	sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sLang, &sSvrRes);

	return process_playnow_data(sSvrRes.sResult.c_str(), vecPlayNowReq);
}

bool CWebData::process_playnow_data(std::string sData, vector<AUTOPLAYNOW_INFO>& vecPlayNowReq)
{
	bool bRet = false;
	TiXmlDocument xmlDoc;

	do
	{
		xmlDoc.Parse(sData.c_str());

		TiXmlElement* pElement = xmlDoc.FirstChildElement("data");
		if(!pElement)
		{
			break;
		}

		pElement = pElement->FirstChildElement("result");
		if(!pElement)
		{
			break;
		}
		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(szResult.CompareNoCase(_T("success")) != 0)
		{
			break;
		}

		pElement = pElement->NextSiblingElement("games");
		if(!pElement)
		{
			break;
		}
		pElement = pElement->FirstChildElement("game");
		TiXmlElement* pItem = NULL;
		for (pElement ; pElement != NULL ; pElement = pElement->NextSiblingElement("game"))
		{
			AUTOPLAYNOW_INFO  autoInfo;
			pItem = pElement->FirstChildElement("game_abbr");
			if (pItem)
			{
				autoInfo._szAbbrName = pItem->GetText();
			}	
			pItem = pElement->FirstChildElement("language");
			if (pItem)
			{
				autoInfo._szAbbrLang = pItem->GetText();
			}
			pItem = pElement->FirstChildElement("is_latest");
			if (pItem)
			{
				autoInfo._szIsLatest = pItem->GetText();
			}
			vecPlayNowReq.push_back(autoInfo);	
		}
		bRet = true;

	}while(false);

	xmlDoc.Clear();
	return bRet;
}

bool CWebData::PostDisplayNameRequestService(_tstringA sSessionId, _tstring sNick, UINT nTimeout, _tstring& sError)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_DISPLAYNAME"));
	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL};
	if(sWebSvr.GetFieldService(sUrl, _tstring(sCookie.c_str()), sNick, &sSvrRes, nTimeout))
	{
		// get data from web service
		return process_displayname_data(sSvrRes.sResult.c_str(), sError);
	}
	else
	{
		// get no data from web service
		sError = _T("network connection error");
		return false;
	}
}

bool CWebData::process_displayname_data(std::string sData, _tstring& sError)
{
	bool bRet = false;
	sError = _T("Unknow Error");
	TiXmlDocument xmlDoc;

	do
	{
		xmlDoc.Parse(sData.c_str());

		TiXmlElement* pElement = xmlDoc.FirstChildElement("data");
		if(!pElement)
		{
			break;
		}

		pElement = pElement->FirstChildElement("result");
		if(!pElement)
		{
			break;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(szResult.CompareNoCase(_T("success")) == 0)
		{
			bRet = true;
			sError = _T("None Error");
			break;
		}
		else if(szResult.CompareNoCase(_T("failed")) == 0)
		{
			pElement = pElement->NextSiblingElement("reason");
			if(!pElement)
			{
				break;
			}
			sError.fromUTF8(pElement->GetText());
			break;
		}

	}while(false);

	xmlDoc.Clear();
	return bRet;
}

bool CWebData::GetUserInfoFromWebService(_tstringA szSessionId, _tstring& szNick, UINT nTimeout)
{
	_tstring szUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_USERINFO"));
	_tstringA szCookie;
	szCookie.Format(1, "PWRD=%s", szSessionId.toUTF8().c_str());

	szNick = _T("");
	CWebService sWebSvr;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL};
	if( sWebSvr.GetFieldService(szUrl, _tstring(szCookie.c_str()), _T(""), &sSvrRes, nTimeout) )
	{
		// get data from web
		return process_userinfo_data(sSvrRes.sResult.c_str(), szNick);
	}
	else
	{
		// get no data from web
		return false;
	}
}

bool CWebData::process_userinfo_data(std::string szData, _tstring& szNick)
{
	bool bRet = false;
	TiXmlDocument xmlDoc;

	do
	{
		xmlDoc.Parse(szData.c_str());

		TiXmlElement* pElement = xmlDoc.FirstChildElement("data");
		if(!pElement)
		{
			break;
		}

		pElement = pElement->FirstChildElement("result");
		if(!pElement)
		{
			break;
		}
		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(szResult != _T("success"))
		{
			break;
		}

		pElement = pElement->NextSiblingElement("userinfo");
		if(!pElement)
		{
			break;
		}

		pElement = pElement->FirstChildElement("nickname");
		if(!pElement)
		{
			break;
		}

		szNick.fromUTF8(pElement->GetText());

		bRet = true;

	}while(false);

	xmlDoc.Clear();
	return bRet;
}

bool CWebData::GetFriendRequestService(_tstringA sSessionId, vector<WEB_FRIEND_REQ>& vecFriendReq)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_FQ")) ;
	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_friend_data(sSvrRes.sResult.c_str(), vecFriendReq) ;
}

bool CWebData::process_friend_data(std::string sData, vector<WEB_FRIEND_REQ>& vecFriendReq)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pElement=xmlDoc.FirstChildElement("data");
		if(NULL == pElement)
		{
			break ;
		}

		pElement=pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}
		
		pElement=pElement->NextSiblingElement();
		pElement=pElement->FirstChildElement();
		while(pElement)
		{

			CWebData::WEB_FRIEND_REQ  webData;
			TiXmlElement *pSubElement=NULL;	

			pSubElement=pElement->FirstChildElement("id");
			if(pSubElement && pSubElement->GetText())
				webData._id=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("senderid");
			if(pSubElement && pSubElement->GetText())
				webData._senderId=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("sender");
			if(pSubElement && pSubElement->GetText())
				webData._szSender.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("content");
			if(pSubElement && pSubElement->GetText())
				webData._szContent.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("dateline");
			if(pSubElement && pSubElement->GetText())
				webData._szDateLine.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("avatar");
			if(pSubElement && pSubElement->GetText())
				webData._szAvatar.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("profile");
			if(pSubElement && pSubElement->GetText())
				webData._szFriendLink.fromUTF8(pSubElement->GetText());

			webData._szHeaderPath=_T("themes\\sonic\\default_header.jpg");  //add default header image

			vecFriendReq.insert(vecFriendReq.begin(), webData);

			pElement=pElement->NextSiblingElement();
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::ResponseFriendReq(_tstringA sSessionId, int fid,int rid,bool bAccept)
{
	_tstring szUrl, szUrlInfo;

	if(bAccept)
		szUrl=theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_REQALLOW"));
	else
		szUrl=theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_REQDENY"));

	szUrlInfo.Format(3,_T("%s{\"fid\":\"%d\",\"requestId\":\"%d\"}"),szUrl.c_str(),fid,rid);

	_tstringA  sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(szUrlInfo, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_response_data(sSvrRes.sResult.c_str()) ;
}

bool CWebData::process_response_data(std::string sData)
{
	if(sData.find("success") != sData.npos)
	{
		return true ;
	}
	return false ;
}

bool CWebData::GetConnectUpdateService(_tstringA sSessionId, vector<WEB_CONNECT_UPDATE>& vecConUpt)
{
	const std::string url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_CONUPT")).toNarrowString() ; ///"http://snsv21.perfectworld.com/client/getconnectupdates";

	_tstringA  sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(_tstring(url.c_str()), _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_connect_data(sSvrRes.sResult.c_str(), vecConUpt);
}

bool CWebData::process_connect_data(std::string sData, vector<WEB_CONNECT_UPDATE>& vecConUpt)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;	

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pElement = xmlDoc.FirstChildElement("data") ;
		if(NULL == pElement)
		{
			break ;
		}

		pElement=pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult;
		szResult.fromUTF8(pElement->GetText());
		if( 0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement = pElement->NextSiblingElement();

		while(pElement)
		{
			CWebData::WEB_CONNECT_UPDATE  webData ;
			TiXmlElement *pSubElement = NULL ;	

			pSubElement=pElement->FirstChildElement("id");
			if(pSubElement && pSubElement->GetText())
				webData._id=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("type");
			if(pSubElement && pSubElement->GetText())
				webData._type=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("link");
			if(pSubElement && pSubElement->GetText())
				webData._szLink.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("message");
			if(pSubElement && pSubElement->GetText())
				webData._szMsg.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("userlist");
			if(pSubElement && pSubElement->GetText())
				webData._szUserList.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("dateline");
			if(pSubElement && pSubElement->GetText())
				webData._szDateLine.fromUTF8(pSubElement->GetText());

			vecConUpt.push_back(webData) ;

			pElement=pElement->NextSiblingElement();
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();
	return bRet ;
}

void CWebData::ClearConncectUpdateItems(_tstringA sSessionId)
{
	_tstringA url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_CONUPT")).toNarrowString() + "?reset"; 
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	sWebSvr.GetFieldService(_tstring(url.c_str()), _tstring(sCookie.c_str()), _T(""), NULL) ;
}

bool CWebData::GetCouponMessageService(_tstringA sSessionId, vector<WEB_COUPON_MSG>& vecCoupon)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_COUPON")) ;
	_tstringA  sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_coupon_data(sSvrRes.sResult.c_str(), vecCoupon);
}

bool CWebData::process_coupon_data(std::string sData, vector<WEB_COUPON_MSG>& vecCoupon)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;	

	do {

		xmlDoc.Parse(sData.c_str()) ;
		TiXmlElement* pElement=xmlDoc.FirstChildElement("data");
		if(NULL == pElement)
		{
			break ;
		}

		pElement = pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if( 0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pElement->NextSiblingElement();

		while(pElement)
		{
			CWebData::WEB_COUPON_MSG  webData;

			TiXmlElement* pSubElement=pElement->FirstChildElement("id");
			if(pSubElement && pSubElement->GetText())
				webData._id=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("ownerid");
			if(pSubElement && pSubElement->GetText())
				webData._ownerId=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("isnew");
			if(pSubElement && pSubElement->GetText())
				webData._isnew=atoi(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("link");
			if(pSubElement && pSubElement->GetText())
				webData._szLink.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("title");
			if(pSubElement && pSubElement->GetText())
				webData._szTitle.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("body");
			if(pSubElement && pSubElement->GetText())
				webData._szMsg.fromUTF8(pSubElement->GetText());

			pSubElement=pElement->FirstChildElement("dateline");
			if(pSubElement && pSubElement->GetText())
				webData._szDateLine.fromUTF8(pSubElement->GetText());

			vecCoupon.push_back(webData);

			pElement=pElement->NextSiblingElement();
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();
	return bRet ;
}

bool CWebData::GetGameCharactorService(_tstringA sSessionId, WEB_CHARACT_MSG& gameInfo)
{
	_tstring szUrl=theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_CHARACT"))+gameInfo._szName.c_str( );
	_tstringA  sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());
	
	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(szUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_charactor_data(sSvrRes.sResult.c_str(), gameInfo) ;
}

bool CWebData::process_charactor_data(std::string sData, CWebData::WEB_CHARACT_MSG &webData)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;

	do {

		xmlDoc.Parse(sData.c_str()) ;
		TiXmlElement* pElement = xmlDoc.FirstChildElement("data") ;
		if(NULL == pElement)
		{
			break ;
		}

		pElement=pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText()) ;
		if( 0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pElement->NextSiblingElement() ;
		if(NULL == pElement)
		{
			break ;
		}

		TiXmlElement* pSubElement=pElement->FirstChildElement("name");
		if(pSubElement && pSubElement->GetText())
			webData._szCharater.fromUTF8(pSubElement->GetText());

		pSubElement=pElement->FirstChildElement("servername");
		if(pSubElement && pSubElement->GetText())
			webData._szServer.fromUTF8(pSubElement->GetText());

		pSubElement=pElement->FirstChildElement("useravatar");
		if(pSubElement && pSubElement->GetText())
			webData._szAvatar.fromUTF8(pSubElement->GetText());

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();
	
	return bRet ;
}

bool CWebData::GetGameServerService(_tstringA sSessionId, vector<WEB_GAMESERVER_INFO>& vecGameServerInfo)
{
	_tstring szUrl=theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_GAMESERVER")) ;
	_tstringA  sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(szUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	return process_gameserver_data(sSvrRes.sResult.c_str(), vecGameServerInfo);
}

bool CWebData::process_gameserver_data(std::string sData, vector<CWebData::WEB_GAMESERVER_INFO>& vecGameServerInfo)
{
	bool bRet = false ;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pElement=NULL;	

	do {

		xmlDoc.Parse(sData.c_str()) ;
		pElement=xmlDoc.FirstChildElement("data");
		if(NULL == pElement)
		{
			break ;
		}

		pElement=pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult;
		szResult.fromUTF8(pElement->GetText( ) );
		if( 0!=szResult.CompareNoCase(_T("success")) )
		{
			break ;
		}

		pElement=pElement->NextSiblingElement( );

		while(pElement)
		{
			CWebData::WEB_GAMESERVER_INFO webData ;

			TiXmlElement* pSubElement=pElement->FirstChildElement("lastlogin");
			if(pSubElement)
			{
				webData._szLastLoginTime.fromUTF8(pSubElement->GetText());
			}

			pSubElement=pElement->FirstChildElement("game");
			if(pSubElement && pSubElement->GetText())
			{
				webData._szGameAbbr.fromUTF8(pSubElement->GetText());
			}

			pSubElement=pElement->FirstChildElement("server");
			if(pSubElement && pSubElement->GetText())
			{
				webData._serverId = atoi(pSubElement->GetText());
			}

			vecGameServerInfo.push_back(webData) ;
			pElement=pElement->NextSiblingElement();
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::RegisterUser(_tstring sUserName, _tstringA sPassword, _tstringA& sSessionId, _tstring& sUserName2, _tstring& sNickName)
{
	char buf[512];
	sprintf(buf, "%d", time(NULL));	

	const std::string t = buf;
	const std::string u = sUserName.toUTF8().c_str();
	std::string p = sPassword.c_str();
	const std::string code = "$Y*n6#Pb81Kg@C!";
	sprintf(buf, "%d", rand() % 10000);

	const std::string r = buf;
	const std::string s = code + u + p + t + r;

	MD5Context ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)s.c_str(), s.size());
	unsigned char sum[16];
	MD5Final(sum, &ctx);	

	sprintf(buf, "%02x%02x", sum[14], sum[15]);
	const std::string vcode = buf;
	const std::string url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_LOGIN")).toNarrowString() + t + "&r=" + r + "&v=" + vcode;

	sprintf(buf, "u=%s&p=%s", u.c_str(), p.c_str());
	const std::string data = buf;

	p = "";

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.PostFieldService(_tstring(url.c_str()), _T(""), _tstring(data.c_str()), &sSvrRes) ;

	return process_login_data(sSvrRes.sResult, sSessionId, sUserName2, sNickName) ;
}

bool CWebData::process_login_data(std::string sData, _tstringA& sSessionId, _tstring& sUserName, _tstring& sNickName)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;
	TiXmlElement  *pElement=NULL;	

	do{

		xmlDoc.Parse(sData.c_str()) ;

		pElement=xmlDoc.FirstChildElement("data");
		if(NULL == pElement)
		{
			break ;
		}

		pElement=pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if( 0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pElement->NextSiblingElement();
		if (pElement)
		{
			pElement=pElement->FirstChildElement( "id"); 
		}
		if (pElement && pElement->GetText())
		{
			sUserName.assign(_tstring(pElement->GetText()).c_str());
			pElement=pElement->NextSiblingElement(); 
		}
		if (pElement && pElement->GetText())
		{
			sSessionId.assign(pElement->GetText());
			pElement=pElement->NextSiblingElement();
		}
		if (pElement)
		{
			pElement=pElement->NextSiblingElement();
		}
		if (pElement && pElement->GetText())
		{
			sNickName.assign(_tstring(pElement->GetText()).c_str());
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::GetUserFromMail(_tstring sEmailName, _tstring& sUserName)
{
	char buf[512];
	sprintf(buf, "%d", time(NULL));	

	const std::string t = buf;
	const std::string u =sEmailName.toNarrowString();
	const std::string code = "$Y*n6#Pb81Kg@C!";
	sprintf(buf, "%d", rand() % 10000);

	const std::string r = buf;
	const std::string s = code + u + t + r;

	MD5Context ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)s.c_str(), s.size());
	unsigned char sum[16];
	MD5Final(sum, &ctx);	

	sprintf(buf, "%02x%02x", sum[14], sum[15]);
	const std::string vcode = buf;
	const std::string url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_GETUSER_MAIL")).toNarrowString() +"?"+"&m="+u+"&t="+ t + "&r=" + r + "&v=" + vcode;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(_tstring(url), _T(""), _T(""), &sSvrRes) ;

	return process_mail_data(sSvrRes.sResult.c_str(), sUserName) ;
}

bool CWebData::process_mail_data(std::string sData, _tstring& sUserName)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pElement = xmlDoc.FirstChildElement("data") ;
		if(NULL == pElement)
		{
			break ;
		}

		pElement = pElement->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult;
		szResult.fromUTF8(pElement->GetText());
		if( 0!=szResult.CompareNoCase(_T("success")) )
		{
			break ;
		}

		pElement=pElement->NextSiblingElement();
		if (pElement)
		{
			pElement=pElement->FirstChildElement("account");
		}
		if (pElement && pElement->GetText())
		{
			sUserName.assign(_tstring(pElement->GetText()).c_str()) ;
		}

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool  CWebData::IsGameSignupForBeta(_tstring sUserName, _tstring sGameAbbr, _tstring sLanguage)
{//replace curl in order to support https GET request
	bool bRet = false ;

	_tstring tsUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_ALLOW_SIGNUP")) + \
		_T("game=") + sGameAbbr + _T("&account=") + sUserName.MakeLower() + _T("&lang=") + sLanguage ; ///"https://register.perfectworld.com/signupbeta/?game=xx&username=xxx&sessid=xxx"

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	TiXmlDocument xmlDoc ;
	TiXmlElement  *pRoot = NULL ;
	TiXmlElement  *pElement = NULL ;

	xmlDoc.Parse(sSvrRes.sResult.c_str()) ;
	pRoot = xmlDoc.FirstChildElement("data") ;
	if(pRoot)
	{
		pElement=pRoot->FirstChildElement("result") ;
		if(pElement && pElement->GetText())
		{
			_tstring   szResult, sGameAbbr ;
			szResult.fromUTF8(pElement->GetText( )) ;
			if(0 == szResult.CompareNoCase(_T("success")))
			{
				if (pElement=pRoot->FirstChildElement("send"))
				{
					if (pElement->GetText() && atoi(pElement->GetText()))
					{
						bRet = true ;
					}
				}
			}
		}
	}

	/*5. return result, if true, succeed, otherwise failed */
	return bRet ;
}

bool  CWebData::GetSignupForBetaService(_tstringA sSessionId, _tstring sUserName, _tstring sGameAbbr, _tstring sLanguage)
{
	const std::string url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SNS_SIGNUP")).toNarrowString() + \
		"game=" + sGameAbbr.MakeLower().toNarrowString() + "&username=" + sUserName.MakeLower().toNarrowString() + "&lang=" + sLanguage.toNarrowString() + \
		"&sessid=" + sSessionId ; ///"https://register.perfectworld.com/signupbeta/?game=xx&username=xxx&sessid=xxx"

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.HttpsGetService(_tstring(url.c_str()), _T(""), _T(""), &sSvrRes) ;

	/*5. return result, if true, succeed, otherwise failed */
	return atoi(sSvrRes.sResult.c_str()) ;
}

bool  CWebData::GetGameBetaKeyInfo(_tstringA sSessionId, map<_tstring, int>& mapBetaKeyFlag)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BETAKEYFLAG")).c_str()) ;
	tsUrl.append(_T("&PWRD=")) ;
	tsUrl.append(_tstring(sSessionId.c_str()).c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	return process_gamebetakey_data(sSvrRes.sResult, mapBetaKeyFlag) ;
}

bool CWebData::process_gamebetakey_data(std::string sData, map<_tstring, int>& mapBetaKeyFlag)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse((const char*)sData.c_str()) ;
		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult, sGameAbbr ;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("flaginfo") ;
		while (pElement != NULL)
		{
			if (pElement->FirstChildElement("game") && pElement->FirstChildElement("flag"))
			{
				sGameAbbr = pElement->FirstChildElement("game")->GetText() ;
				sGameAbbr.MakeLower() ;
				szResult = pElement->FirstChildElement("flag")->GetText()  ;
				mapBetaKeyFlag[sGameAbbr] = _ttoi(szResult.c_str()) ;
			}

			pElement=pElement->NextSiblingElement();
		}

		if (mapBetaKeyFlag.size() <= 0)
		{//no game flag is retrieved.
			bRet = false ;
		}
		else
		{
			bRet = true ;
		}

	}while(false) ;

	xmlDoc.Clear() ;

	return bRet ;
}

bool  CWebData::GetGameBetaPayInfo(_tstring sUserName, _tstring sAbbr, int& nBetaPayFlag)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BETAPAYFLAG")).c_str()) ;
	tsUrl.append(_T("game=")) ;
	tsUrl.append(sAbbr.c_str()) ;
	tsUrl.append(_T("&account=")) ;
	tsUrl.append(sUserName.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.HttpsGetService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	return process_gamebetapay_data(sSvrRes.sResult, nBetaPayFlag) ;
}

bool CWebData::process_gamebetapay_data(std::string sData, int& nBetaPayFlag)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse((const char*)sData.c_str()) ;
		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("available") ;
		if (pElement != NULL)
		{
			nBetaPayFlag = atoi(pElement->GetText());
		}		
		bRet = true ;

	}while(false) ;

	xmlDoc.Clear() ;

	return bRet ;
}

bool  CWebData::IsGameOffline(_tstring sLang, map<_tstring, _tstring>& mapBetaOffline)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BETAOFFLINE")).c_str()) ;
	tsUrl.append(_T("lang=")) ;
	tsUrl.append(sLang.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	return process_gamebetaoffline_data(sSvrRes.sResult, mapBetaOffline) ;
}

bool CWebData::process_gamebetaoffline_data(std::string sData, map<_tstring, _tstring>& mapBetaOffline)
{
	bool bRet = false ;
	TiXmlDocument  xmlDoc ;

	do {

		xmlDoc.Parse((const char*)sData.c_str()) ;
		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult ;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("offlineservers") ;
		if (pElement != NULL)
		{
			pElement = pElement->FirstChildElement("offlineserver");
			while (pElement != NULL)
			{
				if (pElement->FirstChildElement("game") && pElement->FirstChildElement("message"))
				{
					_tstring sGameAbbr, sMsg;
					sGameAbbr.fromUTF8(pElement->FirstChildElement("game")->GetText());
					sGameAbbr.MakeLower();
					sMsg.fromUTF8(pElement->FirstChildElement("message")->GetText());
					mapBetaOffline[sGameAbbr] = sMsg;
				}

				pElement=pElement->NextSiblingElement();
			}
		}

		bRet = true ;

	}while(false) ;

	return bRet ;
}

bool  CWebData::GetPopupNotificationService(_tstring sGameAbbr, _tstring sLang, map<_tstring, _tstring>& mapNotificationText)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_POPUPNOTIFICATION")).c_str()) ;
	tsUrl.append(_T("game=")) ;
	tsUrl.append(sGameAbbr.c_str()) ;
	tsUrl.append(_T("&lang=")) ;
	tsUrl.append(sLang.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	return process_popuptext_data(sSvrRes.sResult, mapNotificationText) ;
}

bool CWebData::process_popuptext_data(std::string sData, map<_tstring, _tstring>& mapNotificationText)
{
	bool bRet = false ;
	TiXmlDocument  xmlDoc ;

	do {

		xmlDoc.Parse((const char*)sData.c_str()) ;
		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}
		_tstring   szResult ;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("item") ;
		while (pElement != NULL)
		{
			if (pElement->FirstChildElement("type") && pElement->FirstChildElement("content"))
			{
				_tstring sType = pElement->FirstChildElement("type")->GetText() ;
				sType.MakeLower() ;
				_tstring sContent = pElement->FirstChildElement("content")->GetText()  ;
				mapNotificationText[sType] = sContent ;

				bRet = true ;
			}

			pElement=pElement->NextSiblingElement();
		}

	}while(false) ;

	/*5. return result, if true, succeed, otherwise failed */
	return bRet ;
}

bool CWebData::GetEmergencyMessageServicce(_tstring sProduct, _tstring sLang, vector<EMERGENCY_MESSAGE_DATA>& vecEmergencyMessage)
{
	_tstring sUrl ;
	sUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SERVEROFFLINE")).c_str()) ;
	sUrl.append(_T("product=")) ;
	sUrl.append(sProduct.c_str()) ;
	sUrl.append(_T("&lang=")) ;
	sUrl.append(sLang.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _T(""), _T(""), &sSvrRes) ;

	return process_emergencymessage_data(sSvrRes.sResult, vecEmergencyMessage) ;
}

bool CWebData::process_emergencymessage_data(std::string sData, vector<EMERGENCY_MESSAGE_DATA>& vecEmergencyMessage)
{
	bool bRet = false ;
	TiXmlDocument  xmlDoc ;

	do {

		xmlDoc.Parse((const char*)sData.c_str()) ;
		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult ;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("messages") ;
		if (pElement != NULL)
		{
			pElement = pElement->FirstChildElement("message");
			while (pElement != NULL)
			{
				if (pElement->FirstChildElement("id") && pElement->FirstChildElement("product") && pElement->FirstChildElement("type") && pElement->FirstChildElement("content"))
				{
					EMERGENCY_MESSAGE_DATA data;
					data._szId.fromUTF8(pElement->FirstChildElement("id")->GetText());
					data._szMode = _T("") ;
					data._szUpdateTimeStamp = _T("") ;
					data._szProduct.fromUTF8(pElement->FirstChildElement("product")->GetText());
					data._szType.fromUTF8(pElement->FirstChildElement("type")->GetText());
					data._szText.fromUTF8(pElement->FirstChildElement("content")->GetText());

					vecEmergencyMessage.push_back(data);

					bRet = true ;
				}

				pElement=pElement->NextSiblingElement("message");
			}
		}

	}while(false) ;

	return bRet ;
}

bool CWebData::IsGameInEarlyBeta(_tstring sLang, map<_tstring, _tstring>& mapEarlyBetaData)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BETAEARLYACCESS")).c_str()) ;
	tsUrl.append(_T("lang=")) ;
	tsUrl.append(sLang.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	bool bRet = false ;
	TiXmlDocument  xmlDoc ;
	do
	{
		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;
		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}
		_tstring   szResult ;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("accesses") ;
		if (pElement != NULL)
		{
			pElement = pElement->FirstChildElement("access");
			while (pElement != NULL)
			{
				if (pElement->FirstChildElement("game") && pElement->FirstChildElement("url"))
				{
					_tstring sGameAbbr, sUrl;
					sGameAbbr.fromUTF8(pElement->FirstChildElement("game")->GetText());
					sGameAbbr.MakeLower();
					sUrl.fromUTF8(pElement->FirstChildElement("url")->GetText());
					mapEarlyBetaData[sGameAbbr] = sUrl;

					bRet = true;
				}

				pElement=pElement->NextSiblingElement();
			}
		}
	}while(false) ;
	return bRet ;
}

bool CWebData::GetDownloadOptionService(int& nDownloadOption)
{
	return false;	//dont change download tool,use hard coded download tool

	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_DOWNLOADOPTION")) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _T(""), _T(""), &sSvrRes) ;

	return process_downloadoption_data(sSvrRes.sResult, nDownloadOption) ;
}

bool CWebData::process_downloadoption_data(std::string sData, int& nDownloadOption)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pRoot=xmlDoc.FirstChildElement("data");
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result");
		if(NULL == pElement)
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement = pRoot->FirstChildElement("service") ;
		if (!pElement || !pElement->GetText())
		{
			break ;
		}

		nDownloadOption = atoi(pElement->GetText()) ;

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::GetEmsService(_tstring sLang,WEB_EMS_DATA& emsData,bool bNotify, const std::string sEmsData)
{
	int bRet=false;
	TiXmlDocument xmlDoc ;

	do{
		CWebService sWebSvr ;
		CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
		if (!bNotify)
		{  // it is not pushed by server,so need be pulled from the web service.
			_tstring sUrl = theUIString._GetStringFromId(_T("IDS_EMS_URL"));
			sUrl.append(sLang.c_str()) ;
			bRet = sWebSvr.GetFieldService(sUrl, _T(""), _T(""), &sSvrRes) ;
			if (!bRet)
			{
				break;
			}
		}
		else
		{  // it is pushed by server, just needs be parsed.
			sSvrRes.sResult = sEmsData;
		}

		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;

		TiXmlElement* pRoot=xmlDoc.FirstChildElement("data");
		if(NULL == pRoot)
		{
			break ;
		}

        TiXmlElement* pElement=NULL;
        if (!bNotify)
        {   //web service result
			pElement=pRoot->FirstChildElement("result");
			if(NULL == pElement)
			{
				break ;
			}

			_tstring szResult;
			szResult.fromUTF8(pElement->GetText());
			if(0!=szResult.CompareNoCase(_T("success")))
			{
				break ;
			}
        }
	
		pElement=pRoot->FirstChildElement(sLang.toNarrowString().c_str());
		if(NULL == pElement)
		{   
			break ;
		}
		emsData._szTimeBegin.fromUTF8(pElement->Attribute("start"));
		emsData._szTimeEnd.fromUTF8(pElement->Attribute("end"));
        
		TiXmlElement* pSubElement=pElement->FirstChildElement("url");
		if (pSubElement)
		{
            emsData._szLink.fromUTF8(pSubElement->GetText());
		}
		
	    pSubElement=pElement->FirstChildElement("content");
		if (pSubElement)
		{
			emsData._szContent.fromUTF8(pSubElement->GetText());
		}
		
		pSubElement=pElement->FirstChildElement("title");
		if (pSubElement)
		{
			emsData._szTitle.fromUTF8(pSubElement->GetText());
		}
       
		pSubElement=pElement->FirstChildElement("id");
		if (pSubElement)
		{
			emsData._szId.fromUTF8(pSubElement->GetText());
		}

		pSubElement=pElement->FirstChildElement("icon");
		if (pSubElement)
		{
			_tstring sUrlTmp;
			sUrlTmp.fromUTF8(pSubElement->GetText());
			int nPos = sUrlTmp.rfind(_T('/'));
			if(nPos>0)
			{
				emsData._szIconPath += sUrlTmp.Right(sUrlTmp.length()-nPos-1);
				bRet = DownloadFile(sUrlTmp.c_str( ), emsData._szIconPath.c_str()); //download ems icon.
			}
		}
		else
		{
           bRet = true;
		}

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::GetBetaFeedbackService(_tstring sUserName, _tstring sSummary, _tstring sDescription, _tstring sLang, _tstring sCCVersion, _tstring sSysInfo, _tstring sZipMd5File)
{
	std::map<_tstring,CWebService::POST_DATA> mapField_Value;
	CWebService::POST_DATA data;

	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("feedback");
	mapField_Value[_T("mailType")] = data;
	data.sFieldValue = sUserName ;
	mapField_Value[_T("username")] = data;
	data.sFieldValue = sSummary ;
	mapField_Value[_T("summary")] = data;
	data.sFieldValue = sDescription ;
	mapField_Value[_T("description")] = data;
	data.sFieldValue = sLang;
	mapField_Value[_T("lang")] = data;
	data.sFieldValue = sCCVersion ;
	mapField_Value[_T("ver")] = data;
	data.sFieldValue = sSysInfo ;
	mapField_Value[_T("sysinfo")] = data;
	data.nFieldType = CURLFORM_FILE;
	data.sFieldValue = sZipMd5File;
	mapField_Value[_T("md5file")] = data;
	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("Send");
	mapField_Value[_T("submit")] = data;

	return GetFeedbackService(mapField_Value) ;
}

bool CWebData::GetBugReportService(_tstring sUserName, _tstring sBugType, _tstring sSummary, _tstring sDescription, _tstring sLang, _tstring sCCVersion, _tstring sSysInfo, _tstring sZipMd5File)
{
	std::map<_tstring,CWebService::POST_DATA> mapField_Value;
	CWebService::POST_DATA data;

	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("bugreport");
	mapField_Value[_T("mailType")] = data;
	data.sFieldValue = sUserName ;
	mapField_Value[_T("username")] = data;
	data.sFieldValue = sBugType ;
	mapField_Value[_T("bugtype")] = data;
	data.sFieldValue = sSummary ;
	mapField_Value[_T("summary")] = data;
	data.sFieldValue = sDescription ;
	mapField_Value[_T("description")] = data;
	data.sFieldValue = sLang;
	mapField_Value[_T("lang")] = data;
	data.sFieldValue = sCCVersion ;
	mapField_Value[_T("ver")] = data;
	data.sFieldValue = sSysInfo ; ;
	mapField_Value[_T("sysinfo")] = data;
	data.nFieldType = CURLFORM_FILE;
	data.sFieldValue = sZipMd5File ;
	mapField_Value[_T("md5file")] = data;
	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("Send");
	mapField_Value[_T("submit")] = data;

	return GetFeedbackService(mapField_Value) ;
}

bool CWebData::GetUninstallGameService(_tstring sUserName, _tstring sFactor, _tstring sGameName, _tstring sStar, _tstring sLang, _tstring sCCVersion, _tstring sComment, _tstring sSysInfo, _tstring sZipMd5File)
{
	std::map<_tstring,CWebService::POST_DATA> mapField_Value;
	CWebService::POST_DATA data;

	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("uninstall");
	mapField_Value[_T("mailType")] = data;
	data.sFieldValue = sUserName ;
	mapField_Value[_T("username")] = data;
	data.sFieldValue = sFactor;
	mapField_Value[_T("factors")] = data;
	data.sFieldValue = sGameName ;
	mapField_Value[_T("gamename")] = data;
	data.sFieldValue = sStar;
	mapField_Value[_T("star")] = data;
	data.sFieldValue = sLang;
	mapField_Value[_T("lang")] = data;
	data.sFieldValue = sCCVersion;
	mapField_Value[_T("ver")] = data;
	data.sFieldValue = sComment ;
	mapField_Value[_T("comments")] = data;
	data.sFieldValue = sSysInfo ;
	mapField_Value[_T("sysinfo")] = data;
	data.nFieldType = CURLFORM_FILE;
	data.sFieldValue = sZipMd5File;
	mapField_Value[_T("md5file")] = data;
	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("Send");
	mapField_Value[_T("submit")] = data;

	return GetFeedbackService(mapField_Value) ;
}

bool CWebData::GetCrashReportService(_tstring sUserName, _tstring sLang, _tstring sCCVersion, _tstring sDumpFile, _tstring sSysInfo, _tstring sZipMd5File,_tstring szDumpTyoe)
{
	std::map<_tstring,CWebService::POST_DATA> mapField_Value;
	CWebService::POST_DATA data;

	data.nFieldType = CURLFORM_COPYCONTENTS;
	if (szDumpTyoe.compare(I_CRASHREPORT) == 0)
	{
		data.sFieldValue = I_CRASHREPORT;
	}
	else
	{
		data.sFieldValue = CRASHREPORT;
	}
	mapField_Value[_T("mailType")] = data;
	data.sFieldValue = sUserName ;
	mapField_Value[_T("username")] = data;
	data.sFieldValue = sLang;
	mapField_Value[_T("lang")] = data;
	data.sFieldValue = sCCVersion ;
	mapField_Value[_T("ver")] = data;
	data.sFieldValue = sSysInfo ;
	mapField_Value[_T("sysinfo")] = data;
	data.nFieldType = CURLFORM_FILE;
	data.sFieldValue = sDumpFile ;
	mapField_Value[_T("dumpfile")] = data;
	data.nFieldType = CURLFORM_FILE;
	data.sFieldValue = sZipMd5File;
	mapField_Value[_T("md5file")] = data;
	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("Send");
	mapField_Value[_T("submit")] = data;
	return GetFeedbackService(mapField_Value) ;
}

bool CWebData::GetUninstallService(_tstring sLang, _tstring sFactor, _tstring sStar, _tstring sSysInfo, _tstring sComment)
{
	std::map<_tstring,CWebService::POST_DATA> mapField_Value;
	CWebService::POST_DATA data;

	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("arc_uninstall");
	mapField_Value[_T("mailType")] = data;
	data.sFieldValue = sFactor;
	mapField_Value[_T("factors")] = data;
	data.sFieldValue = sStar;
	mapField_Value[_T("star")] = data;
	data.sFieldValue = sLang;
	mapField_Value[_T("lang")] = data;
	data.sFieldValue = sSysInfo ;
	mapField_Value[_T("sysinfo")] = data;
	data.sFieldValue = sComment ;
	mapField_Value[_T("comments")] = data;
	data.nFieldType = CURLFORM_COPYCONTENTS;
	data.sFieldValue = _T("Send");
	mapField_Value[_T("submit")] = data;

	return GetFeedbackService(mapField_Value) ;	
}

bool CWebData::GetFeedbackService(_tstring sPostData)
{
	const std::string url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SVR_FEEDBACK")).toNarrowString() ;
	std::string data(sPostData.toUTF8()) ;
	//"mailType=feedback&username=coretest01&summary=shenhui&description=thanks for your games&sysinfo=Intel CPU";
	//"mailType=bugreport&username=coretest01&bugtype=defaulttype&summary=defaultsummary&description=thanks for your games&sysinfo=Intel CPU";

	CWebService sWebSvr ; 
	return sWebSvr.PostFieldService(_tstring(url.c_str()), _T(""), sPostData, NULL) ;
}

bool CWebData::GetFeedbackService(const std::map<_tstring,CWebService::POST_DATA>& mapField_Value)
{
	const std::string url = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SVR_FEEDBACK")).toNarrowString() ;

	CWebService sWebSvr ; 
	return sWebSvr.PostFormService(_tstring(url.c_str()), _T(""), mapField_Value) ;
}

bool CWebData::GetGameEULAService(_tstring sGameAbbr, _tstring sLang, _tstring& sVersion, _tstring& sEula)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_EULA")).c_str()) ;
	tsUrl.append(_T("game=")) ;
	tsUrl.append(sGameAbbr.MakeLower().c_str()) ;
	tsUrl.append(_T("&lang=")) ;
	tsUrl.append(sLang.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;
	
	return process_eula_data(sSvrRes.sResult, sVersion, sEula) ;
}

bool CWebData::GetCOREEULAService(_tstring sLang, _tstring& sVersion, _tstring& sEula)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_EULA")).c_str()) ;
	tsUrl.append(_T("game=core")) ;
	tsUrl.append(_T("&lang=")) ;
	tsUrl.append(sLang.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;
	
	return process_eula_data(sSvrRes.sResult, sVersion, sEula) ;
}

bool CWebData::IsBanedRegion()
{
   static int nRegionBanFlag = -1;

   if (nRegionBanFlag == -1) //just get service once to reducing requirements.
   {
	   _tstring tsUrl ;
	   tsUrl.assign(theUIString._GetStringFromId(_T("IDS_LOGIN_BANREGION")).c_str()) ;

	   CWebService sWebSvr ;
	   CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	   sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	   nRegionBanFlag = _tstring(sSvrRes.sResult).ConvToInt32();
   }

    return (nRegionBanFlag==1);
}

bool CWebData::process_eula_data(std::string sData, _tstring& sVersion, _tstring& sEula)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pRoot=xmlDoc.FirstChildElement("data");
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result");
		if(!pElement || !pElement->GetText())
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement = pRoot->FirstChildElement("version") ;
		if (!pElement || !pElement->GetText())
		{
			break ;
		}
		sVersion.fromUTF8(pElement->GetText()) ;
	
		pElement = pRoot->FirstChildElement("content") ;
		if (!pElement || !pElement->GetText())
		{
			break ;
		}
		sEula.fromUTF8(pElement->GetText()) ;

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::DownloadFile(LPCTSTR lpszUrl,LPCTSTR lpszFile,bool bRetry/*=true*/, int nTimeout)
{
	bool bRet = false ;

	do {
		_tstring szFile(lpszFile);
		DWORD dwRemoteSize = 0 , dwLocalSize = 0 ;

		//get file remote size(at most for five times)
		bool bLocalRet = false ;
		CWebService sWebSvr ;
		for (int i = 0 ; i < 5 ; ++ i)
		{
			bLocalRet = sWebSvr.GetFileLengthService(_tstring(lpszUrl), dwRemoteSize, nTimeout) ;
			if (bLocalRet && (0 != dwRemoteSize))
			{
				break ;
			}
		}

		if (!bLocalRet || (0 == dwRemoteSize))
		{//get file size failed
			break ;
		}

		//download file
		for (int i = 0 ; i < 5 ; ++ i)
		{
			if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(szFile.c_str()))  
			{//file already exist
				//get local file size
				HANDLE hFile = CreateFile(szFile.c_str(), 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
				if (hFile != INVALID_HANDLE_VALUE)
				{
					dwLocalSize  = GetFileSize(hFile, NULL) ;
					CloseHandle(hFile) ;
				}

				if(dwLocalSize == dwRemoteSize)
				{//local files is the same with internet files ,don't need download.
					bRet = true ;
					break;
				}

				DeleteFile(szFile.c_str());//local files is not correct,delete it first.
				dwLocalSize =0;
			}

			//create directory
			_tstring szFolder(szFile.c_str()) ;
			szFolder = szFolder.substr(0,szFolder.rfind(_T('\\')));
			MakeDir(szFolder.c_str());

			//download file
			CWebService::RESULT_DATA sSvrRes = {1, "", szFile, NULL} ;
			sWebSvr.GetFieldService(_tstring(lpszUrl), _T(""), _T(""), &sSvrRes, nTimeout) ;
			if (sSvrRes.pStream)
			{
				fclose(sSvrRes.pStream) ;
				sSvrRes.pStream = NULL ;
			}

			//check downloaded file size
			if(INVALID_FILE_ATTRIBUTES != GetFileAttributes(szFile.c_str())) 
			{
				//get local file size
				HANDLE hFile = CreateFile(szFile.c_str(), 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) ;
				if (hFile != NULL)
				{
					dwLocalSize  = GetFileSize(hFile, NULL) ;
					CloseHandle(hFile) ;
				}
			}

			if (dwLocalSize == dwRemoteSize)
			{
				break ;
			}
			if (!bRetry)
			{
				break;
			}
		}

		if (dwLocalSize == dwRemoteSize)
		{
			bRet=true;
		}

	}while(false) ;

	return bRet;
}

bool CWebData::GetVersionInfoService(_tstring sLang, _tstring sLocalVer, _tstring& sRemoteVer, _tstring& sUrl, int& nMode)
{
	_tstring tsUrl ;
	tsUrl.assign(_T("http://arc01.perfectworld.com/clientapi/coreupdate?"));
	tsUrl.append(_T("lang=")) ;
	tsUrl.append(sLang.c_str()) ;
	tsUrl.append(_T("&ver=")) ;
	tsUrl.append(sLocalVer.c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	bool bRet = false ;
	_tstring sData = sSvrRes.sResult;
	size_t nIndex = sData.Find("|");
	if(nIndex>=0)
	{
		sRemoteVer = sData.Left(nIndex);
		sData = sData.Right(sData.size() - nIndex - 1);

		nIndex = sData.Find("|");
		if(nIndex>=0)
		{
			sUrl = sData.Left(nIndex);
			nMode = _ttoi(sData.Right(sData.size() - nIndex - 1).c_str());
			bRet = true;
		}
	}

	return bRet;
}

bool CWebData::GetSaltService(_tstring sUser, int &nLoginType, _tstring &salt)
{
	_tstring tsUrl ;
	tsUrl.assign(theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_SALT")).c_str()) ;
	tsUrl += _T("email=");
	tsUrl += sUser;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes) ;

	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;

		TiXmlElement* pRoot=xmlDoc.FirstChildElement("data");
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result");
		if(!pElement || !pElement->GetText())
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement = pRoot->FirstChildElement("logintype") ;
		if (!pElement || !pElement->GetText())
		{
			break ;
		}
		nLoginType = atoi(pElement->GetText());

		pElement = pRoot->FirstChildElement("salt") ;
		if (!pElement || !pElement->GetText())
		{
			break ;
		}
		salt = pElement->GetText();

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::MakeDir(LPCTSTR lpszStr)
{
	_tstring strFolder(lpszStr) ;

	if (strFolder.empty())
	{
		return false;
	}

	strFolder.Replace(_T("/"), _T("\\"));
	if (strFolder[strFolder.length()-1] != _T('\\'))
	{
		strFolder += _T("\\");
	}
	size_t nLength = strFolder.length();
	for (size_t i = 0; i<nLength; i++)
	{
		if (strFolder[i] == _T('\\'))
		{
			_tstring strTmpFolder = strFolder.Mid(0, i);
			if (_tmkdir(strTmpFolder.c_str()) == 2)
			{
				return false;
			}
		}
	}
	return true;
}

bool CWebData::GetGameinfoVersion(_tstring sLan, int &nVersion)
{
	_tstring tsUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_GAMEINFO_VERSION")).c_str();
	//tsUrl.Format(2, _T("%s&lang=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_GAMEINFO_VERSION")).c_str(), sLan.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	
	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	bool bRet = false ;
	TiXmlDocument xmlDoc ;

	do {

		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;

		TiXmlElement* pRoot=xmlDoc.FirstChildElement("data");
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result");
		if(!pElement || !pElement->GetText())
		{
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0!=szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement = pRoot->FirstChildElement("version");
		if(!pElement || !pElement->GetText())
		{
			break;
		}
		szResult.fromUTF8(pElement->GetText());
		nVersion = _ttoi(szResult.c_str());
		bRet = true;
	}while(0);

	xmlDoc.Clear();
	return bRet;
}

bool CWebData::GetAllGameinfo(_tstring sLan, _tstring &sData)
{
	_tstring tsUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_GAMEINFO_ALLGAME")).c_str();
	//tsUrl.Format(2, _T("%s&lang=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_GAMEINFO_ALLGAME")).c_str(), sLan.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	sData = sSvrRes.sResult;
	return true;
}

bool CWebData::GetGameinfoByAbbr(_tstring sLan, _tstring sAbbr, _tstring &sData)
{
	_tstring tsUrl;
	tsUrl.Format(3, _T("%s&game=%s&lang=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_GAMEINFO_ONEGAME")).c_str(), 
		sAbbr.c_str(), sLan.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	
	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	sData = sSvrRes.sResult;
	return true;
}

bool CWebData::GetRedemptionKeyInfoService(const _tstring& sSessionID, const _tstring& sKeycode, std::string& resultXml)
{
	_tstring tsUrl;
	tsUrl.Format(3, _T("%skey=%s&PWRD=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_REDEMPTION_KEY_API_INFO")).c_str(), sKeycode.c_str(), sSessionID.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	resultXml = sSvrRes.sResult;
	return true;
}

bool CWebData::GetRedemptionKeyReigistService(const _tstring& sSessionID,const _tstring& keycode, const _tstring& keyType,const _tstring& serverId, const _tstring& roleId, std::string& resultXml)
{
	_tstring tsUrl;
	if (!serverId.empty() && !roleId.empty())
	{
		tsUrl.Format(6, _T("%skey=%s&type=%s&serverid=%s&roleid=%s&PWRD=%s"), 
			theUIString._GetStringFromId(_T("IDS_CONFIG_REDEMPTION_KEY_API_REDEEM")).c_str(),
			keycode.c_str(), keyType.c_str(), serverId.c_str(), roleId.c_str(),sSessionID.c_str());

	}
	else if (serverId.empty() && roleId.empty())
	{
		tsUrl.Format(4, _T("%skey=%s&type=%s&PWRD=%s"), 
			theUIString._GetStringFromId(_T("IDS_CONFIG_REDEMPTION_KEY_API_REDEEM")).c_str(),
			keycode.c_str(), keyType.c_str(), sSessionID.c_str());
	}
	else
	{
		return false; // call error
	}
	
	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	resultXml = sSvrRes.sResult;
	return true;
}

bool CWebData::GetPromotionCampaignListService(const _tstring& sSessionID, std::string& resultXml)
{
	_tstring tsUrl;
	tsUrl.Format(2, _T("%sPWRD=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_PROMOTION")).c_str(), sSessionID.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	resultXml = sSvrRes.sResult;
	return true;
}

bool CWebData::PostPromotionListService(_tstringA sSessionId, _tstring sPostData)
{
	_tstring sUrl;
	sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_PROMOTIONRETURN")).c_str();
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ; 
	return sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sPostData, NULL) ;
}

bool CWebData::PostLocalGameListService(_tstringA sSessionId, _tstring sPostData)
{
	_tstring sUrl;
	sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_LOCALGAMELIST")).c_str();
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ; 
	return sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sPostData, NULL) ;
}

bool CWebData::GetADTrackingService(const _tstring& sSessionID, std::string& resultXml)
{
	_tstring tsUrl;
	tsUrl.Format(2, _T("%sPWRD=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_ADTRACKING")).c_str(), sSessionID.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	resultXml = sSvrRes.sResult;
	return true;
}

bool CWebData::GetGoalInfo(const _tstringA sSessionId, const _tstring& sOfferID, const _tstring& sRefID, _tstring& sGoalID)
{
	_tstring sUrl;
	sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_ADTRACKINGGETGOALID")).c_str();
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ; 
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	_tstring sPostData;
	sPostData.Format(1, _T("refer_id=%s"), sRefID.c_str());
	sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sPostData, &sSvrRes) ;
	return process_goal_info(sSvrRes.sResult, sOfferID, sGoalID);
}

bool CWebData::process_goal_info(std::string sData, const _tstring& sOfferID, _tstring& sGoalID)
{
	bool bRet = false ;
	TiXmlDocument xmlDoc;

	do {

		xmlDoc.Parse(sData.c_str()) ;

		TiXmlElement* pRoot = xmlDoc.FirstChildElement("data") ;
		if(NULL == pRoot)
		{
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result") ;
		if(NULL == pElement)
		{
			break ;
		}

		_tstring   szResult ;
		szResult.fromUTF8(pElement->GetText( )) ;
		if(0 != szResult.CompareNoCase(_T("success")))
		{
			break ;
		}

		pElement=pRoot->FirstChildElement("goals") ;
		if (pElement != NULL)
		{
			pElement = pElement->FirstChildElement("goal");
			while (pElement != NULL)
			{
				if (pElement->FirstChildElement("offer_id") && pElement->FirstChildElement("goal_id"))
				{
					_tstring sID;
					sID.fromUTF8(pElement->FirstChildElement("offer_id")->GetText());
					if(!sID.CompareNoCase(sOfferID))
					{
						sGoalID.fromUTF8(pElement->FirstChildElement("goal_id")->GetText());
						bRet = true ;
						break ;
					}

				}

				pElement=pElement->NextSiblingElement("goal");
			}
		}

	}while(false) ;

	xmlDoc.Clear();
	return bRet ;
}

bool CWebData::PostADTrackingService(_tstringA sSessionId, _tstring sPostData)
{
	_tstring sUrl;
	sUrl = theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_ADTRACKINGPOSTBACK")).c_str();
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ; 
	sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sPostData, NULL) ;
	return true;
}

bool CWebData::PostBackHasoffersGoal(const _tstring& sTransactionID, const _tstring& sGoalID)
{
	_tstring tsUrl;
	tsUrl.Format(3, _T("%sa=lsr&goal_id=%s&transaction_id=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_ADTRACKINGHASOFFERS")).c_str(), sGoalID.c_str(), sTransactionID.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	return true;
}

bool CWebData::GetBannedGameListService(const _tstring& sSessionID, std::string& resultXml)
{
	_tstring tsUrl;
	tsUrl.Format(2, _T("%sPWRD=%s"), theUIString._GetStringFromId(_T("IDS_CONFIG_WEBSITE_BANLIST")).c_str(), sSessionID.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		return false;
	}

	resultXml = sSvrRes.sResult;
	return true;
}

bool CWebData::GetShoppingCartService(_tstringA sSessionId, int& count)
{
	_tstring szUrl=theUIString._GetStringFromId(_T("IDS_WEBSERVICE_SHOPPINGCART")) ;
	_tstringA  sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str()) ;

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(szUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes) ;

	bool bRet = false ;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL, *pElement=NULL;	

	do {

		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;
		pRoot = xmlDoc.RootElement();
		if(!pRoot)
		{
			break;
		}
		pElement = pRoot->FirstChildElement("result");
		if(!pElement)
		{
			break;
		}
		_tstring   szResult;
		szResult.fromUTF8(pElement->GetText( ) );
		if( 0!=szResult.CompareNoCase(_T("success")) )
		{
			break ;
		}

		pElement = pRoot->FirstChildElement("quantity");
		if(!pElement)
		{
			break;
		}

		count = atoi(pElement->GetText());

		bRet = true ;

	}while(false) ;

	xmlDoc.Clear();

	return bRet ;
}

bool CWebData::GetIsFirstPlayingGameService(const _tstring& sSessionId, const _tstring& szGameAbbr, 
											const _tstring& lang,bool& bSuccess, bool& isFirstTime,_tstring& szFaildReason)
{
	_tstring tsUrl;
	tsUrl.Format(4, _T("%s?game_abbr=%s&lang=%s&PWRD=%s"), 
		theUIString._GetStringFromId(_T("IDS_CONFIG_PDL_API_IS_FIRST_PLAYING")).c_str(), 
		szGameAbbr.c_str(), lang.c_str(), sSessionId.c_str());

	CWebService sWebSvr ;
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;

	if(!sWebSvr.GetFieldService(tsUrl, _T(""), _T(""), &sSvrRes))
	{
		szFaildReason = _T("Request error!");
		return false;
	}

	bSuccess = false;
	do {
		TiXmlDocument xmlDoc ;
		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;

		TiXmlElement* pRoot=xmlDoc.FirstChildElement("data");
		if(NULL == pRoot)
		{
			szFaildReason = _T("Xml format error! <data> node not found!");
			break ;
		}

		TiXmlElement* pElement=pRoot->FirstChildElement("result");
		if(!pElement || !pElement->GetText())
		{
			szFaildReason = _T("Xml format error! <result> node not found!");
			break ;
		}

		_tstring szResult;
		szResult.fromUTF8(pElement->GetText());
		if(0==szResult.CompareNoCase(_T("success")))
		{
			pElement = pRoot->FirstChildElement("firsttime");
			if(!pElement || !pElement->GetText())
			{
				szFaildReason = _T("Xml format error! <firsttime> node not found!");
				break;
			}

			_tstring szIsFirst;
			szIsFirst.fromUTF8(pElement->GetText());
			
			bSuccess = true;
			szFaildReason = _T("");
			if (szIsFirst.CompareNoCase(_T("true")) == 0)
			{
				isFirstTime = true;
			}
			else 
			{
				isFirstTime = false;
			}
		}
		else if(0 == szResult.CompareNoCase(_T("failed")))
		{
			pElement = pRoot->FirstChildElement("reason");
			if(!pElement || !pElement->GetText())
			{
				szFaildReason = _T("Xml format error! <reason> node not found!");
				break;
			}

			szFaildReason.fromUTF8(pElement->GetText());
		}
		else
		{
			szFaildReason.Format(1,_T("<result>%s</result> is not an valid result node!"), szResult);
		}

	}while(0);

	return true;
}

int CWebData::EanbleDefenderSystem(const _tstring& sSessionId, const bool bEnable,const _tstring& szPassword)
{
	_tstring sUrl;
	sUrl = theUIString._GetStringFromId(_T("IDS_ENABLE_GUARD")).c_str();
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

    _tstring sPassword = CWebService::miniURLEncodeComponent(szPassword.toNarrowString());
	_tstring sPostData;
	sPostData.Format(2, _T("status=%d&password=%s"), bEnable,sPassword.c_str());

	CWebService sWebSvr ; 
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.HttpsPostService(sUrl, _tstring(sCookie.c_str()), sPostData, &sSvrRes) ;

	int nRet = -1 ;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL, *pElement=NULL;	

	do {

		xmlDoc.Parse(sSvrRes.sResult.c_str()) ;
		pRoot = xmlDoc.RootElement();
		if(!pRoot)
		{
			break;
		}
		pElement = pRoot->FirstChildElement("result");
		if(!pElement)
		{
			break;
		}
		_tstring   szResult;
		szResult.fromUTF8(pElement->GetText( ) );
		if( 0==szResult.CompareNoCase(_T("success")) )
		{
            nRet=0;
			break ;
		}
		else if(0 == szResult.CompareNoCase(_T("failed")))
		{
			pElement = pRoot->FirstChildElement("reason");
			if(!pElement || !pElement->GetText())
			{
				break;
			}
			_tstring   szFaildReason;
			szFaildReason.fromUTF8(pElement->GetText());
			if (szFaildReason.Find(_T("password"))!=-1)
			{
				nRet =1;
				break;
			}
			else if (szFaildReason.Find(_T("blocked"))!=-1)
			{
				nRet =2;
				break;
			}
			else if (szFaildReason.Find(_T("adminBlocked"))!=-1)
			{
				nRet =3;
				break;
			}
			else
			{
				break;
			}
		}

	}while(false) ;

	xmlDoc.Clear();

	return nRet ;
}

bool CWebData::UpdateDevName(const _tstring& sSessionId,const _tstring& szDevId,const _tstring& szDevName)
{
	_tstring sUrl;
	sUrl = theUIString._GetStringFromId(_T("IDS_UPDATE_DEVNAME")).c_str();
	_tstringA sCookie;
	sCookie.Format(1,"PWRD=%s", sSessionId.toUTF8().c_str());

	_tstring sDevName = CWebService::miniURLEncodeComponent(szDevName.toUTF8());

	_tstring sPostData;
	sPostData.Format(2, _T("mid=%s&name=%s"), szDevId.c_str(),sDevName.c_str());

	CWebService sWebSvr ; 
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sPostData, &sSvrRes) ;

	return true;
}

bool CWebData::AddGameToCloud(const _tstring& sSessionId, _tstring sGames)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CLOUD_GAME"));
	sUrl += _T("add-to-cloud");

	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	//CWebService sWebSvr ; 
	//CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	//sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sGames, &sSvrRes) ;
	//_tstring sRes = sSvrRes.sResult;
	//if(sRes.Find(_T("success")) != _tstring::npos)
	//{
	//	return true;
	//}
	//return false;
	AsynWebServiceHelper::Request req;
	req.data.sUrl = sUrl;
	req.data.sCookie = sCookie;
	req.data.sRequestData = sGames;
	return AsynWebServiceHelper::getInstance().SendRequest(AsynWebServiceHelper::HTTP_POST, req);
}

bool CWebData::UpdateCloudGame(const _tstring& sSessionId, _tstring sGames)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CLOUD_GAME"));
	sUrl += _T("update-cloud");

	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	//CWebService sWebSvr ; 
	//CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	//sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sGames, &sSvrRes) ;
	//_tstring sRes = sSvrRes.sResult;
	//if(sRes.Find(_T("success")) != _tstring::npos)
	//{
	//	return true;
	//}
	//return false;
	AsynWebServiceHelper::Request req;
	req.data.sUrl = sUrl;
	req.data.sCookie = sCookie;
	req.data.sRequestData = sGames;
	return AsynWebServiceHelper::getInstance().SendRequest(AsynWebServiceHelper::HTTP_POST, req);
}

bool CWebData::DeleteCloudGame(const _tstring& sSessionId, _tstring sGames)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CLOUD_GAME"));
	sUrl += _T("delete-from-cloud");

	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	//CWebService sWebSvr ; 
	//CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	//sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sGames, &sSvrRes) ;
	//_tstring sRes = sSvrRes.sResult;
	//if(sRes.Find(_T("success")) != _tstring::npos)
	//{
	//	return true;
	//}
	//return false;
	AsynWebServiceHelper::Request req;
	req.data.sUrl = sUrl;
	req.data.sCookie = sCookie;
	req.data.sRequestData = sGames;
	return AsynWebServiceHelper::getInstance().SendRequest(AsynWebServiceHelper::HTTP_POST, req);
}

bool CWebData::GetAllCloudGames(const _tstring& sSessionId, _tstring& sGames)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CLOUD_GAME"));
	sUrl += _T("get-cloud");

	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ; 
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.GetFieldService(sUrl, _tstring(sCookie.c_str()), _T(""), &sSvrRes);
	_tstring sRes = sSvrRes.sResult;
	if(sRes.Find(_T("success")) != _tstring::npos)
	{
		sGames = sRes;
		return true;
	}
	return false;
}

bool CWebData::IsGameDeletable(const _tstring& sSessionId, _tstring sGames)
{
	_tstring sUrl = theUIString._GetStringFromId(_T("IDS_CLOUD_GAME"));
	sUrl += _T("check-deletable");

	_tstringA sCookie;
	sCookie.Format(1, "PWRD=%s", sSessionId.toUTF8().c_str());

	CWebService sWebSvr ; 
	CWebService::RESULT_DATA sSvrRes = {0, "", "", NULL} ;
	sWebSvr.PostFieldService(sUrl, _tstring(sCookie.c_str()), sGames, &sSvrRes) ;
	_tstring sRes = sSvrRes.sResult;
	if(sRes.Find(_T("<deletable>0</deletable>")) != _tstring::npos)
	{
		return false;
	}
	return true;
}


