#include "stdafx.h"
#include "CoreHook_Render.h"
#include "../CDXWrapper.h"
#include "../CoreHook_DirectX.h"
#include "ArcShareMem.h"
#include "../CoreHook.h"
#include "DirectXRender_D3D8.h"
#include "DirectXRender_D3D9.h"
#include "DirectXRender_D3D11.h"




//DirectXRender g_Render;

DirectXRender::DirectXRender( LPVOID pDevice )
{
    m_pDevice = pDevice;
}

DirectXRender::~DirectXRender( void )
{

}


DirectXRender* DirectXRenderFactory::CreateDirectXRender( DWORD dwVer ,LPVOID pDevice)
{
    switch (dwVer)
    {
    case DX_D3D8:
        return new DirectXRender_D3D8(pDevice);
    case DX_D3D9:
    case DX_D3D9EX:
        return new DirectXRender_D3D9(pDevice, dwVer);
    case DX_D3D11:
        return new DirectXRender_D3D11(pDevice);
    default:
        return NULL;
    }
}