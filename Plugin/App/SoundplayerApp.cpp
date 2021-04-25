#include "stdafx.h"

#include "SoundplayerApp.h"

#include <App/Globals.h>
#include <App/UpdateHandler.h>

#include <Audio/wave.h>
#include <Audio/AudioDecoder.h>
//#include <Audio/Steganography.h>

#include "Gui/SettingsDialog.h"
#include <Gui/AudioProcessorDialog.h>
#include <Gui/ConfigDialog.h>
#include <Gui/OverlayWindow.h>
#include <Gui/TwitchIntegrationDialog.h>

#include <Util/TSSoundPlayer.h>
#include <Util/Util.h>
#include <Util/TsHelperFunctions.h>

#include <Twitch/TwitchRewards.h>
#include <Twitch/TwitchChat.h>
#include <Twitch/TwitchApi.h>
#include <Twitch/TwitchPubSub.h>

#include "afxdlgs.h"
#include <Mmsystem.h>
#include <atlpath.h>
#include <filesystem>




#define DEBUG_MODE FALSE
#define USE_KEYBOARD_HOOK !defined(DEBUG_EXE) && !DEBUG_MODE

/*
TODO LIST
első futtatásnál 0x707 hiba a lejátszásnál
normálisan beálíltani a lejátszás paramétereket, új soundpalyer osztály?
megnézni hogy lehet e használni a acquiredata függvényt
update jelzö
reload plugin ne fagyjon szét
hangtorzitás :^)
hangfelvétel
A Sleep paraméterét normálisan kiszámolni ????
cachelni a reasamplezott cuccokat
a connectiont hozzárendelni az apphoz, mert az a szar simán változhat a az ipse két fület nyit T_T
	arra viszont vigyázni kell hogy a keyboardhook az singleton!!!!
az audiocache-nek kéne egy map a különböző méretekhez
a resample-t be kéne tenni az onlófasz eventbe

2019:
bogiéknak hogy hallják egymást
random gomb?
bindolás?
konzol parancsok? vagy valahogy megoldani hogy ha a ts van előtérben akkor is mukodjon
check for updates
*/


namespace TSPlugin {


	using namespace Global;


	void SoundplayerApp::Init() {

		//LoadConfig();

#if USE_KEYBOARD_HOOK
		InitKeyboardHook();
#endif


#if DEBUG_MODE
		OpenDeveloperConsole();
#endif

		// hogy feldobja az ablakot, ha kell
		TryGetSoundsDirectory(AskGui);

		twitchState->session = Global::config.Get(ConfigKeys::TwitchSession);

		InitTwitchIntegration();



		if (Global::config.GetBool(ConfigKeys::CanReceiveNetworkSoundData)) {
			networkAudioHandler->StartService();

			unique_lock<std::mutex> captureBuffersLock(captureBuffersMutex);
			unique_lock<std::mutex> playbackBuffersLock(playbackBuffersMutex);


			captureBuffers.insert(networkAudioHandler->GetOutboundAudioBuffer());
			playbackBuffers.insert(networkAudioHandler->GetInboundAudioBuffer());

			//playbackBuffers.insert(networkAudioHandler->GetOutboundAudioBuffer());
			//captureBuffers.insert(networkAudioHandler->GetInboundAudioBuffer());

			//playbackBuffers.insert(networkAudioHandler->GetCaptureBuffer());
			//playbackBuffers.insert(networkAudioHandler->GetPlaybackBuffer());

		}

		if (Global::config.GetBool(ConfigKeys::ShouldDisplayOverlay)) {
			UpdateOverlay();
		}
	}

	void SoundplayerApp::Shutdown() {
		SaveConfig();
		if (networkAudioHandler) {
			networkAudioHandler->Stop();
		}

		if (twitchChatReader) {
			twitchChatReader->Stop();
		}

		if (twitchPubSub) {
			twitchPubSub->Stop();
		}
	}


