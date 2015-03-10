#include "stdafx.h"
#include "DirectXRender_D3D11.h"
#include "../CDXWrapper.h"
#include "../CoreHook_DirectX.h"
#include "ArcShareMem.h"
#include "../CoreHook.h"


LPCSTR g_strShaderCode = "Texture2D txDiffuse : register( t0 );	\n\
                         SamplerState samPoint : register( s0 );	\n\
                         struct VS_INPUT\n\
                         {\n\
                         float4 Pos : POSITION;\n\
                         float2 Tex : TEXCOORD;\n\
                         };\n\
                         struct PS_INPUT\n\
                         {\n\
                         float4 Pos : SV_POSITION;\n\
                         float2 Tex : TEXCOORD;\n\
                         };\n\
                         PS_INPUT VS( VS_INPUT input )\n\
                         {\n\
                         PS_INPUT output = (PS_INPUT)0;\n\
                         output.Pos = input.Pos;\n\
                         output.Tex = input.Tex;\n\
                         return output;	\n\
                         }\n\
                         float4 PS( PS_INPUT input) : SV_Target0\n\
                         {\n\
                         return txDiffuse.Sample( samPoint, input.Tex );\n\
                         }";

 LPVOID g_pVertexShader = NULL;
 LPVOID g_pPixelShader = NULL;
 LPVOID g_pVertexLayout = NULL;
 LPVOID g_pVertexBuffer = NULL;
 LPVOID g_pTextureRV = NULL;
 LPVOID g_pSamplerPoint = NULL;
 LPVOID g_pBlendState = NULL;
 LPVOID g_pDepthStencilState = NULL;
 LPVOID g_pRasterizerState = NULL;
 LPVOID g_pRenderTargetView = NULL;
 LPVOID g_pDepthStencilView = NULL;


 GUID IID_ID3D10Texture2D = {0x6f15aaf2,0xd208,0x4e89,{0x9a,0xb4,0x48,0x95,0x35,0xd3,0x4f,0x9c}};
 GUID IID_ID3D11Device = {0xdb6f6ddb,0xac77,0x4e88,{0x82,0x53,0x81,0x9d,0xf9,0xbb,0xf1,0x40}};


DirectXRender_D3D11::DirectXRender_D3D11( LPVOID pDevice )
:DirectXRender(pDevice)
{
    m_pDevice = pDevice;
    m_pOSTexture = NULL;
    m_pSysmemTexture = NULL;
    m_pStateBlock = NULL;

}

DirectXRender_D3D11::~DirectXRender_D3D11( void )
{

}

