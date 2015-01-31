#include <tchar.h>
#include <Userenv.h>
#include "data\DataPool.h"
#include "data\Utility.h"
#include "constant.h"
#include "log\local.h"
#include "tinyxml\tinyxml.h"
#include "crypt\kaes.h"
#include <shlobj.h>

CDataPool theDataPool;
char      g_szKey[17] = "xou12907asfgocet";

#define FILE_MD5_LIST       _T("cfgFileList.xml")
#define TSTRING_LANGUAGE_EN				_T("english")
#define TSTRING_LANGUAGE_DE				_T("german")
#define TSTRING_LANGUAGE_FR				_T("french")
#define TSTRING_LANGUAGE_PT				_T("portuguese")

CDataPool::CDataPool()
{
	m_nLang=1;
    m_nAccountType=1;
	m_nLocalTimeTick = 0;
	m_nTime = 0;
}

CDataPool::~CDataPool()
{

}

bool CDataPool::Init()
{
	//load download engine parameters
	::GetPrivateProfileString(INI_CFG_CLIENT_UPDATE,INI_CFG_CLIENT_UPDATE_PARTNER,_T("pwrdcore"),\
		m_szPartner,128, GetConfigPath().c_str());
	::GetPrivateProfileString(INI_CFG_CLIENT_UPDATE,INI_CFG_CLIENT_UPDATE_PARTNERVER,_T("2.6.0.1"),\
		m_szPandoVersion,128,GetConfigPath().c_str());

	//load local timer id
	LoadLocalTimerInfo(GetBaseDir()) ;

	//load overlay DLLs list
	int nDllCount = GetPrivateProfileInt(INI_CFG_CLIENT_INGAMEDLL,INI_CFG_CLIENT_INGAMEDLL_COUNT,0, GetConfigPath().c_str());
	TCHAR szDllList[1024] = {0} ;
	if (GetPrivateProfileString(INI_CFG_CLIENT_INGAMEDLL,INI_CFG_CLIENT_INGAMEDLL_LIST,NULL, szDllList,sizeof(szDllList), GetConfigPath().c_str()))
	{
		TCHAR* pszTmp = szDllList ;
		for (int i = 0; i < nDllCount; ++ i)
		{
			TCHAR* pszColon = _tcschr(pszTmp, _T(';')) ;
			if (pszColon == NULL)
			{
				return false ;
			}
			*pszColon = _T('\0') ;

			_tstring sElement = pszTmp ;
			m_vecDllList.push_back(sElement) ;

			pszTmp = pszTmp + _tcslen(pszTmp) + 1 ;
		}
	}
	
	//load server domain, ip and port
	TCHAR  szText[512];
	memset(szText,0, sizeof(szText));
	if (!GetPrivateProfileString(INI_CFG_CLIENT_CONFIG,INI_CFG_CLIENT_CONFIG_DOMAIN,NULL, szText, 512, GetConfigPath().c_str()))
	{
		return false ;
	}
	SetXmppDomain(szText) ;

	memset(szText,0, sizeof(szText));
	if (!GetPrivateProfileString(INI_CFG_CLIENT_CONFIG,INI_CFG_CLIENT_CONFIG_SERVER,NULL, szText, 512, GetConfigPath().c_str()))
	{
		return false ;
	}
	SetXmppServer(szText);

	memset(szText,0, sizeof(szText));
	if (!GetPrivateProfileString(INI_CFG_CLIENT_CONFIG, INI_CFG_CLIENT_CONFIG_SERVER, NULL,szText,512,GetConfigPath().c_str()))
	{
		return false ;
	}
	SetXmppServer(szText) ;

	memset(szText,0, sizeof(szText));
	if (!GetPrivateProfileString(INI_CFG_CLIENT_CONFIG,INI_CFG_CLIENT_CONFIG_PORT,NULL,szText,512,GetConfigPath().c_str()))
	{
		return false ;
	}
	m_iXmppPort = _ttoi(szText) ;

	memset(szText,0, sizeof(szText));
	if (!GetPrivateProfileString(INI_CFG_CLIENT_LOGIN,INI_CFG_CLIENT_LOGIN_SERVER,NULL,szText,512,GetConfigPath().c_str()))
	{
		return false ;
	}
	SetLoginServer(szText);

	memset(szText,0, sizeof(szText));
	if (!GetPrivateProfileString(INI_CFG_CLIENT_LOGIN,INI_CFG_CLIENT_LOGIN_PORT,NULL,szText,512,GetConfigPath().c_str()))
	{
		return false ;
	}
	m_iLoginPort = _ttoi(szText) ;

	/*
	** write core client process id into Config.ini in current directory
	** user for overlay to judge whether hook function or not.
	** in order to avoid coreclient crash when notification is sent to game.
	*/
	WritePrivateProfileString( _T("Process"),_T("id"), _tstring(GetCurrentProcessId()).c_str(), GetConfigPath().c_str());

	//get current client version
	//TCHAR szCoreLocalVer[128] = {0} ;
	//::GetPrivateProfileString(INI_CFG_CLIENT_VERSION,INI_CFG_CLIENT_VERSION_TEXT,_T(""),szCoreLocalVer,sizeof(szCoreLocalVer),GetConfigPath().c_str());
	//m_sCCVersion = szCoreLocalVer ;
	//SetCCVersion(ARC_CLIENT_VERSION);

	//Check if config.ini in user folder is Unicode
	_tstring sUserPath = GetUserProfilePath();
	FILE* pf = _tfopen(sUserPath.c_str(), _T("rb+"));
	if (pf)
	{
		fseek(pf,0,SEEK_END); 
		int nLen = ftell(pf); 
		fseek(pf,0,SEEK_SET); 
		char* buffer = new char[sizeof(char)*(nLen+1)];
		fread(buffer,sizeof(char),nLen,pf);
		buffer[nLen] = '\0';
		if((UCHAR)buffer[0] != 0xFF || (UCHAR)buffer[1] != 0xFE) 
		{//Not Unicode
			WORD wBOM = 0xFEFF;
			_tstring sBuffer = buffer;

			fseek(pf,0,SEEK_SET); 
			fwrite(&wBOM, sizeof(WORD), 1, pf);
			fwrite(sBuffer.c_str(), sizeof(TCHAR), nLen, pf);
			fflush(pf);
		}
		fclose(pf);
	}

	//set default download path parallel with CORE directory.
	TCHAR szDownloadPath[MAX_PATH] = {0} ;
	GetPrivateProfileString(INI_CFG_CLIENT_NETWORK,INI_CFG_CLIENT_NETWORK_DOWNPATH,_T(""),szDownloadPath,MAX_PATH, GetUserProfilePath().c_str()) ;
	if (0 == _tcslen(szDownloadPath))
	{
		GetModuleFileName(NULL, szDownloadPath, MAX_PATH);
		_tstring strPath = szDownloadPath;
		size_t nPos = strPath.ReverseFind(_T('\\'));
		if(nPos > 0)
		{
			strPath = strPath.Left(nPos);
		}
		nPos = strPath.ReverseFind(_T('\\'));
		if(nPos > 0)
		{
			// the path should end with '\' or check authority may incorrect
			strPath = strPath.Left(nPos+1);
		}

		//// if not authorized pmb fails to download
		//if(!CUtility::IsPathAuthorized(strPath))
		//{
		//	GetWindowsDirectory(szDownloadPath, MAX_PATH) ;
		//	TCHAR* pPointer = _tcschr(szDownloadPath, _T('\\')) ;
		//	if (pPointer != NULL)
		//	{
		//		*(pPointer+1) = _T('\0') ;
		//		_tcscat(szDownloadPath, FILEPATH_DEFAULT_DOWNLOAD) ;
		//		CUtility::MakeDir(szDownloadPath) ;
		//		strPath = szDownloadPath;
		//	}
		//}

		WritePrivateProfileString(INI_CFG_CLIENT_NETWORK,INI_CFG_CLIENT_NETWORK_DOWNPATH, strPath.c_str(), GetUserProfilePath().c_str()) ;
	}

	//get eula local version
	TCHAR szEulaVersion[MAX_PATH] = {0} ;
	_tstring sSection = _T("eula_") + theDataPool.GetLangAbbrString() ;
	GetPrivateProfileString(INI_CFG_CLIENT_VERSION,sSection.c_str(),_T("0"), szEulaVersion, MAX_PATH, GetUserProfilePath().c_str()) ;
	m_sEulaVersion = szEulaVersion ;

	_tstring sFileListCfg(GetBaseDir());
	if (sFileListCfg.Right(1) != _T("\\"))
	{
		sFileListCfg.append(_T("\\"));
	}
	sFileListCfg.append(FILE_MD5_LIST);
	if (!ReadMD5FileList(sFileListCfg.c_str()))
	{
		return false;
	}
	return true ;
}

