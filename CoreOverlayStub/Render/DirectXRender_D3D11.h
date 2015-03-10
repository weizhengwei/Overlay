#pragma once

#include "CoreHook_Render.h"


class DirectXRender_D3D11 : public DirectXRender
{
public:
    DirectXRender_D3D11(LPVOID pDevice);
    ~DirectXRender_D3D11(void);

    virtual void Render(bool bResize);
    virtual HWND GetRenderHwnd();
    virtual int GetRenderWidth();
    virtual int GetRenderHeight();

    void Init();
    void Uninit();

private:
    LPVOID m_pSysmemTexture;
    LPVOID m_pStateBlock;

private:
    bool RenderWndWindowed();
};