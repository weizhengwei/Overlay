#include <tchar.h>
#include "data/Overlay.h"
#include "data/DataPool.h"
#include "constant.h"

static PIMAGE_SECTION_HEADER ImageRVA2Section(PIMAGE_NT_HEADERS pNtHeader,DWORD dwRVA) ;
static DWORD RVA2Offset(PCHAR pImageBase,DWORD dwRVA) ;

bool COverlay::DisplayNotification(LPCTSTR pszLink, LPCTSTR lpszId, LPCTSTR pszTitle, LPCTSTR pszText, RECT rect)
{
	typedef void (*DISPLAYNOTIFICATION)(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, RECT) ;

	TCHAR szText[MAX_NOTIFY_TEXT] = {0} ;
	TCHAR szTitle[MAX_NOTIFY_TITLE] = {0} ;
	if (_tcslen(pszText) >= MAX_NOTIFY_TEXT)
	{
		memcpy(szText, pszText, (MAX_NOTIFY_TEXT-1)*sizeof(TCHAR)) ;
	}
	else
	{
		_tcscpy(szText, pszText) ;
	}

	if (_tcslen(pszTitle) >= MAX_NOTIFY_TITLE)
	{
		memcpy(szTitle, pszTitle, (MAX_NOTIFY_TITLE-1)*sizeof(TCHAR)) ;
	}
	else
	{
		_tcscpy(szTitle, pszTitle) ;
	}

	vector<_tstring> vecNotifyDllList = theDataPool.GetNotifyDllList() ;
	
	for (int i = 0 ; i < vecNotifyDllList.size() ; ++ i)
	{
		_tstring sNotifyDllPath = theDataPool.GetBaseDir() + vecNotifyDllList[i] ;

		HINSTANCE hInst = LoadLibrary(sNotifyDllPath.c_str()) ;
		if (hInst == NULL)
		{
			return false ;
		}

		DISPLAYNOTIFICATION pDisplayNotification = (DISPLAYNOTIFICATION)GetProcAddress(hInst, "DisplayNotification") ;
		if (pDisplayNotification == NULL)
		{
			FreeLibrary(hInst) ;
			return false ;
		}

		pDisplayNotification((LPCTSTR)pszLink, (LPCTSTR)lpszId, (LPCTSTR)szTitle, (LPCTSTR)szText, rect) ;

		FreeLibrary(hInst) ;
	}
	return true ;
}

bool COverlay::DestroyNotification()
{
	typedef void (*DESTROYNOTIFICATION)() ;

	vector<_tstring> vecNotifyDllList = theDataPool.GetNotifyDllList() ;
	for (int i = 0 ; i < vecNotifyDllList.size() ; ++ i)
	{
		_tstring sNotifyDllPath = theDataPool.GetBaseDir() + vecNotifyDllList[i] ;

		HINSTANCE hInst = LoadLibrary(sNotifyDllPath.c_str()) ;
		if (hInst == NULL)
		{
			return false ;
		}

		DESTROYNOTIFICATION pDestroyNotification = (DESTROYNOTIFICATION)GetProcAddress(hInst, "DestroyNotification") ;
		if (pDestroyNotification == NULL)
		{
			FreeLibrary(hInst) ;
			return false ;
		}

		pDestroyNotification() ;

		FreeLibrary(hInst) ;
	}
	return true ;
}

D3D_VERSION COverlay::GetD3DVersion(LPCTSTR lpszProcessPathName)
{
	D3D_VERSION nVersion = NONE ;

	if (lpszProcessPathName == NULL)
	{
		return NONE ;
	}

	HANDLE hFile  =CreateFile(lpszProcessPathName,GENERIC_READ,FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return NONE ;
	}

	HANDLE 		hMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL);
	PCHAR		pHintName;
	PCHAR		pDllName;

	DWORD		dwAPIaddress;
	PCHAR		pAPIName;
	WORD		Hint;
	PCHAR		pImageBase;

	PVOID	    pMapBase = MapViewOfFile(hMap,FILE_MAP_READ,0,0,0);
	pImageBase = (PCHAR)pMapBase;
	if (NULL == pImageBase)
	{
		CloseHandle(hFile) ;
		return NONE ;
	}

	PIMAGE_IMPORT_DESCRIPTOR	pImportDescriptor;
	PIMAGE_THUNK_DATA			pThunkData;
	PIMAGE_DOS_HEADER	pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;

	PIMAGE_NT_HEADERS	pNtHeader = (PIMAGE_NT_HEADERS)(pImageBase + pDosHeader->e_lfanew);

	DWORD 	dwImportDirectory = RVA2Offset( pImageBase, pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if(dwImportDirectory == 0)
	{
		CloseHandle(hFile) ;
		return NONE;
	}

	pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(pImageBase + dwImportDirectory);
	while(pImportDescriptor->Name != 0)			
	{
		pHintName = pImageBase;

		if(pImportDescriptor->OriginalFirstThunk != 0)
		{
			pHintName += RVA2Offset( pImageBase, pImportDescriptor->OriginalFirstThunk);
		}
		else
		{
			pHintName += RVA2Offset( pImageBase, pImportDescriptor->FirstThunk);
		}

		pDllName = pImageBase + RVA2Offset(pImageBase, pImportDescriptor->Name);
		if (!stricmp(pDllName, "d3d8.dll"))
		{
			nVersion = D3D8 ;
		}
		else if (!stricmp(pDllName, "d3d9.dll"))
		{
			nVersion = D3D9 ;
		}
		else if (!stricmp(pDllName, "d3d10.dll"))
		{
			nVersion = D3D10 ;
		}
		else if (!stricmp(pDllName, "d3d11.dll"))
		{
			nVersion = D3D11 ;
		}

		if (nVersion != NONE) 
		{
			break ;
		}

		pImportDescriptor++;
	}

	UnmapViewOfFile(pMapBase);
	CloseHandle(hMap);
	CloseHandle(hFile);

	return nVersion ;
}

static PIMAGE_SECTION_HEADER ImageRVA2Section(PIMAGE_NT_HEADERS pNtHeader,DWORD dwRVA)
{
	int i;
	PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((PCHAR(pNtHeader))+ sizeof(IMAGE_NT_HEADERS));

	for(i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if((pSectionHeader->VirtualAddress)&&(dwRVA <= (pSectionHeader->VirtualAddress + pSectionHeader->SizeOfRawData)))
		{
			return pSectionHeader;
		}
		pSectionHeader++;
	}
	return (NULL);
}

static DWORD RVA2Offset(PCHAR pImageBase,DWORD dwRVA)
{
	DWORD dwOffset;
	PIMAGE_DOS_HEADER	pDosHeader;
	PIMAGE_SECTION_HEADER	pSectionHeader;
	PIMAGE_NT_HEADERS pNtHeader;

	pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;
	pNtHeader = (PIMAGE_NT_HEADERS)(pImageBase + pDosHeader->e_lfanew);
	pSectionHeader = ImageRVA2Section(pNtHeader,dwRVA);

	if(pSectionHeader == NULL)
	{
		return 0;
	}

	dwOffset = dwRVA + pSectionHeader->PointerToRawData -pSectionHeader->VirtualAddress;
	return (dwOffset);
}