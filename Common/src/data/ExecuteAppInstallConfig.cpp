#include <tchar.h>
#include <atlbase.h>
#include "tinyxml\tinyxml.h"
#include "data/ExecuteAppInstallConfig.h"


static bool WriteSilenceRegistry(_tstring sPath, _tstring sLan);
static bool DeleteSilenceRegistry(_tstring sPath);

bool ExecuteInstallConfig(_tstring sPath, int nCmd, _tstring sLan)
{
	if(nCmd == 0)
	{
		return WriteSilenceRegistry(sPath, sLan);
	}
	else
	{
		return DeleteSilenceRegistry(sPath);
	}
}

static HKEY StringToHKey(_tstring sKey)
{
	if(sKey == _T("HKEY_CLASSES_ROOT"))
	{
		return HKEY_CLASSES_ROOT;
	}
	else if(sKey == _T("HKEY_CURRENT_CONFIG"))
	{
		return HKEY_CURRENT_CONFIG;
	}
	else if(sKey == _T("HKEY_CURRENT_USER"))
	{
		return HKEY_CURRENT_USER;
	}
	else if(sKey == _T("HKEY_LOCAL_MACHINE"))
	{
		return HKEY_LOCAL_MACHINE;
	}
	else if(sKey == _T("HKEY_PERFORMANCE_DATA"))
	{
		return HKEY_PERFORMANCE_DATA;
	}
	else if(sKey == _T("HKEY_PERFORMANCE_NLSTEXT"))
	{
		return HKEY_PERFORMANCE_NLSTEXT;
	}
	else if(sKey == _T("HKEY_PERFORMANCE_TEXT"))
	{
		return HKEY_PERFORMANCE_TEXT;
	}
	else if(sKey == _T("HKEY_USERS"))
	{
		return HKEY_USERS;
	}
	else
	{
		return HKEY_CLASSES_ROOT;
	}
}

static bool TranslatePath(_tstring &sPath, _tstring sInstallPath)
{
	_tstring sInstallToken = _T("%INSTALLDIR%");
	_tstring sSystemToken = _T("%SYSTEMDIR%");
	if(sPath.Left(sInstallToken.size()) == sInstallToken)
	{
		sPath = sInstallPath + 
			sPath.Right(sPath.size() - sInstallToken.size());
	}
	else if(sPath.Left(sSystemToken.size()) == sSystemToken)
	{
		TCHAR sysPath[MAX_PATH] = _T("");
		_tstring sSysDisk = _T("C:");
		if(GetSystemDirectory(sysPath, MAX_PATH) > 0)
		{
			sSysDisk[0] = sysPath[0];
		}
		sPath = sSysDisk + sPath.Right(sPath.size() - sSystemToken.size());
	}
	return true;
}

