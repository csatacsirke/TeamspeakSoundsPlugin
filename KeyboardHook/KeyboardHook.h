#pragma once 

#define USE_LOW_LEVEL_KEYBOARD_HOOK 1


#include "CommonParams.h"



#ifndef MAIN
	#include <Windows.h>
	#define KEYBOARD_HOOK_API __declspec(dllexport)
	
#else
	#define KEYBOARD_HOOK_API __declspec(dllimport)
#endif



extern "C" LRESULT KEYBOARD_HOOK_API
	CALLBACK KeyboardProc(int nCode,WPARAM wParam, LPARAM lParam);




BOOL ProcessKeystroke(LPKBDLLHOOKSTRUCT ProcessKeystroke);

namespace KeyboardHook{
	HMODULE KEYBOARD_HOOK_API GetDllHandle();
	void KEYBOARD_HOOK_API SetDllHandle(HMODULE hDll);
}



//BOOL __declspec(dllexport)__stdcall InstallHook();
//BOOL __declspec(dllexport)__stdcall UnInstallhook();

//class StaticHookInstaller{
//	FILE* f1;
//public:
//	
//	StaticHookInstaller(){
//		fopen_s(&f1, "c:\\report.txt","w");
//		fclose(f1);
//		hkb=SetWindowsHookEx(WH_KEYBOARD,(HOOKPROC)KeyboardProc,0,0);
//		MessageBox(0, TEXT("Hook Applied"), 0 , 0);
//	}
//	~StaticHookInstaller(){
//		UnhookWindowsHookEx(hkb);
//		MessageBox(0, TEXT("Hook Deleted"), 0 , 0);
//	}
//
//	
//};