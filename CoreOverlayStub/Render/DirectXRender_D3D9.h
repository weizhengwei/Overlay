#pragma once

#include "CoreHook_Render.h"


class DirectXRender_D3D9 : public DirectXRender
{
public:
    DirectXRender_D3D9(LPVOID pDevice, DWORD dwVer);
    ~DirectXRender_D3D9(void);

    virtual void Render(bool bResize);
    virtual HWND GetRenderHwnd();
    virtual int GetRenderWidth();
    virtual int GetRenderHeight();


    void Init();
    void Uninit();

private:
    bool RenderWndWindowed();

    LPVOID m_pSysmemTexture;
    LPVOID m_pStateBlock;

    DWORD m_dwVer;
};