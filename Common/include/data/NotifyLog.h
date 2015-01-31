#pragma once

#include <tinyxml/tinystr.h>
#include <tinyxml/tinyxml.h>
#include <tstring/tstring.h>
#include <vector>

#include "datatype.h"

using namespace String;

class CAlertLog
{
public:
	CAlertLog(_tstring sBaseDir);
	~CAlertLog(void);

	typedef struct _NOTIFY_LOG_ELEMENT
	{
		_tstring _szTitle;
		_tstring _szContent;
	}NOTIFY_LOG_ELEMENT;

public:
	void SetUserJid(LPCTSTR lpszJid);
	std::vector<ALERT_ITEM> ReadAlertItem();
	std::vector<HIDE_ITEM> ReadHideListLog();
	BOOL InitialLog( );
	BOOL AppendLog(ALERT_ITEM);
	BOOL AppendLog(HIDE_ITEM);
	void RemoveLog( );
private:
	_tstring m_szJid;
	_tstring m_szFilePath;
	std::vector<ALERT_ITEM>        m_vecAlertItem;
	std::vector<HIDE_ITEM>         m_vecHideList;
	_tstring m_sBaseDir ;
};
