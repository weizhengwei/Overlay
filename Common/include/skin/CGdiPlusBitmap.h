#pragma once

class CGdiPlusBitmap
{
public:
	Gdiplus::Bitmap* m_pBitmap;

public:
	CGdiPlusBitmap()							{ m_pBitmap = NULL; }
	CGdiPlusBitmap(LPCWSTR pFile)				{ m_pBitmap = NULL; Load(pFile); }
	virtual ~CGdiPlusBitmap()					{ Empty(); }

	void Empty()								{ delete m_pBitmap; m_pBitmap = NULL; }

	bool Load(LPCWSTR pFile)
	{
		Empty();
		m_pBitmap = Gdiplus::Bitmap::FromFile(pFile);
		return m_pBitmap->GetLastStatus() == Gdiplus::Ok;
	}

	operator Gdiplus::Bitmap*() const			{ return m_pBitmap; }
};


class CGdiPlusBitmapResource : public CGdiPlusBitmap
{
protected:
	HGLOBAL m_hBuffer;

public:
	CGdiPlusBitmapResource()					{ m_hBuffer = NULL; }
	CGdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL)
												{ m_hBuffer = NULL; Load(pName, pType, hInst); }
	CGdiPlusBitmapResource(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL)
												{ m_hBuffer = NULL; Load(id, pType, hInst); }
	CGdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst = NULL)
												{ m_hBuffer = NULL; Load(id, type, hInst); }
	virtual ~CGdiPlusBitmapResource()			{ Empty(); }

	void Empty();

	bool Load(LPCTSTR pName, LPCTSTR pType /*= RT_RCDATA*/, HMODULE hInst = NULL);
	bool Load(LPCTSTR pFilePathName) ;
	bool Load(UINT id, LPCTSTR pType /*= RT_RCDATA*/, HMODULE hInst = NULL)
												{ return Load(MAKEINTRESOURCE(id), pType, hInst); }
	bool Load(UINT id, UINT type, HMODULE hInst = NULL)
												{ return Load(MAKEINTRESOURCE(id), MAKEINTRESOURCE(type), hInst); }
};

inline
void CGdiPlusBitmapResource::Empty()
{
	CGdiPlusBitmap::Empty();
	if (m_hBuffer)
	{
		::GlobalUnlock(m_hBuffer);
		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	} 
}

inline
bool CGdiPlusBitmapResource::Load(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
	Empty();

	HRSRC hResource = ::FindResource(hInst, pName, pType);
	if (!hResource)
		return false;
	
	DWORD imageSize = ::SizeofResource(hInst, hResource);
	if (!imageSize)
		return false;

	const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
	if (!pResourceData)
		return false;

	m_hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (m_hBuffer)
	{
		void* pBuffer = ::GlobalLock(m_hBuffer);
		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;
			if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK)
			{
				m_pBitmap = Gdiplus::Bitmap::FromStream(pStream);
				pStream->Release();
				if (m_pBitmap)
				{ 
					if (m_pBitmap->GetLastStatus() == Gdiplus::Ok)
						return true;

					delete m_pBitmap;
					m_pBitmap = NULL;
				}
			}
			::GlobalUnlock(m_hBuffer);
		}
		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
	return false;
}

inline
bool CGdiPlusBitmapResource::Load(LPCTSTR pFilePathName)
{
	//Empty();

	//HRESULT  hResult;
	//IStream  *pIStream=NULL;

	/////set listitem background
	//hResult=SHCreateStreamOnFile(pFilePathName, STGM_READ| STGM_SHARE_DENY_NONE, &pIStream);
	//if(S_OK!=hResult)
	//{
	//	SetLastError(HRESULT_CODE(hResult)) ;
	//	return FALSE ;
	//}

	//m_pBitmap = new Bitmap(pIStream);
	//if(!m_pBitmap)
	//{
	//	return false ;
	//}
	//pIStream->Release( );
	//pIStream=NULL;

	//return true ;
	Empty();

	WCHAR wszZipFile[MAX_PATH+1] = {0} ;
	TCHAR szFilePathName[MAX_PATH] = {0} ;
	_tcscpy(szFilePathName, pFilePathName) ;

#ifndef _UNICODE
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szFilePathName, MAX_PATH+1, (LPWSTR)wszZipFile, MAX_PATH+1);
#else
	_tcscpy(wszZipFile, szFilePathName) ;
#endif

	m_pBitmap = Gdiplus::Bitmap::FromFile(wszZipFile);
	return m_pBitmap->GetLastStatus() == Gdiplus::Ok;
}
