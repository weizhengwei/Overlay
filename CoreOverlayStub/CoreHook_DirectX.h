#pragma once


//////////////////////////////////////////////////////////////////////////
// dx hooked APIs
typedef HRESULT (WINAPI * FUNC_D3D11CreateDeviceAndSwapChain)(void *pAdapter, UINT DriverType, HMODULE Software, UINT Flags, CONST UINT* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
                                                              CONST void* pSwapChainDesc, void** ppSwapChain, void** ppDevice, UINT* pFeatureLevel, void** ppImmediateContext);


typedef LPVOID (WINAPI * FUNC_Direct3DCreate9)(UINT SDKVersion);
LPVOID WINAPI MyDirect3DCreate9(UINT SDKVersion);


typedef HRESULT (WINAPI * FUNC_Direct3DCreate9Ex)(UINT SDKVersion, LPVOID * ppD3D);
HRESULT WINAPI MyDirect3DCreate9Ex(UINT SDKVersion, LPVOID * ppD3D);


typedef LPVOID (WINAPI * FUNC_Direct3DCreate8)(UINT SDKVersion);
LPVOID WINAPI MyDirect3DCreate8(UINT SDKVersion);



typedef HRESULT (STDMETHODCALLTYPE * FUNC_Present)(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect, HWND hDestWindowOverride, LPVOID pRsv);
HRESULT STDMETHODCALLTYPE MyPresent_8(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
                                    HWND hDestWindowOverride, LPVOID pRsv);

HRESULT STDMETHODCALLTYPE MyPresent_9(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
                                    HWND hDestWindowOverride, LPVOID pRsv);



typedef HRESULT (STDMETHODCALLTYPE * FUNC_SwapChainPresent)(LPVOID pThis, UINT SyncInterval, UINT Flags);
HRESULT STDMETHODCALLTYPE MySwapChainPresent(LPVOID pThis, UINT SyncInterval, UINT Flags);

typedef ULONG (STDMETHODCALLTYPE * FUNC_DX9SwapChainPresent)(LPVOID pThis, const RECT* pSourceRect, const RECT* pDestRect, 
                                                             HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags );
ULONG STDMETHODCALLTYPE MyDX9SwapChainPresent(LPVOID pThis, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags);


typedef ULONG (STDMETHODCALLTYPE * FUNC_SwapChainResizeBuffers)(LPVOID pThis, UINT BufferCount, UINT Width, UINT Height,UINT NewFormat,UINT SwapChainFlags );                                                              
ULONG STDMETHODCALLTYPE MySwapChainResizeBuffers(LPVOID pThis, UINT BufferCount, UINT Width, UINT Height,UINT NewFormat,UINT SwapChainFlags );      


typedef ULONG (STDMETHODCALLTYPE * FUNC_SetFullscreenState)(LPVOID pThis, BOOL Fullscreen, void* pTarget);                                                              
ULONG STDMETHODCALLTYPE MySetFullscreenState(LPVOID pThis, BOOL Fullscreen, void* pTarget);  

typedef HRESULT (STDMETHODCALLTYPE * FUNC_Reset)(LPVOID pThis, LPVOID PresentParameters);
HRESULT STDMETHODCALLTYPE MyReset_8(LPVOID pThis, LPVOID PresentParameters);

HRESULT STDMETHODCALLTYPE MyReset_9(LPVOID pThis, LPVOID PresentParameters);

typedef HRESULT (STDMETHODCALLTYPE * FUNC_PresentEx)(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect, HWND hDestWindowOverride, LPVOID, DWORD);
HRESULT STDMETHODCALLTYPE MyPresentEx(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
                                      HWND hDestWindowOverride, LPVOID pRsv, DWORD dwRsv);


typedef HRESULT (STDMETHODCALLTYPE * FUNC_ResetEx)(LPVOID pThis, LPVOID PresentParameters, LPVOID);
HRESULT STDMETHODCALLTYPE MyResetEx(LPVOID pThis, LPVOID PresentParameters, LPVOID pRsv);


typedef ULONG (STDMETHODCALLTYPE * FUNC_Release)(LPVOID pThis);
ULONG STDMETHODCALLTYPE MyRelease_8(LPVOID pThis);

ULONG STDMETHODCALLTYPE MyRelease_9(LPVOID pThis);

ULONG STDMETHODCALLTYPE MyRelease_11(LPVOID pThis);



typedef HRESULT (WINAPI * FUNC_D3DX11CompileFromMemory)(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, void* pDefines, void* pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags1, UINT Flags2, void * pPump, void** ppShader, void** ppErrorMsgs, HRESULT* pHResult);
HRESULT WINAPI D3DX11CompileFromMemory(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, void* pDefines, void* pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags1, UINT Flags2, void * pPump, void** ppShader, void** ppErrorMsgs, HRESULT* pHResult);