bool CDataPool::Uninit()
{
	return true ;
}

void CDataPool::SetLangId(int id, bool bFlush)
{
	if (id <= 0)
	{
		id = GetPrivateProfileInt(INI_CFG_CLIENT_LOCALE,INI_CFG_CLIENT_LOCALE_LANGUAGE, 0, GetUserProfilePath().c_str()) ;
		if (0 == id)
		{
			id = GetPrivateProfileInt(INI_CFG_CLIENT_LOCALE,INI_CFG_CLIENT_LOCALE_LANGUAGE, 1, GetConfigPath().c_str()) ;
		}
	}
	m_nLangCount = GetPrivateProfileInt(INI_CFG_CLIENT_LOCALE,INI_CFG_CLIENT_LOCALE_COUNT,5, GetConfigPath().c_str()); 

	if (bFlush)
	{
		WritePrivateProfileString(INI_CFG_CLIENT_LOCALE,INI_CFG_CLIENT_LOCALE_LANGUAGE, _tstring(id).c_str(), GetUserProfilePath().c_str()) ;
	}
	m_nLang = id ;
}

int CDataPool::GetLangId(bool bFile)
{
	return m_nLang;
}

_tstring CDataPool::GetLangAbbrString()
{
	TCHAR szLang[128] = {0};
	GetPrivateProfileString(INI_CFG_CLIENT_MULTILANGABBR,_tstring(m_nLang).c_str(),TSTRING_ABBR_LANGUAGE_EN,szLang,127,GetConfigPath().c_str());

	_tstring sLang = szLang;
	return sLang;
}

