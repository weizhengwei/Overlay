#pragma once
#include "constant.h"
#include "tstring/tstring.h"

using namespace String;

/*
Note: execute third party app config file

Param:
[in]      sPath	specify config file path, like "D:\Downloads\1000_en\appinstallconfig.xml"
[in]      nCmd	0 install, 1 uninstall

Return: true if succeeded, otherwise false.
*/
bool ExecuteInstallConfig(_tstring sPath, int nCmd, _tstring sLan = TSTRING_ABBR_LANGUAGE_EN);

bool SetPDLUncheckRegFlag(const _tstring& szPath, const _tstring& szId, bool bSetUncheck);

bool WriteShortcutPath(_tstring sConfigPath, _tstring sDesktopPath, _tstring sStartMenuPath);
bool DeleteShortcut(_tstring sPath);