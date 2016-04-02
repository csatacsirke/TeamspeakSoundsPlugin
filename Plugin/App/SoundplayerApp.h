#pragma once

#include "ts3_functions.h"

#include <HookInstaller\Hook\KeyboardHookInstaller.h>
#include <HookInstaller\Hook\PipeHandler.h>

#include "Config.h"

#include <queue>

class SoundplayerApp  {
	//TS3Functions& ts3Functions;
	//uint64 connection = 0;

	Config config;

	std::mutex playerLock;
	bool stop;

	KeyboardHookInstaller hookInstaller;
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
	

	SoundplayerApp(/*TS3Functions& ts3Functions*/);
	~SoundplayerApp();


	// We need an explicit init function cause
	// we can't create threads from dllmain
	// and thus from constructor... :(
	void Init();
	void InitKeyboardHook();

	void InitHotkeys(struct PluginHotkey*** hotkeys);
	void OnHotkey(CStringA keyword);

	void OpenSettingsDialog();

	//void SetConnectionHandle(uint64 connection);
	//uint64 GetConnectionHandle();
	
	void OnKeyData(const PipeHandler::KeyData& keyData);
	

	void AsyncOpenAndPlayFile();
	void AsyncPlayFile(CString fileName);
	void PlayFile(CString fileName);
	void StopPlayback();

	void AsyncOpenAndPlayFile_advanced();
	void PlayFile_advanced(CString fileName);

	void AsyncEnqueueFile();
	void PlayQueued();
	void Replay();
};

