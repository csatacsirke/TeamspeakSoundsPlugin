#include "stdafx.h"

#include "KeyboardHookInstaller.h"
//#include "errorcheck.h"
#include <sstream>


#include <KeyboardHook/KeyboardHook.h>

#define KEYBOARDHOOOK_DLL_MANUAL_LOAD 1






#if !KEYBOARDHOOOK_DLL_MANUAL_LOAD


#ifdef _DEBUG
	#pragma comment(lib , "../Debug/KeyboardHook")
#else
	#pragma comment(lib , "../Release/KeyboardHook")
#endif


KeyboardHookInstaller::KeyboardHookInstaller(HWND hWnd=0){
	Ha jol emlékszek ez nem jó ötlet, de már nem tom miért	

	hook=SetWindowsHookEx(WH_KEYBOARD,KeyboardProc,GetModuleHandle(TEXT("KeyboardHook")),0);
		
	if(hook != 0){
		//MessageBox(0, TEXT("Hook Applied"), 0 , 0);
	} else {
		int error = GetLastError();
		std::basic_stringstream<TCHAR> ss;
		ss << TEXT("InstallHook failed: ");
		ss << error;
		MessageBox(0, ss.str().data(), 0 , 0);
	}
}
KeyboardHookInstaller::~KeyboardHookInstaller(){
	Ha jol emlékszek ez nem jó ötlet, de már nem tom miért

	if(UnhookWindowsHookEx(hook) == TRUE){
		//MessageBox(0, TEXT("Hook Deleted"), 0 , 0);
	} else {
		int error = GetLastError();
		// Ezzel nem kell foglalkozni
		// akkor van ha az OS letörli magától a hookot
		// pl azért mert a programot bezártuk
		// igy nem hiba
		if(error != ERROR_INVALID_HOOK_HANDLE){
			std::basic_stringstream<TCHAR> ss;
			ss << TEXT("Deleting Hook failed: ");
			ss << error;
			MessageBox(0, ss.str().data(), 0 , 0);
		}
	}
			
		
}

#else

BOOL KeyboardHookInstaller::AttachDll(){
	//hKeyboardHookDll = Check(LoadLibrary(TEXT("KeyboardHook")));

	// TODO valahogy registryböl vagy máshogy okosan abszolut path-t szerezni
	hKeyboardHookDll = LoadLibrary(TEXT("KeyboardHook"));

	if(hKeyboardHookDll == 0) {
		hKeyboardHookDll = LoadLibrary(TEXT("plugins\\KeyboardHook"));
	}

	if( hKeyboardHookDll == 0) {

		int error = GetLastError();
		CString errorMessage = CString("Error loading KeyboardHook.dll: ") + ToString(error);
		MessageBox(0, errorMessage, 0 , 0);
		return FALSE;
	}
	//http://stackoverflow.com/questions/7056461/unable-to-load-some-functions-from-dll-with-getprocaddress
	// Duo to name mangling and __stdcall
	// We have to call a Foo function like _Foo@12
	//HOOKPROC pKeyboardProc = (HOOKPROC)GetProcAddress(hKeyboardHookDll, "_KeyboardProc@12");
	HOOKPROC pKeyboardProc = (HOOKPROC)GetProcAddress(hKeyboardHookDll, "KeyboardProc");

	if( pKeyboardProc == NULL) {
		int error = GetLastError();
		std::basic_stringstream<TCHAR> ss;
		ss << TEXT("GetProcAddress failed: ");
		ss << error;
		MessageBox(0, ss.str().data(), 0 , 0);

		return FALSE;
	}

	// Low-level vagy normal
#ifdef USE_LOW_LEVEL_KEYBOARD_HOOK
	hook=SetWindowsHookEx(WH_KEYBOARD_LL,pKeyboardProc,hKeyboardHookDll,0);
#else	
	hook=SetWindowsHookEx(WH_KEYBOARD,pKeyboardProc,hKeyboardHookDll,0);
#endif


	if( hook == NULL ) {
		int error = GetLastError();
		std::basic_stringstream<TCHAR> ss;
		ss << TEXT("InstallHook failed: ");
		ss << error;
		MessageBox(0, ss.str().data(), 0 , 0);
		return FALSE;
	}

	return TRUE;
}



BOOL KeyboardHookInstaller::DetachDll(){

	if(UnhookWindowsHookEx(hook) == TRUE){
		hook = NULL;
		//MessageBox(0, TEXT("Hook Deleted"), 0 , 0);
	} else {
		int error = GetLastError();
		// Ezzel nem kell foglalkozni
		// akkor van ha az OS letörli magától a hookot
		// pl azért mert a programot bezártuk
		// igy nem hiba
		if(error != ERROR_INVALID_HOOK_HANDLE){
			std::basic_stringstream<TCHAR> ss;
			ss << TEXT("Deleting Hook failed: ");
			ss << error;
			MessageBox(0, ss.str().data(), 0 , 0);
			return FALSE;
		}
	}

	//	
	if(hKeyboardHookDll != 0){
		BOOL result = FreeLibrary(hKeyboardHookDll);
		
		if( !result ) {
			int error = GetLastError();
			std::basic_stringstream<TCHAR> ss;
			ss << TEXT("Error unloading dll: ");
			ss << result << TEXT(" / ");
			ss << error;
			MessageBox(0, ss.str().data(), 0 , 0);

			return FALSE;
		}
	}
	return TRUE;
}


#endif