#pragma once


//#include <HookInstaller\Hook\KeyboardHookInstaller.h>

// TODO kihozni a libböl
//#include <HookInstaller\Hook\PipeHandler.h>

#include "KeyboardHook\LocalKeyboardHookInstaller.h"
#include "KeyboardHook\PipeHandler.h"

#include "Config.h"
#include "Util\HotkeyHandler.h"

class SoundplayerApp  {

	HotkeyHandler hotkeyHandler;

	std::mutex playerLock;
	bool stop;

	//KeyboardHookInstaller hookInstaller;
	LocalKeyboardHookInstaller localHookInstaller;

	PipeHandler pipeHandler;
	
	bool commandInProgress = false;
	CString inputBuffer;

	concurrency::concurrent_queue<CString> playlist;
	CString lastFile;
private:
	// Teamspeak sound related config
	const char* myDeviceId = "BattlechickensId"; 
	//The client lib works at 48Khz internally.
	//It is therefore advisable to use the same for your project 
	const int PLAYBACK_FREQUENCY = 48000;
	const int PLAYBACK_CHANNELS = 2;
public:

	// We need an explicit init function cause
	// we can't create threads from dllmain
	// and thus from constructor... :(
	void Init();
	void InitKeyboardHook();
	void OnKeyData(const KeyboardHook::KeyData& keyData);


	void InitHotkeys(struct PluginHotkey*** hotkeys);
	void OnHotkey(CStringA keyword);

	void OpenSettingsDialog(void* handle, void* qParentWidget);
	void OpenSoundsFolderSelectorDialog();

	
	void AsyncOpenAndPlayFile();
	void AsyncPlayFile(CString fileName);
	void PlayFile(CString fileName);
	void StopPlayback();

	void AsyncOpenAndPlayFile_advanced();
	void PlayFile_advanced(CString fileName);

	void AsyncEnqueueFile();
	void PlayQueued();
	void Replay();
	void PlayRandom();
	void PlayPreset(int ordinal);
	void ProcessRegexCommand(CString str);
private:
	CString GetLikelyFileName(CString str);
};