static bool WriteSingleLanguageRegistry(TiXmlElement *pRegistry, bool bDepend, _tstring sInstallPath)
{
	TiXmlElement *pElement = NULL;

	if(pElement = pRegistry->FirstChildElement("coreitems"))
	{
		pElement = pElement->FirstChildElement("item");
		while(pElement)
		{
			_tstring sRegRoot = pElement->Attribute("rootpath");
			_tstring sRegSub = pElement->Attribute("path");
			_tstring sKeyName = pElement->Attribute("itemname");
			_tstring sKeyType = pElement->Attribute("itemtype");
			_tstring sKeyValue = pElement->Attribute("value");
			CRegKey regKey;
			HKEY hRegRoot = StringToHKey(sRegRoot);
			if(regKey.Open(hRegRoot, sRegSub.c_str()) != ERROR_SUCCESS)
			{
				if(regKey.Create(hRegRoot, sRegSub.c_str()) != ERROR_SUCCESS)
				{
					return false;
				}
			}

			// transform %INSTALLDIR% %SYSTEMDIR%
			TranslatePath(sKeyValue, sInstallPath);

			if(sKeyType.CompareNoCase(_T("dword")) == 0)
			{
				regKey.SetDWORDValue(sKeyName.c_str(), _ttoi(sKeyValue.c_str()));
			}
			else
			{
				regKey.SetStringValue(sKeyName.c_str(), sKeyValue.c_str());
			}

			if(sKeyName.CompareNoCase(_T("INSTALL_PATH")) == 0)
			{
				if(!bDepend)
				{
					regKey.SetDWORDValue(_T("installed"), 1); // write install flag
				}
			}

			regKey.Close();

			pElement = pElement->NextSiblingElement();
		}
	}

	// write app registry
	if(pElement = pRegistry->FirstChildElement("appitems"))
	{
		pElement = pElement->FirstChildElement("item");
		while(pElement)
		{
			_tstring sRegRoot = pElement->Attribute("rootpath");
			_tstring sRegSub = pElement->Attribute("path");
			_tstring sKeyName = pElement->Attribute("itemname");
			_tstring sKeyType = pElement->Attribute("itemtype");
			_tstring sKeyValue = pElement->Attribute("value");
			CRegKey regKey;
			HKEY hRegRoot = StringToHKey(sRegRoot);
			if(regKey.Open(hRegRoot, sRegSub.c_str()) != ERROR_SUCCESS)
			{
				if(regKey.Create(hRegRoot, sRegSub.c_str()) != ERROR_SUCCESS)
				{
					return false;
				}
			}

			// transform %INSTALLDIR% %SYSTEMDIR%
			TranslatePath(sKeyValue, sInstallPath);

			if(sKeyType.CompareNoCase(_T("dword")) == 0)
			{
				regKey.SetDWORDValue(sKeyName.c_str(), _ttoi(sKeyValue.c_str()));
			}
			else
			{
				regKey.SetStringValue(sKeyName.c_str(), sKeyValue.c_str());
			}
			regKey.Close();

			pElement = pElement->NextSiblingElement();
		}
	}
	return true;
}

