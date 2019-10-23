#pragma once

#include <Util/Util.h>
#include <Util/Config.h>
#include <Util/RunLoop.h>



namespace TSPlugin {

	struct FileList {
		const vector<CString>& possibleFiles;
		const size_t& selectedFileIndex;
	};

	class InputHandlerDelegate {

	public:
		//virtual void OnCommandFinished(const CString& command) = 0;
		//virtual void OnInputBufferChanged(const CString& inputBuffer) = 0;
		virtual void OnInputCommandFinished() = 0;
		virtual void OnPossibleFilesChanged(const FileList& fileList) = 0;
		virtual void OnHotkeyCommand(const CString& command) = 0;
		// currently unimplemented
		virtual void OnFileEnqueued(const CString& fileName) {};
	};

	struct InputHandlerBinding {
		CString key;
		CString command;
	};

	class InputHandler {
		
	public:
		InputHandler(InputHandlerDelegate& delegate) : delegate(delegate) {};
		HookResult TryConsumeEvent(const KeyboardHook::KeyData& keyData);

		const CString& GetBuffer() { return inputBuffer; }

	private:
		//std::optional<CString> TryGetSelectedFile(const CString& inputString);

		HookResult TryConsumeArrowKeyEvent(const KeyboardHook::KeyData& keyData);
		void RotateSelection(int indexDelta);
		HookResult TryEnqueueFileFromCommand(CString str);

		void UpdatePossibleFiles(const CString& threadsafeInputBuffer, bool threadsafeCommandInProgress);

		void OnCommandFinished();
		void OnInputEventConsumed();

		bool TrySetBinding(const CString& threadsafeInputBuffer);
		void TryBoundKeyCommand(const KeyboardHook::KeyData& keyData);
	protected:
		InputHandlerDelegate& delegate;

	private:

		bool commandInProgress = false;
		CString inputBuffer;

		vector<CString> possibleFilesForCurrentInput;
		size_t selectedFileIndex = 0;

		RunLoop runLoop = RunLoop();

		//CString queuePrefix = L"q ";
		CString queuePrefix = Global::config.Get(ConfigKeys::QueueCommand);
		CString bindCommand = Global::config.Get(ConfigKeys::BindCommand);
		CString commandStarterCharacter = Global::config.Get(ConfigKeys::CommandStarterCharacter);


		bool clearBindingAfterUse = Global::config.Get(ConfigKeys::ClearBindingAfterUse);
		shared_ptr<const InputHandlerBinding> nextHotkeyBinding;

	};


}