_tstring CDataPool::GetLangString()
{
	TCHAR szLang[128] = {0};
	GetPrivateProfileString(INI_CFG_CLIENT_MULTILANGUAGE,_tstring(m_nLang).c_str(),TSTRING_LANGUAGE_EN,szLang,127,GetConfigPath().c_str());

	_tstring sLang = szLang;
	return sLang;
}

void CDataPool::SetBaseDir(LPCTSTR lpszStr)
{
	m_tsBaseDir=lpszStr;
}

_tstring CDataPool::GetBaseDir()
{
	if (!m_tsBaseDir.empty())
	{
		return m_tsBaseDir ;
	}

	TCHAR szPath[1024] = {0} ;
	DWORD rc = GetModuleFileName(NULL, szPath, 1024) ;
	if(rc != 0)
	{
		_tstring sPath = szPath ;
		m_tsBaseDir = sPath.Left(sPath.rfind(_T("\\")) + 1) ;
	}
	else
	{
		m_tsBaseDir = _T("") ;
	}

	return m_tsBaseDir.c_str( );
}

_tstring CDataPool::GetUserProfilePath() 
{
	if (!m_sUserProfilePath.empty())
	{
		return m_sUserProfilePath ;
	}

	TCHAR pszPath[MAX_PATH] = _T("");
	if(S_OK != SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, pszPath))
	{
		return _T("");
	}
	m_sUserProfilePath = pszPath;
	m_sUserProfilePath += _T("\\Arc\\");
	CreateDirectory(m_sUserProfilePath.c_str(), NULL) ;
	m_sUserProfilePath += FILE_CFG_CLIENT;
	return m_sUserProfilePath;

	//HANDLE hToken;
	//if (!OpenProcessToken(GetCurrentProcess() ,TOKEN_QUERY, &hToken))
	//{
	//	return _T("") ;
	//}
	//if (!GetUserProfileDirectory(hToken, szUserProfilePath, &dwLen))
	//{
	//	return _T("") ;
	//}

	//m_sUserProfilePath = szUserProfilePath ;
	//m_sUserProfilePath.append(_T("\\")) ;
	//m_sUserProfilePath.append(APP_PATH) ;
	//CreateDirectory(m_sUserProfilePath.c_str(), NULL) ;
	//m_sUserProfilePath.append(FILE_CFG_CLIENT) ;

	//return m_sUserProfilePath ;
}

