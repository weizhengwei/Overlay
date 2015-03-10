#include "stdafx.h"
#include "CoreHook_DirectX.h"
#include "CoreHook.h"
#include "CDXWrapper.h"
#include "MsgSwitchboard.h"
#include "CoreHook_Unity.h"
#include "CoreHook_Imp.h"
#include "Render/CoreHook_Render.h"

#define FAKE_CMDLINE_LENGTH		4096

extern Hook g_Hook;
//////////////////////////////////////////////////////////////////////////
// detoured functions
// switch off the complier optimization to assure standard function stack.
#pragma optimize("", off)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// dx hooked APIs
extern FUNC_Present g_pPresent_8;
HRESULT STDMETHODCALLTYPE MyPresent_8(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
                                    HWND hDestWindowOverride, LPVOID pRsv)
{
    __try
    {
        g_CoreHook.RenderOverlay(DX_D3D8, pThis);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

    }	
    return (*g_pPresent_8)(pThis, pSourceRect,pDestRect, hDestWindowOverride, pRsv);
}

extern FUNC_Present g_pPresent_9;
HRESULT STDMETHODCALLTYPE MyPresent_9(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
                                      HWND hDestWindowOverride, LPVOID pRsv)
{
    __try
    {
        g_CoreHook.RenderOverlay(DX_D3D9, pThis);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

    }	
    return (*g_pPresent_9)(pThis, pSourceRect,pDestRect, hDestWindowOverride, pRsv);
}

extern FUNC_SwapChainPresent g_pSwapChainPresent;
HRESULT STDMETHODCALLTYPE MySwapChainPresent(LPVOID pThis, UINT SyncInterval, UINT Flags)
{
    __try
    {
        g_CoreHook.RenderOverlay(DX_D3D11, pThis);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

    }	
    return (*g_pSwapChainPresent)(pThis, SyncInterval, Flags);
}