void DirectXRender_D3D11::Render(bool bResize)
{
    g_CoreHook.m_smMgr.GetHeader()->bFullScreen = RenderWndWindowed();

    LPVOID pSwapChain = m_pDevice;
    LPVOID pDevice = NULL;
    CDXSwapChian::GetDevice(pSwapChain, IID_ID3D11Device, &pDevice);
    if(pDevice)
    {
        LPVOID pDeviceContext = NULL;
        CDXDevice11::GetImmediateContext(pDevice, &pDeviceContext);
        if(pDeviceContext)
        {
            if(!g_pVertexShader)
            {
                // initialize d3d11 objects
                LPVOID pVSBlob = NULL;
                LPVOID pPSBlob = NULL;
                HRESULT hr;
                do 
                {
                    hr = D3DX11CompileFromMemory( g_strShaderCode, strlen(g_strShaderCode), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, NULL, &pVSBlob, NULL, NULL);
                    if(hr != S_OK)
                    {
                        break;
                    }
                    hr = CDXDevice11::CreateVertexShader(pDevice, (const void *)(INT_PTR)CDXBlob::GetBufferPointer(pVSBlob), CDXBlob::GetBufferSize(pVSBlob), NULL, &g_pVertexShader);
                    if(hr != S_OK)
                    {
                        break;
                    }
                    D3D11_INPUT_ELEMENT_DESC layout[] =
                    {
                        { "POSITION", 0, 6/*DXGI_FORMAT_R32G32B32_FLOAT*/, 0, 0, 0/*D3D11_INPUT_PER_VERTEX_DATA*/, 0 },
                        { "TEXCOORD", 0, 16/*DXGI_FORMAT_R32G32_FLOAT*/, 0,12/* DWORD(-1)*//*D3D11_APPEND_ALIGNED_ELEMENT*/, 0/*D3D11_INPUT_PER_VERTEX_DATA*/, 0 },
                    };
                    UINT numElements = 2;
                    // Create the input layout
                    hr = CDXDevice11::CreateInputLayout( pDevice, layout, ARRAYSIZE(layout), (const void *)(INT_PTR)CDXBlob::GetBufferPointer(pVSBlob), 
                        CDXBlob::GetBufferSize(pVSBlob), &g_pVertexLayout );
                    if(hr != S_OK)
                    {
                        break;
                    }
                    hr = D3DX11CompileFromMemory( g_strShaderCode, strlen(g_strShaderCode), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, NULL, &pPSBlob, NULL, NULL);
                    if(hr != S_OK)
                    {
                        break;
                    }
                    // Create the pixel shader
                    hr = CDXDevice11::CreatePixelShader(pDevice, (const void *)(INT_PTR)CDXBlob::GetBufferPointer(pPSBlob), CDXBlob::GetBufferSize(pPSBlob), NULL, &g_pPixelShader);
                    if(hr != S_OK)
                    {
                        break;
                    }
                    SimpleVertex vertices[] =
                    {
                        // 							{-1.0f, 1.0f, 0.0f, 0.0f, 0.0f},
                        // 							{1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
                        // 							{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
                        // 							{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},
                        // 							{1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
                        // 							{1.0f, -1.0f, 0.0f, 1.0f, 1.0f},
                        { 1.0f,  1.0f, 1.0f, 1.0f, 0.0f },
                        { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f },		
                        {-1.0f, -1.0f, 1.0f, 0.0f, 1.0f },

                        {-1.0f, -1.0f, 1.0f, 0.0f, 1.0f },
                        {-1.0f,  1.0f, 1.0f, 0.0f, 0.0f },
                        { 1.0f,  1.0f, 1.0f, 1.0f, 0.0f },		
                    };


                    D3D11_BUFFER_DESC bd;
                    ZeroMemory( &bd, sizeof(bd) );
                    bd.Usage = 1/*0*//*D3D11_USAGE_DEFAULT*/;
                    bd.ByteWidth = sizeof( SimpleVertex ) * 6;
                    bd.BindFlags = 1/*D3D11_BIND_VERTEX_BUFFER*/;
                    bd.CPUAccessFlags = 0;
                    D3D11_SUBRESOURCE_DATA InitData;
                    ZeroMemory( &InitData, sizeof(InitData) );
                    InitData.pSysMem = vertices;
                    hr = CDXDevice11::CreateBuffer(pDevice, &bd, &InitData, &g_pVertexBuffer);
                    if(hr != S_OK)
                    {
                        break;
                    }
                    D3D11_SAMPLER_DESC sampDesc;
                    ZeroMemory( &sampDesc, sizeof(sampDesc) );
                    sampDesc.Filter = 0x15/*D3D11_FILTER_MIN_MAG_MIP_LINEAR*/;
                    sampDesc.AddressU = 1/*D3D11_TEXTURE_ADDRESS_WRAP*/;
                    sampDesc.AddressV = 1/*D3D11_TEXTURE_ADDRESS_WRAP*/;
                    sampDesc.AddressW = 1/*D3D11_TEXTURE_ADDRESS_WRAP*/;
                    sampDesc.ComparisonFunc = 1/*D3D11_COMPARISON_NEVER*/;
                    sampDesc.MinLOD = 0;
                    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
                    hr = CDXDevice11::CreateSamplerState(pDevice, &sampDesc, &g_pSamplerPoint);
                    if(hr != S_OK)
                    {
                        break;
                    }
                    D3D11_RASTERIZER_DESC rasterizerDesc;
                    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
                    rasterizerDesc.CullMode = 1/*3*//*D3D11_CULL_BACK*/;
                    rasterizerDesc.FillMode = 3/*D3D11_FILL_SOLID*/;
                    hr = CDXDevice11::CreateRasterizerState(pDevice, &rasterizerDesc, &g_pRasterizerState);
                    if(hr != S_OK)
                    {
                        break;
                    }


                    D3D11_DEPTH_STENCIL_DESC dsDesc;		
                    ZeroMemory(&dsDesc, sizeof(dsDesc));
                    //dsDesc.DepthEnable = false;
                    hr = CDXDevice11::CreateDepthStencilState(pDevice, &dsDesc, &g_pDepthStencilState);
                    if(hr != S_OK)
                    {
                        break;
                    }



                    // 						D3D11_BLEND_DESC blendDesc;
                    // 						ZeroMemory(&blendDesc, sizeof(blendDesc));
                    // 						blendDesc.RenderTarget[0].BlendEnable = true;
                    // 						blendDesc.RenderTarget[0].BlendOp = 1/*D3D11_BLEND_OP_ADD*/;
                    // 						blendDesc.RenderTarget[0].BlendOpAlpha = 1/*D3D11_BLEND_OP_ADD*/;
                    // 						blendDesc.RenderTarget[0].RenderTargetWriteMask = 0xf/*D3D11_COLOR_WRITE_ENABLE_ALL*/;
                    // 						blendDesc.RenderTarget[0].SrcBlend = 5/*D3D11_BLEND_SRC_ALPHA*/;
                    // 						blendDesc.RenderTarget[0].SrcBlendAlpha = 2/*D3D11_BLEND_ONE*/;
                    // 						blendDesc.RenderTarget[0].DestBlend = 6/*D3D11_BLEND_INV_SRC_ALPHA*/;
                    // 						blendDesc.RenderTarget[0].DestBlendAlpha = 1/*D3D11_BLEND_ZERO*/;
                    // 						blendDesc.RenderTarget[1].BlendEnable = false;

                    D3D11_BLEND_DESC blend;

                    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
                    ZeroMemory( &rtbd, sizeof(rtbd) );
                    ZeroMemory( &blend, sizeof(blend) );


                    rtbd.BlendEnable			 = true;
                    rtbd.SrcBlend				 = 5/*D3D11_BLEND_SRC_ALPHA*/;	
                    rtbd.DestBlend				 = 6/*D3D11_BLEND_INV_SRC_ALPHA*/;	
                    rtbd.BlendOp				 = 1/*D3D11_BLEND_OP_ADD*/;
                    rtbd.SrcBlendAlpha			 = 5/*D3D11_BLEND_SRC_ALPHA*/;
                    rtbd.DestBlendAlpha			 = 6/*D3D11_BLEND_INV_SRC_ALPHA*/;
                    rtbd.BlendOpAlpha			 = 1/*D3D11_BLEND_OP_ADD*/;
                    rtbd.RenderTargetWriteMask	 = 0xf/*D3D11_COLOR_WRITE_ENABLE_ALL*/;

                    blend.AlphaToCoverageEnable=false;
                    blend.IndependentBlendEnable=false; //true;
                    blend.RenderTarget[0]=rtbd;


                    hr = CDXDevice11::CreateBlendState(pDevice, &blend, &g_pBlendState);
                    if(hr != S_OK)
                    {
                        break;
                    }

                } while (FALSE);
                if(pVSBlob)
                {
                    CUnknown::Release(pVSBlob);
                }
                if(pPSBlob)
                {
                    CUnknown::Release(pPSBlob);
                }
            }
            if(bResize || !m_pOSTexture) 
            {
                if(m_pOSTexture)
                {
                    CUnknown::Release(m_pOSTexture);
                }
                D3D11_TEXTURE2D_DESC td = {0};
                td.Width = g_CoreHook.GetDXRenderWidth();
                td.Height = g_CoreHook.GetDXRenderHeight();
                td.Format = 87;
                td.ArraySize = 1;
                td.Count = 1;
                td.MipLevels = 1;
                td.Usage = 2/*D3D11_USAGE_DYNAMIC*/;
                td.BindFlags = 8/*D3D11_BIND_SHADER_RESOURCE*/;
                td.CPUAccessFlags = 0x10000/*D3D11_CPU_ACCESS_WRITE*/;
                CDXDevice11::CreateTexture2D(pDevice, &td, NULL, &m_pOSTexture);
                if(m_pOSTexture)
                {
                    if(g_pTextureRV)
                    {
                        CUnknown::Release(g_pTextureRV);
                    }
                    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {0};
                    srvd.Format = td.Format;
                    srvd.ViewDimension = 4/*D3D11_SRV_DIMENSION_TEXTURE2D*/;
                    srvd.MipLevels = td.MipLevels;
                    CDXDevice11::CreateShaderResourceView(pDevice, m_pOSTexture, &srvd, &g_pTextureRV);
                }
            }

            // render overlay
            if(m_pOSTexture)
            {
                if(g_CoreHook.m_smMgr.GetHeader()->bHasImage)
                {
                    D3D11_MAPPED_SUBRESOURCE data = {0};
                    if(CDXDeviceContext11::Map(pDeviceContext, m_pOSTexture, 0, 4/*D3D11_MAP_WRITE_DISCARD*/, 0, &data) == S_OK)
                    {
                        CArcAutoLock lock;
                        int nLen = g_CoreHook.GetDXRenderWidth() * g_CoreHook.GetDXRenderHeight();
                        if(data.RowPitch == g_CoreHook.GetDXRenderWidth() * 4)
                        {
                            memcpy(data.pData, g_CoreHook.m_smMgr.GetBits(), nLen * 4);
                        }
                        else
                        {
                            LPDWORD pSrc = (LPDWORD)g_CoreHook.m_smMgr.GetBits();
                            LPDWORD pDest = (LPDWORD)data.pData;
                            for(int i = 0; i < nLen; i++)
                            {
                                // ARGB
                                *pDest++ = *pSrc++;
                                if((i + 1) % g_CoreHook.GetDXRenderWidth() == 0)
                                {
                                    pDest += data.RowPitch / 4 - g_CoreHook.GetDXRenderWidth();
                                }
                            }
                        }
                        CDXDeviceContext11::Unmap(pDeviceContext, m_pOSTexture, 0);
                    }

                    //create a rendertarget
                    LPVOID pBackBuffer = NULL;
                    HRESULT hr = CDXSwapChian::GetBuffer(pSwapChain,  0, IID_ID3D10Texture2D, ( LPVOID* )&pBackBuffer );
                    if( FAILED( hr ) )
                        return;

                    hr = CDXDevice11::CreateRenderTargetView(pDevice, pBackBuffer, NULL, &g_pRenderTargetView );
                    CUnknown::Release(pBackBuffer);



                    //                         LPVOID ExtraRenderTargetTexture = NULL;
                    //                         LPVOID ExtraRenderTarget = NULL;
                    //                         D3D11_TEXTURE2D_DESC texdesc;
                    //                         texdesc.ArraySize=1;
                    //                         texdesc.BindFlags=/*D3D11_BIND_RENDER_TARGET*/0x20L | /*D3D11_BIND_SHADER_RESOURCE*/0x8L;
                    //                         texdesc.CPUAccessFlags=0;
                    //                         texdesc.Format=/*DXGI_FORMAT_R8G8B8A8_UNORM*/28;
                    //                         texdesc.Height=g_CoreHook.GetDXRenderHeight();
                    //                         texdesc.Width=g_CoreHook.GetDXRenderWidth();
                    //                         texdesc.MipLevels=1;
                    //                         texdesc.MiscFlags=0;
                    //                         texdesc.Count=1;
                    //                         texdesc.Quality=0;
                    //                         texdesc.Usage=/*D3D11_USAGE_DEFAULT*/0;
                    // 
                    //                         if (SUCCEEDED(CDXDevice11::CreateTexture2D(pDevice, &texdesc, NULL, &ExtraRenderTargetTexture)))
                    //                         {
                    //                             CDXDevice11::CreateRenderTargetView(pDevice, ExtraRenderTargetTexture, NULL,  &ExtraRenderTarget);
                    //                         }		


                    // Create depth stencil texture

                    LPVOID pDepthStencil = NULL;
                    D3D11_TEXTURE2D_DESC descDepth;
                    ZeroMemory( &descDepth, sizeof(descDepth) );
                    descDepth.Width = g_CoreHook.GetDXRenderWidth();
                    descDepth.Height = g_CoreHook.GetDXRenderHeight();
                    descDepth.MipLevels = 1;
                    descDepth.ArraySize = 1;
                    descDepth.Format = 45/*DXGI_FORMAT_D24_UNORM_S8_UINT*/;
                    descDepth.Count = 1;
                    descDepth.Quality = 0;
                    descDepth.Usage = 0/*D3D11_USAGE_DEFAULT*/;
                    descDepth.BindFlags = 0x40L/*D3D11_BIND_DEPTH_STENCIL*/;
                    descDepth.CPUAccessFlags = 0;
                    descDepth.MiscFlags = 0;
                    hr = CDXDevice11::CreateTexture2D(pDevice, &descDepth, NULL, &pDepthStencil );
                    if( FAILED( hr ) )
                        return;

                    // Create the depth stencil view
                    LPVOID pDepthStencilView = NULL;
                    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
                    ZeroMemory( &descDSV, sizeof(descDSV) );
                    descDSV.Format = descDepth.Format;
                    descDSV.ViewDimension = 3/*D3D11_DSV_DIMENSION_TEXTURE2D*/;
                    descDSV.Texture2D.MipSlice = 0;
                    hr = CDXDevice11::CreateDepthStencilView(pDevice,  pDepthStencil, &descDSV, &g_pDepthStencilView );
                    if( FAILED( hr ) )
                        return;


                    LPVOID pOldGeometryShader = NULL;
                    CDXDeviceContext11::GSGetShader(pDeviceContext, &pOldGeometryShader, NULL, NULL);
                    LPVOID pOldVertexShader = NULL;
                    CDXDeviceContext11::VSGetShader(pDeviceContext, &pOldVertexShader, NULL, NULL);
                    LPVOID pOldSampler = NULL;
                    CDXDeviceContext11::PSGetSamplers(pDeviceContext, 0, 1, &pOldSampler);


                    LPVOID pOldLayout = NULL;
                    CDXDeviceContext11::IAGetInputLayout(pDeviceContext, &pOldLayout);
                    LPVOID pOldVertextBuffers = NULL;
                    UINT uOldSride = 0;
                    UINT uOldOffset = 0;
                    CDXDeviceContext11::IAGetVertexBuffers(pDeviceContext, 0, 1, &pOldVertextBuffers, &uOldSride, &uOldOffset);
                    UINT uOldPrimitiveType = 0;
                    CDXDeviceContext11::IAGetPrimitiveTopology(pDeviceContext, &uOldPrimitiveType);

                    LPVOID pOldPixelShader = NULL;
                    CDXDeviceContext11::PSGetShader(pDeviceContext, &pOldPixelShader, NULL, NULL);




                    LPVOID oldRenderTarget = NULL;
                    LPVOID oldDepthStencilView = NULL;
                    CDXDeviceContext11::OMGetRenderTargets(pDeviceContext, 1, &oldRenderTarget, &oldDepthStencilView);


                    LPVOID pOldBlendState = NULL;
                    float fOldFactor[4] = {0.0f};
                    UINT uOldSampleMask = 0;
                    CDXDeviceContext11::OMGetBlendState(pDeviceContext, &pOldBlendState, fOldFactor, &uOldSampleMask);
                    LPVOID pOldDepthStencilState = NULL;
                    UINT uOldDepthRef = 0;
                    CDXDeviceContext11::OMGetDepthStencilState(pDeviceContext, &pOldDepthStencilState, &uOldDepthRef);


                    LPVOID pOldRSState = NULL;
                    CDXDeviceContext11::RSGetState(pDeviceContext, &pOldRSState);


                    UINT oldviewports=0;
                    D3D11_VIEWPORT viewports[16];
                    CDXDeviceContext11::RSGetViewports(pDeviceContext, &oldviewports, NULL);
                    CDXDeviceContext11::RSGetViewports(pDeviceContext, &oldviewports, viewports);


                    /*************************************************************************************/
                    CDXDeviceContext11::GSSetShader(pDeviceContext, NULL,  NULL, 0);
                    CDXDeviceContext11::VSSetShader(pDeviceContext, g_pVertexShader, NULL, 0);
                    CDXDeviceContext11::PSSetSamplers(pDeviceContext, 0, 1, &g_pSamplerPoint);
                    D3D11_VIEWPORT vp;
                    vp.Width = (float)g_CoreHook.GetDXRenderWidth();
                    vp.Height = (float)g_CoreHook.GetDXRenderHeight();
                    vp.MinDepth = 0.0f;
                    vp.MaxDepth = 1.0f;
                    vp.TopLeftX = 0;
                    vp.TopLeftY = 0;
                    CDXDeviceContext11::RSSetViewports(pDeviceContext, 1, &vp );

                    CDXDeviceContext11::OMSetRenderTargetsAndUnorderedAccessViews(pDeviceContext, 0,NULL,NULL,0,0,NULL,NULL);
                    CDXDeviceContext11::OMSetRenderTargets(pDeviceContext,1, &g_pRenderTargetView, g_pDepthStencilView);		
                    CDXDeviceContext11::ClearDepthStencilView(pDeviceContext, g_pDepthStencilView, 1/*D3D11_CLEAR_DEPTH*/, 1.0f, 0 );


                    CDXDeviceContext11::IASetInputLayout(pDeviceContext, g_pVertexLayout);
                    UINT stride = sizeof( SimpleVertex );
                    UINT offset = 0;
                    CDXDeviceContext11::IASetVertexBuffers(pDeviceContext, 0, 1, &g_pVertexBuffer, &stride, &offset);
                    CDXDeviceContext11::IASetPrimitiveTopology(pDeviceContext, 4/*D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST*/);
                    CDXDeviceContext11::PSSetShader(pDeviceContext, g_pPixelShader, NULL, 0);
                    CDXDeviceContext11::PSSetShaderResources(pDeviceContext, 0, 1, &g_pTextureRV);
                    CDXDeviceContext11::RSSetState(pDeviceContext, g_pRasterizerState);
                    float f[4] = {1.0f};
                    CDXDeviceContext11::OMSetBlendState(pDeviceContext, g_pBlendState, f, (DWORD)-1);
                    CDXDeviceContext11::OMSetDepthStencilState(pDeviceContext, g_pDepthStencilState, 0);
                    CDXDeviceContext11::Draw(pDeviceContext, 6, 0);
                    /*************************************************************************************/

                    if(pOldGeometryShader)
                    {
                        CDXDeviceContext11::GSSetShader(pDeviceContext, pOldGeometryShader, NULL, NULL);
                        CUnknown::Release(pOldGeometryShader);
                    }

                    if(pOldVertexShader)
                    {
                        CDXDeviceContext11::VSSetShader(pDeviceContext, pOldVertexShader, NULL, NULL);
                        CUnknown::Release(pOldVertexShader);
                    }

                    if(pOldSampler)
                    {
                        CDXDeviceContext11::PSSetSamplers(pDeviceContext, 0, 1, &pOldSampler);
                        CUnknown::Release(pOldSampler);
                    }

                    if(pOldLayout)
                    {
                        CDXDeviceContext11::IASetInputLayout(pDeviceContext, pOldLayout);
                        CUnknown::Release(pOldLayout);
                    }
                    if(pOldVertextBuffers)
                    {
                        CDXDeviceContext11::IASetVertexBuffers(pDeviceContext, 0, 1, &pOldVertextBuffers, &uOldSride, &uOldOffset);
                        CUnknown::Release(pOldVertextBuffers);
                    }
                    if(uOldPrimitiveType)
                    {
                        CDXDeviceContext11::IASetPrimitiveTopology(pDeviceContext, uOldPrimitiveType);
                    }

                    if(pOldPixelShader)
                    {
                        CDXDeviceContext11::PSSetShader(pDeviceContext, pOldPixelShader, NULL, NULL);
                        CUnknown::Release(pOldPixelShader);
                    }

                    if(pOldRSState)
                    {
                        CDXDeviceContext11::RSSetState(pDeviceContext, pOldRSState);
                        CUnknown::Release(pOldRSState);
                    }

                    if (oldRenderTarget)
                    {
                        CDXDeviceContext11::OMSetRenderTargets(pDeviceContext, 1, &oldRenderTarget, oldDepthStencilView);
                        CUnknown::Release(oldRenderTarget);
                    }
                    if(pOldBlendState)
                    {
                        CDXDeviceContext11::OMSetBlendState(pDeviceContext, pOldBlendState, fOldFactor, uOldSampleMask);
                        CUnknown::Release(pOldBlendState);
                    }
                    if(pOldDepthStencilState)
                    {
                        CDXDeviceContext11::OMSetDepthStencilState(pDeviceContext, pOldDepthStencilState, uOldDepthRef);
                        CUnknown::Release(pOldDepthStencilState);
                    }

                    if (pDepthStencil)
                    {
                        CUnknown::Release(pDepthStencil);
                    }

                    if (g_pDepthStencilView)
                    {
                        CUnknown::Release(g_pDepthStencilView);
                    }

                    if (g_pRenderTargetView)
                    {
                        CUnknown::Release(g_pRenderTargetView);
                    }

                    CDXDeviceContext11::RSSetViewports(pDeviceContext, oldviewports, viewports);
                }
            }
            CUnknown::Release(pDeviceContext);
        }
        CUnknown::Release(pDevice);
    }

}
void DirectXRender_D3D11::Init()
{

}