void CDataPool::SetUserProfilePath(TCHAR* pszUserProfilePath)
{ 
	if(pszUserProfilePath == NULL)
	{
		return ;
	}
	m_sUserProfilePath.assign(pszUserProfilePath) ;
}

_tstring CDataPool::GetConfigPath()
{
	if (m_sIniConfigPath.empty())
	{
		m_sIniConfigPath = GetBaseDir() + FILE_CFG_CLIENT ;
	}

	return m_sIniConfigPath ;
}

_tstring CDataPool::GetPandoVersion()
{
	return _tstring(m_szPandoVersion) ;
}

_tstring CDataPool::GetEulaVersion() 
{
	return m_sEulaVersion ;
}

void CDataPool::SetEulaVersion(_tstring sVer)
{
	_tstring sSection = _T("eula_") + theDataPool.GetLangAbbrString() ;
	WritePrivateProfileString(INI_CFG_CLIENT_VERSION,sSection.c_str(), sVer.c_str(), GetUserProfilePath().c_str()) ;
}

void CDataPool::SetServerTimer( __int64 nTime )
{
	m_nLocalTimeTick = GetTickCount();
	m_nTime = nTime;
}

__int64 CDataPool::GetServerTimer()
{
	if (m_nTime == 0)
	{
		return ::_time64(NULL);
	}
	else
	{
		__int64 nTickGap = GetTickCount() - m_nLocalTimeTick;
		__int64 nCurrentTime = m_nTime + nTickGap/1000;
		return nCurrentTime;
	}
}

void CDataPool::SetUserEmail(LPCTSTR lpszEmail)
{
	m_tsUserEmail = lpszEmail;
	m_tsUserEmail.MakeLower();
}

_tstring CDataPool::GetUserEmail()
{
	return m_tsUserEmail;
}

void CDataPool::SetUserName(LPCTSTR lpszUser)
{
	m_tsUserName=lpszUser;
	m_tsUserName.MakeLower() ;
}

_tstring CDataPool::GetUserName( )
{
	return m_tsUserName.c_str( );
}

void CDataPool::SetPassword(LPCSTR lpszStr)
{
	m_tsPassword2 = "" ; //clear backup credential

	char szSavePwd[65] = {0} ;
    lstrcpynA(szSavePwd,lpszStr,64);//to avoid being out of scape.
	//strncpy(szSavePwd, lpszStr, strlen(lpszStr)) ;
	
	DATA_BLOB DataIn, DataOut ;
	DataIn.cbData = strlen(szSavePwd) + 1 ; DataIn.pbData = (BYTE*)szSavePwd ;
	
	DATA_BLOB Entropy ;
	Entropy.pbData = (BYTE*)g_szKey ; Entropy.cbData = strlen(g_szKey) + 1 ;
	
	if (!CryptProtectData(&DataIn, TEXT_USER_PASSWORD, &Entropy, NULL, NULL, 0, &DataOut) || (DataOut.cbData == 0))
	{//failed to call crypto API, use backup solution to save credential in memory
		m_tsPassword = "";

		char szCrypted[65] = {0} ;
		AES_HANDLE hPassword = AESInitialize(BIT256, (BYTE*)g_szKey) ;
		AESCipher(hPassword, (BYTE*)szSavePwd, (BYTE*)szCrypted, 64) ;
		AESUnInitialize(hPassword);
		m_tsPassword2 = szCrypted ;
	}
	else
	{//use preferred solution to save credential in memory
		_tstringA szCredential ;
		for(int nIndex = 0; nIndex < DataOut.cbData; ++ nIndex)
		{
			_tstringA sztmp;
			sztmp.Format(1, "%02X", DataOut.pbData[nIndex]) ;
			szCredential.append(sztmp) ;
		}
		LocalFree(DataOut.pbData) ;
	    m_tsPassword.assign(szCredential);
	}
}

