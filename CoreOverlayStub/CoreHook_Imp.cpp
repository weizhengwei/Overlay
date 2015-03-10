#include "stdafx.h"
#include "CoreHook_WinApi.h"
#include "CoreHook_DirectX.h"
#include "CoreHook.h"
#include "CoreHook_imp.h"


// dx [2/6/2015 liuyu]


FUNC_Present g_pPresent_8 = NULL;
FUNC_Present g_pPresent_9 = NULL;
FUNC_SwapChainPresent g_pSwapChainPresent = NULL;
FUNC_DX9SwapChainPresent g_pDx9SwapChainPresent = NULL;
FUNC_SwapChainResizeBuffers g_pSwapChainResizeBuffers = NULL;
FUNC_SetFullscreenState g_pSetFullscreenState = NULL;
FUNC_Reset g_pReset_8 = NULL;
FUNC_Reset g_pReset_9 = NULL;
FUNC_PresentEx g_pPresentEx = NULL;
FUNC_ResetEx g_pResetEx = NULL;
FUNC_Release g_pRelease_9 = NULL;
FUNC_Release g_pRelease_8 = NULL;
FUNC_Release g_pRelease_11 = NULL;

PDETOUR_CREATE_PROCESS_ROUTINEA g_pCreateProcessA = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEW g_pCreateProcessW = NULL;

PDETOUR_CREATE_PROCESS_ROUTINEA g_pLdrCreateProcessA = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEW g_pLdrCreateProcessW = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEA g_pLdrForCallerCreateProcessA = NULL;
PDETOUR_CREATE_PROCESS_ROUTINEW g_pLdrForCallerCreateProcessW = NULL;

FUNC_LdrGetProcedureAddressForCaller g_pLdrGetProcedureAddressForCaller = NULL;
FUNC_LdrGetProcedureAddress g_pLdrGetProcedureAddress = NULL;

FUNC_RtlInitAnsiString g_pRtlInitAnsiString = NULL;

FUNC_LoadLibraryExW g_pLoadLibraryExW = NULL;


FUNC_GetCursorPos g_pGetCursorPos = NULL;
FUNC_SetCursorPos g_pSetCursorPos = NULL;
FUNC_ShowCursor g_pShowCursor = NULL;
FUNC_GetAsyncKeyState g_pGetAsyncKeyState = NULL;
FUNC_GetRawInputBuffer g_pGetRawInputBuffer = NULL;
FUNC_GetRawInputData g_pGetRawInputData = NULL;
FUNC_SetCursor g_pSetCursor = NULL;


GUID IID_IDXGIFactory1 = {0x770aae78,0xf26f,0x4dba,{0xa8,0x29,0x25,0x3c,0x83,0xd1,0xb3,0x87}};
GUID IID_IDXGIFactory2 = {0x7b7166ec,0x21c7,0x44ae,{0xb2,0x1a,0xc9,0xae,0x32,0x1a,0xe3,0x69}};

Hook g_Hook;