static bool WriteSilenceRegistry(_tstring sPath, _tstring sLan)
{
	_tstring sInstallPath = sPath.Left(sPath.ReverseFind(_T('\\')));

	if(GetFileAttributes(sPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	TiXmlDocument xmlDoc;
	TiXmlElement *pLanElement = NULL, *pRoot = NULL, *pRegistry, *pDependency;

	if(!xmlDoc.LoadFile(sPath.toNarrowString().c_str())
		|| !(pRoot = xmlDoc.RootElement()))
	{
		return false;
	}

	// check if the game has dependency application
	bool bDepend = false;
	pDependency = pRoot->FirstChildElement("dependency");
	if(pDependency->FirstChildElement("item"))
	{
		bDepend = true;
	}

	pRegistry = pRoot->FirstChildElement("registry");
	while(pRegistry)
	{
		_tstring sLanInFile = pRegistry->Attribute("language");

		if(sLanInFile.CompareNoCase(sLan) == 0)
		{
			pLanElement = pRegistry;
		}
		else
		{
			if(!WriteSingleLanguageRegistry(pRegistry, bDepend, sInstallPath))
			{
				return false;
			}
		}

		pRegistry = pRegistry->NextSiblingElement("registry");
	}

	// write specified language registry at the end
	if(pLanElement)
	{
		if(!WriteSingleLanguageRegistry(pLanElement, bDepend, sInstallPath))
		{
			return false;
		}
	}

	xmlDoc.Clear();

	return true;
}

bool SetPDLUncheckRegFlag(const _tstring& szPath, const _tstring& szId,bool bSetUncheck)
{
	if(GetFileAttributes(szPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	TiXmlDocument xmlDoc;
	TiXmlElement *pRoot = NULL;
	TiXmlElement *pRegistry = NULL;

	if(!xmlDoc.LoadFile(szPath.toNarrowString().c_str())
		|| !(pRoot = xmlDoc.RootElement()))
	{
		return false;
	}

	pRegistry = pRoot->FirstChildElement("registry");
	while(pRegistry)
	{
		_tstring sLanInFile = pRegistry->Attribute("language");
		CRegKey regKey;
		_tstring sRegSub;
		sRegSub.Format(2, _T("SOFTWARE\\Perfect World Entertainment\\Core\\%s%s"), 
			szId.c_str(), sLanInFile.MakeLower().c_str());

		if(regKey.Open(HKEY_LOCAL_MACHINE, sRegSub.c_str()) == ERROR_SUCCESS)
		{
			if (bSetUncheck)
				regKey.SetDWORDValue(_T("PDL_UNCHECKED"), 1);
			else
				regKey.SetDWORDValue(_T("PDL_UNCHECKED"), 0);

			regKey.Close();
		}

		pRegistry = pRegistry->NextSiblingElement("registry");
	}

	xmlDoc.Clear();

	return true;
}

static bool DeleteSilenceRegistry(_tstring sPath)
{
	if(GetFileAttributes(sPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	TiXmlDocument xmlDoc;
	TiXmlElement *pElement = NULL, *pRoot = NULL, *pRegistry, *pDependency;

	if(!xmlDoc.LoadFile(sPath.toNarrowString().c_str())
		|| !(pRoot = xmlDoc.RootElement()))
	{
		return false;
	}

	// check language
	pRegistry = pRoot->FirstChildElement("registry");

	// delete registry
	while(pRegistry)
	{
		_tstring sLan = pRegistry->Attribute("language");
		if(pElement = pRegistry->FirstChildElement("coreitems"))
		{
			pElement = pElement->FirstChildElement("item");
			while(pElement)
			{
				_tstring sRegRoot = pElement->Attribute("rootpath");
				_tstring sRegSub = pElement->Attribute("path");

				CRegKey regKey;
				HKEY hRegRoot = StringToHKey(sRegRoot);
				if(regKey.Open(hRegRoot, NULL) == ERROR_SUCCESS)
				{
					regKey.RecurseDeleteKey(sRegSub.c_str());
					break;
				}

				pElement = pElement->NextSiblingElement();
			}
		}
		if(pElement = pRegistry->FirstChildElement("appitems"))
		{
			pElement = pElement->FirstChildElement("item");
			while(pElement)
			{
				_tstring sRegRoot = pElement->Attribute("rootpath");
				_tstring sRegSub = pElement->Attribute("path");

				CRegKey regKey;
				HKEY hRegRoot = StringToHKey(sRegRoot);
				if(regKey.Open(hRegRoot, NULL) == ERROR_SUCCESS)
				{
					regKey.RecurseDeleteKey(sRegSub.c_str());
				}

				pElement = pElement->NextSiblingElement();
			}
		}
		pRegistry = pRegistry->NextSiblingElement("registry");
	}

	xmlDoc.Clear() ;

	return true;
}

bool WriteShortcutPath(_tstring sConfigPath, _tstring sDesktopPath, _tstring sStartMenuPath)
{
	if(GetFileAttributes(sConfigPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	TiXmlDocument xmlDoc;
	TiXmlElement *pElement = NULL, *pRoot = NULL;

	if(!xmlDoc.LoadFile(sConfigPath.toNarrowString().c_str()) || !(pRoot = xmlDoc.RootElement()))
	{
		return false;
	}

	pElement = pRoot->FirstChildElement("shortcut");
	if (!pElement)
	{
		pElement = new TiXmlElement("shortcut");
		pRoot->LinkEndChild(pElement);
	}

	pElement->SetAttribute("desktop", sDesktopPath.toUTF8().c_str());
	pElement->SetAttribute("startmenu", sStartMenuPath.toUTF8().c_str());
	xmlDoc.SaveFile(sConfigPath.toNarrowString().c_str());

	xmlDoc.Clear();
	return true;
}

bool DeleteShortcut(_tstring sPath)
{
	if(GetFileAttributes(sPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	TiXmlDocument xmlDoc;
	TiXmlElement *pElement = NULL, *pRoot = NULL;

	if(!xmlDoc.LoadFile(sPath.toNarrowString().c_str()) || !(pRoot = xmlDoc.RootElement()))
	{
		return false;
	}

	pElement = pRoot->FirstChildElement("shortcut");
	if (pElement)
	{
		_tstring sDesktopPath;
		sDesktopPath.fromUTF8(pElement->Attribute("desktop"));
		if(GetFileAttributes(sDesktopPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			DeleteFile(sDesktopPath.c_str());
		}
		_tstring sStartMenuPath;
		sStartMenuPath.fromUTF8(pElement->Attribute("startmenu"));
		if(GetFileAttributes(sStartMenuPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			DeleteFile(sStartMenuPath.c_str());
		}

		pRoot->RemoveChild(pElement);
		xmlDoc.SaveFile(sPath.toNarrowString().c_str());
	}
	xmlDoc.Clear();
	return true;
}