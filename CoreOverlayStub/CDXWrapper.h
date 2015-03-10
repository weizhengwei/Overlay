#pragma once


typedef struct D3DLOCKED_RECT {
	INT  Pitch;
	void *pBits;
} D3DLOCKED_RECT;

/* Surface Description */
typedef struct D3D9_D3DSURFACE_DESC
{
    int                 Format;
    int                 Type;
    DWORD               Usage;
    int                 Pool;

    int                 MultiSampleType;
    DWORD               MultiSampleQuality;
    UINT                Width;
    UINT                Height;
} D3D9_D3DSURFACE_DESC;

typedef struct _D3DDEVICE_CREATION_PARAMETERS
{
    UINT            AdapterOrdinal;
    UINT            DeviceType;
    HWND            hFocusWindow;
    DWORD           BehaviorFlags;
} D3DDEVICE_CREATION_PARAMETERS;

#define FAST_DEFINE_VIRFUNC(funcname, params, nake_params, index) static int funcname##params{ \
	typedef int(* FUNC_##funcname)##params; \
    FUNC_##funcname p##funcname = (FUNC_##funcname)CoreHook::Unity::GetVirtualCall(pObject, index); \
	return (*p##funcname)##nake_params;}


class CUnknown
{
public: 
	FAST_DEFINE_VIRFUNC(Release, (LPVOID pObject), (pObject), 2);
};

#define D3D11_FLOAT32_MAX	( 3.402823466e+38f )

// directx9 wrappers
class CDXDevice9
{
public:
	FAST_DEFINE_VIRFUNC(CreateStateBlock, (LPVOID pObject, DWORD dwStateType, LPVOID * ppSB), (pObject, dwStateType, ppSB), 59);
	FAST_DEFINE_VIRFUNC(Clear, (LPVOID pObject, DWORD Count, CONST LPVOID pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil), (pObject, Count, pRects, Flags, Color, Z, Stencil), 43);
	FAST_DEFINE_VIRFUNC(GetSwapChain, (LPVOID pObject, UINT iSwapChain, VOID ** ppSwapChain), (pObject, iSwapChain, ppSwapChain), 14);
	FAST_DEFINE_VIRFUNC(CreateOffscreenPlainSurface, (LPVOID pObject, int nWidth, int nHeight, int nFormat, int nPool, void ** ppSurface, void * pReserve), (pObject, nWidth, nHeight, nFormat, nPool, ppSurface, pReserve), 36);
	FAST_DEFINE_VIRFUNC(GetBackBuffer, (LPVOID pObject, UINT iSwapChain, UINT BackBuffer, int nBackBufferType, void ** pSurface), (pObject, iSwapChain, BackBuffer, nBackBufferType, pSurface), 18);
	FAST_DEFINE_VIRFUNC(GetRenderTargetData, (LPVOID pObject, void * pRenderTarget, void * pSurface), (pObject, pRenderTarget, pSurface), 32);
   
    FAST_DEFINE_VIRFUNC(SetRenderTarget, (LPVOID pObject, DWORD RenderTargetIndex,void* pRenderTarget), (pObject, RenderTargetIndex, pRenderTarget),37);
    FAST_DEFINE_VIRFUNC(GetRenderTarget, (LPVOID pObject, DWORD RenderTargetIndex, void ** ppRenderTarget),  (pObject,  RenderTargetIndex, ppRenderTarget), 38);

	FAST_DEFINE_VIRFUNC(UpdateSurface, (LPVOID pObject, void *pSrc, const RECT * pSrcRect, void *pDest, const RECT * pDestRect), (pObject, pSrc, pSrcRect, pDest, pDestRect), 30);
	FAST_DEFINE_VIRFUNC(CreateTexture, (LPVOID pObject, UINT Width, UINT Height, UINT Levels, DWORD Usage, int Format, int Pool, void ** ppTexture, HANDLE * pSharedHandle), (pObject, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle), 23);
	FAST_DEFINE_VIRFUNC(SetRenderState, (LPVOID pObject, int State, DWORD Value), (pObject, State, Value), 57);
	FAST_DEFINE_VIRFUNC(SetTextureStageState, (LPVOID pObject, DWORD Stage, DWORD Type, DWORD Value), (pObject, Stage, Type, Value), 67);
	FAST_DEFINE_VIRFUNC(SetSamplerState, (LPVOID pObject, DWORD Sampler, DWORD Type, DWORD Value), (pObject, Sampler, Type, Value), 69);
	FAST_DEFINE_VIRFUNC(SetTexture, (LPVOID pObject, DWORD Stage, void * pTexture), (pObject, Stage, pTexture), 65);
	FAST_DEFINE_VIRFUNC(UpdateTexture, (LPVOID pObject, LPVOID pSrcTexture, LPVOID pDestTexture), (pObject, pSrcTexture, pDestTexture), 31);
	FAST_DEFINE_VIRFUNC(SetFVF, (LPVOID pObject, DWORD dwFVF),(pObject, dwFVF), 89);
	FAST_DEFINE_VIRFUNC(GetFVF, (LPVOID pObject, DWORD * pFVF), (pObject, pFVF), 90);
	FAST_DEFINE_VIRFUNC(BeginSence, (LPVOID pObject), (pObject), 41);
	FAST_DEFINE_VIRFUNC(EndScene, (LPVOID pObject), (pObject), 42);
	FAST_DEFINE_VIRFUNC(DrawPrimitiveUp, (LPVOID pObject, int PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride), (pObject, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride), 83);
	FAST_DEFINE_VIRFUNC(SetPixelShader, (LPVOID pObject, void * pShader), (pObject, pShader), 107);
	FAST_DEFINE_VIRFUNC(SetVertextShader, (LPVOID pObject, void * pShader), (pObject, pShader), 92);

    FAST_DEFINE_VIRFUNC(GetCreationParameters, (LPVOID pObject, D3DDEVICE_CREATION_PARAMETERS *pParameters), (pObject, pParameters), 9);
};

class CDXStateBlock9
{
public:
	FAST_DEFINE_VIRFUNC(Capture, (LPVOID pObject), (pObject), 4);
	FAST_DEFINE_VIRFUNC(Apply, (LPVOID pObject), (pObject), 5);
};

class CDXTexture9
{
public:
	FAST_DEFINE_VIRFUNC(LockRect, (LPVOID pObject, UINT uLevel, D3DLOCKED_RECT * pLockedRect, const RECT * pRect, DWORD Flags), (pObject, uLevel, pLockedRect, pRect, Flags), 19);
	FAST_DEFINE_VIRFUNC(UnlockRect, (LPVOID pObject, UINT uLevel), (pObject, uLevel), 20);
};

class CDXSurface9
{
public:
	FAST_DEFINE_VIRFUNC(GetDesc, (LPVOID pObject, D3D9_D3DSURFACE_DESC* pDesc), (pObject, pDesc), 12);
	FAST_DEFINE_VIRFUNC(LockRect, (LPVOID pObject, D3DLOCKED_RECT * pLockedRect, const RECT * pRect, DWORD Flags), (pObject, pLockedRect, pRect, Flags), 13);
	FAST_DEFINE_VIRFUNC(UnlockRect, (LPVOID pObject), (pObject), 14);
};



// directx8 wrappers
class CDXDevice8
{
public:
	FAST_DEFINE_VIRFUNC(CreateTexture, (LPVOID pObject, UINT Width, UINT Height, UINT Levels, DWORD Usage, int Format, int Pool, void ** ppTexture), (pObject, Width, Height, Levels, Usage, Format, Pool, ppTexture), 20);
	FAST_DEFINE_VIRFUNC(SetRenderState, (LPVOID pObject, int State, DWORD Value), (pObject, State, Value), 50);
	FAST_DEFINE_VIRFUNC(SetTexture, (LPVOID pObject, DWORD Stage, void * pTexture), (pObject, Stage, pTexture), 61);
	FAST_DEFINE_VIRFUNC(SetVertexShader, (LPVOID pObject, DWORD dwFVF), (pObject, dwFVF), 76);
	FAST_DEFINE_VIRFUNC(BeginSence, (LPVOID pObject), (pObject), 34);
	FAST_DEFINE_VIRFUNC(EndScene, (LPVOID pObject), (pObject), 35);
	FAST_DEFINE_VIRFUNC(DrawPrimitiveUp, (LPVOID pObject, int PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride), (pObject, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride), 72);
    FAST_DEFINE_VIRFUNC(GetBackBuffer, (LPVOID pObject, UINT BackBuffer, UINT Type, void** ppBackBuffer), (pObject, BackBuffer, Type, ppBackBuffer), 16);
    FAST_DEFINE_VIRFUNC(GetCreationParameters, (LPVOID pObject, D3DDEVICE_CREATION_PARAMETERS *pParameters), (pObject, pParameters), 9);
};

class CDXTexture8
{
public:
	FAST_DEFINE_VIRFUNC(LockRect, (LPVOID pObject, UINT uLevel, D3DLOCKED_RECT * pLockedRect, const RECT * pRect, DWORD Flags), (pObject, uLevel, pLockedRect, pRect, Flags), 16);
	FAST_DEFINE_VIRFUNC(UnlockRect, (LPVOID pObject, UINT uLevel), (pObject, uLevel), 17);
};


/* Surface Description */
struct D3DSURFACE_DESC8
{
    UINT                Format;
    UINT                Type;
    DWORD               Usage;
    int                 Pool;
    UINT                Size;

    int                 MultiSampleType;
    UINT                Width;
    UINT                Height;
} ;


class CDXSurface8
{
public:
    FAST_DEFINE_VIRFUNC(GetDesc, (LPVOID pObject, D3DSURFACE_DESC8* pDesc), (pObject, pDesc), 8);
    FAST_DEFINE_VIRFUNC(LockRect, (LPVOID pObject, D3DLOCKED_RECT * pLockedRect, const RECT * pRect, DWORD Flags), (pObject, pLockedRect, pRect, Flags), 9);
    FAST_DEFINE_VIRFUNC(UnlockRect, (LPVOID pObject), (pObject), 10);
};

// directx11 wrappers
typedef struct D3D11_TEXTURE2D_DESC
{
	UINT Width;
	UINT Height;
	UINT MipLevels;
	UINT ArraySize;
	UINT Format;
	UINT Count;
	UINT Quality;
	UINT Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
} D3D11_TEXTURE2D_DESC;

typedef struct D3D11_MAPPED_SUBRESOURCE
{
	void *pData;
	UINT RowPitch;
	UINT DepthPitch;
} D3D11_MAPPED_SUBRESOURCE;

typedef struct DXGI_RATIONAL
{
	UINT Numerator;
	UINT Denominator;
} DXGI_RATIONAL;

typedef struct DXGI_MODE_DESC
{
	UINT Width;
	UINT Height;
	DXGI_RATIONAL RefreshRate;
	UINT Format;
	UINT ScanlineOrdering;
	UINT Scaling;
} DXGI_MODE_DESC;

typedef struct DXGI_SWAP_CHAIN_DESC
{
	DXGI_MODE_DESC BufferDesc;
	UINT Count;
	UINT Quality;
	UINT BufferUsage;
	UINT BufferCount;
	HWND OutputWindow;
	BOOL Windowed;
	UINT SwapEffect;
	UINT Flags;
} 	DXGI_SWAP_CHAIN_DESC;

typedef struct D3D11_INPUT_ELEMENT_DESC
{
	LPCSTR SemanticName;
	UINT SemanticIndex;
	UINT Format;
	UINT InputSlot;
	UINT AlignedByteOffset;
	UINT InputSlotClass;
	UINT InstanceDataStepRate;
} 	D3D11_INPUT_ELEMENT_DESC;

typedef struct D3D11_SAMPLER_DESC
{
	UINT Filter;
	UINT AddressU;
	UINT AddressV;
	UINT AddressW;
	FLOAT MipLODBias;
	UINT MaxAnisotropy;
	UINT ComparisonFunc;
	FLOAT BorderColor[ 4 ];
	FLOAT MinLOD;
	FLOAT MaxLOD;
} 	D3D11_SAMPLER_DESC;

typedef struct D3D11_RASTERIZER_DESC
{
	UINT FillMode;
	UINT CullMode;
	BOOL FrontCounterClockwise;
	INT DepthBias;
	FLOAT DepthBiasClamp;
	FLOAT SlopeScaledDepthBias;
	BOOL DepthClipEnable;
	BOOL ScissorEnable;
	BOOL MultisampleEnable;
	BOOL AntialiasedLineEnable;
} 	D3D11_RASTERIZER_DESC;

typedef struct D3D11_RENDER_TARGET_BLEND_DESC
{
	BOOL BlendEnable;
	UINT SrcBlend;
	UINT DestBlend;
	UINT BlendOp;
	UINT SrcBlendAlpha;
	UINT DestBlendAlpha;
	UINT BlendOpAlpha;
	UCHAR RenderTargetWriteMask;
} 	D3D11_RENDER_TARGET_BLEND_DESC;

typedef struct D3D11_BLEND_DESC
{
	BOOL AlphaToCoverageEnable;
	BOOL IndependentBlendEnable;
	D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[ 8 ];
} 	D3D11_BLEND_DESC;

typedef struct D3D11_BUFFER_DESC
{
	UINT ByteWidth;
	UINT Usage;
	UINT BindFlags;
	UINT CPUAccessFlags;
	UINT MiscFlags;
	UINT StructureByteStride;
} 	D3D11_BUFFER_DESC;

typedef struct D3D11_SUBRESOURCE_DATA
{
	const void *pSysMem;
	UINT SysMemPitch;
	UINT SysMemSlicePitch;
} 	D3D11_SUBRESOURCE_DATA;

typedef struct D3D11_DEPTH_STENCILOP_DESC
{
	UINT StencilFailOp;
	UINT StencilDepthFailOp;
	UINT StencilPassOp;
	UINT StencilFunc;
} 	D3D11_DEPTH_STENCILOP_DESC;

typedef struct D3D11_DEPTH_STENCIL_DESC
{
	BOOL DepthEnable;
	UINT DepthWriteMask;
	UINT DepthFunc;
	BOOL StencilEnable;
	UINT8 StencilReadMask;
	UINT8 StencilWriteMask;
	D3D11_DEPTH_STENCILOP_DESC FrontFace;
	D3D11_DEPTH_STENCILOP_DESC BackFace;
} 	D3D11_DEPTH_STENCIL_DESC;

typedef struct D3D11_SHADER_RESOURCE_VIEW_DESC {
	UINT Format;
	UINT ViewDimension;
	UINT MostDetailedMip;
	UINT MipLevels;
} D3D11_SHADER_RESOURCE_VIEW_DESC;

struct SimpleVertex
{
	float x;
	float y;
	float z;
	float u;
	float v;
};



typedef struct D3D11_TEX1D_DSV
{
    UINT MipSlice;
} 	D3D11_TEX1D_DSV;

typedef struct D3D11_TEX1D_ARRAY_DSV
{
    UINT MipSlice;
    UINT FirstArraySlice;
    UINT ArraySize;
} 	D3D11_TEX1D_ARRAY_DSV;

typedef struct D3D11_TEX2D_DSV
{
    UINT MipSlice;
} 	D3D11_TEX2D_DSV;

typedef struct D3D11_TEX2D_ARRAY_DSV
{
    UINT MipSlice;
    UINT FirstArraySlice;
    UINT ArraySize;
} 	D3D11_TEX2D_ARRAY_DSV;

typedef struct D3D11_TEX2DMS_DSV
{
    UINT UnusedField_NothingToDefine;
} 	D3D11_TEX2DMS_DSV;

typedef struct D3D11_TEX2DMS_ARRAY_DSV
{
    UINT FirstArraySlice;
    UINT ArraySize;
} 	D3D11_TEX2DMS_ARRAY_DSV;

typedef 
enum D3D11_DSV_FLAG
{	D3D11_DSV_READ_ONLY_DEPTH	= 0x1L,
D3D11_DSV_READ_ONLY_STENCIL	= 0x2L
} 	D3D11_DSV_FLAG;

typedef struct D3D11_DEPTH_STENCIL_VIEW_DESC
{
    UINT Format;
    UINT ViewDimension;
    UINT Flags;
    union 
    {
        D3D11_TEX1D_DSV Texture1D;
        D3D11_TEX1D_ARRAY_DSV Texture1DArray;
        D3D11_TEX2D_DSV Texture2D;
        D3D11_TEX2D_ARRAY_DSV Texture2DArray;
        D3D11_TEX2DMS_DSV Texture2DMS;
        D3D11_TEX2DMS_ARRAY_DSV Texture2DMSArray;
    } 	;
} 	D3D11_DEPTH_STENCIL_VIEW_DESC;


typedef struct D3D11_VIEWPORT
{
    FLOAT TopLeftX;
    FLOAT TopLeftY;
    FLOAT Width;
    FLOAT Height;
    FLOAT MinDepth;
    FLOAT MaxDepth;
} 	D3D11_VIEWPORT;


class CDXDevice11
{
public:
	FAST_DEFINE_VIRFUNC(CreateTexture2D, (LPVOID pObject, const D3D11_TEXTURE2D_DESC *pDesc, LPVOID *pInitialData, void **ppTexture2D), (pObject, pDesc, pInitialData, ppTexture2D), 5);
	FAST_DEFINE_VIRFUNC(GetImmediateContext, (LPVOID pObject, void **pDeviceContext), (pObject, pDeviceContext), 40);
	FAST_DEFINE_VIRFUNC(CreateShaderResourceView, (LPVOID pObject, void *pResource, const void *pDesc, void**ppSRView), (pObject, pResource, pDesc, ppSRView), 7);
	FAST_DEFINE_VIRFUNC(CreateVertexShader, (LPVOID pObject, const void *pShaderBytecode, SIZE_T BytecodeLength, void *pClassLinkage, void **ppVertexShader), (pObject, pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader), 12);
	FAST_DEFINE_VIRFUNC(CreatePixelShader, (LPVOID pObject, const void *pShaderBytecode, SIZE_T BytecodeLength, void *pClassLinkage, void **ppPixelShader), (pObject, pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader), 15);
	FAST_DEFINE_VIRFUNC(CreateInputLayout, (LPVOID pObject, const void *pInputElementDescs, UINT NumElements, const void *pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, void **ppInputLayout), (pObject, pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout), 11);
	FAST_DEFINE_VIRFUNC(CreateBuffer, (LPVOID pObject, const void *pDesc, const void *pInitialData, void **ppBuffer), (pObject, pDesc, pInitialData, ppBuffer), 3);
	FAST_DEFINE_VIRFUNC(CreateBlendState, (LPVOID pObject, const void *pBlendStateDesc, void **ppBlendState), (pObject, pBlendStateDesc, ppBlendState), 20);
	FAST_DEFINE_VIRFUNC(CreateDepthStencilState, (LPVOID pObject, const void *pDepthStencilDesc, void **ppDepthStencilState), (pObject, pDepthStencilDesc, ppDepthStencilState), 21);
	FAST_DEFINE_VIRFUNC(CreateRasterizerState, (LPVOID pObject, const void *pRasterizerDesc, void **ppRasterizerState), (pObject, pRasterizerDesc, ppRasterizerState), 22);
    FAST_DEFINE_VIRFUNC(CreateSamplerState, (LPVOID pObject, const void *pSamplerDesc, void **ppSamplerState), (pObject, pSamplerDesc, ppSamplerState), 23);
    FAST_DEFINE_VIRFUNC(CreateRenderTargetView, (LPVOID pObject, void *pResource, const void *pDesc, void** ppRTView), (pObject, pResource, pDesc, ppRTView), 9);
    FAST_DEFINE_VIRFUNC(CreateDepthStencilView, (LPVOID pObject, void *pResource, const void *pDesc, void** ppDepthStencilView), (pObject, pResource, pDesc, ppDepthStencilView), 10);




};


struct D3DPRESENT_PARAMETERS_9
{
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    UINT                BackBufferFormat;
    UINT                BackBufferCount;

    UINT                MultiSampleType;
    DWORD               MultiSampleQuality;

    UINT                SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    UINT                AutoDepthStencilFormat;
    DWORD               Flags;

    /* FullScreen_RefreshRateInHz must be zero for Windowed mode */
    UINT                FullScreen_RefreshRateInHz;
    UINT                PresentationInterval;
};


struct D3DPRESENT_PARAMETERS_8
{
    UINT                BackBufferWidth;
    UINT                BackBufferHeight;
    UINT                BackBufferFormat;
    UINT                BackBufferCount;

    UINT                MultiSampleType;

    UINT                SwapEffect;
    HWND                hDeviceWindow;
    BOOL                Windowed;
    BOOL                EnableAutoDepthStencil;
    UINT                AutoDepthStencilFormat;
    DWORD               Flags;

    /* Following elements must be zero for Windowed mode */
    UINT                FullScreen_RefreshRateInHz;
    UINT                FullScreen_PresentationInterval;

};


class CDXSwapChian
{
public:
	FAST_DEFINE_VIRFUNC(GetBuffer, (LPVOID pObject, UINT Buffer, REFIID riid, void **ppSurface), (pObject, Buffer, riid, ppSurface), 9);
	FAST_DEFINE_VIRFUNC(GetDesc, (LPVOID pObject, DXGI_SWAP_CHAIN_DESC * pDesc), (pObject, pDesc), 12);
	FAST_DEFINE_VIRFUNC(GetDevice, (LPVOID pObject, REFIID riid, void **ppDevice), (pObject, riid, ppDevice), 7);
};

class CDXSwapChian9
{
public:
    FAST_DEFINE_VIRFUNC(GetDevice, (LPVOID pObject, LPVOID * ppDevice), (pObject, ppDevice), 8);
    FAST_DEFINE_VIRFUNC(GetPresentParameters, (LPVOID pObject, D3DPRESENT_PARAMETERS_9* pPresentationParameters), (pObject, pPresentationParameters), 9);
};

class CDXDeviceContext11
{
public:
	FAST_DEFINE_VIRFUNC(Map,(LPVOID pObject, LPVOID pResource, UINT Subresource, UINT MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE *pMappedResource), (pObject, pResource, Subresource, MapType, MapFlags, pMappedResource), 14);
	FAST_DEFINE_VIRFUNC(Unmap, (LPVOID pObject, LPVOID pResource, UINT Subresource), (pObject, pResource, Subresource), 15);
	FAST_DEFINE_VIRFUNC(CopyResource, (LPVOID pObject, LPVOID pDest, LPVOID pSrc), (pObject, pDest, pSrc), 47);
	FAST_DEFINE_VIRFUNC(IAGetIndexBuffer, (LPVOID pObject, LPVOID *ppIndexBuffer, UINT *Format, UINT *Offset), (pObject, ppIndexBuffer, Format, Offset), 80);
	FAST_DEFINE_VIRFUNC(IAGetInputLayout, (LPVOID pObject, LPVOID *ppInputLayout), (pObject, ppInputLayout), 78);
	FAST_DEFINE_VIRFUNC(IAGetPrimitiveTopology, (LPVOID pObject, LPVOID pTopology), (pObject, pTopology), 83);
	FAST_DEFINE_VIRFUNC(IAGetVertexBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers, LPVOID *ppVertexBuffers, UINT *pStrides, UINT *pOffsets), (pObject, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets), 79);
	FAST_DEFINE_VIRFUNC(IASetIndexBuffer, (LPVOID pObject, LPVOID *pIndexBuffer, UINT Format, UINT Offset), (pObject, pIndexBuffer, Format, Offset), 19);
	FAST_DEFINE_VIRFUNC(IASetInputLayout, (LPVOID pObject, LPVOID pInputLayout), (pObject, pInputLayout), 17);

    FAST_DEFINE_VIRFUNC(GSGetShader, (LPVOID pObject, void **ppGeometryShader, void **ppClassInstances, UINT *pNumClassInstances), (pObject, ppGeometryShader, ppClassInstances, pNumClassInstances), 82);
    FAST_DEFINE_VIRFUNC(HSGetShader, (LPVOID pObject, void **ppHullShader, void **ppClassInstances, UINT *pNumClassInstances), (pObject, ppHullShader, ppClassInstances, pNumClassInstances), 98);
    FAST_DEFINE_VIRFUNC(DSGetShader, (LPVOID pObject, void **ppDomainShader, void **ppClassInstances, UINT *pNumClassInstances), (pObject, ppDomainShader, ppClassInstances, pNumClassInstances), 102);
    FAST_DEFINE_VIRFUNC(CSGetShader, (LPVOID pObject, void **ppComputeShader, void **ppClassInstances, UINT *pNumClassInstances), (pObject, ppComputeShader, ppClassInstances, pNumClassInstances), 107);


	FAST_DEFINE_VIRFUNC(IASetPrimitiveTopology, (LPVOID pObject, UINT Topology), (pObject, Topology), 24);
	FAST_DEFINE_VIRFUNC(IASetVertexBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers, LPVOID *ppVertexBuffers, const UINT *pStrides, const UINT *pOffsets), (pObject, StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets), 18);
	FAST_DEFINE_VIRFUNC(OMGetBlendState, (LPVOID pObject, LPVOID *ppBlendState, FLOAT BlendFactor[4], UINT *pSampleMask), (pObject, ppBlendState, BlendFactor, pSampleMask), 91);
	FAST_DEFINE_VIRFUNC(OMGetDepthStencilState, (LPVOID pObject, LPVOID *ppDepthStencilState, UINT *pStencilRef), (pObject, ppDepthStencilState, pStencilRef), 92);
	FAST_DEFINE_VIRFUNC(OMSetBlendState, (LPVOID pObject, LPVOID pBlendState, const FLOAT BlendFactor[4], UINT SampleMask), (pObject, pBlendState, BlendFactor, SampleMask), 35);
	FAST_DEFINE_VIRFUNC(OMSetDepthStencilState, (LPVOID pObject, LPVOID pDepthStencilState, UINT StencilRef), (pObject, pDepthStencilState, StencilRef), 36);
	FAST_DEFINE_VIRFUNC(VSGetShader, (LPVOID pObject, void **ppVertexShader, void **ppClassInstances, UINT *pNumClassInstances), (pObject, ppVertexShader, ppClassInstances, pNumClassInstances), 76);
	FAST_DEFINE_VIRFUNC(VSSetShader, (LPVOID pObject, void *pVertexShader, void **ppClassInstances, UINT NumClassInstances), (pObject, pVertexShader, ppClassInstances, NumClassInstances), 11);
	FAST_DEFINE_VIRFUNC(PSGetShader, (LPVOID pObject, void **ppPixelShader, void **ppClassInstances, UINT *pNumClassInstances), (pObject, ppPixelShader, ppClassInstances, pNumClassInstances), 74);
	FAST_DEFINE_VIRFUNC(PSSetShader, (LPVOID pObject, void *pPixelShader, void ** ppClassInstances, UINT NumClassInstances), (pObject, pPixelShader, ppClassInstances, NumClassInstances), 9);
	FAST_DEFINE_VIRFUNC(PSGetSamplers, (LPVOID pObject, UINT StartSlot, UINT NumSamplers, void **ppSamplers), (pObject, StartSlot, NumSamplers, ppSamplers), 75);
	FAST_DEFINE_VIRFUNC(PSSetSamplers, (LPVOID pObject, UINT StartSlot, UINT NumSamplers, void **ppSamplers), (pObject, StartSlot, NumSamplers, ppSamplers), 10);
	FAST_DEFINE_VIRFUNC(PSGetShaderResources, (LPVOID pObject, UINT StartSlot, UINT NumViews, void **ppShaderResourceViews), (pObject, StartSlot,  NumViews, ppShaderResourceViews), 73);
	FAST_DEFINE_VIRFUNC(PSSetShaderResources, (LPVOID pObject, UINT StartSlot, UINT NumViews, void **ppShaderResourceViews), (pObject, StartSlot,  NumViews, ppShaderResourceViews), 8);
	FAST_DEFINE_VIRFUNC(RSGetState, (LPVOID pObject, void **ppRasterizerState), (pObject, ppRasterizerState), 94);
	FAST_DEFINE_VIRFUNC(RSSetState, (LPVOID pObject, void *pRasterizerState), (pObject, pRasterizerState), 43);
	FAST_DEFINE_VIRFUNC(Draw, (LPVOID pObject, UINT VertexCount, UINT StartVertexLocation), (pObject, VertexCount, StartVertexLocation), 13);
    FAST_DEFINE_VIRFUNC(OMGetRenderTargets, (LPVOID pObject, UINT NumViews, void **ppRenderTargetViews, void** ppDepthStencilView ), (pObject, NumViews, ppRenderTargetViews, ppDepthStencilView), 89);
    FAST_DEFINE_VIRFUNC(OMGetRenderTargetsAndUnorderedAccessViews, (LPVOID pObject, UINT NumRTVs, void **ppRenderTargetViews, void** ppDepthStencilView, UINT UAVStartSlot,UINT NumUAVs, void **ppUnorderedAccessViews), (pObject, NumRTVs, ppRenderTargetViews, ppDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews), 90);

    FAST_DEFINE_VIRFUNC(ClearRenderTargetView, (LPVOID pObject, void *pRenderTargetViews, const FLOAT ColorRGBA[ 4 ] ), (pObject, pRenderTargetViews, ColorRGBA), 50);
    FAST_DEFINE_VIRFUNC(OMSetRenderTargets, (LPVOID pObject, UINT NumViews, void **ppRenderTargetViews, void* pDepthStencilView ), (pObject, NumViews, ppRenderTargetViews, pDepthStencilView), 33);
    
    FAST_DEFINE_VIRFUNC(VSSetConstantBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers,  LPVOID *ppConstantBuffers ), (pObject, StartSlot, NumBuffers, ppConstantBuffers), 7);
    FAST_DEFINE_VIRFUNC(PSSetConstantBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers,  LPVOID *ppConstantBuffers ), (pObject, StartSlot, NumBuffers, ppConstantBuffers), 16);
    FAST_DEFINE_VIRFUNC(GSSetConstantBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers,  LPVOID *ppConstantBuffers ), (pObject, StartSlot, NumBuffers, ppConstantBuffers), 22);
    FAST_DEFINE_VIRFUNC(HSSetConstantBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers,  LPVOID *ppConstantBuffers ), (pObject, StartSlot, NumBuffers, ppConstantBuffers), 62);
    FAST_DEFINE_VIRFUNC(DSSetConstantBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers,  LPVOID *ppConstantBuffers ), (pObject, StartSlot, NumBuffers, ppConstantBuffers), 66);
    FAST_DEFINE_VIRFUNC(CSSetConstantBuffers, (LPVOID pObject, UINT StartSlot, UINT NumBuffers,  LPVOID *ppConstantBuffers ), (pObject, StartSlot, NumBuffers, ppConstantBuffers), 71);
    FAST_DEFINE_VIRFUNC(ClearState, (LPVOID pObject), ( pObject), 110);
    FAST_DEFINE_VIRFUNC(FinishCommandList, (LPVOID pObject, BOOL RestoreDeferredContextState, LPVOID *ppCommandList), ( pObject, RestoreDeferredContextState, ppCommandList), 114);
    FAST_DEFINE_VIRFUNC(RSGetViewports, (LPVOID pObject, UINT* pNumViewports, LPVOID pViewports), ( pObject, pNumViewports, pViewports), 95);

    FAST_DEFINE_VIRFUNC(GSSetShader, (LPVOID pObject, void *pShader, void **ppClassInstances, UINT NumClassInstances), (pObject, pShader, ppClassInstances, NumClassInstances), 23);
    FAST_DEFINE_VIRFUNC(RSSetViewports, (LPVOID pObject, UINT NumViewports, const LPVOID pViewports), (pObject, NumViewports, pViewports), 44);

    FAST_DEFINE_VIRFUNC(OMSetRenderTargetsAndUnorderedAccessViews, (LPVOID pObject, UINT NumRTVs, void **ppRenderTargetViews, void* ppDepthStencilView, UINT UAVStartSlot,UINT NumUAVs, void **ppUnorderedAccessViewsconst, UINT *pUAVInitialCounts), (pObject, NumRTVs, ppRenderTargetViews, ppDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViewsconst, pUAVInitialCounts), 34);

    FAST_DEFINE_VIRFUNC(ClearDepthStencilView, (LPVOID pObject, void *pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil), (pObject, pDepthStencilView, ClearFlags, Depth, Stencil), 53);


};

class CDXBlob
{
public:
	FAST_DEFINE_VIRFUNC(GetBufferPointer, (LPVOID pObject), (pObject), 3);
	FAST_DEFINE_VIRFUNC(GetBufferSize, (LPVOID pObject), (pObject), 4);
};





/* Display Modes */
typedef struct _D3DDISPLAYMODE
{
    UINT            Width;
    UINT            Height;
    UINT            RefreshRate;
    UINT            Format;
} D3DDISPLAYMODE;


class CDXIDirect3D8
{
public:
    FAST_DEFINE_VIRFUNC(GetAdapterDisplayMode, (LPVOID pObject, UINT Adapter, D3DDISPLAYMODE* pMode), (pObject, Adapter, pMode), 8);
    FAST_DEFINE_VIRFUNC(CreateDevice, (LPVOID pObject, UINT Adapter, UINT DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS_8* pPresentationParameters, void** ppReturnedDeviceInterface),
        (pObject, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface), 15);
};

class CDXIDirect3D9
{
public:
    FAST_DEFINE_VIRFUNC(CreateDevice, (LPVOID pObject, UINT Adapter, UINT DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS_9* pPresentationParameters, void** ppReturnedDeviceInterface),
        (pObject, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface), 16);
};