	void SoundplayerApp::InitKeyboardHook() {
		localHookInstaller.Attach();
	}

	void SoundplayerApp::InitTwitchIntegration() try {
		// TODO lehet hogy ezt threadben jobb lenne

		const CString session = twitchState->session;
		if (session.GetLength() == 0) {
			twitchChatReader = nullptr;
			twitchPubSub = nullptr;
			return;
		}


		if (!Twitch::PollAccessToken(*twitchState)) {
			return;
		}

		const CString twitchToken = twitchState->accessToken;
		if (twitchToken.GetLength() == 0) {
			return;
		}

		auto validationInfo = twitchState->validationInfo;
		if (!validationInfo) {
			return;
		}

		//auto channel = Global::config.Get(ConfigKeys::TwitchChannel);
		const CString channel = validationInfo->clientName;
		if (channel.GetLength() == 0) {
			return;
		}

		const CStringA ircChannel = CStringA("#") + ConvertUnicodeToUTF8(channel);

		if (!twitchChatReader) {
			twitchChatReader = TwitchChat::CreateTwitchChatReader();
			twitchChatReader->Start(*this, ircChannel.GetString(), ConvertUnicodeToUTF8(twitchToken).GetString());
		}

		if (!twitchPubSub) {
			twitchPubSub = TwitchPubSub::CreateTwitchPubSub(*this, twitchState);
			twitchPubSub->Start();
		}

	} catch (...) {

	}


	/*
	* Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
	* Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
	* This function is automatically called by the client after ts3plugin_init.
	*/
	void SoundplayerApp::InitHotkeys(struct PluginHotkey*** hotkeys) {
		*hotkeys = nullptr;
		// removed
	}

	void SoundplayerApp::OnHotkey(CStringA keyword) {
		// removed
	}
	


	/*
	 * Initialize plugin menus.
	 * This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
	 * Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
	 * If plugin menus are not used by a plugin, do not implement this function or return NULL.
	 */
	void SoundplayerApp::InitMenus(PluginMenuItem *** menuItems, char ** menuIcon) {
		/*
		 * Create the menus
		 * There are three types of menu items:
		 * - PLUGIN_MENU_TYPE_CLIENT:  Client context menu
		 * - PLUGIN_MENU_TYPE_CHANNEL: Channel context menu
		 * - PLUGIN_MENU_TYPE_GLOBAL:  "Plugins" menu in menu bar of main window
		 *
		 * Menu IDs are used to identify the menu item when ts3plugin_onMenuItemEvent is called
		 *
		 * The menu text is required, max length is 128 characters
		 *
		 * The icon is optional, max length is 128 characters. When not using icons, just pass an empty string.
		 * Icons are loaded from a subdirectory in the TeamSpeak client plugins folder. The subdirectory must be named like the
		 * plugin filename, without dll/so/dylib suffix
		 * e.g. for "test_plugin.dll", icon "1.png" is loaded from <TeamSpeak 3 Client install dir>\plugins\test_plugin\1.png
		 */

		menuHandler.Add("Play sound from file...", [&] { this->AsyncOpenAndPlayFile(); });
		menuHandler.Add("Enqueue sound from file...", [&] { this->AsyncEnqueueFile(); });
		menuHandler.Add("Settings...", [&] { this->OpenConfigDialog(); });
		//menuHandler.Add("Open observer...", [&] { this->OpenObserverDialog(); });
		menuHandler.Add("Check for updates", [&] { CheckForUpdates(); });

#ifdef _DEBUG
		menuHandler.Add("Open Developer Console", [&] { this->OpenDeveloperConsole(); });
#endif
		menuHandler.Add("Twitch Integration...", [&] { this->OpenTwitchDialog(); });

		menuHandler.Configure(menuItems);
		//


		// * Specify an optional icon for the plugin. This icon is used for the plugins submenu within context and main menus
		// * If unused, set menuIcon to NULL
		// */
		*menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
		_strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "t.png");

