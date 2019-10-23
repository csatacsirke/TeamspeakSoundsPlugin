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

		

		if (commandInProgress) {
			
			if (keyData.hookData.vkCode == VK_ESCAPE) {

				commandInProgress = false;
				inputBuffer = "";

				OnInputEventConsumed();
				return HookResult::ConsumeEvent;
			}

			if (keyData.hookData.vkCode == VK_BACK) {
				if (inputBuffer.GetLength() > 0) {
					inputBuffer.Truncate(inputBuffer.GetLength() - 1);
				}

				OnInputEventConsumed();
				return HookResult::ConsumeEvent;
			}
		}


		CString unicodeLiteral = keyData.unicodeLiteral;
		if (unicodeLiteral == commandStarterCharacter) {
			commandInProgress = true;
			inputBuffer = "";
			OnInputEventConsumed();

			return HookResult::ConsumeEvent;
		}


		if (commandInProgress) {
			if (TryConsumeArrowKeyEvent(keyData) == HookResult::ConsumeEvent) {
				OnInputEventConsumed();
				return HookResult::ConsumeEvent;
			}
			

			if (keyData.hookData.vkCode == VK_RETURN) {

				_Log(inputBuffer);

				//delegate.OnCommand(inputBuffer);
				OnCommandFinished();

				inputBuffer = "";
				commandInProgress = false;

				OnInputEventConsumed();
				return HookResult::ConsumeEvent;
			} 


			inputBuffer += unicodeLiteral;
			OnInputEventConsumed();

			return HookResult::ConsumeEvent;
			//Log::Debug(unicodeLiteral);

		} else {

			TryBoundKeyCommand(keyData);
			
		}


		//if (TryPlayQuickSound(inputBuffer)) {
		//	return;
		//}

		return HookResult::PassEvent;

		//SetScrollLockState();
	}


	


	HookResult InputHandler::TryConsumeArrowKeyEvent(const KeyboardHook::KeyData& keyData) {

		if (possibleFilesForCurrentInput.size() == 0) {
			return HookResult::PassEvent;
		}

		if (keyData.hookData.vkCode == VK_UP) {
			RotateSelection(-1);
			return HookResult::ConsumeEvent;
		}

		if (keyData.hookData.vkCode == VK_DOWN) {
			RotateSelection(1);
			return HookResult::ConsumeEvent;
		}

		return HookResult::PassEvent;
	}

	void InputHandler::RotateSelection(int indexDelta) {
		if (possibleFilesForCurrentInput.size() == 0) {
			selectedFileIndex = 0;
			return;
		}


		selectedFileIndex = (selectedFileIndex + indexDelta) % possibleFilesForCurrentInput.size();

	}

	void InputHandler::OnCommandFinished() {
		CString threadsafeInputBuffer = inputBuffer;
		runLoop.Add([threadsafeInputBuffer, this] {

			if (TrySetBinding(threadsafeInputBuffer)) {
				// semmi
			} else {
				delegate.OnInputCommandFinished();
			}
			// UpdatePossibleFiles(threadsafeInputBuffer);
			// delegate.OnPossibleFilesChanged({ possibleFiles, selectedFileIndex });
			
			selectedFileIndex = 0;
		});
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

			nextHotkeyBinding = make_shared<InputHandlerBinding>(InputHandlerBinding{ hotkey, command});
			
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
				//std::vector<CString> possibleFiles = GetPossibleFiles(command);
				//if (possibleFiles.size() > 0) {
				//	delegate.Play
				//}
				

				delegate.OnHotkeyCommand(command);
				//UpdatePossibleFiles(threadsafeInputBuffer, threadsafeCommandInProgress);
				//delegate.OnPossibleFilesChanged({ possibleFiles, selectedFileIndex });
			};
		}

	}

	void InputHandler::OnInputEventConsumed() {
		CString threadsafeInputBuffer = inputBuffer;
		bool threadsafeCommandInProgress = commandInProgress;
		runLoop.Add([threadsafeInputBuffer, threadsafeCommandInProgress, this] {
			UpdatePossibleFiles(threadsafeInputBuffer, threadsafeCommandInProgress);
			delegate.OnPossibleFilesChanged({ possibleFilesForCurrentInput, selectedFileIndex });
		});
	}

	void InputHandler::UpdatePossibleFiles(const CString& threadsafeInputBuffer, bool threadsafeCommandInProgress) {
		if (!threadsafeCommandInProgress) {
			possibleFilesForCurrentInput.resize(0);
			selectedFileIndex = 0;
		} else {
			possibleFilesForCurrentInput = GetPossibleFiles(threadsafeInputBuffer);
			if (selectedFileIndex < possibleFilesForCurrentInput.size()) {
				if (possibleFilesForCurrentInput.size() == 0) {
					selectedFileIndex = 0;
				} else {
					selectedFileIndex = std::min<size_t>(possibleFilesForCurrentInput.size() - 1u, selectedFileIndex);
				}
			}
		}

		
	}

	HookResult InputHandler::TryEnqueueFileFromCommand(CString str) {
		

		if (StartsWith(str, queuePrefix)) {
			CString command = str.Right(str.GetLength() - queuePrefix.GetLength());
			//CString fileName;
			if (auto fileName = TryGetLikelyFileName(command)) {
				delegate.OnFileEnqueued(*fileName);
				//playlist.push(*fileName);
			}

			return ConsumeEvent;
		}
		return PassEvent;
	}

}