_tstringA CDataPool::GetPassword()
{	
	if (!m_tsPassword2.empty())
	{//m_tsPassword cannot be set correctly, use backup solution to get password
		char szDecrypted[65] = {0} ;
		AES_HANDLE hPassword = AESInitialize(BIT256, (BYTE*)g_szKey);
		AESInvCipher(hPassword, (BYTE*)m_tsPassword2.c_str(), (BYTE*)szDecrypted, 64);
		AESUnInitialize(hPassword);
		return szDecrypted ;
	}

	if (m_tsPassword.empty())
	{//m_tsPassword has not been read from config file
		TCHAR szCredential[1024]={0};
		GetPrivateProfileString(INI_CFG_CLIENT_LOGIN,INI_CFG_CLIENT_LOGIN_PASSWORD,NULL,szCredential,1024,theDataPool.GetUserProfilePath().c_str());
		m_tsPassword = szCredential ;
		if (m_tsPassword.empty())
		{
			return "" ;
		}
	}

	BYTE szSavePwd[2048] = {0} ;
	char szStr[2048] = {0} ;
	DWORD dwLen = m_tsPassword.toUTF8().length()/2 ;
	lstrcpynA(szStr, m_tsPassword.toUTF8().c_str(), 2048) ;
	for(int i=0; i<dwLen; i++)
	{
		char ch[3]={0};
		ch[0]=szStr[2*i];
		ch[1]=szStr[2*i+1];
		sscanf(ch,"%x",&szSavePwd[i]);  // HEX to int
	}
  
	DATA_BLOB DataIn, DataOut ;
	DataIn.cbData = dwLen ; DataIn.pbData = szSavePwd ;

	DATA_BLOB Entropy ;
	Entropy.pbData = (BYTE*)g_szKey ; Entropy.cbData = strlen(g_szKey) + 1 ;
	
	if (!CryptUnprotectData(&DataIn, NULL, &Entropy, NULL, NULL, 0, &DataOut))
	{
		return "";
	}
	_tstringA szPwd = (char *)DataOut.pbData;
	LocalFree(DataOut.pbData) ;
    return szPwd;
}

_tstringA CDataPool::GetCredential()
{
	return m_tsPassword ;
}

void CDataPool::SetXmppDomain(LPCTSTR lpszDomain )
{
	m_tsXmppDomain=lpszDomain;
}

_tstring CDataPool::GetXmppDomain( )
{
	return m_tsXmppDomain;
}

void CDataPool::SetXmppServer( LPCTSTR lpszServer)
{
	m_sXmppServer = lpszServer ;
}

_tstring CDataPool::GetXmppServer( )
{
	return m_sXmppServer;
}

void CDataPool::SetXmppPort( int iPort)
{
	m_iXmppPort=iPort;
}

int CDataPool::GetXmppPort( )
{
	return m_iXmppPort;
}

void CDataPool::SetLoginServer( LPCTSTR lpszServer)
{
	m_sLoginServer = lpszServer ;
}

_tstring CDataPool::GetLoginServer( )
{
	return m_sLoginServer ;
}

void CDataPool::SetLoginPort( int iPort)
{
	m_iLoginPort=iPort;
}

int CDataPool::GetLoginPort( )
{
	return m_iLoginPort;
}

void CDataPool::SetGameToken(_tstring sGameAbbr,LPCSTR lpszStr) 
{
    m_mapGametoken[sGameAbbr] = lpszStr;
}

_tstring CDataPool::GetGameToken(_tstring sGameAbbr)
{
	if (m_mapGametoken.find(sGameAbbr) != m_mapGametoken.end())
	{
		return m_mapGametoken[sGameAbbr];
	}
    return "";
}

