#include "stdafx.h"
#include "DirectXRender_D3D9.h"
#include "../CDXWrapper.h"
#include "../CoreHook_DirectX.h"
#include "ArcShareMem.h"
#include "../CoreHook.h"



DirectXRender_D3D9::DirectXRender_D3D9( LPVOID pDevice, DWORD dwVer )
:DirectXRender(pDevice)
{
    m_pOSTexture = NULL;
    m_pSysmemTexture = NULL;
    m_pStateBlock = NULL;
    m_dwVer = dwVer;

}

DirectXRender_D3D9::~DirectXRender_D3D9( void )
{
    Uninit();
}

void DirectXRender_D3D9::Render(bool bResize)
{
    g_CoreHook.m_smMgr.GetHeader()->bFullScreen = RenderWndWindowed();
    if(bResize || !m_pOSTexture)
    {
        // create or recreate texture with a new size
        if(m_pOSTexture)
        {
            CUnknown::Release(m_pOSTexture);
        }			
        CDXDevice9::CreateTexture(m_pDevice, g_CoreHook.GetDXRenderWidth(), g_CoreHook.GetDXRenderHeight(), 1, 0, 21/*D3DFMT_A8R8G8B8*/, (m_dwVer == DX_D3D9EX ? 0/*D3DPOOL_DEFAULT*/ : 1/*D3DPOOL_MANAGED*/),
            &m_pOSTexture, NULL);
        if(m_dwVer == DX_D3D9EX)
        {
            if(m_pSysmemTexture)
            {
                CUnknown::Release(m_pSysmemTexture);
            }
            CDXDevice9::CreateTexture(m_pDevice, g_CoreHook.GetDXRenderWidth(), g_CoreHook.GetDXRenderHeight(), 1, 0, 21/*D3DFMT_A8R8G8B8*/, 2/*D3DPOOL_SYSTEMMEM*/, &m_pSysmemTexture, NULL);
        }
        CArcAutoLock lock;
        g_CoreHook.m_smMgr.GetHeader()->bUpdate = TRUE;
        ::SetRect(&g_CoreHook.m_smMgr.GetHeader()->rtDirty, 0, 0, g_CoreHook.m_smMgr.GetHeader()->nWidth, g_CoreHook.m_smMgr.GetHeader()->nHeight);
    }
    if(!m_pStateBlock)
    {
        CDXDevice9::CreateStateBlock(m_pDevice, 1, &m_pStateBlock);
    }

    // render overlay
    if(m_pOSTexture)
    {
        CArcAutoLock lock;
        LPVOID pLockTexture = m_dwVer == DX_D3D9 ? m_pOSTexture : m_pSysmemTexture;
        if(pLockTexture)
        {
            D3DLOCKED_RECT lr;
            if(g_CoreHook.m_smMgr.GetHeader()->bUpdate)
            {
                // update texture with dirty region if needed
                if(CDXTexture9::LockRect(pLockTexture, 0, &lr, NULL, 0x2000/*D3DLOCK_DISCARD*/) == S_OK)
                {
                    CRect rtDirty = g_CoreHook.m_smMgr.GetHeader()->rtDirty;
                    rtDirty.InflateRect(2, 2, 2, 2);
                    CRect rtAll(0, 0, g_CoreHook.m_smMgr.GetHeader()->nWidth, g_CoreHook.m_smMgr.GetHeader()->nHeight);
                    rtDirty.IntersectRect(rtDirty, rtAll);
                    if(rtDirty.left >= 0 && rtDirty.top >= 0 && rtDirty.right <= g_CoreHook.GetDXRenderWidth() && rtDirty.bottom <= g_CoreHook.GetDXRenderHeight())
                    {
                        for(int i = rtDirty.top; i < rtDirty.bottom; i++)
                        {
                            memcpy((LPBYTE)lr.pBits + (i * g_CoreHook.GetDXRenderWidth() + rtDirty.left) * 4, g_CoreHook.m_smMgr.GetBits() + (i * g_CoreHook.GetDXRenderWidth() + rtDirty.left) * 4, rtDirty.Width() * 4);
                        }
                    }
                    CDXTexture9::UnlockRect(pLockTexture, 0);
                    g_CoreHook.m_smMgr.GetHeader()->bUpdate = FALSE;
                    ::SetRect(&g_CoreHook.m_smMgr.GetHeader()->rtDirty, 0, 0, 0, 0);
                }
                if(m_dwVer == DX_D3D9EX)
                {
                    CDXDevice9::UpdateTexture(m_pDevice, m_pSysmemTexture, m_pOSTexture);
                }
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
            if(CDXDevice9::BeginSence(m_pDevice) == S_OK && m_pStateBlock)
            {
                CDXStateBlock9::Capture(m_pStateBlock);
                CDXDevice9::SetPixelShader(m_pDevice, NULL);
                CDXDevice9::SetVertextShader(m_pDevice, NULL);
                CDXDevice9::SetTexture(m_pDevice, 0, m_pOSTexture);

                LPVOID pSurfaceOld = NULL;
                LPVOID pSurfaceNew = NULL;

                D3D9_D3DSURFACE_DESC desc = {0};
                CDXDevice9::GetRenderTarget(m_pDevice, 0, &pSurfaceOld);
                if (pSurfaceOld)
                    CDXSurface9::GetDesc(pSurfaceOld, &desc);

                if (g_CoreHook.GetDXRenderWidth() != desc.Width)
                {
                    CDXDevice9::GetBackBuffer(m_pDevice, 0, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurfaceNew);
                    CDXDevice9::SetRenderTarget(m_pDevice, 0, pSurfaceNew);
                }

                CDXDevice9::SetRenderState(m_pDevice, 27/*D3DRS_ALPHABLENDENABLE*/, TRUE);
                CDXDevice9::SetRenderState(m_pDevice, 19/*D3DRS_SRCBLEND*/, 5/*D3DBLEND_SRCALPHA*/);
                CDXDevice9::SetRenderState(m_pDevice, 20/*D3DRS_DESTBLEND*/, 6/*D3DBLEND_INVSRCALPHA*/);
                CDXDevice9::SetRenderState(m_pDevice, 174/*D3DRS_SCISSORTESTENABLE*/, FALSE);
                CDXDevice9::SetRenderState(m_pDevice, 15/*D3DRS_ALPHATESTENABLE*/, FALSE);
                CDXDevice9::SetRenderState(m_pDevice, 22/*D3DRS_CULLMODE*/, 1/*D3DCULL_NONE*/);
                CDXDevice9::SetRenderState(m_pDevice, 168/*D3DRS_COLORWRITEENABLE*/, 0xf);
                CDXDevice9::SetRenderState(m_pDevice, 137/*D3DRS_LIGHTING*/, FALSE);
                CDXDevice9::SetRenderState(m_pDevice, 8/*D3DRS_FILLMODE*/, 3/*D3DFILL_SOLID*/);
                CDXDevice9::SetRenderState(m_pDevice, 206/*D3DRS_SEPARATEALPHABLENDENABLE*/, FALSE);
                CDXDevice9::SetRenderState(m_pDevice, 194/*D3DRS_SRGBWRITEENABLE*/, FALSE);
                CDXDevice9::SetRenderState(m_pDevice, 14/*D3DRS_ZWRITEENABLE*/, FALSE);
                CDXDevice9::SetRenderState(m_pDevice, 7/*D3DRS_ZENABLE*/, FALSE);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 1/*D3DTSS_COLOROP*/, 4/*D3DTOP_MODULATE*/);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 2/*D3DTSS_COLORARG1*/, 2/*D3DTA_TEXTURE*/);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 3/*D3DTSS_COLORARG2*/, 1/*D3DTA_CURRENT*/);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 4/*D3DTSS_ALPHAOP*/, 2/*D3DTOP_SELECTARG1*/);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 5/*D3DTSS_ALPHAARG1*/, 2/*D3DTA_TEXTURE*/);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 6/*D3DTSS_ALPHAARG2*/, 1/*D3DTA_CURRENT*/);
                CDXDevice9::SetTextureStageState(m_pDevice, 0, 11/*D3DTSS_TEXCOORDINDEX*/, 0);
                CDXDevice9::SetSamplerState(m_pDevice, 0, 11/*D3DSAMP_SRGBTEXTURE*/, 0);
                CDXDevice9::SetFVF(m_pDevice, 0x104/*D3DFVF_XYZRHW | D3DFVF_TEX1*/);
                CDXDevice9::DrawPrimitiveUp(m_pDevice, 5/*D3DPT_TRIANGLESTRIP*/, 2, vert, sizeof(MYVERTEX));

                if (pSurfaceOld)
                    CUnknown::Release(pSurfaceOld);

                if (pSurfaceNew)
                {
                    CDXDevice9::SetRenderTarget(m_pDevice, 0, pSurfaceOld);
                    CUnknown::Release(pSurfaceNew);
                }

                CDXDevice9::SetTexture(m_pDevice, 0, NULL);
                CDXStateBlock9::Apply(m_pStateBlock);
                CDXDevice9::EndScene(m_pDevice);
            }
        }
    }
}
void DirectXRender_D3D9::Init()
{

}

