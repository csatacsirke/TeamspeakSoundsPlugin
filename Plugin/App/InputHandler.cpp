#include "stdafx.h"
#include "InputHandler.h"

#include <regex>


namespace TSPlugin {

	static inline void _Log(const CString& inputBuffer) {
		std::wcout << std::endl << ">>" << (const wchar_t*)inputBuffer << "<<" << std::endl;

		if (wcout.fail()) {
			wcout.clear();
		}
	}


	HookResult InputHandler::TryConsumeEvent(const KeyboardHook::KeyData& keyData) {

		const CString& unicodeLiteral = keyData.unicodeLiteral;


		// dupla '//' -re kirakunk egyet, és megszakitunk mindent
		if (lastCharacter == commandStarterCharacter && unicodeLiteral == commandStarterCharacter) {
			lastCharacter = L"";
			commandInProgress = false;
			ClearInput();
			return HookResult::PassEvent;
		}

		lastCharacter = unicodeLiteral;
 

		if (commandInProgress) {
			if (keyData.hookData.vkCode == VK_ESCAPE) {

				commandInProgress = false;
				ClearInput();
				
				return HookResult::ConsumeEvent;
			}

		}



		if (unicodeLiteral == commandStarterCharacter) {
			commandInProgress = true;
			ClearInput();

			return HookResult::ConsumeEvent;
		}
		

		if (commandInProgress) {
			if (TryConsumeArrowKeyEvent(keyData) == HookResult::ConsumeEvent) {
				return HookResult::ConsumeEvent;
			}
			

			if (keyData.hookData.vkCode == VK_RETURN) {

				OnCommandFinished();
				ClearInput();
				commandInProgress = false;

				return HookResult::ConsumeEvent;
			} 


			OnInputEventConsumed(keyData);

			return HookResult::ConsumeEvent;

		} else {

			TryBoundKeyCommand(keyData);
			
		}


		return HookResult::PassEvent;

	}



	


	HookResult InputHandler::TryConsumeArrowKeyEvent(const KeyboardHook::KeyData& keyData) {



		if (keyData.hookData.vkCode == VK_UP) {
			runLoop << [&] {
				commandLineInterface.ShiftSelection(-1);
				delegate.OnInterfaceInvalidated();
			};
			return HookResult::ConsumeEvent;
		}

		if (keyData.hookData.vkCode == VK_DOWN) {
			runLoop << [&] {
				commandLineInterface.ShiftSelection(1);
				delegate.OnInterfaceInvalidated();
			};
			return HookResult::ConsumeEvent;
		}

		if (keyData.hookData.vkCode == VK_NEXT) {
			runLoop << [&] {
				commandLineInterface.PageDown();
				delegate.OnInterfaceInvalidated();
			};
			return HookResult::ConsumeEvent;
		}

		if (keyData.hookData.vkCode == VK_PRIOR) {
			runLoop << [&] {
				commandLineInterface.PageUp();
				delegate.OnInterfaceInvalidated();
			};
			return HookResult::ConsumeEvent;
		}

		return HookResult::PassEvent;
	}


	void InputHandler::OnCommandFinished() {

		runLoop << [this] {
			CString threadsafeInputBuffer = commandLineInterface.CopyBuffer();

			if (TrySetBinding(threadsafeInputBuffer)) {
				// semmi
			} else {
				delegate.OnInputCommandFinished();
			}
			
			commandLineInterface.Clear();
			delegate.OnInterfaceInvalidated();
		};
	}


	bool InputHandler::TrySetBinding(const CString& threadsafeInputBuffer) {

		const CString regex_format = FormatString(L"%s (\\w) (.*)", bindCommand);

		try {
			const wregex binding_regex(regex_format, regex_constants::icase);

			wcmatch matches;
			if (!regex_match((const wchar_t*)threadsafeInputBuffer, matches, binding_regex)) {
				return false;
			}


			const CString hotkey = matches[1].str().c_str();
			const CString command = matches[2].str().c_str();

			nextHotkeyBinding = make_shared<InputHandlerBinding>(InputHandlerBinding{ hotkey, command });
			
			return true;

		} catch (const regex_error&) {
			//return false;
		}

		

		return false;
	}

	void InputHandler::TryBoundKeyCommand(const KeyboardHook::KeyData& keyData) {
		const shared_ptr<const InputHandlerBinding> nextHotkeyBinding_guard = nextHotkeyBinding;
		if (!nextHotkeyBinding_guard) {
			return;
		}

		if (keyData.unicodeLiteral == nextHotkeyBinding_guard->key) {
			const CString command = nextHotkeyBinding_guard->command;

			if (clearBindingAfterUse) {
				nextHotkeyBinding = nullptr;
			}

			runLoop << [command, this] {
				
				delegate.OnHotkeyCommand(command);
				
			};
		}

	}

	void InputHandler::OnInputEventConsumed(const KeyboardHook::KeyData& keyData) {
		
		runLoop << [keyData, this] {

			commandLineInterface.AddInput(keyData);
			delegate.OnInterfaceInvalidated();
		};
	}

	void InputHandler::ClearInput() {
		runLoop << [this] {
			commandLineInterface.Clear();
			delegate.OnInterfaceInvalidated();
		};
		
	}


	HookResult InputHandler::TryEnqueueFileFromCommand(CString str) {
		

		if (StartsWith(str, queuePrefix)) {
			CString command = str.Right(str.GetLength() - queuePrefix.GetLength());
			
			if (auto fileName = TryGetLikelyFileName(command)) {
				delegate.OnFileEnqueued(*fileName);
			
			}

			return ConsumeEvent;
		}
		return PassEvent;
	}


	CString InputHandler::CreateTextInterface() {
		if (commandInProgress) {
			return commandLineInterface.CreateTextInterface();
		} 

		return {};
	}

	optional<fs::path> InputHandler::TryGetSelectedFile() {
		return commandLineInterface.TryGetSelectedFile();
	}
}

