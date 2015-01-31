/****************************** Module Header ******************************\
* Module Name:  ServiceInstaller.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* The file declares functions that install and uninstall the service.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

//
//   FUNCTION: InstallService
//
//   PURPOSE: Install the current application as a service to the local 
//   service control manager database.
//
//   PARAMETERS:
//   * pszServiceName - the name of the service to be installed
//   * pszDisplayName - the display name of the service
//   * dwStartType - the service start option. This parameter can be one of 
//     the following values: SERVICE_AUTO_START, SERVICE_BOOT_START, 
//     SERVICE_DEMAND_START, SERVICE_DISABLED, SERVICE_SYSTEM_START.
//   * pszDependencies - a pointer to a double null-terminated array of null-
//     separated names of services or load ordering groups that the system 
//     must start before this service.
//   * pszDesc - the description of the service.
//   * pszAccount - the name of the account under which the service runs.
//   * pszPassword - the password to the account name.
//
HRESULT InstallService(PWSTR pszServiceName, 
                    PWSTR pszDisplayName, 
                    DWORD dwStartType,
                    PWSTR pszDependencies, 
					PWSTR pszDesc,
                    PWSTR pszAccount, 
                    PWSTR pszPassword);


//
//   FUNCTION: UninstallService
//
//   PURPOSE: Stop and remove the service from the local service control 
//   manager database.
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT UninstallService(PWSTR pszServiceName);

//
//   FUNCTION: StartService
//
//   PURPOSE: Stop and remove the service from the local service control 
//   manager database.
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service.
//   * dwArgc - the count of parameters passed to the Service.
//   * pszArgv - the parameters of the service to be passed.
//
HRESULT StartServiceWithName(PWSTR pszServiceName,DWORD dwArgc, LPCWSTR *pszArgv);

//
//   FUNCTION: StopService
//
//   PURPOSE: Stop the service from the local service control 
//   manager database.
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT StopService(PWSTR pszServiceName);

//
//   FUNCTION: RestoreServiceConfig
//
//   PURPOSE: restore the service config. 
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT RestoreServiceConfig(PWSTR pszServiceName,DWORD dwServiceType,DWORD dwStartType,LPCTSTR lpServiceStartName,LPCTSTR lpPassword,LPCTSTR lpDisplayName);

//
//   FUNCTION: IsServiceRunAsLocalSystem
//
//   PURPOSE: check the service is running as local sytem account. 
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT IsServiceRunAsLocalSystem(PWSTR pszServiceName);

//
//   FUNCTION: ChangeServiceDesc
//
//   PURPOSE: change the service descreiption. 
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//   * lpszDesc - the description string of the service to be changed.
//
HRESULT ChangeServiceDesc(LPWSTR pszServiceName,LPWSTR lpszDesc);