		/*
		 * Menus can be enabled or disabled with: ts3Functions.setPluginMenuEnabled(pluginID, menuID, 0|1);
		 * Test it with plugin command: /test enablemenu <menuID> <0|1>
		 * Menus are enabled by default. Please note that shown menus will not automatically enable or disable when calling this function to
		 * ensure Qt menus are not modified by any thread other the UI thread. The enabled or disable state will change the next time a
		 * menu is displayed.
		 */
		 /* For example, this would disable MENU_ID_GLOBAL_2: */
		 /* ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 0); */

		 /* All memory allocated in this function will be automatically released by the TeamSpeak client later by calling ts3plugin_freeMemory */

	}

	void SoundplayerApp::OnMenuItemEvent(PluginMenuType type, int menuItemID, uint64 selectedItemID) {
		menuHandler.OnMenuItemEvent(type, menuItemID, selectedItemID);
	}




	void SoundplayerApp::AsyncOpenAndPlayFile() {

		std::thread soundPlayerThread([&]() {
			// Mivel ez nem egy generált mfc alkalmazás nincs minden
			// alapból inicializálva, és assert-et dob a dialog konstruktor
			// ha ez nincs itt
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			CFileDialog dialog(TRUE);

			auto result = dialog.DoModal();
			if (result == IDOK) {
				CString fileName = dialog.GetPathName();
				PlayFile(fileName.GetString());
			}


		});
		soundPlayerThread.detach();

	}


	void SoundplayerApp::PlayFile(const fs::path& filePath, const PlayFileOptions& options) {

		//if (filePath.Find(L".mp3") >= 0) {
		//	return;
		//}

		if (!fs::exists(filePath)) {
			return;
		}
		//if (!PathFileExists(filePath)) {
		//	return;
		//}

		std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(filePath.c_str());


		if (!track) {
			Log::Warning(L"LoadWaveFile failed");
			return;
		}

		track->metadata.comment = options.comment;



#ifdef DEBUG 
		if (playerLock.try_lock()) {
			playerLock.unlock();
		} else {
			Log::Warning(L"SoundplayerApp::PlayFile(CString fileName) is locked.");
		}
#endif
		std::unique_lock<std::mutex> lock(playerLock);
		

		this->lastFile = filePath; // csak nem akad össze...


		unique_lock<std::mutex> captureBuffersLock(captureBuffersMutex);
		for (const auto& captureBuffer : captureBuffers) {
			captureBuffer->AddTrackToQueue(make_shared<WaveTrackPlaybackState>(track));
		}
		captureBuffersLock.unlock();



		//unique_lock<std::mutex> playbackBuffersLock(playbackBuffersMutex);
		//for (const auto& playbackBuffer : playbackBuffers) {
		//	audioBufferForPlayback->AddSamples(track);
		//}

		auto playbackState = make_shared<WaveTrackPlaybackState>(track);
		audioBufferForPlayback->AddTrackToQueue(playbackState);
		//playbackBuffersLock.unlock();

		// ha uj hangot hátszunk be, felejtsük el az elözöt
		pausedTrack = nullptr;


		// this is not the best approach
		// <overlay refreshing> 
		// TODO ezt valahogy automatizálni?
		InvalidateOverlay();

		while (track == audioBufferForPlayback->GetCurrentTrack()) {
			this_thread::sleep_for(1000ms);
		}
		InvalidateOverlay();
		// </overlay refreshing> 
	}



	SoundplayerApp::StopResult SoundplayerApp::StopPlayback() {
		StopResult stopResult = audioBufferForCapture->IsEmpty() ? StopResult::WasNotPlaying : StopResult::DidStop;
		
		audioBufferForCapture->Clear();
		audioBufferForPlayback->Clear();


		return stopResult;
	}

	shared_ptr<WaveTrackPlaybackState> SoundplayerApp::PausePlayback() {
		
		shared_ptr<WaveTrackPlaybackState> track = audioBufferForCapture->TryPopTrack();
		audioBufferForPlayback->TryPopTrack();

		return track;
	}

	void SoundplayerApp::ResumePlayback(shared_ptr<WaveTrackPlaybackState> track) {
		audioBufferForCapture->AddTrackToQueue(track);
		audioBufferForPlayback->AddTrackToQueue(track);
	}


	HookResult SoundplayerApp::PauseOrResumePlayback() {

		const shared_ptr<WaveTrackPlaybackState> track = PausePlayback();
		if (track) {
			pausedTrack = track;
			return ConsumeEvent;
		}

		const shared_ptr<WaveTrackPlaybackState> pausedTrack_guard = pausedTrack;
		if (pausedTrack_guard) {
			ResumePlayback(pausedTrack_guard);
		}

		return PassEvent;
	}

	void SoundplayerApp::AsyncPlayFile(const fs::path& file, const PlayFileOptions& options) {
		std::thread([this, file, options] {
			PlayFile(file, options);
		}).detach();

	}


	void SoundplayerApp::AsyncEnqueueFile() {

		std::thread soundPlayerThread([&]() {
			// Mivel ez nem egy generált mfc alkalmazás nincs minden
			// alapból inicializálva, és assert-et dob a dialog konstruktor
			// ha ez nincs itt
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			CFileDialog dialog(TRUE);

			auto result = dialog.DoModal();
			if (result == IDOK) {
				CString fileName = dialog.GetPathName();
				playlist.push(fileName.GetString());
			}
		});
		soundPlayerThread.detach();
	}

	void SoundplayerApp::PlayQueued() {
		fs::path fileName;
		if (playlist.try_pop(fileName)) {
			AsyncPlayFile(fileName);
		}
	}

	void SoundplayerApp::Replay() {
		if (lastFile) {
			AsyncPlayFile(*lastFile);
		}
	}


	void SoundplayerApp::OpenSettingsDialog(void* handle, void* qParentWidget) {
#if 0 
		ts3Functions.showHotkeySetup();
#else 
		CWnd parent;
		parent.Attach(((HWND)handle));
		SettingsDialog dialog(&parent);
		dialog.qParentWidget = qParentWidget;
		dialog.DoModal();
		parent.Detach();
#endif
	}

	void SoundplayerApp::OpenSoundsFolderSelectorDialog() {
		assert(0 && "nincs megirva...");
	}

	void SoundplayerApp::OpenConfigDialog() {
		try {
			// mivel dll ben vagyunk fasztudja, hogy az AfxGetMainWnd() tényleg jót ad e vissza...
			ConfigDialog dialog(Global::config.MakeCopyOfEntries(), NULL);
			const INT_PTR dialogResult = dialog.DoModal();
			if (dialogResult == IDOK) {
				Global::config.SetEntries(dialog.GetEntries());
				SaveConfig();
			}
		} catch (...) {
			MessageBoxA(NULL, "Unexpected error", 0, 0);
		}
	}

	void SoundplayerApp::PlayRandom() {
		
		if (auto folder = Global::config.TryGet(ConfigKeys::SoundFolder)) {

			fs::path folderPath = folder->GetString();

			//if (folder.Right(1) != "\\" && folder.Right(1) != "/") {
			//	folder += "\\";
			//}

			vector<fs::path> files = ListFilesInDirectory(folderPath);

			srand((unsigned int)time(0));
			int random = rand() % files.size();

			AsyncPlayFile(folderPath / files[random]);
		}
	}




	void SoundplayerApp::PlayAlarmSound() {
		std::thread([&] {
			PlaySound(
				(LPCWSTR)SND_ALIAS_SYSTEMEXCLAMATION,
				GetModuleHandle(0),
				SND_ALIAS_ID
			);
		}).detach();
	}

	void SoundplayerApp::OpenObserverDialog() {
		// unimplemented
	}

	void SoundplayerApp::OpenDeveloperConsole() {
		OpenConsole();
	}

	void SoundplayerApp::InvalidateOverlay() {

		if (overlayWindow) {
			overlayWindow->SetInterfaceText(CreateTextUI());
		}
	}


	void SoundplayerApp::UpdateOverlay() {
		if (!overlayWindow) {
			overlayWindow = make_shared<OverlayWindow>();
			overlayWindow->Create(IDD_OVERLAY_WINDOW);
			overlayWindow->ShowWindow(SW_SHOW);
		}
		
		InvalidateOverlay();
	}

	void SoundplayerApp::SaveConfig() {
		const fs::path path = fs::path(Global::configPath) / Global::config.defaultFileName.GetString();
		Global::config.SaveToFile(path);
	}

	void SoundplayerApp::LoadConfig() {
		const fs::path path = fs::path(Global::configPath) / Global::config.defaultFileName.GetString();
		Global::config.LoadFromFile(path);
	}

	void SoundplayerApp::OpenTwitchDialog() {

		TwitchIntegrationDialog twitchDialog(twitchState);
		twitchDialog.DoModal();
		
		Global::config.Add(ConfigKeys::TwitchSession, twitchState->session);
		Global::config.Save();

		InitTwitchIntegration();
	}



	void SoundplayerApp::UpdateObserverDialog() {
		RefreshTsInterface();
	}


	void SoundplayerApp::StoreGetPluginInfoData(uint64 id, PluginItemType type) {
		this->GetPluginInfoData_lastId = id;
		this->GetPluginInfoData_lastType = type;
	}

	// create a text UI that is displayed both in the TS info panel,
	// and in the overlay window (if visible)
	CString SoundplayerApp::CreateTextUI() {
		CString textUI;

		if (auto track = audioBufferForPlayback->GetCurrentTrack()) {
			const auto& metadata = track->metadata;
			if (metadata.fileName) {
				textUI += FormatString(L"Now playing: %s", metadata.fileName->GetString());
			}

			if (metadata.comment) {
				textUI += FormatString(L" (%s)", metadata.comment->GetString());
			}

			if (!textUI.IsEmpty()) {
				textUI += L"\n";
			}
		}

		textUI += inputHandler.CreateTextInterface();

		return textUI;
	}


	void SoundplayerApp::RefreshTsInterface() {
		// https://forum.teamspeak.com/threads/81701-Client-window-refresh?p=415973#post415973
		ts3Functions.requestServerVariables(Global::connection);
		if (Global::config.GetBool(ConfigKeys::ShouldDisplayOverlay)) {
			UpdateOverlay();
		}
		
	}

	void SoundplayerApp::OnServerUpdatedEvent() {
		// https://forum.teamspeak.com/threads/81701-Client-window-refresh?p=415973#post415973
		ts3Functions.requestInfoUpdate(Global::connection, GetPluginInfoData_lastType, GetPluginInfoData_lastId);
	}


	void SoundplayerApp::SendFileNameToChat(CString path) {
		// TODO ide kéne idözito h pl max 10 secenként 1x küldj be, mert láárka kifagy töle...
		const UINT minDelay = 5 * 1000;
		static UINT lastMessageTime = 0;
		UINT time = GetTickCount();


		CString fileName = FileNameFromPath(path);
		CString message = L"Playing: " + fileName;
		if (time - lastMessageTime > minDelay) {
			SendMessageToChannelChat(message);
			lastMessageTime = time;
		} else {
			Log::Debug(L"Not sending to chat to avoid spam protection: " + message);
		}


	}

	void SoundplayerApp::SendMessageToChannelChat(CString message) {
		anyID myID;
		ts3Functions.getClientID(Global::connection, &myID);

		uint64 channelId;
		ts3Functions.getChannelOfClient(Global::connection, myID, &channelId);
		CStringA utfMessage(message);

		ts3Functions.requestSendChannelTextMsg(Global::connection, utfMessage, channelId, NULL);
	}

	void SoundplayerApp::AsyncOpenAudioProcessorDialog() {
		std::thread dialogThread([&] {
			AudioProcessorDialog dialog(*audioProcessor);
			dialog.DoModal();
		});
		dialogThread.detach();
	}

