/****************************** Module Header ******************************\
* Module Name:  ServiceInstaller.cpp
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* The file implements functions that install and uninstall the service.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include <tchar.h>
#include <aclapi.h>
#include "data/ServiceMgr.h"

//
//   FUNCTION: AdjustServiceUserPermission
//
//   PURPOSE: Adjust the service access permission so that the common user can start/stop the service.
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT AdjustServiceUserPermission(SC_HANDLE schService)
{	
	EXPLICIT_ACCESS      ea;
	SECURITY_DESCRIPTOR  sd;
	PSECURITY_DESCRIPTOR psd            = NULL;
	PACL                 pacl           = NULL;
	PACL                 pNewAcl        = NULL;
	BOOL                 bDaclPresent   = FALSE;
	BOOL                 bDaclDefaulted = FALSE;
	DWORD                dwError        = 0;
	DWORD                dwSize         = 0;
	DWORD                dwBytesNeeded  = 0;
	HRESULT              hRes            = S_FALSE;

	__try
	{
		// Get the current security descriptor.
		if (!QueryServiceObjectSecurity(schService,DACL_SECURITY_INFORMATION,&psd,0,&dwBytesNeeded))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				dwSize = dwBytesNeeded;
				psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwSize);
				if (psd == NULL)
				{
					// Note: HeapAlloc does not support GetLastError.
					//wprintf(L"HeapAlloc failed\n");
					__leave;
				}

				if (!QueryServiceObjectSecurity(schService,DACL_SECURITY_INFORMATION,psd,dwSize,&dwBytesNeeded))
				{
					//wprintf(L"QueryServiceObjectSecurity failed (%d)\n", GetLastError());
					__leave;
				}
			}
			else 
			{
				//wprintf(L"QueryServiceObjectSecurity failed (%d)\n", GetLastError());
				__leave;
			}
		}

		// Get the DACL.
		if (!GetSecurityDescriptorDacl(psd,&bDaclPresent,&pacl,&bDaclDefaulted))
		{
			//wprintf(L"GetSecurityDescriptorDacl failed(%d)\n", GetLastError());
			__leave;
		}

		// Build the ACE.
		BuildExplicitAccessWithName(&ea,TEXT("users"),SERVICE_START|SERVICE_STOP|READ_CONTROL,SET_ACCESS,NO_INHERITANCE);

		dwError = SetEntriesInAcl(1,&ea,pacl,&pNewAcl);
		if (dwError != ERROR_SUCCESS)
		{
			//wprintf(L"SetEntriesInAcl failed(%d)\n",dwError);
			__leave;
		}

		// Initialize a new security descriptor.
		if (!InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION))
		{
			//wprintf(L"InitializeSecurityDescriptor failed(%d)\n", GetLastError());
			__leave;
		}

		// Set the new DACL in the security descriptor.
		if (!SetSecurityDescriptorDacl(&sd,TRUE,pNewAcl,FALSE))
		{
			//wprintf(L"SetSecurityDescriptorDacl failed(%d)\n",GetLastError());
			__leave;
		}

		// Set the new DACL for the service object.
		if (!SetServiceObjectSecurity(schService,DACL_SECURITY_INFORMATION,&sd))
		{
			//wprintf(L"SetServiceObjectSecurity failed(%d)\n", GetLastError());
			__leave;
		}

		hRes = S_OK;
	}
	__finally
	{
		if(NULL != pNewAcl)
		{
			LocalFree((HLOCAL)pNewAcl);
		}
		if(NULL != psd)
		{
			HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
		}
	}

	return hRes;
}

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
                    PWSTR pszPassword)
{
    wchar_t   szPath[MAX_PATH];
    SC_HANDLE schSCManager = NULL;
    SC_HANDLE schService   = NULL;
	HRESULT   hRes         = S_FALSE;

	__try
	{
		if (GetModuleFileName(NULL,szPath,ARRAYSIZE(szPath)) == 0)
		{
			//wprintf(L"GetModuleFileName failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT|SC_MANAGER_CREATE_SERVICE);
		if (schSCManager == NULL)
		{
			//wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Install the service into SCM by calling CreateService
		schService = CreateService(
			schSCManager,                   // SCManager database
			pszServiceName,                 // Name of service
			pszDisplayName,                 // Name to display
			SERVICE_QUERY_STATUS|READ_CONTROL|WRITE_DAC,// Desired access
			SERVICE_WIN32_OWN_PROCESS,      // Service type
			dwStartType,                    // Service start type
			SERVICE_ERROR_NORMAL,           // Error control type
			szPath,                         // Service's binary
			NULL,                           // No load ordering group
			NULL,                           // No tag identifier
			pszDependencies,                // Dependencies
			pszAccount,                     // Service running account
			pszPassword                     // Password of the account
			);
		if (schService == NULL)
		{
			//wprintf(L"CreateService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}
        if (S_OK != ChangeServiceDesc(pszServiceName,pszDesc))
        {
			//wprintf(L"ChangeServiceDesc failed w/err 0x%08lx\n", GetLastError());
			__leave;
        }
		// Adjust the service permission to let common user can start and stop the service.
		if (S_OK != AdjustServiceUserPermission(schService))
		{
			//wprintf(L"AdjustServiceUserPermission failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

        hRes = S_OK;
	}
	__finally
	{
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}
  
	return hRes;
}

//
//   FUNCTION: UninstallService
//
//   PURPOSE: Stop and remove the service from the local service control 
//   manager database.
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT UninstallService(PWSTR pszServiceName)
{
    SC_HANDLE      schSCManager = NULL;
    SC_HANDLE      schService   = NULL;
    SERVICE_STATUS ssSvcStatus  = {};
	HRESULT        hRes         = S_FALSE;

	__try
	{
		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
		if (schSCManager == NULL)
		{
			//wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Open the service with delete, stop, and query status permissions
		schService = OpenService(schSCManager,pszServiceName,SERVICE_STOP|SERVICE_QUERY_STATUS|DELETE);
		if (schService == NULL)
		{
			//wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Try to stop the service
		if (ControlService(schService,SERVICE_CONTROL_STOP,&ssSvcStatus))
		{
			Sleep(500);
			while (QueryServiceStatus(schService, &ssSvcStatus))
			{
				if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
				{
					Sleep(500);
				}
				else
				{
					break;
				}
			}

			if (ssSvcStatus.dwCurrentState != SERVICE_STOPPED)
			{
				//wprintf(L"\n%s failed to stop.\n", pszServiceName);
			}
		}

		// Now remove the service by calling DeleteService.
		if (!DeleteService(schService))
		{
			//wprintf(L"DeleteService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

        hRes = S_OK;
	}
	__finally
	{
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}

    return hRes;
}

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
HRESULT StartServiceWithName(PWSTR pszServiceName,DWORD dwArgc, LPCWSTR *pszArgv)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	HRESULT hRes = S_FALSE;

	__try
	{
		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL, NULL,SC_MANAGER_CONNECT);
		if (schSCManager == NULL)
		{
			//wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Open the service with start,and query status permissions
		schService = OpenService(schSCManager,pszServiceName,SERVICE_START|SERVICE_QUERY_STATUS);
		if (schService == NULL)
		{
			//wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}
        if (!StartService(schService,dwArgc,pszArgv))
        {
			//wprintf(L"StartService failed w/err 0x%08lx,dwArgc = 0x%08lx,pszArgv[0] =%s\n", GetLastError(),dwArgc,pszArgv[0]);
			__leave;
        }
        hRes = S_OK;
	}
	__finally
	{
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}

	return hRes;
}

//
//   FUNCTION: StopService
//
//   PURPOSE: Stop the service from the local service control 
//   manager database.
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT StopService(PWSTR pszServiceName)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {};
	HRESULT hRes = S_FALSE;

	__try
	{
		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL, NULL,SC_MANAGER_CONNECT);
		if (schSCManager == NULL)
		{
			//wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Open the service with stop,and query status permissions
		schService = OpenService(schSCManager,pszServiceName,SERVICE_STOP|SERVICE_QUERY_STATUS);
		if (schService == NULL)
		{
			//wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Try to stop the service
		if (!ControlService(schService,SERVICE_CONTROL_STOP,&ssSvcStatus))
		{
			//wprintf(L"ControlService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		Sleep(500);
		while (QueryServiceStatus(schService, &ssSvcStatus))
		{
			if (ssSvcStatus.dwCurrentState != SERVICE_STOP_PENDING)
			{
				break;
			}
			Sleep(500);
		}

		if (ssSvcStatus.dwCurrentState != SERVICE_STOPPED)
		{
			//wprintf(L"\n%s failed to stop.\n", pszServiceName);
			__leave;
		}

		hRes = S_OK;
	}
	__finally
	{
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}

	return hRes;
}

//
//   FUNCTION: RestoreServiceConfig
//
//   PURPOSE: restore the service config. 
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//
HRESULT RestoreServiceConfig(PWSTR pszServiceName,DWORD dwServiceType,DWORD dwStartType,LPCTSTR lpServiceStartName,LPCTSTR lpPassword,LPCTSTR lpDisplayName)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	HRESULT hRes = S_FALSE;

	__try
	{
		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL, NULL,SC_MANAGER_CONNECT);
		if (schSCManager == NULL)
		{
			//wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Open the service with start,and query status permissions
		schService = OpenService(schSCManager,pszServiceName,SERVICE_CHANGE_CONFIG|SERVICE_QUERY_STATUS);
		if (schService == NULL)
		{
			//wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}
		if (!ChangeServiceConfig(schService,dwServiceType,dwStartType,SERVICE_ERROR_NORMAL,NULL,NULL,NULL,TEXT(""),lpServiceStartName,lpPassword,lpDisplayName))
		{
			//wprintf(L"StartService failed w/err 0x%08lx,dwArgc = 0x%08lx,pszArgv[0] =%s\n", GetLastError(),dwArgc,pszArgv[0]);
			__leave;
		}
		hRes = S_OK;
	}
	__finally
	{
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}

	return hRes;
}

//
//   FUNCTION: ChangeServiceDesc
//
//   PURPOSE: change the service descreiption. 
//
//   PARAMETERS: 
//   * pszServiceName - the name of the service to be removed.
//   * lpszDesc - the description string of the service to be changed.
//
HRESULT ChangeServiceDesc(LPWSTR pszServiceName,LPWSTR lpszDesc)
{
	SC_HANDLE schSCManager =NULL;
	SC_HANDLE schService =NULL;
	HRESULT hRes = S_FALSE;
	SERVICE_DESCRIPTION sd;

	__try
	{
		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
		if (schSCManager == NULL)
		{
			//printf("OpenSCManager failed (%d)\n", GetLastError());
			__leave;
		}

		// Open the service with change config and query status permissions
		schService = OpenService(schSCManager,pszServiceName,SERVICE_CHANGE_CONFIG|SERVICE_QUERY_STATUS); 
		if (schService == NULL)
		{
			//printf("OpenService failed (%d)\n", GetLastError());
			__leave;
		}

		// Change the service description.
		sd.lpDescription = lpszDesc;
		if(!ChangeServiceConfig2(schService,SERVICE_CONFIG_DESCRIPTION,&sd))
		{
			//printf("ChangeServiceConfig2 failed\n");
			__leave;
		}
		hRes = S_OK;
	}
	__finally
	{
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}

	return hRes;
}

HRESULT IsServiceRunAsLocalSystem(PWSTR pszServiceName)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	LPQUERY_SERVICE_CONFIG lpsc = NULL; 
	DWORD dwBytesNeeded=0, cbBufSize=0, dwError=0; 
	HRESULT hRes = S_FALSE;

	__try
	{
		// Open the local default service control manager database
		schSCManager = OpenSCManager(NULL, NULL,SC_MANAGER_CONNECT);
		if (schSCManager == NULL)
		{
			//wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}

		// Open the service with start,and query status permissions
		schService = OpenService(schSCManager,pszServiceName,SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS);
		if (schService == NULL)
		{
			//wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
			__leave;
		}
		//query service config.
		if( !QueryServiceConfig(schService, NULL, 0, &dwBytesNeeded))
		{
			dwError = GetLastError();
			if( ERROR_INSUFFICIENT_BUFFER == dwError )
			{
				cbBufSize = dwBytesNeeded;
				lpsc = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, cbBufSize);
			}
			else
			{
				//wprintf(L"QueryServiceConfig failed (%d)", dwError);
				__leave;
			}
		}

		if( !QueryServiceConfig(schService,lpsc,cbBufSize,&dwBytesNeeded) ) 
		{
			//wprintf(L"QueryServiceConfig failed (%d)", GetLastError());
			__leave;
		}
		if (_tcsicmp(_T("LocalSystem"),lpsc->lpServiceStartName))
		{
			//wprintf(L"QueryServiceConfig service run as account:%s", lpsc->lpServiceStartName);
			__leave;
		}
		hRes = S_OK;
	}
	__finally
	{
		if (lpsc)
		{
			LocalFree(lpsc);
			lpsc=NULL;
		}
		if (schService)
		{
			CloseServiceHandle(schService);
			schService = NULL;
		}
		if (schSCManager)
		{
			CloseServiceHandle(schSCManager);
			schSCManager = NULL;
		}
	}

	return hRes;
}