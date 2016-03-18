#pragma once

#include <concurrent_queue.h>
#include <KeyboardHook\CommonParams.h>

// TODO move
static BYTE keyboardState[256];


class PipeHandler
{
	
public:
	struct KeyData {
		KeyData() {}
		KeyData(KBDLLHOOKSTRUCT hookData) {
			this->hookData = hookData;
			
			GetKeyboardState(keyboardState);
			wchar_t buffer[10];
			ZeroMemory(buffer, sizeof(buffer));


			int result = ToUnicodeEx(hookData.vkCode, hookData.scanCode, keyboardState, buffer, 10, 0, /*keyboardLayout*/NULL );
			
			if (result > 0) {
				unicodeLiteral = buffer;
			}
		}
		KBDLLHOOKSTRUCT hookData;
		CString unicodeLiteral = _T("");
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