void Hook::HookCreateProcess()
{
#ifndef _WIN64
    FAST_USE_DETOUR

    // on Win7 64, sometimes you cannot get the real function address by explicitly passing the function to a variable, like LPVOID pFunc = CreateProcessW,
    // so we have to retrieve this real address from the export directory in the PE header of kernel32(in memory)
    HMODULE hModKernel32 = GetModuleHandle(_T("kernel32.dll"));

    g_pCreateProcessA = (PDETOUR_CREATE_PROCESS_ROUTINEA)GetProcAddress(hModKernel32, "CreateProcessA");
    DetourAttach(&(PVOID&)g_pCreateProcessA, &MyCreateProcessA);
    g_pCreateProcessW = (PDETOUR_CREATE_PROCESS_ROUTINEW)GetProcAddress(hModKernel32, "CreateProcessW");
    DetourAttach(&(PVOID&)g_pCreateProcessW, &MyCreateProcessW);

    // [10/24/2014 liuyu]
    // On vista and above , windows shim engine will hook GetProcAddress, so when we call GetProcAddress to get address of CreateProcess, it will give us a fake
    // address which located in aclayers.dll named like NS_EVELTER_APIHOOK_CreateProcess, sometimes it works fine, cause some games will use the fake address,
    // but sometimes there are some games calling the real address in kernel32 ,so we have to use LdrGetProcedureAddress to get the real address

    HMODULE hModNTdll = GetModuleHandle(_T("ntdll.dll"));

    g_pRtlInitAnsiString = (FUNC_RtlInitAnsiString)GetProcAddress(hModNTdll, "RtlInitAnsiString");
    g_pLdrGetProcedureAddress = (FUNC_LdrGetProcedureAddress)GetProcAddress(hModNTdll, "LdrGetProcedureAddress");

    //win8 :LdrGetProcedureAddressForCaller replaces LdrGetProcedureAddress
    g_pLdrGetProcedureAddressForCaller = (FUNC_LdrGetProcedureAddressForCaller)GetProcAddress(hModNTdll, "LdrGetProcedureAddressForCaller"); 
    //if (NULL == g_pLdrGetProcedureAddressForCaller)
    //    OutputDebugString(_T("LdrGetProcedureAddressForCaller failed"));

    if (NULL == g_pLdrGetProcedureAddress || NULL == g_pRtlInitAnsiString)
    {
        OutputDebugString(_T("GetProcAddress RtlInitAnsiString or LdrGetProcedureAddress failed"));
        return ;
    }

    ANSI_STRING name;
    (*g_pRtlInitAnsiString)(&name, "CreateProcessW");

    (*g_pLdrGetProcedureAddress)(hModKernel32, &name, 0, (PVOID*)&g_pLdrCreateProcessW);
    DetourAttach(&(PVOID&)g_pLdrCreateProcessW, &MyLdrCreateProcessW);   

    (*g_pRtlInitAnsiString)(&name, "CreateProcessA");

    (*g_pLdrGetProcedureAddress)(hModKernel32, &name, 0, (PVOID*)&g_pLdrCreateProcessA);
    DetourAttach(&(PVOID&)g_pLdrCreateProcessA, &MyLdrCreateProcessA);  

    if (g_pLdrCreateProcessW == g_pCreateProcessW && NULL != g_pLdrGetProcedureAddressForCaller)  //means win8 platform
    {
        (*g_pRtlInitAnsiString)(&name, "CreateProcessW");
        (*g_pLdrGetProcedureAddressForCaller)(hModKernel32, &name, 0, (PVOID*)&g_pLdrForCallerCreateProcessW, 0, NULL);
        DetourAttach(&(PVOID&)g_pLdrForCallerCreateProcessW, &MyLdrForCallerCreateProcessW);   

        (*g_pRtlInitAnsiString)(&name, "CreateProcessA");
        (*g_pLdrGetProcedureAddressForCaller)(hModKernel32, &name, 0, (PVOID*)&g_pLdrForCallerCreateProcessA, 0, NULL);
        DetourAttach(&(PVOID&)g_pLdrForCallerCreateProcessA, &MyLdrForCallerCreateProcessA);  
    }
#endif
}

void Hook::UnHookCreateProcess()
{
    FAST_USE_DETOUR 

    if (NULL != g_pCreateProcessA)
    {
        DetourDetach(&(PVOID&)g_pCreateProcessA, &MyCreateProcessA);
    }
    if (NULL != g_pCreateProcessW)
    {
        DetourDetach(&(PVOID&)g_pCreateProcessW, &MyCreateProcessW);
    }

    if (NULL != g_pLdrCreateProcessW)
    {
        DetourDetach(&(PVOID&)g_pLdrCreateProcessW, &MyLdrCreateProcessW);   
    }

    if (NULL != g_pLdrCreateProcessA)
    {
        DetourDetach(&(PVOID&)g_pLdrCreateProcessA, &MyLdrCreateProcessA);   
    }

    if (NULL != g_pLdrForCallerCreateProcessW)
    {
        DetourDetach(&(PVOID&)g_pLdrForCallerCreateProcessW, &MyLdrForCallerCreateProcessW);   
    }

    if (NULL != g_pLdrForCallerCreateProcessA)
    {
        DetourDetach(&(PVOID&)g_pLdrForCallerCreateProcessA, &MyLdrForCallerCreateProcessA);   
    }
}

void Hook::HookLoadLibrary()
{
    FAST_USE_DETOUR

    if (GetModuleHandle(_T("kernelbase.dll")))
        g_pLoadLibraryExW = (FUNC_LoadLibraryExW)GetProcAddress(GetModuleHandle(_T("kernelbase.dll")), "LoadLibraryExW");
    else
        g_pLoadLibraryExW = (FUNC_LoadLibraryExW)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryExW");

    DetourAttach(&(PVOID&)g_pLoadLibraryExW, &MyLoadLibraryExW);

}

void Hook::UnHookLoadLibrary()
{
    FAST_USE_DETOUR

    if (NULL == g_pLoadLibraryExW)
    {
        DetourDetach(&(PVOID&)g_pLoadLibraryExW, &MyLoadLibraryExW);
    }
}





