#include "stdafx.h"


#include "LocalKeyboardHookInstaller.h"

#include <sstream>


static BOOL ProcessKeystroke(KBDLLHOOKSTRUCT* pKeyBoardHookStruct) {

	// Open the named pipe
	// Most of these parameters aren't very relevant for pipes.
	HANDLE pipe = CreateFile(
		KeyboardHook::defaultPipeName,
		GENERIC_WRITE, // only need read access
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(pipe == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	
	KeyboardHook::KeyData data = KeyboardHook::KeyData::CreateFromHookData(*pKeyBoardHookStruct);


	DWORD nBytesWritten = 0;
	BOOL result = WriteFile(
		pipe, // handle to our outbound pipe
		&data,//vBuffer.data(), // data to send
		sizeof(data), //vBuffer.size(), // length of data to send (bytes)
		&nBytesWritten, // will store actual amount of data sent
		NULL // not using overlapped IO
	);

	if(!result) {
		return FALSE;
	}

	// Close our pipe handle
	CloseHandle(pipe);
	return TRUE;
}



extern "C" LRESULT _declspec(dllexport)
CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

	bool isPressed = (wParam == WM_KEYDOWN);


	LPKBDLLHOOKSTRUCT pLLKeyboardHookEvent = (LPKBDLLHOOKSTRUCT)lParam;


	if(isPressed) {
		auto result = ProcessKeystroke(pLLKeyboardHookEvent);
		(void)result;
	}


	// 1st param is ignored
	LRESULT RetVal = CallNextHookEx(0, nCode, wParam, lParam);
	return  RetVal;
}

LocalKeyboardHookInstaller::LocalKeyboardHookInstaller() {
	
};

LocalKeyboardHookInstaller::~LocalKeyboardHookInstaller() {
	Detach();
};


bool LocalKeyboardHookInstaller::Attach() {
	hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);

	// gratulálok ehhez a hibakezeléshez... majd egyszer pls...
	if(hook == NULL) {
		int error = GetLastError();
		std::basic_stringstream<TCHAR> ss;
		ss << TEXT("InstallHook failed: ");
		ss << error;
		MessageBox(0, ss.str().data(), 0, 0);
	}


	return hook != NULL;
}


void LocalKeyboardHookInstaller::Detach() {

	if(UnhookWindowsHookEx(hook) == TRUE) {
		hook = NULL;
		//MessageBox(0, TEXT("Hook Deleted"), 0 , 0);
	} else {
		int error = GetLastError();
		// Ezzel nem kell foglalkozni
		// akkor van ha az OS letörli magától a hookot
		// pl azért mert a programot bezártuk
		// igy nem hiba
		if(error != ERROR_INVALID_HOOK_HANDLE) {
			std::basic_stringstream<TCHAR> ss;
			ss << TEXT("Deleting Hook failed: ");
			ss << error;
			MessageBox(0, ss.str().data(), 0, 0);
		}
	}
}






