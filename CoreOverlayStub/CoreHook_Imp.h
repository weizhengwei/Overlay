#pragma once

#include <vector>
#include "CDXWrapper.h"

#define FAST_USE_DETOUR DetourWrap detour##__LINE__;

class Hook
{
public:
    Hook(){};
    ~Hook(){};

    typedef struct tagDXModuleInfo
    {
        HMODULE hMod;
        DWORD dwVer;
    }DXMODULE_INFO;

    typedef std::vector<DXMODULE_INFO> VEC_DXMODULE_INFO;

    class DetourWrap
    {
    public:
        DetourWrap()
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
        }

        ~DetourWrap()
        {
            DetourTransactionCommit();
        }
    };


public:
    void HookD3D();
    void UnHookD3D();


    static void HookCreateProcess();
    static void UnHookCreateProcess();

    static void HookLoadLibrary();
    static void UnHookLoadLibrary();

    static void HookUser32Method();
    static void UnHookUser32Method();


private:
    void HookD3D8();
    void HookD3D9();
    void HookD3D11();

    void CreateD3DWindow();
    void DestroyD3DWindow();
    void CreateSwapChainDESC();

private:
    HWND m_hD3DWindow;
    DXGI_SWAP_CHAIN_DESC m_sd;
};