#if NEW_SOUND_PROCESSING_VERSION
	class TsVoiceHandler {


	private:
		bool soundPlaying = false;
		int pushToTalkActivated = 0;
		CStringA defaultVadState;
	public:
		void ForceEnableMicrophone() {
			
			assert(Global::connection);

			if (!soundPlaying) {
				soundPlaying = true;
				defaultVadState = Ts::GetPreProcessorConfigValue(Ts::VoiceActivation);
				Ts::SetPreProcessorConfigValue(Ts::VoiceActivation, Ts::False);
				
				pushToTalkActivated = Ts::GetClientSelfVariableAsInt(CLIENT_INPUT_DEACTIVATED);
				Ts::SetClientSelfVariableAsInt(CLIENT_INPUT_DEACTIVATED, INPUT_ACTIVE);
				Ts::FlushClientSelfUpdates();
			}



		}

		void ResetMicrophone() {
			if (soundPlaying) {
				soundPlaying = false;

				Ts::SetPreProcessorConfigValue(Ts::VoiceActivation, defaultVadState);
				Ts::SetClientSelfVariableAsInt(CLIENT_INPUT_DEACTIVATED, pushToTalkActivated);
				Ts::FlushClientSelfUpdates();
			}
		}
		
	} tsVoiceHandler;

