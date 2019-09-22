#pragma once


//#include <HookInstaller\Hook\KeyboardHookInstaller.h>

// TODO kihozni a libböl
//#include <HookInstaller\Hook\PipeHandler.h>

#include "KeyboardHook\LocalKeyboardHookInstaller.h"
//#include "KeyboardHook\PipeHandler.h"

#include <Util\Config.h>
#include <Wave\OnlineMicrophone.h>
#include <Wave\AudioProcessor.h>
#include <Wave\AudioPlayer.h>
#include <Wave\AudioBuffer.h>
#include <Gui\InputObserverDialog.h>


#include "HotkeyHandler.h"
#include "MenuHandler.h"
#include "QuickSoundHandler.h"
#include "InputHandler.h"

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

		void Shutdown();

		void CheckForUpdates();

		void InitMenus(struct PluginMenuItem*** menuItems, char** menuIcon);
		void OnMenuItemEvent(PluginMenuType type, int menuItemID, uint64 selectedItemID);

		void InitHotkeys(struct PluginHotkey*** hotkeys);
		void OnHotkey(CStringA keyword);


		// for sending sounds and playing for ourselves
		void OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited);
		void OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);

		// unused
		void OnEditPlaybackVoiceDataEvent(anyID clientID, short* samples, int sampleCount, int channels);
		

		void OnClientMoved(anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, CString moveMessage);

		void OpenSettingsDialog(void* handle, void* qParentWidget);
		void OpenSoundsFolderSelectorDialog();

		void AsyncOpenAudioProcessorDialog();

		void AsyncOpenAndPlayFile();
		void AsyncPlayFile(CString fileName);
		void PlayFile(CString fileName);
		
		enum class StopResult { DidStop, WasNotPlaying };
		StopResult StopPlayback();

		
		void AsyncEnqueueFile();
		void PlayQueued();
		void Replay();
		void PlayRandom();

		void UpdateObserverDialog();

		void PlayAlarmSound();
		void OpenObserverDialog();


		volatile PluginItemType GetPluginInfoData_lastType = PluginItemType::PLUGIN_SERVER;
		volatile uint64 GetPluginInfoData_lastId = 0;
		void StoreGetPluginInfoData(uint64 id, PluginItemType type);
		CStringA GetPluginInfoData();
		void RefreshTsInterface();
		// for workaround
		void OnServerUpdatedEvent();

	protected:
		HookResult LocalKeyboardHookInstallerDelegate::OnKeyboardHookEvent(const KeyboardHook::KeyData& keyData) override;
		void LocalKeyboardHookInstallerDelegate::OnMessage(const CString& message) override;


		void InputHandlerDelegate::OnPossibleFilesChanged(const FileList& fileList) override;
		void InputHandlerDelegate::OnInputCommandFinished() override;
		

		void QuickSoundHandlerDelegate::OnQuickSoundMatch(const CString& path);

	private:

		optional<CString> SoundplayerApp::TryGetSelectedFile();

		void SendFileNameToChat(CString fileName);
		void SendMessageToChannelChat(CString message);
		void OpenDeveloperConsole();


	private:
		MenuHandler menuHandler;

		std::mutex playerLock;

		concurrency::concurrent_queue<CString> playlist;

		CString lastFile;

		AudioProcessor audioProcessor;



		// TODO : EZ KIBASZOTT ANTIHATÉKONY, CSAK ELÖSZÖR MUKODJON EGYALTALAN
		AudioBuffer audioBufferForCapture;
		AudioBuffer audioBufferForPlayback;


		LocalKeyboardHookInstaller localHookInstaller = LocalKeyboardHookInstaller(*this);
		bool shouldDisableHookWhenScrollLockIsEnabled = true;

		QuickSoundHandler quickSoundHandler = QuickSoundHandler(*this);
		InputHandler inputHandler = InputHandler(*this);

		// különböző thread-ek buzerálhatják, le kell előtte másolni a ptr-t
		shared_ptr<FileList> unsafeFileList;
		
	};

}

