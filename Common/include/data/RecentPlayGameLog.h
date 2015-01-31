#pragma once

#include <tinyxml/tinystr.h>
#include <tinyxml/tinyxml.h>
#include <tstring/tstring.h>
#include <vector>

using namespace String;

class CRecentPlayGameLog
{
public:
	CRecentPlayGameLog();
	~CRecentPlayGameLog();
	typedef struct _GAMEDES
	{
		_tstring GameName;
		_tstring GameAbbr;
		_tstring GameLang;
		_tstring GameIconPath;
	}GAMEDES, *PGAMEDES;

public:
	std::vector<GAMEDES> ReadItems();
	BOOL InitialLog( );
	BOOL AppendLog(GAMEDES& stGameDes);
	BOOL RemoveLog(_tstring sGameAbbr, _tstring sGameLang);
	BOOL RemoveLog(_tstring sArgument);
private:
	void SetUserJid(LPCTSTR lpszJid);
	TiXmlDocument* m_pXmlDoc;
	_tstring m_szJid;
	_tstring m_szFilePath;
	std::vector<GAMEDES> m_vecRecentPlayGame;
	_tstring m_sBaseDir ;
};

typedef CRecentPlayGameLog::GAMEDES	GAMESHELLLNK;

extern CRecentPlayGameLog recentPlayGameLog;