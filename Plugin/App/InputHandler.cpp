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

		}


		//if (TryPlayQuickSound(inputBuffer)) {
		//	return;
		//}

		return HookResult::PassEvent;

		//SetScrollLockState();
	}


	


	HookResult InputHandler::TryConsumeArrowKeyEvent(const KeyboardHook::KeyData& keyData) {

		if (possibleFiles.size() == 0) {
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
		if (possibleFiles.size() == 0) {
			selectedFileIndex = 0;
			return;
		}


		selectedFileIndex = (selectedFileIndex + indexDelta) % possibleFiles.size();

	}

	void InputHandler::OnCommandFinished() {
		CString threadsafeInputBuffer = inputBuffer;
		runLoop.Add([threadsafeInputBuffer, this] {
			// UpdatePossibleFiles(threadsafeInputBuffer);
			// delegate.OnPossibleFilesChanged({ possibleFiles, selectedFileIndex });
			delegate.OnInputCommandFinished();
			selectedFileIndex = 0;
		});
	}

	void InputHandler::OnInputEventConsumed() {
		CString threadsafeInputBuffer = inputBuffer;
		bool threadsafeCommandInProgress = commandInProgress;
		runLoop.Add([threadsafeInputBuffer, threadsafeCommandInProgress, this] {
			UpdatePossibleFiles(threadsafeInputBuffer, threadsafeCommandInProgress);
			delegate.OnPossibleFilesChanged({ possibleFiles, selectedFileIndex });
		});
	}

	void InputHandler::UpdatePossibleFiles(const CString& threadsafeInputBuffer, bool threadsafeCommandInProgress) {
		if (!threadsafeCommandInProgress) {
			possibleFiles.resize(0);
			selectedFileIndex = 0;
		} else {
			possibleFiles = GetPossibleFiles(threadsafeInputBuffer);
			if (selectedFileIndex < possibleFiles.size()) {
				if (possibleFiles.size() == 0) {
					selectedFileIndex = 0;
				} else {
					selectedFileIndex = std::min<size_t>(possibleFiles.size() - 1u, selectedFileIndex);
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

