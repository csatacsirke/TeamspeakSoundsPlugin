#include "stdafx.h"
#include "InputHandler.h"


namespace TSPlugin {

	static inline void _Log(const CString& inputBuffer) {
		std::wcout << std::endl << ">>" << (const wchar_t*)inputBuffer << "<<" << std::endl;

		if (wcout.fail()) {
			wcout.clear();
		}
	}


	HookResult InputHandler::TryConsumeEvent(const KeyboardHook::KeyData& keyData) {


		if (commandInProgress) {

			if (keyData.hookData.vkCode == VK_ESCAPE) {

				commandInProgress = false;
				inputBuffer = "";

				return HookResult::ConsumeEvent;
			}

			if (keyData.hookData.vkCode == VK_BACK) {
				if (inputBuffer.GetLength() > 0) {
					inputBuffer.Truncate(inputBuffer.GetLength() - 1);
				}

				return HookResult::ConsumeEvent;
			}
		}


		CString unicodeLiteral = keyData.unicodeLiteral;
		if (unicodeLiteral == CString("/")) {
			commandInProgress = true;
			inputBuffer = "";
			return HookResult::ConsumeEvent;
		}


		if (commandInProgress) {
			if (keyData.hookData.vkCode == VK_RETURN) {

				_Log(inputBuffer);

				delegate.OnCommand(inputBuffer);

				inputBuffer = "";


				commandInProgress = false;
				return HookResult::ConsumeEvent;
			} else {
				inputBuffer += unicodeLiteral;
				return HookResult::ConsumeEvent;
				//Log::Debug(unicodeLiteral);
			}
		}


		//if (TryPlayQuickSound(inputBuffer)) {
		//	return;
		//}

		return HookResult::PassEvent;

		//SetScrollLockState();
	}


}