BOOL CDataPool::LoadLocalTimerInfo(_tstring sBaseDir)
{
	if (sBaseDir.empty())
	{
		return FALSE ;
	}
	
	_tstring sConfigPath = sBaseDir + FILE_CFG_CLIENT ;
	m_mapLocalTimer[TIMER_NID_SET_SPAMMING] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_SET_SPAMMING"),30000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_LOGIN_TIMEOUT] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_LOGIN_TIMEOUT"),30000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_CHECK_UPDATE] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_CHECK_UPDATE"),86400000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_TRACK_UPDATE_SESSION] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_TRACK_UPDATE_SESSION"),600000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_TRACK_NOTIFICATION_DISPLAY] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_TRACK_NOTIFICATION_DISPLAY"),900000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_LOGIN_RECONNECT] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_LOGIN_RECONNECT"),3000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_XMPP_ALIVE] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_XMPP_ALIVE"),60000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_SET_COOKIE] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_SET_COOKIE"),500,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GET_WEBSERVICE_FIRSTTIME] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GET_WEBSERVICE_FIRSTTIME"),12000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GET_WEBSERVICE] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GET_WEBSERVICE"),120000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_PROGRESSBAR_DOWNLOAD_STATUS] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_PROGRESSBAR_DOWNLOAD_STATUS"),100,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_XMPP_RECONNECT] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_XMPP_RECONNECT"),15000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_UNINSTALL_GAME] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_UNINSTALL_GAME"),1000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_START_TUTORIAL] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_START_TUTORIAL"),500,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_MSG_BLINK] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_MSG_BLINK"),500,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_INSTALL_GAME] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_INSTALL_GAME"),1000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_CC_LOGIN] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_CC_LOGIN"),30000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GAME_UPDATE_CHECK] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GAME_UPDATE_CHECK"),60000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GAME_GET_BETAKEYFLAG] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GAME_GET_BETAKEYFLAG"),300000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GAME_GET_BETAOFFLINEFLAG] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GAME_GET_BETAOFFLINEFLAG"),300000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GAME_SEARCH] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GAME_SEARCH"),5000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_DETECT_UNINSTALL] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_DETECT_UNINSTALL"),30000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_GAME_GAMECLIENT_RUNNING] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_GAME_GAMECLIENT_RUNNING"),1000,sConfigPath.c_str());
	m_mapLocalTimer[TIMER_NID_CCU_NEW] = GetPrivateProfileInt(INI_CFG_CLIENT_TIMER,_T("TIMER_NID_CCU_NEW"),60000,sConfigPath.c_str());
	return TRUE ;
}

UINT_PTR CDataPool::GetTimerInterval(UINT_PTR uTimerId)
{
	return m_mapLocalTimer[uTimerId] ;
}

bool CDataPool::ReadMD5FileList(LPCTSTR lpsCfgFilePath)
{
	m_vecMd5FileList.clear();
	if (lpsCfgFilePath == NULL || _tcslen(lpsCfgFilePath)<=0)
	{
		return false;
	}

	FILE   *fp=NULL;
	fp=_tfopen(lpsCfgFilePath,_T("rb"));
	if(!fp)
	{
		return false;
	}

	_tstring sFilePath(lpsCfgFilePath) ;
	TiXmlDocument  xmlDoc(sFilePath.toNarrowString().c_str());
	TiXmlElement     *pElement=NULL;
	bool bRet = false;
	do 
	{
		if (!xmlDoc.LoadFile(fp))
		{
			break;;
		}
		pElement=xmlDoc.RootElement( );
		if(!pElement)
		{
			break;
		}
		pElement=pElement->FirstChildElement("fileMD5");
		if(!pElement)
		{
			break;
		}
		pElement = pElement->FirstChildElement("filepath");
		while(pElement)
		{
			m_vecMd5FileList.push_back(_tstring(pElement->GetText()));
			pElement=pElement->NextSiblingElement( );
		}
		bRet = true;
	} while (FALSE);
	
	xmlDoc.Clear( );
	fclose(fp) ;
	return bRet;
}

bool CDataPool::WriteMD5ListToLog(LPCTSTR szBaseDir,LPCTSTR lpsLogFileName)
{
	
	TCHAR szLogFilePath[MAX_PATH] = {0} ;
	_tcscpy(szLogFilePath, szBaseDir) ;
	_tcscat(szLogFilePath, lpsLogFileName) ;

	FILE* pFile = _tfopen(szLogFilePath, _T("w+"));
	if (pFile == NULL)
	{
		return false ;
	}
	
	std::vector<_tstring>::iterator iter = m_vecMd5FileList.begin();
	for(;iter != m_vecMd5FileList.end();++iter)
	{
		_tstring sTextLine = _T("") ;
		_tstring szPath(szBaseDir);
		if (szPath.Right(1) != _T("\\"))
		{
			szPath.append(_T("\\"));
		}
		szPath.append((*iter).c_str());
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(szPath.c_str( )))
		{
			sTextLine.assign((*iter).c_str()) ;
			sTextLine.append(_T(":the file does not exist!\r\n")) ;
			fwrite(sTextLine.toNarrowString().c_str(), 1, sTextLine.toNarrowString().length(), pFile) ;
			fflush(pFile);
		}
		else
		{
			unsigned char szMD5[16] = {0} ;
			CUtility::GenerateMD5ForFile(szPath,szMD5);
			_tstring sMd5= CUtility::ConvertHexToString(szMD5, sizeof(szMD5));
			
			sTextLine.assign((*iter).c_str()) ;
			sTextLine.append(_T(":")) ;
			sTextLine.append(sMd5.c_str()) ;
			sTextLine.append(_T("\r\n")) ;
			fwrite(sTextLine.toNarrowString().c_str(), 1, sTextLine.toNarrowString().length(), pFile) ;
			fflush(pFile);
		}
	}

	fclose(pFile) ;

	return true ;
}

