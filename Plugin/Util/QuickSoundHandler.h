#pragma once


#include <Util/Util.h>


class QuickSoundHandlerDelegate {
public:
	virtual void OnQuickSoundMatch(const CString& path) = 0;
};

class QuickSoundHandler {
	QuickSoundHandlerDelegate& delegate;
public:

	QuickSoundHandler(QuickSoundHandlerDelegate& delegate) : delegate(delegate) {}
	HookResult TryConsumeEvent(const KeyboardHook::KeyData& keyData);


};



