#pragma once


//#include <HookInstaller\Hook\KeyboardHookInstaller.h>

// TODO kihozni a libböl
//#include <HookInstaller\Hook\PipeHandler.h>

#include "KeyboardHook\LocalKeyboardHookInstaller.h"
#include "KeyboardHook\PipeHandler.h"

#include "Config.h"
#include "Util\HotkeyHandler.h"
#include "Wave\OnlineMicrophone.h"
#include "Wave\AudioProcessor.h"
#include "Wave\AudioPlayer.h"
#include "Wave\AudioBuffer.h"

#include <Web/SoundBroadcaster.h>


class SoundplayerApp  {

public:

	// We need an explicit init function cause
	// we can't create threads from dllmain
	// and thus from constructor... :(
	void Init();
	void InitKeyboardHook();
	void OnKeyData(const KeyboardHook::KeyData& keyData);


	void InitHotkeys(struct PluginHotkey*** hotkeys);
	void OnHotkey(CStringA keyword);

	
	// for sending sounds and playing for ourselves
	void OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited);
	void OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);


	// for Steganography
	void OnEditPlaybackVoiceDataEvent(anyID clientID, short* samples, int sampleCount, int channels);
	bool steganographyEnabled = true;

	//void OnEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels);
	//void OnEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);
	//void OnEditCapturedVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited);


	void OnClientMoved(anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, CString moveMessage);

	void OpenSettingsDialog(void* handle, void* qParentWidget);
	void OpenSoundsFolderSelectorDialog();

	void AsyncOpenAudioProcessorDialog();
	
	void AsyncOpenAndPlayFile();
	void AsyncPlayFile(CString fileName);
	void PlayFile(CString fileName);
	//void PlayFile_old(CString fileName);
	void StopPlayback();

	//void AsyncOpenAndPlayFile_advanced();
	//void PlayFile_advanced(CString fileName);

	void AsyncEnqueueFile();
	void PlayQueued();
	void Replay();
	void PlayRandom();
	void PlayPreset(int ordinal);


	void ProcessCommand(CString str);
private:
	bool TryEnqueueFileFromCommand(CString str);
	//CString GetLikelyFileName(CString str);
	
	void SendFileNameToChat(CString fileName);
	void SendMessageToChannelChat(CString message);

public:
	static bool GetLikelyFileName(_Out_ CString& result, CString str);

private:
	HotkeyHandler hotkeyHandler;

	std::mutex playerLock;
	volatile bool stop;
	
	//KeyboardHookInstaller hookInstaller;
	LocalKeyboardHookInstaller localHookInstaller;

	PipeHandler pipeHandler;

	//OnlineMicrophone onlineMicrophone;

	//AudioPlayer audioPlayer;
	AudioProcessor audioProcessor;

	bool commandInProgress = false;
	CString inputBuffer;

	concurrency::concurrent_queue<CString> playlist;
	
	CString lastFile;

	// TODO : EZ KIBASZOTT ANTIHATÉKONY, CSAK ELÖSZÖR MUKODJON EGYALTALAN
	AudioBuffer audioBufferForCapture;
	AudioBuffer audioBufferForPlayback;




	
	//TsVoiceHandler tsVoiceHandler;

	//// Teamspeak sound related config
	//const char* myDeviceId = "BattlechickensId"; 
	////The client lib works at 48Khz internally.
	////It is therefore advisable to use the same for your project 
	//const int PLAYBACK_FREQUENCY = 48000;
	//const int PLAYBACK_CHANNELS = 2;
};

