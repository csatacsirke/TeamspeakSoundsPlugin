#pragma once


//#include <HookInstaller\Hook\KeyboardHookInstaller.h>

// TODO kihozni a libböl
//#include <HookInstaller\Hook\PipeHandler.h>

#include "KeyboardHook\LocalKeyboardHookInstaller.h"
//#include "KeyboardHook\PipeHandler.h"

#include <Util\Config.h>
#include <Util\HotkeyHandler.h>
#include <Util\MenuHandler.h>
#include <Util\QuickSoundHandler.h>
#include <Util\InputHandler.h>
#include <Wave\OnlineMicrophone.h>
#include <Wave\AudioProcessor.h>
#include <Wave\AudioPlayer.h>
#include <Wave\AudioBuffer.h>
#include <Gui\InputObserverDialog.h>

#include <optional>


//#include <Web/SoundBroadcaster.h>

namespace TSPlugin {

	class SoundplayerApp : public LocalKeyboardHookInstallerDelegate, InputHandlerDelegate, QuickSoundHandlerDelegate {

	public:

		// We need an explicit init function cause
		// we can't create threads from dllmain
		// and thus from constructor... :(
		void Init();
		void InitKeyboardHook();
		//HookResult OnKeyData(const KeyboardHook::KeyData& keyData);

		void InitMenus(struct PluginMenuItem*** menuItems, char** menuIcon);
		void OnMenuItemEvent(PluginMenuType type, int menuItemID, uint64 selectedItemID);

		void InitHotkeys(struct PluginHotkey*** hotkeys);
		void OnHotkey(CStringA keyword);


		// for sending sounds and playing for ourselves
		void OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited);
		void OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);


		// for Steganography
		void OnEditPlaybackVoiceDataEvent(anyID clientID, short* samples, int sampleCount, int channels);
		//bool steganographyEnabled = true;

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
		enum class StopResult { DidStop, WasNotPlaying };
		StopResult StopPlayback();

		//void AsyncOpenAndPlayFile_advanced();
		//void PlayFile_advanced(CString fileName);

		void AsyncEnqueueFile();
		void PlayQueued();
		void Replay();
		void PlayRandom();
		//void PlayPreset(int ordinal);


		//void ProcessCommand(const CString& inputString);
		void UpdateObserverDialog();
		//void UpdatePossibleFiles();

		//bool GetLikelyFileName(_Out_ CString& result, CString str);

		void PlayAlarmSound();
		void OpenObserverDialog();


		volatile PluginItemType GetPluginInfoData_lastType = PluginItemType::PLUGIN_SERVER;
		volatile uint64 GetPluginInfoData_lastId = 0;
		void StoreGetPluginInfoData(uint64 id, PluginItemType type);
		CStringA GetPluginInfoData();
		void RefreshTsInterface();
		// for workaround
		void OnServerUpdatedEvent();
	private:
		//std::vector<CString> files;
		//std::vector<CString> GetPossibleFiles(const CString& inputString);

	protected:
		HookResult LocalKeyboardHookInstallerDelegate::OnKeyboardHookEvent(const KeyboardHook::KeyData& keyData) override;
		void LocalKeyboardHookInstallerDelegate::OnMessage(const CString& message) override;

		//void InputHandlerDelegate::OnCommand(const CString& command);

		//void InputHandlerDelegate::OnCommandFinished(const CString& command) override;
		//void InputHandlerDelegate::OnInputBufferChanged(const CString& inputBuffer) override;
		void InputHandlerDelegate::OnPossibleFilesChanged(const FileList& fileList) override;
		void InputHandlerDelegate::OnInputCommandFinished() override;
		

		void QuickSoundHandlerDelegate::OnQuickSoundMatch(const CString& path);

	private:
		//bool TryEnqueueFileFromCommand(CString str);
		
		optional<CString> SoundplayerApp::TryGetSelectedFile();

		//bool TryPlayQuickSound(CString str);
		//CString GetLikelyFileName(CString str);

		void SendFileNameToChat(CString fileName);
		void SendMessageToChannelChat(CString message);
		void OpenDeveloperConsole();



	public:


	private:
		//HotkeyHandler hotkeyHandler;
		MenuHandler menuHandler;

		std::mutex playerLock;
		//volatile bool stop;

		concurrency::concurrent_queue<CString> playlist;

		CString lastFile;


		//optional<int> selectedFileIndex;
		//optional<CString> selectedFile;


		//KeyboardHookInstaller hookInstaller;


		//OnlineMicrophone onlineMicrophone;

		//AudioPlayer audioPlayer;
		AudioProcessor audioProcessor;



		// TODO : EZ KIBASZOTT ANTIHATÉKONY, CSAK ELÖSZÖR MUKODJON EGYALTALAN
		AudioBuffer audioBufferForCapture;
		AudioBuffer audioBufferForPlayback;

		//unique_ptr<InputObserverDialog> inputObserverDialog;

		LocalKeyboardHookInstaller localHookInstaller = LocalKeyboardHookInstaller(*this);
		bool shouldDisableHookWhenScrollLockIsEnabled = true;

		QuickSoundHandler quickSoundHandler = QuickSoundHandler(*this);
		InputHandler inputHandler = InputHandler(*this);

		// különböző thread-ek buzerálhatják, le kell előtte másolni a ptr-t
		shared_ptr<FileList> unsafeFileList;
		//RunLoop runLoop = RunLoop(RunLoop::DeferredInit);


		//QuickVoiceChatHandler quickVoiceChatHandler;


		//TsVoiceHandler tsVoiceHandler;

		//// Teamspeak sound related config
		//const char* myDeviceId = "BattlechickensId"; 
		////The client lib works at 48Khz internally.
		////It is therefore advisable to use the same for your project 
		//const int PLAYBACK_FREQUENCY = 48000;
		//const int PLAYBACK_CHANNELS = 2;
	};

}