void Hook::HookUser32Method()
{
    FAST_USE_DETOUR

    HMODULE hMod = GetModuleHandle(_T("user32"));

    g_pSetCursor = (FUNC_SetCursor)GetProcAddress(hMod, "SetCursor");
    DetourAttach(&(PVOID&)g_pSetCursor, &MySetCursor);
    g_pGetRawInputBuffer = (FUNC_GetRawInputBuffer)GetProcAddress(hMod, "GetRawInputBuffer");
    DetourAttach(&(PVOID&)g_pGetRawInputBuffer, &MyGetRawInputBuffer);
    g_pGetRawInputData = (FUNC_GetRawInputData)GetProcAddress(hMod, "GetRawInputData");
    DetourAttach(&(PVOID&)g_pGetRawInputData, &MyGetRawInputData);
    g_pGetAsyncKeyState = (FUNC_GetAsyncKeyState)GetProcAddress(hMod, "GetAsyncKeyState");
    DetourAttach(&(PVOID&)g_pGetAsyncKeyState, &MyGetAsyncKeyState);
    g_pGetCursorPos = (FUNC_GetCursorPos)GetProcAddress(hMod, "GetCursorPos");
    DetourAttach(&(PVOID&)g_pGetCursorPos, &MyGetCursorPos);
    g_pSetCursorPos = (FUNC_SetCursorPos)GetProcAddress(hMod, "SetCursorPos");
    DetourAttach(&(PVOID&)g_pSetCursorPos, &MySetCursorPos);
    g_pShowCursor = (FUNC_ShowCursor)GetProcAddress(hMod, "ShowCursor");
    DetourAttach(&(PVOID&)g_pShowCursor, &MyShowCursor);

}

void Hook::UnHookUser32Method()
{
    if (NULL != g_pSetCursor)
    {
        DetourDetach(&(PVOID&)g_pSetCursor, &MySetCursor);
    }

    if (NULL != g_pGetRawInputBuffer)
    {
        DetourDetach(&(PVOID&)g_pGetRawInputBuffer, &MyGetRawInputBuffer);
    }

    if (NULL != g_pGetRawInputData)
    {
        DetourDetach(&(PVOID&)g_pGetRawInputData, &MyGetRawInputData);
    }

    if (NULL != g_pGetAsyncKeyState)
    {
        DetourDetach(&(PVOID&)g_pGetAsyncKeyState, &MyGetAsyncKeyState);
    }

    if (NULL != g_pGetCursorPos)
    {
        DetourDetach(&(PVOID&)g_pGetCursorPos, &MyGetCursorPos);
    }

    if (NULL != g_pSetCursorPos)
    {
        DetourDetach(&(PVOID&)g_pSetCursorPos, &MySetCursorPos);
    }

    if (NULL != g_pShowCursor)
    {
        DetourDetach(&(PVOID&)g_pShowCursor, &MyShowCursor);
    }
}

void Hook::HookD3D()
{
    CreateD3DWindow();
    CreateSwapChainDESC();

    HookD3D8();
    HookD3D9();
    HookD3D11();

    DestroyD3DWindow();
}


void Hook::UnHookD3D()
{
    FAST_USE_DETOUR
    
    if (NULL != g_pPresent_8)
    {
        DetourDetach(&(PVOID&)g_pPresent_8, &MyPresent_8);
    }

    if (NULL != g_pReset_8)
    {
        DetourDetach(&(PVOID&)g_pReset_8, &MyReset_8);
    }

    if (NULL != g_pRelease_8)
    {
        DetourDetach(&(PVOID&)g_pRelease_8, &MyRelease_8);
    }

    if (NULL != g_pPresent_9)
    {
        DetourDetach(&(PVOID&)g_pPresent_9, &MyPresent_9);
    }

    if (NULL != g_pReset_9)
    {
        DetourDetach(&(PVOID&)g_pReset_9, &MyReset_9);
    }

    if (NULL != g_pRelease_9)
    {
        DetourDetach(&(PVOID&)g_pRelease_9, &MyRelease_9);
    }

    if (NULL != g_pPresentEx)
    {
        DetourDetach(&(PVOID&)g_pPresentEx, &MyPresentEx);
    }

    if (NULL != g_pSwapChainPresent)
    {
        DetourDetach(&(PVOID&)g_pSwapChainPresent, &MySwapChainPresent);
    }

    if (NULL != g_pDx9SwapChainPresent)
    {
        DetourDetach(&(PVOID&)g_pDx9SwapChainPresent, &MyDX9SwapChainPresent);
    }

    if (NULL != g_pSwapChainResizeBuffers)
    {
        DetourDetach(&(PVOID&)g_pSwapChainResizeBuffers, &MySwapChainResizeBuffers);
    }

    if (NULL != g_pRelease_11)
    {
        DetourDetach(&(PVOID&)g_pRelease_11, &MyRelease_11);
    }
}

