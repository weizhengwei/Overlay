#include "stdafx.h"
#include "DirectXRender_D3D8.h"
#include "../CDXWrapper.h"
#include "../CoreHook_DirectX.h"
#include "ArcShareMem.h"
#include "../CoreHook.h"


DirectXRender_D3D8::DirectXRender_D3D8( LPVOID pDevice )
:DirectXRender(pDevice)
{
    m_pOSTexture = NULL;

}

DirectXRender_D3D8::~DirectXRender_D3D8( void )
{
    Uninit();
}

void DirectXRender_D3D8::Render(bool bResize)
{
    LPVOID pSurface = NULL;


    D3DSURFACE_DESC8 desc = {0};
    CDXDevice8::GetBackBuffer(m_pDevice, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurface);
    CDXSurface8::GetDesc(pSurface, &desc);
    CUnknown::Release(pSurface);

    if(bResize || !m_pOSTexture)
    {
        // create or recreate texture with a new size
        if(m_pOSTexture)
        {
            CUnknown::Release(m_pOSTexture);
        }
        CDXDevice8::CreateTexture(m_pDevice, g_CoreHook.GetDXRenderWidth(), g_CoreHook.GetDXRenderHeight(), 1, 0, 21/*D3DFMT_A8R8G8B8*/, 1/*D3DPOOL_MANAGED*/, &m_pOSTexture);
    }

    // render overlay
    if(m_pOSTexture)
    {
        CArcAutoLock lock;
        D3DLOCKED_RECT lr;
        if(g_CoreHook.m_smMgr.GetHeader()->bUpdate)
        {
            // update texture with dirty region if needed
            if(CDXTexture8::LockRect(m_pOSTexture, 0, &lr, NULL, 0x2000/*D3DLOCK_DISCARD*/) == S_OK)
            {
                CRect rtDirty = g_CoreHook.m_smMgr.GetHeader()->rtDirty;
                if(rtDirty.left >= 0 && rtDirty.top >= 0 && rtDirty.right <= g_CoreHook.GetDXRenderWidth() && rtDirty.bottom <= g_CoreHook.GetDXRenderHeight())
                {
                    for(int i = rtDirty.top; i < rtDirty.bottom; i++)
                    {
                        memcpy((LPBYTE)lr.pBits + (i * g_CoreHook.GetDXRenderWidth() + rtDirty.left) * 4, g_CoreHook.m_smMgr.GetBits() + (i * g_CoreHook.GetDXRenderWidth() + rtDirty.left) * 4, rtDirty.Width() * 4);
                    }
                }
                CDXTexture8::UnlockRect(m_pOSTexture, 0);
                g_CoreHook.m_smMgr.GetHeader()->bUpdate = FALSE;
                ::SetRect(&g_CoreHook.m_smMgr.GetHeader()->rtDirty, 0, 0, 0, 0);
            }
        }
        if(g_CoreHook.m_smMgr.GetHeader()->bHasImage)
        {    
            // not fully transparent, render it
            struct MYVERTEX
            {
                float x, y, z, rhw;
                float tu, tv;
            };
            MYVERTEX vert[4] = 
            {
                {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f},
                {g_CoreHook.GetDXRenderWidth() - 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f},
                {-0.5f, g_CoreHook.GetDXRenderHeight() - 0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
                {g_CoreHook.GetDXRenderWidth() - 0.5f, g_CoreHook.GetDXRenderHeight() - 0.5f, 0.0f, 1.0f, 1.0f, 1.0f}
            };
            if(CDXDevice8::BeginSence(m_pDevice) == S_OK)
            {
                CDXDevice8::SetRenderState(m_pDevice, 27/*D3DRS_ALPHABLENDENABLE*/, TRUE);
                CDXDevice8::SetRenderState(m_pDevice, 19/*D3DRS_SRCBLEND*/, 5/*D3DBLEND_SRCALPHA*/);
                CDXDevice8::SetRenderState(m_pDevice, 20/*D3DRS_DESTBLEND*/, 6/*D3DBLEND_INVSRCALPHA*/);
                CDXDevice8::SetTexture(m_pDevice, 0, m_pOSTexture);
                CDXDevice8::SetVertexShader(m_pDevice, 0x104/*D3DFVF_XYZRHW | D3DFVF_TEX1*/);
                CDXDevice8::DrawPrimitiveUp(m_pDevice, 5/*D3DPT_TRIANGLESTRIP*/, 2, vert, sizeof(MYVERTEX));
                CDXDevice8::SetTexture(m_pDevice, 0, NULL);
                CDXDevice8::EndScene(m_pDevice);
            }				
        }
    }
}


void DirectXRender_D3D8::Init()
{

}

void DirectXRender_D3D8::Uninit()
{
    if(m_pOSTexture)
    {
        CUnknown::Release(m_pOSTexture);
        m_pOSTexture = NULL;
    }
}

HWND DirectXRender_D3D8::GetRenderHwnd()
{
    D3DDEVICE_CREATION_PARAMETERS cp;
    CDXDevice8::GetCreationParameters(m_pDevice, &cp);

    return cp.hFocusWindow;	
}

int DirectXRender_D3D8::GetRenderWidth()
{
    LPVOID pSurface = NULL;

    D3DSURFACE_DESC8 desc = {0};
    CDXDevice8::GetBackBuffer(m_pDevice, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurface);
    CDXSurface8::GetDesc(pSurface, &desc);
    CUnknown::Release(pSurface);

    return desc.Width;
}

int DirectXRender_D3D8::GetRenderHeight()
{
    LPVOID pSurface = NULL;

    D3DSURFACE_DESC8 desc = {0};
    CDXDevice8::GetBackBuffer(m_pDevice, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurface);
    CDXSurface8::GetDesc(pSurface, &desc);
    CUnknown::Release(pSurface);

    return desc.Height;
}