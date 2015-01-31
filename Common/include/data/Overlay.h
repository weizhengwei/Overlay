#pragma once 

#include <windows.h>

enum D3D_VERSION{
	NONE   = -1 ,
	D3D8   = 0 ,
	D3D9   = 1 ,
	D3D10  = 2 ,
	D3D11  = 3 ,
	OPENGL = 4
};	

class COverlay
{
public:
	static bool DisplayNotification(LPCTSTR pszLink, LPCTSTR lpszId, \
		LPCTSTR pszTitle, LPCTSTR pszText, RECT rect) ;

	static bool DestroyNotification() ;

	static D3D_VERSION GetD3DVersion(LPCTSTR lpszProcessPathName) ;
} ;