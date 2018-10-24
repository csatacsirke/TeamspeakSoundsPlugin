#include "stdafx.h"


#include "LocalKeyboardHookInstaller.h"

#include <sstream>

using namespace KeyboardHook;


LocalKeyboardHookInstallerDelegate* g_KeyboardHookDelegate = nullptr;
static BOOL ProcessKeystroke(KBDLLHOOKSTRUCT* pKeyBoardHookStruct) {

	KeyboardHook::KeyData data = KeyboardHook::KeyData::CreateFromHookData(*pKeyBoardHookStruct);

	ASSERT(g_KeyboardHookDelegate);
	if (g_KeyboardHookDelegate) {
		return g_KeyboardHookDelegate->OnKeyboardHookEvent(data);
	}

	return FALSE;
}




extern "C" LRESULT _declspec(dllexport)
CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {


	if (nCode == HC_ACTION) {

		bool isPressed = (wParam == WM_KEYDOWN);

		LPKBDLLHOOKSTRUCT pLLKeyboardHookEvent = (LPKBDLLHOOKSTRUCT)lParam;


		if (isPressed) {
			BOOL result = ProcessKeystroke(pLLKeyboardHookEvent);
			if (result) {
				// feldolgoztuk az üzenetet, és nem akarjuk tovább adni
				return TRUE;
			}
		}

		//// 1st param is ignored, as it is always zero
		//LRESULT RetVal = CallNextHookEx(0, nCode, wParam, lParam);
		//return RetVal;
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}


LocalKeyboardHookInstaller::LocalKeyboardHookInstaller(LocalKeyboardHookInstallerDelegate& delegate) : delegate(delegate) {
}

LocalKeyboardHookInstaller::~LocalKeyboardHookInstaller() {
	Detach();
};


bool LocalKeyboardHookInstaller::Attach() {

	g_KeyboardHookDelegate = &this->delegate;
	hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);

	// gratulálok ehhez a hibakezeléshez... majd egyszer pls...
	// update: ^^
	if (hook == NULL) {
		int error = GetLastError();
		std::basic_stringstream<TCHAR> ss;
		ss << TEXT("InstallHook failed: ");
		ss << error;
		delegate.OnMessage(ss.str().data());
		//MessageBox(0, ss.str().data(), 0, 0);
	}


	return hook != NULL;
}


void LocalKeyboardHookInstaller::Detach() {

	if (UnhookWindowsHookEx(hook) == TRUE) {
		hook = NULL;
		g_KeyboardHookDelegate = nullptr;
		//MessageBox(0, TEXT("Hook Deleted"), 0 , 0);
	} else {
		int error = GetLastError();
		// Ezzel nem kell foglalkozni
		// akkor van ha az OS letörli magától a hookot
		// pl azért mert a programot bezártuk
		// igy nem hiba
		if (error != ERROR_INVALID_HOOK_HANDLE) {
			std::basic_stringstream<TCHAR> ss;
			ss << TEXT("Deleting Hook failed: ");
			ss << error;
			//MessageBox(0, ss.str().data(), 0, 0);
			delegate.OnMessage(ss.str().data());
		}
	}
}