void DirectXRender_D3D9::Uninit()
{
    if(m_pOSTexture)
    {
        CUnknown::Release(m_pOSTexture);
        m_pOSTexture = NULL;
    }

    if(m_pSysmemTexture)
    {
        CUnknown::Release(m_pSysmemTexture);
        m_pSysmemTexture = NULL;
    }

    if(m_pStateBlock)
    {
        CUnknown::Release(m_pStateBlock);
        m_pStateBlock = NULL;
    }
}

HWND DirectXRender_D3D9::GetRenderHwnd()
{
    D3DDEVICE_CREATION_PARAMETERS cp;
    CDXDevice9::GetCreationParameters(m_pDevice, &cp);

    return cp.hFocusWindow;	
}

int DirectXRender_D3D9::GetRenderWidth()
{
    LPVOID pSurface = NULL;

    D3D9_D3DSURFACE_DESC desc = {0};
    CDXDevice9::GetBackBuffer(m_pDevice, 0, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurface);
    CDXSurface9::GetDesc(pSurface, &desc);
    CUnknown::Release(pSurface);

    return desc.Width;
}

int DirectXRender_D3D9::GetRenderHeight()
{
    LPVOID pSurface = NULL;

    D3D9_D3DSURFACE_DESC desc = {0};
    CDXDevice9::GetBackBuffer(m_pDevice, 0, 0, 0/*D3DBACKBUFFER_TYPE_MONO*/, &pSurface);
    CDXSurface9::GetDesc(pSurface, &desc);
    CUnknown::Release(pSurface);

    return desc.Height;
}


bool DirectXRender_D3D9::RenderWndWindowed()
{
    LPVOID pSwapChain = NULL;
    CDXDevice9::GetSwapChain(m_pDevice, 0, &pSwapChain);
    if (NULL != pSwapChain)
    {
        D3DPRESENT_PARAMETERS_9 d3dpp; 
        ZeroMemory( &d3dpp, sizeof(d3dpp) );
        CDXSwapChian9::GetPresentParameters(pSwapChain, &d3dpp);
        return d3dpp.Windowed;
    }

    return false;
}

