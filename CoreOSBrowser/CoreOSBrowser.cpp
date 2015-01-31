// CoreOSBrowser.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "CoreOSBrowser.h"
#include "data/DataPool.h"
#include "data\ErrorReport.h"
#include "constant.h"
#include "BrowserImpl.h"
#include "..\CoreOverlayStub\detours\detours.h"
#include "browser\client_app.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

void InitHook();

typedef HWND (WINAPI *PDETOUR_SETFOCUS)(HWND hWnd);
PDETOUR_SETFOCUS g_pSetFocus = NULL;

// when osbrowser was running in background, sometimes cef would call setfocus(NULL)
// this makes our windows lose input focus, so we have to make sure this won't work [12/15/2014 liuyu]
HWND WINAPI MySetFocus(HWND hWnd)
{
    if(hWnd == NULL)
    {
        OutputDebugStringA("my set focus");
        return NULL;
    }
    return g_pSetFocus(hWnd);
}




// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	//::MessageBox(NULL,lpCmdLine,lpCmdLine,0);
	CefMainArgs main_args(hInstance);
	CefRefPtr<ClientApp> app(new ClientApp);
	int exit_code = CefExecuteProcess(main_args, app.get());
	if (exit_code >= 0)
		return exit_code;

	if(1 == __argc)
	{
		return FALSE ;
	}
	_tstring strCommandLineType = __targv[1];
	_tstring strhWnd = __targv[2];
	_tstring strClientWnd = __targv[3];
	_tstring strUrl = __targv[4];
	_tstring strAllGameUrl = __targv[5];




	CBrowserImpl::GetInstance()->CursorInit(NULL);
	InitErrorReport(theDataPool.GetBaseDir().c_str(),I_CRASHREPORT);

	CBrowserImpl::GetInstance()->Init((HWND)strhWnd.ConvToInt32(),(HWND)strClientWnd.ConvToInt32(),hInstance,app.get());

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_COREOSBROWSER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
// 	if (!InitInstance (hInstance, nCmdShow))
// 	{
// 		return FALSE;
// 	}

	HWND hWnd;
	hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

    InitHook();

//	ShowWindow(hWnd, nCmdShow);
//	UpdateWindow(hWnd);

	CBrowserImpl::GetInstance()->InitBaseInfo(hWnd,strCommandLineType.ConvToInt32(),strUrl);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COREOSBROWSER));

	//CefRunMessageLoop();
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	CBrowserImpl::GetInstance()->UnInit();
	return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COREOSBROWSER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_COREOSBROWSER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
	
// 		if (CBrowserImpl::GetInstance()->m_smMgr.GetBits())
// 		{
// 			BITMAPINFOHEADER m_bmpHeader;
// 			m_bmpHeader.biSize = sizeof(m_bmpHeader);
// 			m_bmpHeader.biBitCount = 32;
// 			m_bmpHeader.biCompression = BI_RGB;
// 			m_bmpHeader.biWidth = 800;
// 			m_bmpHeader.biHeight = -(600);
// 			m_bmpHeader.biPlanes = 1;
// 			SetDIBitsToDevice(ps.hdc,0,0 , 800,600, 0,0, 0, 600,CBrowserImpl::GetInstance()->m_smMgr.GetBits(), (const BITMAPINFO *)&m_bmpHeader, DIB_RGB_COLORS);
// 		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		CBrowserImpl::GetInstance()->GetMenMgr().Close();
		PostQuitMessage(0);
		break;
 	case WM_COREMESSAGE:
 		CBrowserImpl::GetInstance()->OnProcessMsg(wParam,lParam);
 		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_LBUTTONUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSCHAR:
	case WM_CHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_MOUSEMOVE:
	case WM_MOUSELEAVE:
	case WM_MOUSEWHEEL:
		{
			CBrowserImpl::GetInstance()->SetBrowserEvent(message,wParam,lParam);
			return 0;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



void InitHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    HMODULE hMod = GetModuleHandle(_T("user32.dll"));
    if(hMod)
    {
        g_pSetFocus = (PDETOUR_SETFOCUS)GetProcAddress(hMod, "SetFocus");
        DetourAttach(&(PVOID&)g_pSetFocus, &MySetFocus);
       
    }
    DetourTransactionCommit();

}