void DirectXRender_D3D11::Uninit()
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

    if(g_pVertexShader)
    {
        CUnknown::Release(g_pVertexShader);
        g_pVertexShader = NULL;
    }
    if(g_pPixelShader)
    {
        CUnknown::Release(g_pPixelShader);
        g_pPixelShader = NULL;
    }
    if(g_pVertexBuffer)
    {
        CUnknown::Release((g_pVertexBuffer));
        g_pVertexBuffer = NULL;
    }
    if(g_pVertexLayout)
    {
        CUnknown::Release(g_pVertexLayout);
        g_pVertexLayout = NULL;
    }
    if(g_pTextureRV)
    {
        CUnknown::Release(g_pTextureRV);
        g_pTextureRV = NULL;
    }
    if(g_pSamplerPoint)
    {
        CUnknown::Release(g_pSamplerPoint);
        g_pSamplerPoint = NULL;
    }
    if(g_pBlendState)
    {
        CUnknown::Release(g_pBlendState);
        g_pBlendState = NULL;
    }
    if(g_pDepthStencilState)
    {
        CUnknown::Release(g_pDepthStencilState);
        g_pDepthStencilState = NULL;
    }
    if(g_pRasterizerState)
    {
        CUnknown::Release(g_pRasterizerState);
        g_pRasterizerState = NULL;
    }
}

HWND DirectXRender_D3D11::GetRenderHwnd()
{
    DXGI_SWAP_CHAIN_DESC desc;
    if(CDXSwapChian::GetDesc(m_pDevice, &desc) == S_OK)
        return desc.OutputWindow;

    return 0;
}

int DirectXRender_D3D11::GetRenderWidth()
{
    DXGI_SWAP_CHAIN_DESC desc;
    if(CDXSwapChian::GetDesc(m_pDevice, &desc) == S_OK)
        return desc.BufferDesc.Width;

    return 0;
}

int DirectXRender_D3D11::GetRenderHeight()
{
    DXGI_SWAP_CHAIN_DESC desc;
    if(CDXSwapChian::GetDesc(m_pDevice, &desc) == S_OK)
        return desc.BufferDesc.Height;

    return 0;
}

bool DirectXRender_D3D11::RenderWndWindowed()
{
    DXGI_SWAP_CHAIN_DESC desc;
    if(CDXSwapChian::GetDesc(m_pDevice, &desc) == S_OK)
        return desc.Windowed;

    return false;
}