void Hook::HookD3D8()
{
    HMODULE d3d8dll = LoadLibraryA("D3D8.dll");
    if (NULL == d3d8dll)
        return;

    FUNC_Direct3DCreate8 d3dCreate8 = (FUNC_Direct3DCreate8)GetProcAddress(d3d8dll, "Direct3DCreate8");
    if (NULL == d3dCreate8)
        return;

    LPVOID pIDirect3D8 = d3dCreate8(/*D3D_SDK_VERSION*/220);
    if (NULL == pIDirect3D8)
        return;

    D3DDISPLAYMODE displayMode;
    CDXIDirect3D8::GetAdapterDisplayMode(pIDirect3D8, /*D3DADAPTER_DEFAULT*/0, &displayMode);

    D3DPRESENT_PARAMETERS_8 d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed   = TRUE;
    d3dpp.SwapEffect = /*D3DSWAPEFFECT_COPY*/1;
    d3dpp.BackBufferFormat = displayMode.Format;

    LPVOID pD3DDevice = NULL;
    CDXIDirect3D8::CreateDevice(pIDirect3D8, /*D3DADAPTER_DEFAULT*/0, /*D3DDEVTYPE_HAL*/1, m_hD3DWindow, /*D3DCREATE_SOFTWARE_VERTEXPROCESSING*/0x00000020L, &d3dpp, &pD3DDevice);
    if (NULL == pD3DDevice)
        return;

    FAST_USE_DETOUR

    g_pPresent_8 = (FUNC_Present)CoreHook::Unity::GetVirtualCall(pD3DDevice, 15);
    DetourAttach(&(PVOID&)g_pPresent_8, &MyPresent_8);
    g_pReset_8 = (FUNC_Reset)CoreHook::Unity::GetVirtualCall(pD3DDevice, 14);
    DetourAttach(&(PVOID&)g_pReset_8, &MyReset_8);

    g_pRelease_8 = (FUNC_Release)CoreHook::Unity::GetVirtualCall(pD3DDevice, 2);
    DetourAttach(&(PVOID&)g_pRelease_8, &MyRelease_8);

    CUnknown::Release(pD3DDevice);
    CUnknown::Release(pIDirect3D8);

    OutputDebugStringA("Hook D3D8 success");
}

void Hook::HookD3D9()
{    
    HMODULE d3d9dll = LoadLibraryA("D3D9.dll");
    if (NULL == d3d9dll)
        return;

    FUNC_Direct3DCreate9 d3dCreate9 = (FUNC_Direct3DCreate9)GetProcAddress(d3d9dll, "Direct3DCreate9");
    if (NULL == d3dCreate9)
        return;

    LPVOID pIDirect3D9 = d3dCreate9(/*D3D_SDK_VERSION*/32);
    if (NULL == pIDirect3D9)
        return;


    D3DPRESENT_PARAMETERS_9 d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed   = TRUE;
    d3dpp.SwapEffect = /*D3DSWAPEFFECT_COPY*/1;


    LPVOID pD3DDevice = NULL;
    CDXIDirect3D9::CreateDevice(pIDirect3D9, /*D3DADAPTER_DEFAULT*/0, /*D3DDEVTYPE_HAL*/1, m_hD3DWindow, /*D3DCREATE_SOFTWARE_VERTEXPROCESSING*/0x00000020L, &d3dpp, &pD3DDevice);
    if (NULL == pD3DDevice)
        return;

    FAST_USE_DETOUR

    g_pPresent_9 = (FUNC_Present)CoreHook::Unity::GetVirtualCall(pD3DDevice, 17);
    DetourAttach(&(PVOID&)g_pPresent_9, &MyPresent_9);
    g_pReset_9 = (FUNC_Reset)CoreHook::Unity::GetVirtualCall(pD3DDevice, 16);
    DetourAttach(&(PVOID&)g_pReset_9, &MyReset_9);

    g_pPresentEx = (FUNC_PresentEx)CoreHook::Unity::GetVirtualCall(pD3DDevice, 121);
    DetourAttach(&(PVOID&)g_pPresentEx, &MyPresentEx);


    LPVOID pSwapChain = NULL;
    CDXDevice9::GetSwapChain(pD3DDevice, 0, &pSwapChain);
    if (NULL != pSwapChain)
    {
        g_pDx9SwapChainPresent = (FUNC_DX9SwapChainPresent)CoreHook::Unity::GetVirtualCall(pSwapChain, 0x3);
        DetourAttach(&(PVOID&)g_pDx9SwapChainPresent, &MyDX9SwapChainPresent);
        CUnknown::Release(pSwapChain);
    }


    g_pRelease_9 = (FUNC_Release)CoreHook::Unity::GetVirtualCall(pD3DDevice, 2);
    DetourAttach(&(PVOID&)g_pRelease_9, &MyRelease_9);

    CUnknown::Release(pD3DDevice);
    CUnknown::Release(pIDirect3D9);

    OutputDebugStringA("Hook D3D9 success");
}