extern FUNC_DX9SwapChainPresent g_pDx9SwapChainPresent;
ULONG STDMETHODCALLTYPE MyDX9SwapChainPresent(LPVOID pThis, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags)
{
    LPVOID pDevice = NULL;
    CDXSwapChian9::GetDevice(pThis, &pDevice);
    if(pDevice)
    {
        g_CoreHook.RenderOverlay(DX_D3D9, pDevice);
        CUnknown::Release(pDevice);
    }
    return (*g_pDx9SwapChainPresent)(pThis, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}


extern FUNC_SwapChainResizeBuffers g_pSwapChainResizeBuffers;
ULONG STDMETHODCALLTYPE MySwapChainResizeBuffers(LPVOID pThis, UINT BufferCount, UINT Width, UINT Height,UINT NewFormat,UINT SwapChainFlags )
{
    GLOG(_T("resize buffer"));
    g_CoreHook.InitMsgHook();
    return (*g_pSwapChainResizeBuffers)(pThis, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}


extern FUNC_SetFullscreenState g_pSetFullscreenState;
ULONG STDMETHODCALLTYPE MySetFullscreenState(LPVOID pThis, BOOL Fullscreen, void* pTarget)
{
    GLOG(_T("MySetFullscreenState buffer"));
    g_CoreHook.InitMsgHook();
    return (*g_pSetFullscreenState)(pThis, Fullscreen, pTarget);
}


extern FUNC_Reset g_pReset_8;
HRESULT STDMETHODCALLTYPE MyReset_8(LPVOID pThis, LPVOID PresentParameters)
{
    HRESULT hRet = (*g_pReset_8)(pThis, PresentParameters);
//    g_CoreHook.m_nWidth = *(int *)PresentParameters;
//     g_CoreHook.m_nHeight = *((int *)PresentParameters + 1);
// 
    g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentParameters + 7);

//    GLOG(_T("MyReset_8 Width:%d, Height:%d, bFullScreen:%d"), g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
    return hRet;
}

extern FUNC_Reset g_pReset_9;
HRESULT STDMETHODCALLTYPE MyReset_9(LPVOID pThis, LPVOID PresentParameters)
{

        g_CoreHook.UninitOverlay();

    HRESULT hRet = (*g_pReset_9)(pThis, PresentParameters);
//     g_CoreHook.m_nWidth = *(int *)PresentParameters;
//     g_CoreHook.m_nHeight = *((int *)PresentParameters + 1);

//        g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentParameters + 8);

    GLOG(_T("Width:%d, Height:%d, bFullScreen:%d"), g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
    return hRet;
}


extern FUNC_PresentEx g_pPresentEx;
HRESULT STDMETHODCALLTYPE MyPresentEx(LPVOID pThis, const RECT* pSourceRect,const RECT* pDestRect,
                                      HWND hDestWindowOverride, LPVOID pRsv, DWORD dwRsv)
{
    __try
    {
        g_CoreHook.RenderOverlay(DX_D3D9EX, pThis);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

    }	
    return (*g_pPresentEx)(pThis, pSourceRect, pDestRect, hDestWindowOverride, pRsv, dwRsv);
}

extern FUNC_ResetEx g_pResetEx;
HRESULT STDMETHODCALLTYPE MyResetEx(LPVOID pThis, LPVOID PresentParameters, LPVOID pRsv)
{
    g_CoreHook.UninitOverlay();
    HRESULT hRet = (*g_pResetEx)(pThis, PresentParameters, pRsv);
    //g_CoreHook.m_nWidth = *(int *)PresentParameters;
    //g_CoreHook.m_nHeight = *((int *)PresentParameters + 1);
    g_CoreHook.m_smMgr.GetHeader()->bFullScreen = !*((int *)PresentParameters + 8);
    GLOG(_T("Width:%d, Height:%d, bFullScreen:%d"), g_CoreHook.m_nWidth, g_CoreHook.m_nHeight, g_CoreHook.m_smMgr.GetHeader()->bFullScreen);
    return hRet;
}

//////////////////////////////////////////////////////////////////////////
// maintain a reference count to uninitialize timely
extern FUNC_Release g_pRelease_11;
ULONG STDMETHODCALLTYPE MyRelease_11(LPVOID pThis)
{
    ULONG uLong = (*g_pRelease_11)(pThis);
    static BOOL bOnce = FALSE;
    if(uLong == 2 && !bOnce)
    {
        // final releasing device
        bOnce = TRUE;
        g_MsgSwitchboard.SendMessage(WM_COREUNINIT);
        uLong = 0;
    }
    return uLong;
}

extern FUNC_Release g_pRelease_9;
ULONG STDMETHODCALLTYPE MyRelease_9(LPVOID pThis)
{
    ULONG uLong = (*g_pRelease_9)(pThis);
    static BOOL bOnce = FALSE;
    if((uLong == 2 || uLong == 3) && !bOnce)
    {
        // final releasing device
        bOnce = TRUE;
        g_MsgSwitchboard.SendMessage(WM_COREUNINIT);
        uLong = 0;
    }
    return uLong;
}

extern FUNC_Release g_pRelease_8;
ULONG STDMETHODCALLTYPE MyRelease_8(LPVOID pThis)
{
    ULONG uLong = (*g_pRelease_8)(pThis);
    static BOOL bOnce = FALSE;
    if(uLong == 1 && !bOnce)
    {
        // final releasing device
        bOnce = TRUE;
        g_MsgSwitchboard.SendMessage(WM_COREUNINIT);
        uLong = 0;
    }

    return uLong;
}


HRESULT WINAPI D3DX11CompileFromMemory(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, void* pDefines, void* pInclude, LPCSTR pFunctionName, LPCSTR pProfile, UINT Flags1, UINT Flags2, void * pPump, void** ppShader, void** ppErrorMsgs, HRESULT* pHResult)
{
    static FUNC_D3DX11CompileFromMemory pFunc = NULL;
    if(!pFunc)
    {
        HMODULE hMod = /*GetDxModule()*/CoreHook::Unity::GetModuleByPrefix(_T("d3dx11_"));
        if (NULL == hMod)  
            hMod = ::LoadLibrary(_T("d3dx11_43.dll"));
       
        pFunc = (FUNC_D3DX11CompileFromMemory)GetProcAddress(hMod, "D3DX11CompileFromMemory");
        if(!pFunc)
        {
            pFunc = (FUNC_D3DX11CompileFromMemory)-1;
        }
    }
    if(pFunc == (FUNC_D3DX11CompileFromMemory)-1)
    {
        return -1;
    }
    return (*pFunc)(pSrcData, SrcDataLen, pFileName, pDefines, pInclude, pFunctionName, pProfile, Flags1, Flags2, pPump, ppShader, ppErrorMsgs, pHResult);
}

#pragma optimize("", on)

