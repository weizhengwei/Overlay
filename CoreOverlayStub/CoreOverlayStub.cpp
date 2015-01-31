// CoreOverlayStub.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CoreHook.h"
#include "detours\detours.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	/*
	** 1. called when dll is loaded.
	*/
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		DetourRestoreAfterWith();
		g_CoreHook.Init(hModule);
	}
	/*
	** 2. called when dll is unloaded
	*/
	else if(ul_reason_for_call == DLL_PROCESS_DETACH)
	{
        DisableThreadLibraryCalls(hModule);
        DetourRestoreAfterWith();
        g_CoreHook.UnInit();
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif