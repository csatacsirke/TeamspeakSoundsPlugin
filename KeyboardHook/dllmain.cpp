// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "KeyboardHook.h"






BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	KeyboardHook::SetDllHandle(hModule);


	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		//MessageBoxW(0, L"Loaded the dll", 0,0 );
		//SetCallerWindow(0);
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		//SetCallerWindow(0);
		//MessageBoxW(0, L"Unloaded the dll", 0,0 );
		break;

	}
	return TRUE;
}
//
//
//LRESULT CALLBACK DllKeyboardProc(
//	_In_  int code,
//	_In_  WPARAM wParam,
//	_In_  LPARAM lParam
//){
//	//blank
//
//	MessageBox(0, L"TOPKEK", L"msg", 0);
//	return CallNextHookEx( 0, code, wParam, lParam );
//
//}

