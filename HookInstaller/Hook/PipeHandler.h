#pragma once

#include <concurrent_queue.h>
#include <KeyboardHook\CommonParams.h>

// TODO move



class PipeHandler
{
	
public:
	struct KeyData {
		KBDLLHOOKSTRUCT hookData;
		CString unicodeLiteral = _T("");

		static KeyData CreateFromHookData(KBDLLHOOKSTRUCT hookData) {
			KeyData keyData;
			keyData.hookData = hookData;
			BYTE keyboardState[256];
			//if(hookData.vkCode == VK_SHIFT || hookData.vkCode == VK_LSHIFT) return;
			GetKeyboardState(keyboardState);
			SetKeyboardState(keyboardState);
			wchar_t buffer[10];
			ZeroMemory(buffer, sizeof(buffer));
			auto keyboardLayout = GetKeyboardLayout(NULL);

			int result = ToUnicodeEx(hookData.vkCode, hookData.scanCode, keyboardState, buffer, 10, /*hookData.flags*/NULL, /*keyboardLayout */NULL);

			if(result > 0) {
				keyData.unicodeLiteral = buffer;
			}
			return keyData;
		}
	};
private:
	std::mutex mutex;

	volatile bool stop;
	concurrency::concurrent_queue<KeyData> queue;
	std::function<void(PipeHandler&)> OnNewEntry;

public:
	PipeHandler(void);
	// TODO a destruktor csinálhat vicces dolgokat... 
	// majd egyszer írd meg rendesen... 
	//update: lehet hogy megoldódott
	~PipeHandler(void);
	
	bool ListenPipe(CString pipeName = KeyboardHook::defaultPipeName);
	void Stop();

	void SetOnNewEntryListener(std::function<void(PipeHandler&)> callback);

	bool TryPop(_Out_ KeyData& keyData);
	void Push(const KBDLLHOOKSTRUCT& hookStruct);
private:
	BOOL RunPipe(CString pipeName);
};

