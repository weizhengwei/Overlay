#pragma once

#include "CoreHook_Render.h"


class DirectXRender_D3D8 : public DirectXRender
{
public:
    DirectXRender_D3D8(LPVOID pDevice);
    ~DirectXRender_D3D8(void);

    virtual void Render(bool bResize);
    virtual HWND GetRenderHwnd();
    virtual int GetRenderWidth();
    virtual int GetRenderHeight();

    void Init();
    void Uninit();
};