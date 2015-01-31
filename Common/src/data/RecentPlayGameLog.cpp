#include <tchar.h>
#include <gloox/md5.h>
#include "data/RecentPlayGameLog.h"
#include "data/utility.h"
#include "data/DataPool.h"

using namespace gloox;

CRecentPlayGameLog recentPlayGameLog;

CRecentPlayGameLog::CRecentPlayGameLog()
:m_pXmlDoc(NULL),
m_sBaseDir(theDataPool.GetBaseDir())
{
	SetUserJid(theDataPool.GetUserName().c_str());
	InitialLog();
}

CRecentPlayGameLog::~CRecentPlayGameLog(void)
{
	delete m_pXmlDoc;
}

void CRecentPlayGameLog::SetUserJid(LPCTSTR	 lpszJid)
{
	_tstring szFileName;
	MD5  md5;
	m_szJid=lpszJid;
	md5.feed(m_szJid.toNarrowString( ).c_str( ) );
	md5.finalize( );
	szFileName.fromUTF8(md5.hex().c_str());
	m_szFilePath.Format(2,_T("%stmp\\%s_RecentPlayGame.xml"),m_sBaseDir.c_str( ),szFileName.c_str() );
	if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(m_szFilePath.c_str( )))
	{
		//		CUtility::MakeDir(szFolder.c_str( ));
	}
	return;
}

BOOL  CRecentPlayGameLog::InitialLog( )
{
	if (m_pXmlDoc)
	{
		return TRUE;
	}
	TiXmlElement   *pRoot=NULL;
	TiXmlDeclaration xmlDec("1.0","UTF-8","no");
	BOOL bRet = FALSE;
	if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(m_szFilePath.c_str()))
	{
		m_pXmlDoc = new TiXmlDocument;
		if (m_pXmlDoc)
		{
			return m_pXmlDoc->LoadFile(m_szFilePath.toNarrowString().c_str());
		}
		
	}

	m_pXmlDoc = new TiXmlDocument(m_szFilePath.toNarrowString().c_str());

	if (!m_pXmlDoc)
	{
		bRet = FALSE;
		goto END;
	}

	pRoot=new TiXmlElement("root");
	if(!pRoot)
	{
		bRet = FALSE;
		goto END;
	}

	m_pXmlDoc->LinkEndChild(pRoot);
	m_pXmlDoc->SaveFile();
	bRet = TRUE;
END:
	return bRet;
}

BOOL CRecentPlayGameLog::AppendLog(GAMEDES& stGameDes)
{
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pCurrElement = NULL;
	TiXmlElement   *pNewElement = NULL;
	BOOL bRet = FALSE;
	if (!m_pXmlDoc)
	{
		if (!InitialLog())
		{
			return FALSE;
		}
	}

	pRoot= m_pXmlDoc->RootElement( );
	if (!pRoot)
	{
		bRet = FALSE;
		goto END;
	}

	pCurrElement = pRoot->FirstChildElement();
	while(pCurrElement)
	{
		_tstring sCurrGameAbbr = pCurrElement->Attribute("GameAbbr");
		if (!sCurrGameAbbr.CompareNoCase(stGameDes.GameAbbr))
		{
			//the item has been in the log
			if(pRoot->RemoveChild(pCurrElement))
			{
				break;
			}
			else
			{
				bRet = FALSE;
				goto END;
			}
			
		}
		pCurrElement = pCurrElement->NextSiblingElement();
	}

	//Add the element to the end
	pNewElement=new TiXmlElement( "item" );
	if (pNewElement)
	{
		pNewElement->SetAttribute("GameAbbr", stGameDes.GameAbbr.toNarrowString().c_str());
		pNewElement->SetAttribute("GameName", stGameDes.GameName.toNarrowString().c_str());
		pNewElement->SetAttribute("GameLang", stGameDes.GameLang.toNarrowString().c_str());
		pNewElement->SetAttribute("GameIcon", stGameDes.GameIconPath.toNarrowString().c_str());
		TiXmlNode *pFirstChild = pRoot->FirstChild();
		if (pFirstChild)
		{
			bRet = pRoot->InsertBeforeChild(pFirstChild, *pNewElement) == NULL ? FALSE : TRUE;
		}
		else
		{
			bRet = pRoot->InsertEndChild(*pNewElement) == NULL ? FALSE : TRUE;
		}
		
		if (bRet)
		{
			m_pXmlDoc->SaveFile();
		}
	}
END:
	delete pNewElement;
	return bRet;
}

std::vector<GAMESHELLLNK> CRecentPlayGameLog::ReadItems()
{
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pElement=NULL;
	if (!m_pXmlDoc)
	{
		if (!InitialLog())
		{
			return m_vecRecentPlayGame;
		}
	}

	pRoot=m_pXmlDoc->RootElement();
	if(!pRoot)
	{
		return m_vecRecentPlayGame;
	}

	pElement=pRoot->FirstChildElement( ); 
	GAMEDES  stRecentPlayGame;
	m_vecRecentPlayGame.clear();
	while(pElement)
	{
		
		stRecentPlayGame.GameAbbr.fromUTF8(pElement->Attribute("GameAbbr"));
		stRecentPlayGame.GameName.fromUTF8(pElement->Attribute("GameName"));
		stRecentPlayGame.GameLang.fromUTF8(pElement->Attribute("GameLang"));
		stRecentPlayGame.GameIconPath.fromUTF8(pElement->Attribute("GameIcon"));
		m_vecRecentPlayGame.push_back(stRecentPlayGame);
		pElement=pElement->NextSiblingElement();
	}

	return m_vecRecentPlayGame;
}

BOOL CRecentPlayGameLog::RemoveLog(_tstring sGameAbbr, _tstring sGameLang)
{
	TiXmlElement   *pRoot=NULL;
	TiXmlElement   *pCurrElement = NULL;
	BOOL bRet = FALSE;
	if (!m_pXmlDoc)
	{
		if (!InitialLog())
		{
			return FALSE;
		}
	}

	pRoot=m_pXmlDoc->RootElement( );
	if (!pRoot)
	{
		return FALSE;
	}
	pCurrElement = pRoot->FirstChildElement();
	while(pCurrElement)
	{
		_tstring sCurrGameAbbr = pCurrElement->Attribute("GameAbbr");
		_tstring sCurrGameLang = pCurrElement->Attribute("GameLang");
		if (!sCurrGameAbbr.CompareNoCase(sGameAbbr) && !sCurrGameLang.CompareNoCase(sGameLang))
		{
			pRoot->RemoveChild(pCurrElement);
			bRet = TRUE;
			break;
			
		}
		pCurrElement = pCurrElement->NextSiblingElement();
	}
	m_pXmlDoc->SaveFile();
	return bRet;
}

BOOL CRecentPlayGameLog::RemoveLog(_tstring sArgument)
{
	_tstring sGameAbbr = sArgument.Left(sArgument.Find(_T(":")));
	_tstring sGameLang = sArgument.Right(sArgument.length() - sArgument.Find(_T(":")) - 1);
	if (!sGameAbbr.empty() && !sGameLang.empty())
	{
		return RemoveLog(sGameAbbr, sGameLang);
	}
	return FALSE;
}