#pragma once
#include <tchar.h>
#include <string.h>
#include "tinyxml/tinystr.h"
#include "tinyxml/tinyxml.h"
#include <tstring/tstring.h>
#include <vector>
#include <map>

#include <gdiplus.h>
using namespace Gdiplus;

using namespace String;
using namespace std;

typedef struct _BTN_INFO
{
	_tstring   _tsName;
	_tstring   _tsFile;
	RECT    _rect;
	_tstring   _tsTips;
	_tstring   _tsHref;
}BTN_INFO;

typedef struct _BG_INFO
{
	_tstring  _tsName;
	_tstring  _tsFile;
	RECT      _rect ;
}BG_INFO;

typedef struct  _TRAY_INFO
{
	_tstring _tsName;
	_tstring _tsFile;
	_tstring _tsTips;
}TRAY_INFO;

typedef struct _ROSTER_IMAGE_INFO
{
	UINT    uStatus;
	_tstring  _tsFilePath;
}ROSTER_IMAGE_INFO;

typedef struct _THEMES_XML_INFO
{
	UINT    _uType;
	_tstring  _tsName;
	_tstring  _tsFilePath;
}THEMES_XML_INFO;

typedef struct _THEMES_INFO
{
	_tstring   _tsName;
	_tstring   _tsFolder;
}THEMES_INFO;

typedef struct _THEMES_SKIN_FILE
{
	_tstring  _tsID;
	_tstring  _tsPath;
	RECT    _rect;
	_tstring   _tsTips;
	_tstring   _tsHref;
}THEMES_SKIN_FILE;

typedef struct _SKIN_RES_ITEM
{
	_tstring  _tsID;
	_tstring  _tsPath;
	RECT    _rect;
	_tstring   _tsTips;
	_tstring   _tsHref;
	Bitmap   *pImage;
}SKIN_RES_ITEM;

class CUIConfig
{
public:
	CUIConfig( void  );
	~CUIConfig( void );

	bool   Init(_tstring sBaseDir, _tstring sUIConfigPath) ;

	bool   SetFilePath( LPCTSTR lpszStr );

	bool   ReadBtnInfo(LPCTSTR lpszStr );
	bool   ReadBtnInfo( )
	{
		return ReadBtnInfo(m_tsFilePath.c_str( ));
	}
	vector <BTN_INFO> ExportBtnInfo( )
	{
		return m_vecBtnInfo;
	}

	bool   ReadBgInfo(LPCTSTR lpszStr);
	bool   ReadBgInfo( )
	{
		return ReadBgInfo(m_tsFilePath.c_str( ));
	}
    vector <BG_INFO> ExportBgInfo( )
	{
		return m_vecBgInfo;
	}

	bool ReadTrayInfo(LPCTSTR lpszStr);
	bool ReadTrayInfo( )
	{
		return ReadTrayInfo( m_tsFilePath.c_str( ));
	}
	vector <TRAY_INFO> ExportTrayInfo( )
	{
		return m_vecTrayInfo;
	}

	bool ReadRosterImageInfo(LPCTSTR lpszStr);
	bool ReadRosterImageInfo( )
	{
		return ReadRosterImageInfo( m_tsFilePath.c_str( ));
	}

	vector <ROSTER_IMAGE_INFO> ExportRosterImageInfo( )
	{
		return m_vecRosterImageInfo;
	}

	bool ReadThemesXml(LPCTSTR lpszStr);
	bool ReadThemesXml( )
	{
		return ReadThemesXml(m_tsFilePath.c_str( ) );
	}

	vector <THEMES_XML_INFO> ExportThemeXml( )
	{
		return m_vecThemesXml;
	}

	bool ReadThemesInfo(  );
	vector<THEMES_INFO> ExportThemesInfo( )
	{
		return m_vecThemesInfo;
	}

	bool ReadThemeSkin(   );
	vector <THEMES_SKIN_FILE> ExportThemeSkin( )
	{
		return m_vecThemeItem;
	}

	bool  LoadSkinResource(int nIndex); //this function should be dropped in the future,
	                                    //it's better to load image file when using it,
	                                    //otherwise, we cannot overwrite image file when updating cc.
	SKIN_RES_ITEM* GetSkinResItem(LPCTSTR lpszItemId);
	void  ResetSkinResource( );

	void SetBaseDir(_tstring sBaseDir) ;

private:
	_tstring                         m_tsFilePath;
	vector <BTN_INFO>  m_vecBtnInfo;
	vector <BG_INFO>   m_vecBgInfo;
	vector <TRAY_INFO> m_vecTrayInfo;
	vector <ROSTER_IMAGE_INFO>  m_vecRosterImageInfo;

	vector <THEMES_XML_INFO> m_vecThemesXml;
 	vector <THEMES_INFO>            m_vecThemesInfo;
	vector <THEMES_SKIN_FILE>  m_vecThemeItem;

	std::map<_tstring,SKIN_RES_ITEM>  m_mapSkin;

	_tstring m_sBaseDir ;
};

extern CUIConfig         theUIConfig;