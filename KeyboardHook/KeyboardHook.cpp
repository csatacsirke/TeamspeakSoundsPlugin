// KeyboardHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KeyboardHook.h"
#include <stdio.h>
#include <sstream>
#include <vector>
#include <iostream>


namespace KeyboardHook{
	HMODULE hKeyboardHookDllModule;
	HMODULE KEYBOARD_HOOK_API GetDllHandle(){
		return hKeyboardHookDllModule;
	}
	void KEYBOARD_HOOK_API SetDllHandle(HMODULE hDll){
		hKeyboardHookDllModule = hDll;
	}
}

BOOL ProcessKeystroke(KBDLLHOOKSTRUCT* pKeyBoardHookStruct){
	
	
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
 
    if (pipe == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

	// teszt
	KeyboardHook::KeyData data = KeyboardHook::KeyData::CreateFromHookData(*pKeyBoardHookStruct);
	

    DWORD nBytesWritten = 0;
	BOOL result = WriteFile(
		pipe, // handle to our outbound pipe
		&data,//vBuffer.data(), // data to send
		sizeof(data), //vBuffer.size(), // length of data to send (bytes)
		&nBytesWritten, // will store actual amount of data sent
		NULL // not using overlapped IO
	);

  //  BOOL result = WriteFile(
  //      pipe, // handle to our outbound pipe
		//pKeyBoardHookStruct,//vBuffer.data(), // data to send
		//sizeof(KBDLLHOOKSTRUCT), //vBuffer.size(), // length of data to send (bytes)
  //      &nBytesWritten, // will store actual amount of data sent
  //      NULL // not using overlapped IO
  //  );
 
    if (!result) {
        return FALSE;
    }
 
    // Close our pipe handle
    CloseHandle(pipe);
	return TRUE;
}


extern "C" LRESULT KEYBOARD_HOOK_API 
	CALLBACK KeyboardProc(int nCode,WPARAM wParam, LPARAM lParam)
{
	
	bool isPressed = (wParam == WM_KEYDOWN);


	LPKBDLLHOOKSTRUCT pLLKeyboardHookEvent = (LPKBDLLHOOKSTRUCT)lParam;
	
	
	if(isPressed){
		auto result = ProcessKeystroke(pLLKeyboardHookEvent);
		(void)result;
	}

	            
	// 1st param is ignored
	LRESULT RetVal = CallNextHookEx( 0, nCode, wParam, lParam );
	return  RetVal;
}

