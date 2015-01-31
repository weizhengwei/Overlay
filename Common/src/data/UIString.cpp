#include <windows.h>
#include <tchar.h>
#include "data/UIString.h"
#include "data/CSVToolInterface.h"

CUIString         theUIString;

CUIString::CUIString()
{
	m_nLangType = 0 ;///English

	return;
}

CUIString::~CUIString( )
{
	return ;
}

bool CUIString::SetFilePath(LPCTSTR lpszStr)
{
	if(_tcslen(lpszStr)<=0)
	{
		return false;
	}

	m_tsFilePath=lpszStr;
	return true;
}

BOOL  CUIString::ReadLocaleString(LPCTSTR lpszFilePath, int nLangType)
{
	if (lpszFilePath == NULL)
	{
		return FALSE;
	}

	ICSVTool* tool = create_csv_tool();
	if (!tool->load(lpszFilePath))
	{
		release_csv_tool(tool);
		return FALSE;
	}

	int rowCount = tool->get_row_count();
	for (int i=0; i<rowCount; ++i)
	{
		ICSVRow* row = tool->get_row_by_index(i);

		_tstring szID = row->get_colum_string(0);
		
		int columCount = row->get_size();
		if(nLangType + 1 >= columCount)
		{
			m_mapStringTable[szID] = _T("");
		}
		else
		{
			_tstring szText = row->get_colum_string(nLangType + 1);
			if (szText.length() > 0 && szText[0] == EscapeChar)
			{
				szText = szText.Right(szText.size() - 1);
			}
			m_mapStringTable[szID] = szText;
		}
	}
	
	release_csv_tool(tool);

	return TRUE;	
}

_tstring CUIString::_GetStringFromId( LPCTSTR  lpszStr)
{
	_tstring siInfo = m_mapStringTable[lpszStr] ;

	return siInfo;
}