#endif


	//If the sound data will be send, (*edited | 2) is true.
	//If the sound data is changed, set bit 1 (*edited |= 1).
	//If the sound should not be send, clear bit 2. (*edited &= ~2)
	void SoundplayerApp::OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited) {

		assert(channels == 1);


		bool didChangeData = false;

		CachedAudioSample48k playbackSamples = audioBufferForCapture->TryGetSamples(sampleCount, channels);
		if (playbackSamples) {
			if (sampleCount*channels == playbackSamples->size()) {
				SgnProc::Mix(samples, playbackSamples->data(), sampleCount);
				didChangeData = true;
			}
		}

		if (audioProcessorEnabled) {
			if (audioProcessor->Process(samples, sampleCount, channels)) {
				didChangeData = true;
			}
		}
		

		if (didChangeData) {
			// hát ezt lehet hogy nem ide kéne rakni :D dehát lófasz
			tsVoiceHandler.ForceEnableMicrophone();
			*edited |= (0x1 | 0x2);
		} else {
			tsVoiceHandler.ResetMicrophone();
			*edited &= ~1;
		}

	}



	void SoundplayerApp::OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {

	

		bool didAddAudio = false;

		unique_lock<std::mutex> playbackBuffersLock(playbackBuffersMutex);

		for (auto& buffer : playbackBuffers) {
			CachedAudioSample48k playbackSamples = buffer->TryGetSamples(sampleCount, channels);
			if (playbackSamples) {
				// ha alapból üres lett volna, akk kinullázzuk, ( és kséőbb beállítjuk, hogy raktunk bele dolgot)
				if (!(*channelFillMask & 0x3)) {
					memset(samples, 0, sampleCount * channels * sizeof(short));
				}

				if (sampleCount*channels == playbackSamples->size()) {
					SgnProc::Mix(samples, playbackSamples->data(), sampleCount*channels);
					didAddAudio = true;
				}
			}
		}

		playbackBuffersLock.unlock();

		if (didAddAudio) {
			// beállítjuk, hogy írtunk erre a 2 csatornára
			*channelFillMask |= 3;
		}

	}

	void SoundplayerApp::OnEditPlaybackVoiceDataEvent(anyID clientID, short* samples, int sampleCount, int channels) {
		// már semmi, ki is lehetne rakni
	}

