#pragma once


//#include <HookInstaller/Hook/KeyboardHookInstaller.h>

// TODO kihozni a libböl
//#include <HookInstaller/Hook/PipeHandler.h>

#include "KeyboardHook/LocalKeyboardHookInstaller.h"
//#include "KeyboardHook/PipeHandler.h"

#include <Util/Config.h>
#include <Audio/OnlineMicrophone.h>
#include <Audio/AudioProcessor.h>
#include <Audio/AudioPlayer.h>
#include <Audio/AudioBuffer.h>
#include <Gui/InputObserverDialog.h>

#include <Twitch/TwitchChat.h>
#include <Twitch/TwitchApi.h>

#include "HotkeyHandler.h"
#include "MenuHandler.h"
#include "QuickSoundHandler.h"
#include "InputHandler.h"
#include "NetworkAudioHandler.h"

#include <optional>


//#include <Web/SoundBroadcaster.h>

namespace TSPlugin {

	class SoundplayerApp : public LocalKeyboardHookInstallerDelegate, InputHandlerDelegate, QuickSoundHandlerDelegate, TwitchChat::ITwitchMessageHandler {

	public:

		// We need an explicit init function cause
		// we can't create threads from dllmain
		// and thus from constructor... :(
		void Init();
		void Shutdown();

		//void CheckForUpdates();

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
		void OpenConfigDialog();

		void AsyncOpenAudioProcessorDialog();

		void AsyncOpenAndPlayFile();

		struct PlayFileOptions {
			optional<CString> comment;
		};

		void AsyncPlayFile(const fs::path& fileName, const PlayFileOptions& options = {});
		void PlayFile(const fs::path& fileName, const PlayFileOptions& options = {});
		
		enum class StopResult { DidStop, WasNotPlaying };
		StopResult StopPlayback();
		shared_ptr<WaveTrackPlaybackState> PausePlayback();
		void ResumePlayback(shared_ptr<WaveTrackPlaybackState> track);
		HookResult PauseOrResumePlayback();

		
		void AsyncEnqueueFile();
		void PlayQueued();
		void Replay();
		void PlayRandom();

		void UpdateObserverDialog();

		void PlayAlarmSound();
		void OpenObserverDialog();

		void InvalidateOverlay();

		volatile PluginItemType GetPluginInfoData_lastType = PluginItemType::PLUGIN_SERVER;
		volatile uint64 GetPluginInfoData_lastId = 0;
		void StoreGetPluginInfoData(uint64 id, PluginItemType type);
		CString CreateTextUI();
		void RefreshTsInterface();
		// for workaround
		void OnServerUpdatedEvent();


	protected:
		HookResult LocalKeyboardHookInstallerDelegate::OnKeyboardHookEvent(const KeyboardHook::KeyData& keyData) override;
		void LocalKeyboardHookInstallerDelegate::OnMessage(const CString& message) override;


		void InputHandlerDelegate::OnInterfaceInvalidated() override;
		void InputHandlerDelegate::OnInputCommandFinished() override;
		void InputHandlerDelegate::OnHotkeyCommand(const CString& command) override;
		void InputHandlerDelegate::OnFileEnqueued(const fs::path& file) override;

		void QuickSoundHandlerDelegate::OnQuickSoundMatch(const fs::path& path) override;

		TwitchChat::TwitchResponse OnTwitchMessage(
			const std::string& channel, 
			const std::string& sender, 
			const std::string& message
		) override;
		
	private:

		//optional<CString> SoundplayerApp::TryGetSelectedFile();


		void InitKeyboardHook();
		void InitTwitchChat();

		void SendFileNameToChat(CString fileName);
		void SendMessageToChannelChat(CString message);
		void OpenDeveloperConsole();
		void UpdateOverlay();
		void SaveConfig();
		void LoadConfig();
		void OpenTwitchDialog();

	private:
		MenuHandler menuHandler;

		std::mutex playerLock;

		concurrency::concurrent_queue<fs::path> playlist;

		optional<fs::path> lastFile;

		bool audioProcessorEnabled = true;
		shared_ptr<AudioProcessor> audioProcessor = make_shared<AudioProcessor>();



		shared_ptr<AudioBuffer> audioBufferForCapture = make_shared<AudioBuffer>();
		shared_ptr<AudioBuffer> audioBufferForPlayback = make_shared<AudioBuffer>();


		mutex captureBuffersMutex;
		set<shared_ptr<AudioBuffer>> captureBuffers = {
			audioBufferForCapture
		};

		mutex playbackBuffersMutex;
		set<shared_ptr<AudioBuffer>> playbackBuffers = {
			audioBufferForPlayback
		};


		LocalKeyboardHookInstaller localHookInstaller = LocalKeyboardHookInstaller(*this);
		bool shouldDisableHookWhenScrollLockIsEnabled = true;

		QuickSoundHandler quickSoundHandler = QuickSoundHandler(*this);
		InputHandler inputHandler = InputHandler(*this);
		shared_ptr<NetworkAudioHandler> networkAudioHandler = NetworkAudioHandler::Create();

		shared_ptr<class OverlayWindow> overlayWindow;

		shared_ptr<TwitchChat::ITwitchChatReader> twitchChatReader;

		shared_ptr<WaveTrackPlaybackState> pausedTrack;

		shared_ptr<Twitch::TwitchState> twitchState = make_shared<Twitch::TwitchState>();
	};

}

