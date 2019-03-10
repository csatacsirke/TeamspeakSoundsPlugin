#pragma once

#include "KeyboardHookUtils.h"

namespace TSPlugin {

	using namespace KeyboardHook;

	class LocalKeyboardHookInstallerDelegate {
	public:
		virtual HookResult OnKeyboardHookEvent(const KeyboardHook::KeyData& keyData) = NULL;
		virtual void OnMessage(const CString& message) = NULL;
	};

	class LocalKeyboardHookInstaller {
		LocalKeyboardHookInstallerDelegate& delegate;
	public:
		LocalKeyboardHookInstaller(LocalKeyboardHookInstallerDelegate& delegate);
		~LocalKeyboardHookInstaller();

		bool Attach();
		void Detach();

	private:
		HHOOK hook;
	};


}