#ifdef _DEBUG
#define MARCI_VERZIO TRUE
#else
#define MARCI_VERZIO FALSE
#endif

	void SoundplayerApp::OnClientMoved(anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, CString moveMessage) {

		// ez csak az én gépemen legyen
		if (!MARCI_VERZIO) return;

		//if (clientID == Global::connection) {
		anyID myID;
		// ezt a globalozást kurva gyorsan meg kéne szüntetni
		ts3Functions.getClientID(Global::connection, &myID);

		if (clientID == myID) {
			cout << "Moved to channel id " << newChannelID << endl;
			wcout << moveMessage.GetString() << endl;

		}

		char name_utf8[256];
		ts3Functions.getClientDisplayName(Global::connection, clientID, name_utf8, sizeof(name_utf8));

		const CString name = Utf8ToCString(name_utf8);

		uint64 clientChannelId;
		ts3Functions.getChannelOfClient(Global::connection, clientID, &clientChannelId);

		uint64 ownChannelId;
		ts3Functions.getChannelOfClient(Global::connection, myID, &ownChannelId);

		if (ownChannelId == clientChannelId) {
			Log::Debug(name);
		} else {
			Log::Debug(CString("masik channel: ") + name);
		}

		if (ownChannelId == clientChannelId) {

			const static std::map<CString, CString> userWelcomeSoundsMapping = {
				{L"Hodi", L"szarhazi"},
				{L"Ugyis", L"itt a ku"},
				{L"Kurátor", L"itt a ku"},
				{L"Battlechicken", L"itt vagyok"},
				{L"yoloczki", L"dorime"},
				{L"Bogecz", L"bogi"},
				{L"Antekirt", L"jaj egy szazmeteres"},
				{L"Tachibana", L"jakab"},
			};

			//(name, predicate);
			auto predicate = [&name](const pair<CString, CString>& pair) -> bool {
				return name.Find(pair.first) >= 0;
			};

			const auto it = find_if(userWelcomeSoundsMapping.begin(), userWelcomeSoundsMapping.end(), predicate);
			if (it != userWelcomeSoundsMapping.end()) {
				const CString soundName = it->second;
				std::thread([=] {

					Sleep(500);
					if (auto fileName = TryGetSoundFileForUserInput(soundName)) {
						AsyncPlayFile(*fileName);
					}

				}).detach();
			}

		}
	}


	static inline bool IsScrollLockPressed() {
		return (GetKeyState(VK_SCROLL) & 0x1) != 0;
	}


	HookResult SoundplayerApp::OnKeyboardHookEvent(const KeyboardHook::KeyData& keyData) {

		if (shouldDisableHookWhenScrollLockIsEnabled && IsScrollLockPressed()) {
			return HookResult::PassEvent;
		}

		if (keyData.hookData.vkCode == VK_SCROLL) {
			return HookResult::PassEvent;
		}

		if (keyData.hookData.vkCode == VK_ESCAPE) {
			const StopResult result = StopPlayback();
			if (result == StopResult::DidStop) {
				return ConsumeEvent;
			}
		}

		if (keyData.hookData.vkCode == VK_MEDIA_PLAY_PAUSE) {
			const HookResult evantResult = PauseOrResumePlayback();
			if (evantResult == HookResult::ConsumeEvent) {
				return ConsumeEvent;
			}
		}


		if (inputHandler.TryConsumeEvent(keyData) == HookResult::ConsumeEvent) {
			return HookResult::ConsumeEvent;
		}

		if (quickSoundHandler.TryConsumeEvent(keyData) == HookResult::ConsumeEvent) {
			return HookResult::ConsumeEvent;
		}

		return HookResult::PassEvent;


	}

	void SoundplayerApp::OnMessage(const CString& message) {
		std::wcout << message << std::endl;
	}



	void SoundplayerApp::OnInterfaceInvalidated() {
		RefreshTsInterface();
	}

	void SoundplayerApp::OnInputCommandFinished() {

		if (auto fileName = inputHandler.TryGetSelectedFile()) {
			AsyncPlayFile(*fileName);
		} else {
			PlayAlarmSound();
		}
	}

	void SoundplayerApp::OnHotkeyCommand(const CString& command) {
		if (auto fileToPlay = TryGetSoundFileForUserInput(command)) {
			AsyncPlayFile(*fileToPlay);
		}
		//std::vector<CString> possibleFiles = GetPossibleFiles(command);
		//if (possibleFiles.size() > 0) {
		//	const CString& fileToPlay = possibleFiles[rand() % possibleFiles.size()];
		//	AsyncPlayFile(fileToPlay);
		//}
	}

	void SoundplayerApp::OnFileEnqueued(const fs::path& file) {
		// inimplemeted
	}

	void SoundplayerApp::OnQuickSoundMatch(const fs::path& path) {
		AsyncPlayFile(path);
	}

	void SoundplayerApp::OnTwitchChannelPointRedemption(const Twitch::RewardRedemption& rewardRedemption) {
		if (!twitchState) {
			ASSERT(0);
			return;
		}


		auto soundFilePath = TryGetSoundFileForUserInput(Utf8ToCString(rewardRedemption.user_input.c_str()));

		std::string chatResponseText;
		if (!soundFilePath) {
			chatResponseText = format_string("No matching file found: '%s'", rewardRedemption.user_input.c_str());
		} else {
			chatResponseText = format_string("Playing '%s'", soundFilePath->filename().u8string().c_str());
		}

		const CString comment = FormatString(L"requested by: %s", Utf8ToCString(rewardRedemption.user_name.c_str()).GetString());
		AsyncPlayFile(*soundFilePath, { .comment = comment });


		const Twitch::RedemptionStatus newStatus = soundFilePath ? Twitch::RedemptionStatus::Fulfilled : Twitch::RedemptionStatus::Cancelled;

		Twitch::UpdateRewardRedemption(*twitchState, rewardRedemption, newStatus);
		twitchChatReader->SendChannelMessage(chatResponseText.c_str());
	}

	TwitchChat::TwitchResponse SoundplayerApp::OnTwitchMessage(const std::string& channel, const std::string& sender, const std::string& message) {
		(void)channel;

		TwitchChat::TwitchResponse response;

		//vector<string> authorizeUsers;
		istringstream authorizedUsersList = istringstream(ConvertUnicodeToUTF8(Global::config.Get(ConfigKeys::AuthorizedUsers)).GetString());
		string name;
		bool authorized = false;

		while (std::getline(authorizedUsersList, name, ',')) {
			if (name == sender) {
				authorized = true;
				break;
			}
		}


		std::regex re(R"""(!sound (.+))""");
		std::match_results<const char*> captures;
		if (!std::regex_search(message.c_str(), captures, re)) {
			return {};
		}

		const std::string userInput = captures[1];

		if (!authorized) {
			Log::Debug(L"Unauthorized user: " + Utf8ToCString(CStringA(sender.c_str())));
			return {
				.chatResponseText = format_string("Unauthorized user: %s", sender.c_str()),
			};
		}

		
		auto fileName = TryGetSoundFileForUserInput(Utf8ToCString(userInput.c_str()));

		if (!fileName) {
			return {
				.chatResponseText = format_string("No matching file found: '%s'", userInput.c_str()),
			};
		}

		const CString comment = FormatString(L"requested by: %s", Utf8ToCString(sender.c_str()).GetString());
		AsyncPlayFile(*fileName, { .comment = comment });

		return { 
			.chatResponseText = format_string("Playing %s", fileName->filename().u8string().c_str()),
		};
	}


	//optional<CString> SoundplayerApp::TryGetSelectedFile() {
	//	shared_ptr<const FileList> fileList = unsafeFileList;

	//	if (!fileList) {
	//		return nullopt;
	//	}

	//	if (fileList->selectedFileIndex < fileList->possibleFiles.size()) {
	//		return fileList->possibleFiles[fileList->selectedFileIndex];
	//	}

	//	return nullopt;
	//}


}

