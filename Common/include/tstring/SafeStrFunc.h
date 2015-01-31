#pragma once

#include "typedef.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>

namespace String
{


#define SNPRINTF /*Share::*/String::shared_snprintf
#define STRNCPY /*Share::*/String::shared_strncpy

int shared_snprintf(char* pszDst, uint32 uDstLength, const char* pszFormat, ...);
int shared_snprintf(wchar_t* pszwDst, uint32 uDstLength, const wchar_t* pszwFormat, ...);

char* shared_strncpy(char* pszDst, const char* pszSrc, uint32 uDstLength);
wchar_t* shared_strncpy(wchar_t* pszwDst, const wchar_t* pszwSrc, uint32 uDstLength);

inline void _StrSafeCopy(char* pszDest, const char* pszSrc, size_t nLen) throw()
{
	if(NULL == pszSrc)
	{
		pszDest[0] = '\0';
		return;
	}

	lstrcpynA(pszDest, pszSrc, (INT32)nLen);
}

template <class T>
inline void StrSafeCopy(T& Dest, const char* pszSrc) throw()
{
	(static_cast<char[sizeof(Dest)]>(Dest));

	_StrSafeCopy(Dest, pszSrc, sizeof(Dest));
}

//NAMESPACEEND(String)
//NAMESPACEEND(Share)
};