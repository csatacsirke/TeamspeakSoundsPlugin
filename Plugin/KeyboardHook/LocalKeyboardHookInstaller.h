#pragma once


class LocalKeyboardHookInstaller {
	HHOOK hook;
public:
	LocalKeyboardHookInstaller();
	~LocalKeyboardHookInstaller();
	
	bool Attach();
	void Detach();
};

