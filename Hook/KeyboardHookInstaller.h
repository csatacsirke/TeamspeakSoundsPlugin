#pragma once





class KeyboardHookInstaller{
	HHOOK hook;
	HMODULE hKeyboardHookDll;

public:
	
	KeyboardHookInstaller(){};
	~KeyboardHookInstaller(){ DetachDll();};

	BOOL AttachDll();
	BOOL DetachDll();
};

