#pragma once

#include <Util/Util.h>
#include <Util/Config.h>
#include <Util/RunLoop.h>

#include <App/CommandLineInterface.h>
#include <KeyboardHook/KeyboardHookUtils.h>


namespace TSPlugin {
	using namespace KeyboardHook;

	class InputHandlerDelegate {

	public:
		virtual void OnInputCommandFinished() = 0;
		virtual void OnHotkeyCommand(const CString& command) = 0;
		virtual void OnFileEnqueued(const fs::path& file) = 0;
		virtual void OnInterfaceInvalidated() = 0;
	};

	struct InputHandlerBinding {
		CString key;
		CString command;
	};


	class InputHandler {
		
	public:
		InputHandler(InputHandlerDelegate& delegate) : delegate(delegate) {};
		HookResult TryConsumeEvent(const KeyboardHook::KeyData& keyData);

		

		CStringA CreateTextInterface();
		optional<fs::path> TryGetSelectedFile();

	private:

		HookResult TryConsumeArrowKeyEvent(const KeyboardHook::KeyData& keyData);
		void ShiftSelection(int indexDelta);
		HookResult TryEnqueueFileFromCommand(CString str);

		
		void OnCommandFinished();
		void OnInputEventConsumed(const KeyboardHook::KeyData& keyData);
		void ClearInput();

		bool TrySetBinding(const CString& threadsafeInputBuffer);
		void TryBoundKeyCommand(const KeyboardHook::KeyData& keyData);
	protected:
		InputHandlerDelegate& delegate;

	private:

		bool commandInProgress = false;
		
		CommandLineInterface commandLineInterface;


		RunLoop runLoop = RunLoop();

		CString queuePrefix = Global::config.Get(ConfigKeys::QueueCommand);
		CString bindCommand = Global::config.Get(ConfigKeys::BindCommand);
		CString commandStarterCharacter = Global::config.Get(ConfigKeys::CommandStarterCharacter);
		CString lastCharacter;

		bool clearBindingAfterUse = Global::config.Get(ConfigKeys::ClearBindingAfterUse);
		shared_ptr<const InputHandlerBinding> nextHotkeyBinding;

	};


}
