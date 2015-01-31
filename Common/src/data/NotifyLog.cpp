#include <tchar.h>
#include <gloox/md5.h>
#include "data/NotifyLog.h"
#include "data/utility.h"

using namespace gloox;

CAlertLog::CAlertLog(_tstring sBaseDir)
{
	m_sBaseDir = sBaseDir ;
}

CAlertLog::~CAlertLog(void)
{
}

void CAlertLog::SetUserJid(LPCTSTR	 lpszJid)
{
	_tstring szFolder,szFileName;
	MD5  md5;
	m_szJid=lpszJid;
	md5.feed(m_szJid.toNarrowString( ).c_str( ) );
	md5.finalize( );
	szFileName.fromUTF8(md5.hex().c_str());
	szFolder.Format(1,_T("%slog"),m_sBaseDir.c_str());
	m_szFilePath.Format(2,_T("%stmp\\%s_notify.xml"),m_sBaseDir.c_str( ),szFileName.c_str() );
	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(m_szFilePath.c_str( )))
	{
//		CUtility::MakeDir(szFolder.c_str( ));
	}
	return;
}

BOOL  CAlertLog::InitialLog( )
{
	FILE                * fp=NULL;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pElement=NULL;
	TiXmlDeclaration xmlDec("1.0","UTF-8","no");

	if(INVALID_FILE_ATTRIBUTES!=GetFileAttributes(m_szFilePath.c_str( )))
	{
		return true;;
	}
	fp=_tfopen(m_szFilePath.c_str() ,_T("wb+"));
	if(!fp)
	{
		return false;
	}

	xmlDoc.InsertEndChild(xmlDec);

	pRoot=new TiXmlElement("root");
//	xmlDoc.InsertEndChild(*pRoot);
	xmlDoc.LinkEndChild(pRoot);
	xmlDoc.SaveFile(fp);
	fflush(fp);
	fclose(fp);
	return true;
}

BOOL CAlertLog::AppendLog(ALERT_ITEM sData)
{
	FILE           *fp=NULL;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pElement=NULL;
	TiXmlDeclaration xmlDec("1.0","UTF-8","no");

	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(m_szFilePath.c_str( )))
	{
		this->InitialLog( );
	}

	fp=_tfopen(m_szFilePath.c_str() ,_T("rb"));
	if(!fp)
	{
		return false;
	}
	if(xmlDoc.LoadFile(fp) == false)
	{	
		fclose(fp);
		return FALSE;
	}

	fclose(fp);
	fp=_tfopen(m_szFilePath.c_str() ,_T("wb+"));
	if(!fp)
	{
		return false;
	}

	pRoot=xmlDoc.RootElement( );

	pElement=new TiXmlElement( "item" );
	pElement->SetAttribute("type", sData.m_sType.toUTF8().c_str());
	pElement->SetAttribute("id", sData.m_sId.toUTF8().c_str()) ;
	pElement->SetAttribute("title", sData.m_sTitle.toUTF8().c_str());
	pElement->SetAttribute("time", sData.m_sTimeStamp.toUTF8().c_str());
	pElement->SetAttribute("content", sData.m_sContent.toUTF8().c_str());
	pElement->SetAttribute("GameList", sData.m_sGameList.toUTF8().c_str());
	pElement->SetAttribute("GameAbbr", sData.m_sGameAbbr.toUTF8().c_str());
	pElement->SetAttribute("GameName", sData.m_sGameName.toUTF8().c_str());
	pElement->SetAttribute("GameLang", sData.m_sGameLang.toUTF8().c_str());
	pElement->SetAttribute("GameLogoPath", sData.m_sGameLogoPath.toUTF8().c_str());
	pElement->SetAttribute("showcmd", sData.m_nShowCmd);
	pElement->SetAttribute("link", sData.m_sLink.toUTF8().c_str());

	pRoot->LinkEndChild(pElement);

	xmlDoc.SaveFile(fp);
	fflush(fp);
	fclose(fp);

	return true;
}


