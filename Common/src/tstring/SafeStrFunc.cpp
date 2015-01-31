
#include "tstring/SafeStrFunc.h"

//NAMESPACEBEGIN(Share)
//NAMESPACEBEGIN(String)
namespace String
{

	#pragma warning(push)
	#pragma warning(disable : 4996)
	int shared_snprintf(char* pszDst, uint32 uDstLength, const char* pszFormat, ...)
	{
		va_list args;
		va_start(args, pszFormat);
		int nRet = _vsnprintf_s(pszDst, _TRUNCATE, uDstLength-1, pszFormat, args);
		va_end(args);

		return nRet;
	}
	int shared_snprintf(wchar_t* pszwDst, uint32 uDstLength, const wchar_t* pszwFormat, ...)
	{
		va_list args;
		va_start(args, pszwFormat);
		int nRet = _vsnwprintf_s(pszwDst, _TRUNCATE, uDstLength-1, pszwFormat, args);
		va_end(args);

		return nRet;
	}

	char* shared_strncpy(char* pszDst, const char* pszSrc, uint32 uDstLength)
	{
		return strncpy(pszDst, pszSrc, uDstLength-1);
	}

	wchar_t* shared_strncpy(wchar_t* pszwDst, const wchar_t* pszwSrc, uint32 uDstLength)
	{
		return wcsncpy(pszwDst, pszwSrc, uDstLength-1);
	}

	#pragma warning(pop)

	//NAMESPACEEND(String)
	//NAMESPACEEND(Share)
}