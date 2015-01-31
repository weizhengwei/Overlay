/**
* @file      WebService.cpp
  @brief     The implementation file for web service.
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


#include <windows.h>
#include <Wininet.h>
#include <tchar.h>
#include "curl/curl.h"
#include "data/WebService.h"

#pragma comment(lib,"Wininet.lib")


CWebService::CWebService()
{
}
	
CWebService::~CWebService()
{
}

bool CWebService::Init()
{
	bool bRet = true ;

	CURLcode rc = curl_global_init(CURL_GLOBAL_ALL) ;
	if (rc != CURLE_OK)
	{
		bRet = false ;
	}

	return bRet ;
}

bool CWebService::Uninit()
{
	bool bRet = true ;

	curl_global_cleanup() ;

	return bRet ;
}

bool CWebService::PostFieldService(_tstring sUrl, _tstring sCookie, _tstring sPostData, PRESULT_DATA pResult, UINT nTimeout)
{
	return FieldService(true, sUrl, sCookie, sPostData, pResult, nTimeout) ;
}

bool CWebService::GetFieldService(_tstring sUrl, _tstring sCookie, _tstring sGetData, PRESULT_DATA pResult, UINT nTimeout)
{
	return FieldService(false, sUrl, sCookie, sGetData, pResult, nTimeout) ;
}

bool CWebService::PostFormService(_tstring sUrl, _tstring sCookie, const std::map<_tstring, POST_DATA>& mapFieldValue, UINT nTimeout)
{
	bool bRet = false ;
	CURL* curl = NULL ; 
	CURLcode rc = CURLE_OK ;
	struct curl_httppost *pHttpPost = NULL ;
	struct curl_httppost *pLastPost = NULL ;

	do {

		if (sUrl.empty() || mapFieldValue.empty())
		{
			break ;
		}

		curl = curl_easy_init();
		if (NULL == curl) 
		{
			break ;
		}

		std::map<_tstring,POST_DATA>::const_iterator iter = mapFieldValue.begin();
		for (;iter != mapFieldValue.end();++iter)
		{
			curl_formadd(&pHttpPost, &pLastPost, CURLFORM_COPYNAME, _tstring(iter->first).toUTF8().c_str(), \
				iter->second.nFieldType, _tstring(iter->second.sFieldValue).toUTF8().c_str(), CURLFORM_END) ;
		}

		if(nTimeout > 0)
		{
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);//disable signal here to guranttee thread-safety
			rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, nTimeout);
			if(rc != CURLE_OK)
			{
				break;
			}
		}

		rc = curl_easy_setopt(curl, CURLOPT_URL, sUrl.toNarrowString().c_str()) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		rc = curl_easy_setopt(curl, CURLOPT_HTTPPOST, pHttpPost) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		if (!sCookie.empty())
		{
			rc = curl_easy_setopt(curl,CURLOPT_COOKIE, sCookie.toUTF8().c_str()) ;
			if (rc != CURLE_OK)
			{
				break ;
			}
		}
		
		rc = curl_easy_perform(curl);
		if (rc != CURLE_OK) 
		{
			break ;
		}
		
		bRet = true ;

	}while(false) ;

	if (pHttpPost != NULL)
	{
		curl_formfree(pHttpPost) ;
		pHttpPost = NULL ;
	}

	if (curl != NULL)
	{
		curl_easy_cleanup(curl) ;
		curl = NULL ;
	}

	return bRet ;
}

bool CWebService::HttpsGetService(_tstring sUrl, _tstring sCookie, _tstring sGetData, PRESULT_DATA pResult)
{
	bool bRet = false ;
	_tstring sUrlRequest ;

	do {

		if (sUrl.empty())
		{
			break ;
		}

		sUrlRequest = miniURLEncode(sUrl.toNarrowString()); 
        sGetData =    miniURLEncode(sGetData.toNarrowString());
		sUrlRequest.append(sGetData.c_str());

		const int MAX_BUFFER_SIZE = 1024*8;
		BYTE szBuffer[MAX_BUFFER_SIZE] = {0} ;
		if (!HttpsRequest(sUrlRequest.c_str(), szBuffer, MAX_BUFFER_SIZE))
		{
			break ;
		}

		if (pResult != NULL)
		{
			pResult->sResult.clear() ;
			pResult->sResult.assign((char*)szBuffer) ;
		}

		bRet = true ;

	}while(false) ;

	return bRet ;
}

bool CWebService::GetFileLengthService(_tstring sUrl, unsigned long& nLen, int nTimeout)
{
	bool bRet = false ;
	CURL* curl= NULL ;
	CURLcode rc = CURLE_OK ;
	_tstring sUrlRequest = sUrl ;

	do {

		if (sUrl.empty())
		{
			break ;
		}

		curl = curl_easy_init() ;
		if (NULL == curl) 
		{
			break ;
		}

		rc = curl_easy_setopt(curl, CURLOPT_HEADER, 1) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		rc = curl_easy_setopt(curl, CURLOPT_NOBODY, 1) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		if (nTimeout > 0)
		{
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, nTimeout) ;
			if (rc != CURLE_OK)
			{
				break ;
			}
		}

		rc = curl_easy_setopt(curl, CURLOPT_URL, sUrlRequest.toNarrowString().c_str()) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		rc = curl_easy_perform(curl) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		double d = 0.0 ;
		rc = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d); 
		if (rc != CURLE_OK)
		{
			break ;
		}

		nLen = (unsigned long)d ;
		bRet = true ;

	}while(false) ;

	if (NULL != curl)
	{
		curl_easy_cleanup(curl) ;
		curl = NULL ;
	}

	return bRet ;
}

bool CWebService::FieldService(bool bPost, _tstring sUrl, _tstring sCookie, _tstring sData, PRESULT_DATA pResult, UINT nTimeout)
{
	bool bRet = false ;
	CURL* curl= NULL ;
	CURLcode rc = CURLE_OK ;
	_tstring sUrlRequest = miniURLEncode(sUrl.toNarrowString());
	std::string sPostData ;

	do {

		if (sUrl.empty())
		{
			break ;
		}

		curl = curl_easy_init() ;
		if (NULL == curl) 
		{
			break ;
		}

		if (bPost)
		{
			rc = curl_easy_setopt(curl, CURLOPT_POST, 1) ;
			if (rc != CURLE_OK)
			{
				break ;
			}
            //sData = miniURLEncode(sData.toNarrowString());
			sPostData = sData.toUTF8() ; //(don't know why)do not pass sData.toUTF8().c_str() 
										 //into curl_easy_setopt directly, define a std::string 
										 //outside the do...while..., then put the post data 
										 //into sPostData and pass it to the curl API

			rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sPostData.c_str()) ;
			if (rc != CURLE_OK)
			{
				break ;
			}
		}
		else if (!sData.empty())
		{
            sData = miniURLEncode(sData.toNarrowString());
			sUrlRequest.append(sData.c_str()) ;
		}

		if(nTimeout > 0)
		{
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, nTimeout);
			if(rc != CURLE_OK)
			{
				break;
			}
		}

		rc = curl_easy_setopt(curl, CURLOPT_URL, sUrlRequest.toNarrowString().c_str()) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		if (pResult != NULL)
		{
			rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CWebService::Callback_Func) ;
			if (rc != CURLE_OK)
			{
				break ;
			}

			rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, pResult) ; 
			if (rc != CURLE_OK)
			{
				break ;
			}
		}

		if (!sCookie.empty())
		{
			rc = curl_easy_setopt(curl,CURLOPT_COOKIE, sCookie.toUTF8().c_str()) ;
			if (rc != CURLE_OK)
			{
				break ;
			}
		}

		rc = curl_easy_perform(curl) ;
		if (rc != CURLE_OK)
		{
			break ;
		}

		bRet = true ;

	}while(false) ;

	if (NULL != curl)
	{
		curl_easy_cleanup(curl) ;
		curl = NULL ;
	}

	return bRet ;
}

size_t CWebService::Callback_Func(void *pBuffer, size_t nSize, size_t nMemb, VOID* pParam)
{
	PRESULT_DATA pResult = (PRESULT_DATA)pParam ;

	if (pResult && (pResult->nResFormat == 0))
	{//result data is stored in string text.
		pResult->sResult.append((char*)pBuffer, nSize*nMemb) ;
	}
	else
	{//result data is store in file
		if (pResult && !pResult->pStream)
		{
			pResult->pStream = _tfopen(pResult->sFile.c_str(), _T("wb"));
			if (!pResult->pStream)
			{
				return -1;
			}
		}
		size_t nWriten = fwrite(pBuffer, nSize, nMemb, pResult->pStream) ;
		if (nWriten != nSize*nMemb)
		{
			return -1;
		}
	}

	return nSize*nMemb ;
}

std::string CWebService::miniURLEncode(const std::string& s)
{
	std::string encoded;
	char buf[16]; // way longer than needed

	unsigned char c;
	for(int i=0; i < (signed)s.length(); i++) 
	{    
		c = s[i];
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||

			c == '.' || c == '-' || c == '_' || c== '=' || c== '&' || c=='/' || c=='?' ||
            c==':' || c==';' || c=='+' || c=='$' || c==',' || c=='@') 

		{    

			// allowed characters in a url that have non special meaning

			encoded += c;
			continue;
		}    
		if(c == ' ') 
		{    
			encoded += "\%20"; 
			continue;
		}    
		sprintf(buf, "%x", c); 
		encoded += "%"; 
		encoded += buf; 
	}    
	return encoded;
}

std::string CWebService::miniURLEncodeComponent(const std::string& s)
{
	std::string encoded;
	char buf[16]; // way longer than needed

	unsigned char c;
	for(int i=0; i < (signed)s.length(); i++) 
	{    
		c = s[i];
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||

			c == '.' || c == '-' || c == '_' || c== '~') 

		{    

			// allowed characters in a url that have non special meaning

			encoded += c;
			continue;
		}    
		//if(c == ' ') 
		//{    
		//	encoded += "\%20"; 
		//	continue;
		//}    
		sprintf(buf, "%x", c); 
		encoded += "%"; 
		encoded += buf; 
	}    
	return encoded;
}

bool CWebService::HttpsRequest(const TCHAR* pszUrl, BYTE* pszResult, DWORD dwResultSize)
{
	bool bRet = false ;
	HINTERNET hINet = NULL ;
	HINTERNET hFile = NULL ;

	__try{

		if (NULL == pszUrl || pszUrl[0] == _T('\0') || NULL == pszResult)
		{
			__leave ;
		}

		hINet = ::InternetOpen(_T("WebService"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		if(hINet == NULL)
		{
			__leave ;
		}

		unsigned long nTimeOut = 2000 ;
		::InternetSetOption(hINet,INTERNET_OPTION_CONNECT_TIMEOUT,&nTimeOut,sizeof(unsigned long));

		hFile = ::InternetOpenUrl(hINet, pszUrl, NULL, 0, INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE, 0) ;
		if(hFile == NULL)
		{
			__leave ;
		}

		unsigned char dBuf[8] = {0} ;   
		DWORD  dwBufSize0=sizeof(dBuf) ;   
		if(!::HttpQueryInfo(hFile,HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER,dBuf,&dwBufSize0,NULL))
		{
			__leave;
		}
		if (dBuf[0] != 200 && dBuf[0] != 206)
		{
			__leave ;
		}

		unsigned long nSize = 0;
		BOOL bRet2 = ::InternetReadFile(hFile, pszResult, dwResultSize, &nSize);
		if(!bRet2 || (nSize == 0))
		{
			__leave ;
		}

		bRet = true ;

	}__finally{

		if (hFile != NULL)
		{
			::InternetCloseHandle(hFile) ;
			hFile = NULL ;
		}

		if (hINet != NULL)
		{
			::InternetCloseHandle(hINet) ;
			hINet = NULL ;
		}
	}

	return bRet ;
}

bool CWebService::HttpsPostService(_tstring sUrl, _tstring sCookie, _tstring sPostData, PRESULT_DATA sResult)
{   
	HINTERNET  hInternet = NULL;
    HINTERNET  hConnect  = NULL;
    HINTERNET  hRequest  = NULL;
    bool       bRet = false;
	do 
	{
		hInternet = InternetOpen(_T("WinInetGet/0.1"),INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);   
		if(!hInternet) 
		{   
			break; 
		}   

		int nPos = sUrl.Find('/',8);//check whether the head is "https://".
		if (nPos<=8)
		{
			break;
		}
		_tstring szHost = sUrl.Mid(8,nPos-8);
		_tstring szObjectName = sUrl.Right(sUrl.length()-szHost.length()-8);

		DWORD dwConnectFlags = 0;   
		DWORD dwConnectContext = 0;   
		hConnect = InternetConnect(hInternet,szHost.c_str(),INTERNET_DEFAULT_HTTPS_PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,0);   
		if(!hConnect) 
		{
			break;   
		}  

		DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |   
			INTERNET_FLAG_KEEP_CONNECTION |   
			INTERNET_FLAG_NO_AUTH |   
			INTERNET_FLAG_NO_COOKIES |   
			INTERNET_FLAG_NO_UI |   
			//enable HTTPS   
			INTERNET_FLAG_SECURE |   
			INTERNET_FLAG_IGNORE_CERT_CN_INVALID|  
			INTERNET_FLAG_RELOAD;   

		//intial Request   
		hRequest = HttpOpenRequest(hConnect,_T("POST"),szObjectName.c_str(),NULL,NULL,NULL,dwOpenRequestFlags,0);   
		if(!hRequest) 
		{   
			break;   
		}   

		DWORD dwFlags = 0;   
		DWORD dwBuffLen = sizeof(dwFlags);   
		InternetQueryOption (hRequest,INTERNET_OPTION_SECURITY_FLAGS,(LPVOID)&dwFlags,&dwBuffLen);   
		dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;   
		InternetSetOption (hRequest, INTERNET_OPTION_SECURITY_FLAGS,&dwFlags,sizeof (dwFlags));

		_tstring sHead;//please be careful about the blank usage.
		sHead.Format(1,_T("Cookie: %s\r\nContent-Type: application/x-www-form-urlencoded"),sCookie.c_str());
        _tstring szHead = sHead.toUTF8();
        std::string  szPostData = sPostData.toUTF8();

		//send Request   
		BOOL bResult = HttpSendRequest(hRequest,szHead.c_str(),szHead.length(),(LPVOID)szPostData.c_str(),szPostData.length());   
		if(!bResult)    
		{    
			break;   
		}   

		//fetch HTTP Response Header info.   
		char szBuff[1024]={0};   
		DWORD dwReadSize = 1024;   
		bResult = HttpQueryInfo(hRequest,HTTP_QUERY_RAW_HEADERS_CRLF,szBuff,&dwReadSize,NULL);   
		if(!bResult)
		{   
			break;
		}   
		szBuff[dwReadSize] =0;   

		//HTTP Response Body   
		DWORD dwBytesAvailable = 0;   
		bResult = InternetQueryDataAvailable(hRequest,&dwBytesAvailable,0,0);   
		if(!bResult) 
		{   
			break;   
		}   
		if(dwBytesAvailable>1024)    
		{   
			break; 
		}   

		DWORD dwBytesRead = 0;   
		bResult = InternetReadFile(hRequest,szBuff,dwBytesAvailable,&dwBytesRead);   
		if(!bResult)
		{   
			break;   
		}   
		szBuff[dwBytesRead] =0;   

		if (sResult!=NULL)
		{
			sResult->sResult.clear();
			sResult->sResult.assign(szBuff) ;
		}
        bRet = true;

	} while (FALSE);

	if(hRequest) {   
		InternetCloseHandle(hRequest);   
	}   
	if(hConnect) {   
		InternetCloseHandle(hConnect);   
	}   
	if(hInternet) {   
		InternetCloseHandle(hInternet);   
	}  
 
    return bRet;
}
