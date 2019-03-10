#pragma once

#include <Util/Util.h>

namespace TSPlugin {

	class InputHandlerDelegate {

	public:
		virtual void OnCommand(const CString& command) = 0;
	};


	class InputHandler {
		bool commandInProgress = false;
		CString inputBuffer;

	protected:
		InputHandlerDelegate& delegate;
	public:

		InputHandler(InputHandlerDelegate& delegate) : delegate(delegate) {};
		HookResult TryConsumeEvent(const KeyboardHook::KeyData& keyData);

		const CString& GetBuffer() { return inputBuffer; }
	};


}