_tstring CDataPool::GetLangAbbrById(int nId)
{
	_tstring sLangId(nId);
	_tstring sPath = theDataPool.GetBaseDir() + FILE_CFG_CLIENT;

	TCHAR szLangAbbr[128] = {0} ;
	GetPrivateProfileString(INI_CFG_CLIENT_MULTILANGABBR, sLangId.c_str(), _T(""), szLangAbbr, 128, sPath.c_str());
	_tstring sLang = szLangAbbr;
	return sLang;
}

int CDataPool::GetLangIdByAbbr(_tstring sLangAbbr)
{
	_tstring sPath = theDataPool.GetBaseDir() + FILE_CFG_CLIENT;

	int nLangCount = GetPrivateProfileInt(INI_CFG_CLIENT_LOCALE,INI_CFG_CLIENT_LOCALE_COUNT,10, sPath.c_str()); 
	TCHAR szLang[128] = {0};
	for (int i=1; i<=nLangCount; i++)
	{
		_tstring sLangId(i);
		GetPrivateProfileString(INI_CFG_CLIENT_MULTILANGABBR,sLangId.c_str(),_T(""),szLang,127,sPath.c_str());
		if (sLangAbbr == szLang)
		{
			return i;
		}
	}

	return 0;
}

_tstring CDataPool::GetCommonPath()
{
	TCHAR MyDir[_MAX_PATH]={0};  
	SHGetSpecialFolderPath(NULL,MyDir,CSIDL_COMMON_DOCUMENTS,TRUE);
	return MyDir;
}

bool CDataPool::FormatRegisterInfo(_tstring& sRegister)
{
	_tstring sPath = GetBaseDir() + FILE_INI_REGISTER;
	bool bRegistered = GetPrivateProfileInt(INI_REGISTER_FROM,INI_REGISTER_FROM_FLAG,0,sPath.c_str());
	if (!bRegistered)
	{
		TCHAR szTransID[128] = {0};
		if (!GetPrivateProfileString(INI_REGISTER_FROM,INI_REGISTER_FROM_TRANSID,_T(""), szTransID,128,sPath.c_str()))
		{
			return false;
		}
		TCHAR szOfferID[128] = {0};
		if (!GetPrivateProfileString(INI_REGISTER_FROM,INI_REGISTER_FROM_OFFERID,_T(""), szOfferID,128,sPath.c_str()))
		{
			return false;
		}
		TCHAR szVendorID[128] = {0};
		if (!GetPrivateProfileString(INI_REGISTER_FROM,INI_REGISTER_FROM_VENDORID,_T(""), szVendorID,128,sPath.c_str()))
		{
			return false;
		}
		TCHAR szLang[128] = {0};
		if (!GetPrivateProfileString(INI_REGISTER_FROM,INI_REGISTER_FROM_LANG,_T(""), szLang,128,sPath.c_str()))
		{
			return false;
		}
		TCHAR szGameAbbr[128] = {0};
		GetPrivateProfileString(INI_REGISTER_FROM,INI_REGISTER_FROM_GAMEABBR,_T(""), szGameAbbr,128,sPath.c_str());//GameAbbr can be empty

		sRegister.Format(5, _T("?hoff_transaction_id=%s&hoff_offer_id=%s&hoff_vendor_id=%s&region=%s&game_abbr=%s"), szTransID,szOfferID,szVendorID,szLang,szGameAbbr);
		return true;
	}

	return false;
}