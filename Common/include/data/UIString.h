#pragma once

//#include "stdafx.h"
#include <map>
#include "tstring/tstring.h"

using namespace std;
using namespace String;

class CUIString
{
public:
	CUIString( );
	~CUIString( );
	bool   SetFilePath(LPCTSTR lpszStr);
	
	_tstring _GetStringFromId( LPCTSTR  lpszStr);
	BOOL  ReadLocaleString(LPCTSTR lpszFilePath, int nLangType = 0);
	BOOL  ReadLocaleString( )
	{
		return ReadLocaleString(m_tsFilePath.c_str( ), m_nLangType);
	}
	
	static const wchar_t EscapeChar = L'#';
private:
	_tstring  m_tsFilePath;
	int m_nLangType ;

	map<_tstring,_tstring>  m_mapStringTable ;
};

extern CUIString         theUIString;