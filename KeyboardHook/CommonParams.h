#pragma once

#include <tchar.h>
namespace KeyboardHook {
	static const TCHAR* defaultPipeName = TEXT("\\\\.\\pipe\\soundplayer_pipe");


	struct KeyData {
		KBDLLHOOKSTRUCT hookData;
		wchar_t unicodeLiteral[10];
		//CString unicodeLiteral = _T("");

		static KeyData CreateFromHookData(KBDLLHOOKSTRUCT hookData) {
			KeyData keyData;
			keyData.hookData = hookData;
			BYTE keyboardState[256];
			ZeroMemory(keyboardState, sizeof(keyboardState));
			//if(hookData.vkCode == VK_SHIFT || hookData.vkCode == VK_LSHIFT) return;
			BOOL __result = GetKeyboardState(keyboardState);
			if(!__result) {
				//std::wcout << L"LÓÓÓÓÓÓÓFASZ" << std::endl;
			}

			//SetKeyboardState(keyboardState);
			
			ZeroMemory(keyData.unicodeLiteral, sizeof(keyData.unicodeLiteral));
			auto keyboardLayout = GetKeyboardLayout(NULL);
			//keyboardLayout = NULL;

			//int result = ToUnicodeEx(hookData.vkCode, hookData.scanCode, keyboardState, buffer, 10, /*hookData.flags*/NULL, /*keyboardLayout */NULL);
			int result = ToUnicodeEx(hookData.vkCode, hookData.scanCode, keyboardState, keyData.unicodeLiteral, 10, hookData.flags, keyboardLayout);

			if(result > 0) {
				//std::wcout << L"{" << (const wchar_t*)keyData.unicodeLiteral << L"}";
				
			} else {
				//std::wcout << L"{0}";
			}
			return keyData;
		}
	};

}
