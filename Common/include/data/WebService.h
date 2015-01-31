/**
 * @file      WebService.h
   @brief     The header file for web service.
   @author    shenhui@pwrd.com
   @data      2011/09/16
   @version   01.00.00
 */

/* 
 *  Copyright(C) 2010. Perfect World Entertainment Inc
 *
 *  This PROPRIETARY SOURCE CODE is the valuable property of PWE and 
 *  its licensors and is protected by copyright and other intellectual 
 *  property laws and treaties.  We and our licensors own all rights, 
 *  title and interest in and to the SOURCE CODE, including all copyright 
 *  and other intellectual property rights in the SOURCE CODE.
 */

/*
 * $LastChangedBy: shenhui@pwrd.com
 * $LastChangedDate: 
 * $Revision: 01.00.01
 * $Id: 
 * $notes:
 */

#pragma once

#include <tstring/tstring.h>
#include <map>

#ifdef _DEBUG
#pragma comment(lib,"libcurld.lib")
#else
#pragma comment(lib,"libcurl.lib")
#endif

#pragma comment(lib,"wldap32.lib")

using namespace std ;
using namespace String ;

class CWebService{

public:
	typedef struct _POST_DATA{

		int nFieldType ;      //type of field to post
		_tstring sFieldValue ;//data value of field to post

	}POST_DATA, *PPOST_DATA ;

	typedef struct _RESULT_DATA{
		
		int nResFormat ; //0: string text, 1: file
		string sResult ; // string to store the result
		_tstring sFile ; // file path name to store the result
		FILE* pStream  ; // file stream pointer

	}RESULT_DATA, *PRESULT_DATA ;

public:
	CWebService() ;
	virtual ~CWebService() ;

	static bool Init() ;
	static bool Uninit() ;

	bool PostFieldService(_tstring sUrl, _tstring sCookie, _tstring sPostData, PRESULT_DATA sResult, UINT nTimeout = 0) ;
	bool PostFormService(_tstring sUrl, _tstring sCookie, const std::map<_tstring, POST_DATA>& mapFieldValue, UINT nTimeout = 5) ;
	bool GetFieldService(_tstring sUrl, _tstring sCookie, _tstring sGetData, PRESULT_DATA sResult, UINT nTimeout = 5) ;
	bool HttpsGetService(_tstring sUrl, _tstring sCookie, _tstring sGetData, PRESULT_DATA sResult) ;
	bool GetFileLengthService(_tstring sUrl, unsigned long& nLen, int nTimeout = 0) ;
    bool HttpsPostService(_tstring sUrl, _tstring sCookie, _tstring sPostData, PRESULT_DATA sResult);
protected:

	/* Timeout unit in second */
	bool FieldService(bool bPost, _tstring sUrl, _tstring sCookie, _tstring sData, PRESULT_DATA sResult, UINT nTimeout = 0) ;
	bool HttpsRequest(const TCHAR* pszUrl, BYTE* pszResult, DWORD dwResultSize) ;

public:
	static size_t Callback_Func(void *pBuffer, size_t nSize, size_t nMemb, VOID* pParam) ;

	static std::string miniURLEncode(const std::string& s);
	static std::string miniURLEncodeComponent(const std::string& s);
};