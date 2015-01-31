#include "data/UIConfig.h"

CUIConfig         theUIConfig;

CUIConfig::CUIConfig( void )
{
	return ;
}

CUIConfig::~CUIConfig( )
{
	return;
}

bool CUIConfig::Init(_tstring sBaseDir, _tstring sUIConfigPath)
{
	SetBaseDir(sBaseDir) ;
	SetFilePath(sUIConfigPath.c_str());
	ReadBtnInfo();
	ReadBgInfo();
	ReadTrayInfo();
	ReadRosterImageInfo();
	ReadThemesXml();
	ReadThemesInfo();
	ReadThemeSkin();

	return true ;
}

void CUIConfig::SetBaseDir(_tstring sBaseDir)
{
	m_sBaseDir = sBaseDir ;
}

bool CUIConfig::SetFilePath(LPCTSTR lpszStr)
{
	if(_tcslen(lpszStr)<=0)
		return false;
	m_tsFilePath=lpszStr;
	return true;
}


bool CUIConfig::ReadBtnInfo(LPCTSTR lpszStr)
{
	FILE   *fp=NULL;
	bool     bResult=false;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	if(_tcslen(lpszStr)<=0)
		return false;

	fp=_tfopen(lpszStr,_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("button");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	while(pElement)
	{
		const char            *szValue;
		BTN_INFO        btnInfo;

		btnInfo._tsName=pElement->Attribute("name");
		btnInfo._tsFile=pElement->Attribute("file");
		btnInfo._tsHref=pElement->Attribute("url");
		btnInfo._tsTips=pElement->Attribute("tip");
		szValue=pElement->Attribute("rect");
		sscanf(szValue,"%d,%d,%d,%d",&btnInfo._rect.left,&btnInfo._rect.top,&btnInfo._rect.right,&btnInfo._rect.bottom);
		m_vecBtnInfo.push_back(btnInfo);

		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);
	return true;	
}

bool   CUIConfig::ReadBgInfo(LPCTSTR lpszStr)
{
	FILE   *fp=NULL;
	bool     bResult=false;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	if(_tcslen(lpszStr)<=0)
		return false;

	fp=_tfopen(lpszStr,_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("bgsrc");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	while(pElement)
	{
		const char            *szValue;
		BG_INFO          bgInfo;

		bgInfo._tsName=pElement->Attribute("name");
		bgInfo._tsFile=pElement->Attribute("file");
		szValue=pElement->Attribute("rect");
		sscanf(szValue,"%d,%d,%d,%d",&bgInfo._rect.left,&bgInfo._rect.top,&bgInfo._rect.right,&bgInfo._rect.bottom);
		m_vecBgInfo.push_back(bgInfo);

		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);
	return true;
}


bool CUIConfig::ReadTrayInfo(LPCTSTR lpszStr)
{
	FILE   *fp=NULL;
	bool     bResult=false;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	if(_tcslen(lpszStr)<=0)
		return false;

	fp=_tfopen(lpszStr,_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("trayicon");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	while(pElement)
	{
		const char            *szValue;
		TRAY_INFO          trayInfo;

		trayInfo._tsName=pElement->Attribute("name");
		trayInfo._tsFile=pElement->Attribute("file");
		trayInfo._tsTips=pElement->Attribute("tip");
		m_vecTrayInfo.push_back(trayInfo);

		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);
	return true;
}

bool CUIConfig::ReadRosterImageInfo(LPCTSTR lpszStr)
{
	FILE   *fp=NULL;
	bool     bResult=false;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	if(_tcslen(lpszStr)<=0)
		return false;

	fp=_tfopen(lpszStr,_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("roster");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	while(pElement)
	{
		const char            *szValue;
		ROSTER_IMAGE_INFO         imageInfo;

		imageInfo.uStatus=atoi(pElement->Attribute("status"));
		imageInfo._tsFilePath=pElement->Attribute("file");
		m_vecRosterImageInfo.push_back(imageInfo);

		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);
	return true;
}

bool CUIConfig::ReadThemesXml(LPCTSTR lpszStr)
{
	FILE   *fp=NULL;
	bool     bResult=false;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	if(_tcslen(lpszStr)<=0)
		return false;

	fp=_tfopen(lpszStr,_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("themes");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	while(pElement)
	{
		const char            *szValue;
		THEMES_XML_INFO         themeXmlInfo;

		themeXmlInfo._uType=0;
		 themeXmlInfo._tsName=pElement->Attribute( "name");
		 themeXmlInfo._tsFilePath=pElement->Attribute("path");
		m_vecThemesXml.push_back(themeXmlInfo);
		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);

	return true;
}

bool CUIConfig::ReadThemesInfo(  )
{
	FILE   *fp=NULL;
	bool     bResult=false;
	_tstring szXmlPath;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	for(int i=0;i<m_vecThemesXml.size( );i++)
	{
		szXmlPath=m_sBaseDir;
		szXmlPath.append(m_vecThemesXml[i]._tsFilePath);

		if(szXmlPath.length( )<=0)
			return false;

		fp=_tfopen(szXmlPath.c_str( ),_T("rb"));
		if(!fp)
		{
			return false;
		}

		bResult=xmlDoc.LoadFile(fp);
		pElement=xmlDoc.RootElement( );
		if(!pElement)
		{
			return false;
		}

		pElement=pElement->FirstChildElement("theme");
		if(!pElement)
		{
			return false;
		}
		else
		{
			const char                    *szValue;
			THEMES_INFO         themeInfo;

			themeInfo._tsName=pElement->Attribute( "name");
			themeInfo._tsFolder=pElement->Attribute("path");
			
			m_vecThemesInfo.push_back(themeInfo);
		}
		xmlDoc.Clear( );
		fclose(fp);
	}
	return true;

}

bool CUIConfig::ReadThemeSkin(  )
{
	FILE   *fp=NULL;
	bool     bResult=false;
	_tstring szXmlPath;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	szXmlPath=m_sBaseDir;
	szXmlPath.append(m_vecThemesXml[0]._tsFilePath);

	if(szXmlPath.length( )<=0)
		return false;

	fp=_tfopen(szXmlPath.c_str( ),_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("themeitem");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	while(pElement)
	{
		const char            *szValue;
		THEMES_SKIN_FILE        themeItem;

		themeItem._tsID=pElement->Attribute( "id");
		themeItem._tsPath=pElement->Attribute("file");
		themeItem._tsHref=pElement->Attribute("url");
		themeItem._tsTips=pElement->Attribute("tip");
		szValue=pElement->Attribute("rect");
		sscanf(szValue,"%d,%d,%d,%d",&themeItem._rect.left,&themeItem._rect.top,&themeItem._rect.right,&themeItem._rect.bottom);
		m_vecThemeItem.push_back(themeItem);

		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);
	return true;
}

bool  CUIConfig::LoadSkinResource( int nIndex)
{
	FILE   *fp=NULL;
	bool     bResult=false;
	_tstring szXmlPath,szFolder,szFile;

	TiXmlDocument  xmlDoc;
	TiXmlElement     *pElement=NULL;	

	if(nIndex>=m_vecThemesXml.size( ))
		return false;

	szXmlPath=m_sBaseDir;
	szXmlPath.append(m_vecThemesXml[nIndex]._tsFilePath);

	szFolder=m_sBaseDir;
	szFolder+=m_vecThemesInfo[nIndex]._tsFolder;

	if(szXmlPath.length( )<=0)
		return false;

	fp=_tfopen(szXmlPath.c_str( ),_T("rb"));
	if(!fp)
	{
		return false;
	}

	bResult=xmlDoc.LoadFile(fp);
	pElement=xmlDoc.RootElement( );
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement("themeitem");
	if(!pElement)
	{
		return false;
	}

	pElement=pElement->FirstChildElement( );
	if(!pElement)
	{
		return false;
	}

	this->ResetSkinResource( );

	while(pElement)
	{
		const char            *szValue;
		SKIN_RES_ITEM       themeItem;
		themeItem.pImage=NULL;

		themeItem._tsID=pElement->Attribute( "id");
		themeItem._tsPath=pElement->Attribute("file");
		themeItem._tsHref=pElement->Attribute("url");
		themeItem._tsTips=pElement->Attribute("tip");
		szValue=pElement->Attribute("rect");
		sscanf(szValue,"%d,%d,%d,%d",&themeItem._rect.left,&themeItem._rect.top,&themeItem._rect.right,&themeItem._rect.bottom);

		szFile=szFolder+themeItem._tsPath;
		szFile.Replace('\\','/');
		if(INVALID_FILE_ATTRIBUTES!=GetFileAttributes(szFile.c_str( )) )
		{
			themeItem.pImage=Bitmap::FromFile(szFile.c_str( ));
		}
		m_mapSkin.insert(pair<_tstring,SKIN_RES_ITEM>(themeItem._tsID,themeItem));

		pElement=pElement->NextSiblingElement( );
	}
	xmlDoc.Clear( );
	fclose(fp);
	return true;

}

SKIN_RES_ITEM* CUIConfig::GetSkinResItem(LPCTSTR lpszItemId)
{
	map<_tstring,SKIN_RES_ITEM>::iterator  iter=m_mapSkin.begin( );
	iter=m_mapSkin.find(lpszItemId);
	if(m_mapSkin.end( )==iter)
		return NULL;
	else
		return &(iter->second);
}

void CUIConfig::ResetSkinResource( )
{
	map<_tstring,SKIN_RES_ITEM>::iterator  iter=m_mapSkin.begin( );
	for(;iter!=m_mapSkin.end();iter++)
	{
		if(NULL!=iter->second.pImage)
		{
			delete iter->second.pImage;
			iter->second.pImage=NULL;
		}
	}
	m_mapSkin.clear( );
	return;
}