BOOL CAlertLog::AppendLog(HIDE_ITEM sData)
{
	FILE           *fp=NULL;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pElement=NULL;
	TiXmlDeclaration xmlDec("1.0","UTF-8","no");

	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(m_szFilePath.c_str( )))
	{
		this->InitialLog( );
	}

	fp=_tfopen(m_szFilePath.c_str() ,_T("rb"));
	if(!fp)
	{
		return false;
	}
	if(xmlDoc.LoadFile(fp) == false)
	{
		fclose(fp);
		return FALSE;
	}

	fclose(fp);
	fp=_tfopen(m_szFilePath.c_str() ,_T("wb+"));
	if(!fp)
	{
		return false;
	}

	pRoot=xmlDoc.RootElement( );

	pElement=new TiXmlElement( "item" );
	pElement->SetAttribute("HideId", sData.m_sId.toUTF8().c_str());
	pElement->SetAttribute("HideType",sData.m_sType.toUTF8().c_str());

	pRoot->LinkEndChild(pElement);

	xmlDoc.SaveFile(fp);
	fflush(fp);
	fclose(fp);

	return true;
}

std::vector<ALERT_ITEM> CAlertLog::ReadAlertItem()
{
	FILE                *fp=NULL;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pElement=NULL;
	TiXmlDeclaration xmlDec("1.0","UTF-8","no");

	_tstring   szMsg;

	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(m_szFilePath.c_str( )))
	{
		return m_vecAlertItem;
	}

	fp=_tfopen(m_szFilePath.c_str() ,_T("rb"));
	if(!fp)
	{
		return m_vecAlertItem;
	}
	if(xmlDoc.LoadFile(fp) == false)
	{
		return m_vecAlertItem;
	}

	pRoot=xmlDoc.RootElement( );
	if(!pRoot)
	{
		fclose(fp);
		return m_vecAlertItem;
	}

	pElement=pRoot->FirstChildElement( ); 

	while(pElement)
	{
		ALERT_ITEM  msgElement;
		msgElement.m_sType.fromUTF8(pElement->Attribute("type"));
		msgElement.m_sId.fromUTF8(pElement->Attribute("id"));
		msgElement.m_sTitle.fromUTF8(pElement->Attribute("title"));
		msgElement.m_sContent.fromUTF8(pElement->Attribute("content"));
		msgElement.m_sLink.fromUTF8(pElement->Attribute("link"));
		msgElement.m_sGameList.fromUTF8(pElement->Attribute("GameList"));
		msgElement.m_sGameAbbr.fromUTF8(pElement->Attribute("GameAbbr"));
		msgElement.m_sGameName.fromUTF8(pElement->Attribute("GameName"));
		msgElement.m_sGameLang.fromUTF8(pElement->Attribute("GameLang"));
		msgElement.m_sGameLogoPath.fromUTF8(pElement->Attribute("GameLogoPath"));
		msgElement.m_sTimeStamp.fromUTF8(pElement->Attribute("time"));
		if(NULL!=pElement->Attribute("showcmd"))
			msgElement.m_nShowCmd=atoi(pElement->Attribute("showcmd") );
		else
			msgElement.m_nShowCmd=0;

		pElement=pElement->NextSiblingElement( );
		//If the game name is not empty and the game language is empty, we just ignore it
		//This can happen when the user has some old game download complete notification
		if (!msgElement.m_sGameName.empty() && msgElement.m_sGameLang.empty())
		{
			continue;
		}
		m_vecAlertItem.push_back(msgElement);
		
	}
	xmlDoc.Clear( );
	fclose(fp);

	return m_vecAlertItem;
}

std::vector<HIDE_ITEM> CAlertLog::ReadHideListLog()
{
	FILE                *fp=NULL;
	TiXmlDocument  xmlDoc;
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pElement=NULL;
	TiXmlDeclaration xmlDec("1.0","UTF-8","no");

	_tstring   szMsg;

	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(m_szFilePath.c_str( )))
	{
		return m_vecHideList;
	}

	fp=_tfopen(m_szFilePath.c_str() ,_T("rb"));
	if(!fp)
	{
		return m_vecHideList;
	}

	if(xmlDoc.LoadFile(fp) == false)
	{
		return m_vecHideList;
	}

	pRoot=xmlDoc.RootElement( );
	if(!pRoot)
	{
		fclose(fp);
		return m_vecHideList;
	}

	pElement=pRoot->FirstChildElement( ); 

	while(pElement)
	{
		HIDE_ITEM  msgElement;
		msgElement.m_sId.fromUTF8(pElement->Attribute("HideId"));
		msgElement.m_sType.fromUTF8(pElement->Attribute("HideType"));
		m_vecHideList.push_back(msgElement);
		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);

	return m_vecHideList;
}

void CAlertLog::RemoveLog()
{
	DeleteFile(m_szFilePath.c_str());
}

