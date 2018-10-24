#pragma once

#include "KeyboardHookUtils.h"



class LocalKeyboardHookInstallerDelegate {
public:
	virtual BOOL OnKeyboardHookEvent(const KeyboardHook::KeyData& keyData) = NULL;
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