void Hook::HookD3D11()
{
    HMODULE d3d11dll=LoadLibraryA("D3D11.dll");

    if (NULL == d3d11dll)
        return;
        
    FUNC_D3D11CreateDeviceAndSwapChain d3d11create=(FUNC_D3D11CreateDeviceAndSwapChain)GetProcAddress(d3d11dll, "D3D11CreateDeviceAndSwapChain");
    if (NULL == d3d11create)
        return;

    LPVOID pSwapChain = NULL;
    LPVOID pd3dDevice11 = NULL;
    LPVOID pd3dDevice11Context = NULL;
    UINT fl = 0;
    d3d11create( NULL, /*D3D_DRIVER_TYPE_HARDWARE*/1, NULL, 0, NULL, 0, /*D3D11_SDK_VERSION*/7, (void*)&m_sd, &pSwapChain, &pd3dDevice11, &fl, &pd3dDevice11Context);
    if (NULL == pSwapChain)
        return;
    
    FAST_USE_DETOUR

    g_pSwapChainPresent = (FUNC_SwapChainPresent)CoreHook::Unity::GetVirtualCall(pSwapChain, 8);
    DetourAttach(&(PVOID&)g_pSwapChainPresent, &MySwapChainPresent);
    g_pSwapChainResizeBuffers = (FUNC_SwapChainResizeBuffers)CoreHook::Unity::GetVirtualCall(pSwapChain, 13);
    DetourAttach(&(PVOID&)g_pSwapChainResizeBuffers, &MySwapChainResizeBuffers);

    g_pSetFullscreenState = (FUNC_SetFullscreenState)CoreHook::Unity::GetVirtualCall(pSwapChain, 10);
    DetourAttach(&(PVOID&)g_pSetFullscreenState, &MySetFullscreenState);

    g_pRelease_11 = (FUNC_Release)CoreHook::Unity::GetVirtualCall(pSwapChain, 2);
    DetourAttach(&(PVOID&)g_pRelease_11, &MyRelease_11);

    CUnknown::Release(pSwapChain);

    if (NULL != pd3dDevice11Context)
        CUnknown::Release(pd3dDevice11Context);

    if (NULL != pd3dDevice11)
        CUnknown::Release(pd3dDevice11);

    OutputDebugStringA("Hook D3D11 success");

}

void Hook::CreateD3DWindow()
{
    WNDCLASSEXW wcex;

    ZeroMemory(&wcex, sizeof(wcex));

    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DefWindowProc;//WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;

    wcex.hInstance = GetModuleHandle(0);
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"OVERLAY";
    if( !RegisterClassExW( &wcex ) )
        OutputDebugStringA("Failure\n");


    // Create window   
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    m_hD3DWindow = CreateWindowW( L"OVERLAY", L"OVERLAY",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, GetModuleHandle(0),
        NULL );
}

void Hook::CreateSwapChainDESC()
{
    ZeroMemory( &m_sd, sizeof( m_sd ) );
    m_sd.BufferCount = 1;
    m_sd.BufferDesc.Width = 640;
    m_sd.BufferDesc.Height = 480;
    m_sd.BufferDesc.Format = /*DXGI_FORMAT_R8G8B8A8_UNORM*/28;
    m_sd.BufferDesc.RefreshRate.Numerator = 60;
    m_sd.BufferDesc.RefreshRate.Denominator = 1;
    m_sd.BufferUsage = /*DXGI_USAGE_RENDER_TARGET_OUTPUT*/( 1L << (1 + 4) );
    m_sd.OutputWindow = m_hD3DWindow;
    m_sd.Count = 1;
    m_sd.Quality = 0;
    m_sd.Windowed = TRUE;

}

void Hook::DestroyD3DWindow()
{
    DestroyWindow(m_hD3DWindow);
}