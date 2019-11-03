#include "stdafx.h"

#include "SoundplayerApp.h"
#include "Wave\wave.h"
#include "Wave\AudioDecoder.h"
#include "Wave\Steganography.h"
#include "Web/Http.h"


#include "Gui\SettingsDialog.h"
#include <Gui/AudioProcessorDialog.h>
#include <Gui/ConfigDialog.h>

#include "Util\TSSoundPlayer.h"
#include "Util\Util.h"


#include "afxdlgs.h"
#include <Mmsystem.h>
#include <atlpath.h>
#include <filesystem>



#define USE_KEYBOARD_HOOK TRUE

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

	void SoundplayerApp::InitKeyboardHook() {

		localHookInstaller.Attach();

	}


	void SoundplayerApp::Init() {

#if USE_KEYBOARD_HOOK
		InitKeyboardHook();
#endif


#if defined(_DEBUG ) && FALSE	
		OpenDeveloperConsole();
#endif

		// hogy feldobja az ablakot, ha kell
		TryGetSoundsDirectory(AskGui);


		std::thread([this] {
			CheckForUpdates();
		}).detach();


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
		
	}

	void SoundplayerApp::Shutdown() {
		Global::config.Save();
		if (networkAudioHandler) {
			networkAudioHandler->Stop();
		}
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
		menuHandler.Add("Check for updates", [&] { this->CheckForUpdates(); });

#ifdef _DEBUG
		menuHandler.Add("Open Developer Console", [&] { this->OpenDeveloperConsole(); });
#endif

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
				PlayFile(fileName);
			}


		});
		soundPlayerThread.detach();

	}


	void SoundplayerApp::PlayFile(CString fileName) {

		if (fileName.Find(L".mp3") >= 0) {
			return;
		}

		if (!PathFileExists(fileName)) {
			return;
		}

		std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(fileName);


		if (!track) {
			Log::Warning(L"LoadWaveFile failed");
			return;
		}




#ifdef DEBUG 
		if (playerLock.try_lock()) {
			playerLock.unlock();
		} else {
			Log::Warning(L"SoundplayerApp::PlayFile(CString fileName) is locked.");
		}
#endif
		std::unique_lock<std::mutex> lock(playerLock);
		

		this->lastFile = fileName; // csak nem akad össze...


		unique_lock<std::mutex> captureBuffersLock(captureBuffersMutex);
		for (const auto& captureBuffer : captureBuffers) {
			captureBuffer->AddSamples(track);
		}
		captureBuffersLock.unlock();



		//unique_lock<std::mutex> playbackBuffersLock(playbackBuffersMutex);
		//for (const auto& playbackBuffer : playbackBuffers) {
		//	audioBufferForPlayback->AddSamples(track);
		//}
		audioBufferForPlayback->AddSamples(track);
		//playbackBuffersLock.unlock();
		


	}



	SoundplayerApp::StopResult SoundplayerApp::StopPlayback() {
		StopResult stopResult = audioBufferForCapture->IsEmpty() ? StopResult::WasNotPlaying : StopResult::DidStop;
		
		audioBufferForCapture->Clear();
		audioBufferForPlayback->Clear();


		return stopResult;
	}


	void SoundplayerApp::AsyncPlayFile(CString fileName) {
		std::thread([this, fileName] {
			PlayFile(fileName);
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
				playlist.push(fileName);
			}
		});
		soundPlayerThread.detach();
	}

	void SoundplayerApp::PlayQueued() {
		CString fileName;
		if (playlist.try_pop(fileName)) {
			AsyncPlayFile(fileName);
		}
	}

	void SoundplayerApp::Replay() {
		if (lastFile.GetLength() > 0) {
			AsyncPlayFile(lastFile);
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
		// mivel dll ben vagyunk fasztudja, hogy az AfxGetMainWnd() tényleg jót ad e vissza...
		ConfigDialog dialog(Global::config.MakeCopyOfEntries(), AfxGetMainWnd());
		const INT_PTR dialogResult = dialog.DoModal();
		if (dialogResult == IDOK) {
			Global::config.SetEntries(dialog.m_configDictionary);
			Global::config.Save();
		}
	}

	void SoundplayerApp::PlayRandom() {
		CString folder;
		if (Global::config.TryGet(ConfigKeys::SoundFolder, folder)) {


			if (folder.Right(1) != "\\" && folder.Right(1) != "/") {
				folder += "\\";
			}

			vector<CString> files = ListFilesInDirectory(folder);

			srand((unsigned int)time(0));
			int random = rand() % files.size();

			AsyncPlayFile(folder + files[random]);
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
		CreateConsole();
		QuickSoundsFileSystem fs;
	}


	void SoundplayerApp::UpdateObserverDialog() {
		RefreshTsInterface();
	}


	void SoundplayerApp::StoreGetPluginInfoData(uint64 id, PluginItemType type) {
		this->GetPluginInfoData_lastId = id;
		this->GetPluginInfoData_lastType = type;
	}

	CStringA SoundplayerApp::GetPluginInfoData() {

		const shared_ptr<FileList> fileList = unsafeFileList;

		if (!fileList) {
			return "<No matching files>";
		}

		const auto& possibleFiles = fileList->possibleFiles;
		const size_t selectedFileIndex = fileList->selectedFileIndex;

		CStringA info = "Teszt: \n";

		//for (CString& file : possibleFiles) {
		for (int index = 0; index < possibleFiles.size(); ++index) {
			const CString& filePath = possibleFiles[index];
			const int startIndex = filePath.ReverseFind(L'\\');
			const CString fileName = (startIndex > -1) ? filePath.Mid(startIndex + 1) : filePath;
			const CStringA fileUtf = ConvertUnicodeToUTF8(fileName);
			if (index == selectedFileIndex) {
				info += "*";
			}
			info += fileUtf + "\n";
		}

		return info;
	}


	void SoundplayerApp::RefreshTsInterface() {
		// https://forum.teamspeak.com/threads/81701-Client-window-refresh?p=415973#post415973
		ts3Functions.requestServerVariables(Global::connection);
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
			AudioProcessorDialog dialog(audioProcessor);
			dialog.DoModal();
		});
		dialogThread.detach();
	}

#if NEW_SOUND_PROCESSING_VERSION
	class TsVoiceHandler {


	private:
		bool soundPlaying = false;
		CStringA defaultVadState;
	public:
		void ForceEnableMicrophone() {
			//logDebug("TSMGR: Setting talk state of %ull to %s, previous was %s",
			//	(unsigned long long)scHandlerID, toString(state), toString(previousTalkState));
			assert(Global::connection);


			if (!soundPlaying) {
				soundPlaying = true;
				defaultVadState = Ts::GetPreProcessorConfigValue(Ts::VoiceActivation);
				Ts::SetPreProcessorConfigValue(Ts::VoiceActivation, Ts::False);
			}



		}

		void ResetMicrophone() {
			if (soundPlaying) {
				soundPlaying = false;
				Ts::SetPreProcessorConfigValue(Ts::VoiceActivation, defaultVadState);
				// ez kel
				//ts3Functions.flushClientSelfUpdates(Global::connection, NULL);
			}
		}
		//void SetState(CStringA key, CStringA value) {
		//	ts3Functions.setPreProcessorConfigValue(Global::connection, key, value);
		//}
	} tsVoiceHandler;

#endif






	void SoundplayerApp::OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited) {

		assert(channels == 1);

		//CachedAudioSample48k playbackSamples = audioBufferForCapture.TryGetSamples(sampleCount, channels);
		//if (playbackSamples) {

		//	// hát ezt lehet hogy nem ide kéne rakni :D dehát lófasz
		//	tsVoiceHandler.ForceEnableMicrophone();

		//	if (!(*edited &= 2)) {
		//		// ha nincs küldendö adat
		//		memset(samples, 0, sizeof(short)*sampleCount*channels);
		//	}

		//	assert(sampleCount*channels == playbackSamples->size());
		//	if (sampleCount*channels != playbackSamples->size()) {
		//		Log::Warning(L"if(sampleCount != playbackSamples->size()) {");
		//	}
		//	SgnProc::Mix(samples, playbackSamples->data(), sampleCount);
		//	//*edited |= 2;
		//	*edited |= 1;
		//} else {
		//	tsVoiceHandler.ResetMicrophone();
		//	*edited &= ~1;
		//}




		bool didChangeData = false;

		// ezt nemtom mi a fasz volt
		//if (!(*edited &= 2)) {
		//	// ha nincs küldendö adat
		//	memset(samples, 0, sizeof(short)*sampleCount*channels);
		//}

		//unique_lock<std::mutex> captureBuffersLock(captureBuffersMutex);

		//for (auto& buffer : captureBuffers) {
			auto& buffer = audioBufferForCapture;
			CachedAudioSample48k playbackSamples = buffer->TryGetSamples(sampleCount, channels);
			if (playbackSamples) {
				if (sampleCount*channels == playbackSamples->size()) {
					SgnProc::Mix(samples, playbackSamples->data(), sampleCount);
					didChangeData = true;
				} else {
					//WarnForBullshit();
				}
			}
		//}

		//captureBuffersLock.unlock();

		if (didChangeData) {
			// hát ezt lehet hogy nem ide kéne rakni :D dehát lófasz
			tsVoiceHandler.ForceEnableMicrophone();

			*edited |= 1;
			//*edited |= 2;
		} else {
			tsVoiceHandler.ResetMicrophone();
			*edited &= ~1;
		}


		//If the sound data will be send, (*edited | 2) is true.
		//If the sound data is changed, set bit 1 (*edited |= 1).
		//If the sound should not be send, clear bit 2. (*edited &= ~2)

	}



	void SoundplayerApp::OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {

		//CachedAudioSample48k playbackSamples = audioBufferForPlayback.TryGetSamples(sampleCount, channels);

		//if (playbackSamples) {

		//	if (!(*channelFillMask & 0x3)) {
		//		*channelFillMask |= 3;
		//		memset(samples, 0, sampleCount*channels * sizeof(short));
		//	}
		//	// ez is gecikulturált lett.... TODO


		//	if (sampleCount*channels == playbackSamples->size()) {
		//		SgnProc::Mix(samples, playbackSamples->data(), sampleCount*channels);
		//	} else {
		//		static int asdf = 0;
		//		if (!asdf) {
		//			asdf = 1;
		//			Log::Error(L".......... anyád");
		//		}
		//	}

		//}



		bool didAddAudio = false;

		unique_lock<std::mutex> playbackBuffersLock(playbackBuffersMutex);

		for (auto& buffer : playbackBuffers) {
			CachedAudioSample48k playbackSamples = buffer->TryGetSamples(sampleCount, channels);
			if (playbackSamples) {
				if (sampleCount*channels == playbackSamples->size()) {
					SgnProc::Mix(samples, playbackSamples->data(), sampleCount*channels);
					didAddAudio = true;
				} else {
					//WarnForBullshit();
				}
			}
		}

		playbackBuffersLock.unlock();

		if (didAddAudio) {
			*channelFillMask |= 3;
			// ha alapból üres lett volna, akk kinullázzuk, és beállítjuk, hogy raktunk bele dolgot
			//if (!(*channelFillMask & 0x3)) {
			//	*channelFillMask |= 3;
			//	//memset(samples, 0, sampleCount*channels * sizeof(short));
			//}
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
			cout << moveMessage << endl;

		}

		char name[256];
		ts3Functions.getClientDisplayName(Global::connection, clientID, name, sizeof(name));

		uint64 clientChannelId;
		ts3Functions.getChannelOfClient(Global::connection, clientID, &clientChannelId);

		uint64 ownChannelId;
		ts3Functions.getChannelOfClient(Global::connection, myID, &ownChannelId);

		if (ownChannelId == clientChannelId) {
			Log::Debug(CString(name));
		} else {
			Log::Debug(CString("masik channel: ") + CString(name));
		}

		if (ownChannelId == clientChannelId) {

			const static std::map<CStringA, CString> userWelcomeSoundsMapping = {
				{"Hodi", L"szarhazi"},
				{"Ugyis", L"itt a ku"},
				{"Battlechicken", L"itt vagyok"},
				{"yoloczki", L"dorime"},
				{"Antekirt", L"jaj egy szazmeteres"},
			};

			//(name, predicate);
			auto predicate = [&](const pair<CStringA, CString>& pair) -> bool {
				return CStringA(name).Find(pair.first) >= 0;
			};

			const auto it = find_if(userWelcomeSoundsMapping.begin(), userWelcomeSoundsMapping.end(), predicate);
			if (it != userWelcomeSoundsMapping.end()) {
				const CString soundName = it->second;
				std::thread([=] {

					Sleep(500);
					if (auto fileName = TryGetLikelyFileName(soundName)) {
						AsyncPlayFile(*fileName);
					}

				}).detach();
			}

		}
/*
		bool nameMatchHodi = (strcmp(name, "Hodi") == 0);
		bool nameMatchTomi = (strcmp(name, "Ugyis") == 0);
		if (ownChannelId == clientChannelId && (nameMatchHodi || nameMatchTomi)) {
			std::thread playAndSleepThread = std::thread([=] {

				if (nameMatchHodi) {
					Sleep(500);
					if (auto fileName = TryGetLikelyFileName(L"szar")) {
						AsyncPlayFile(*fileName);
					}
				}

				if (nameMatchTomi) {
					Sleep(500);
					if (auto fileName = TryGetLikelyFileName(L"itt a ku")) {
						AsyncPlayFile(*fileName);
					}
				}
			});
			playAndSleepThread.detach();

		}*/
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



	void SoundplayerApp::OnPossibleFilesChanged(const FileList& fileList) {
		unsafeFileList = make_shared<FileList>(fileList);
		RefreshTsInterface();
	}

	void SoundplayerApp::OnInputCommandFinished() {

		if (auto fileName = TryGetSelectedFile()) {
			AsyncPlayFile(*fileName);
		} else {
			PlayAlarmSound();
		}
	}

	void SoundplayerApp::OnHotkeyCommand(const CString& command) {
		if (auto fileToPlay = TryGetLikelyFileName(command)) {
			AsyncPlayFile(*fileToPlay);
		}
		//std::vector<CString> possibleFiles = GetPossibleFiles(command);
		//if (possibleFiles.size() > 0) {
		//	const CString& fileToPlay = possibleFiles[rand() % possibleFiles.size()];
		//	AsyncPlayFile(fileToPlay);
		//}
	}

	void SoundplayerApp::OnQuickSoundMatch(const CString& path) {
		AsyncPlayFile(path);
	}

	optional<CString> SoundplayerApp::TryGetSelectedFile() {
		shared_ptr<FileList> fileList = unsafeFileList;

		if (!fileList) {
			return nullopt;
		}

		if (fileList->selectedFileIndex < fileList->possibleFiles.size()) {
			return fileList->possibleFiles[fileList->selectedFileIndex];
		}

		return nullopt;
	}

	static void DownloadAndInstallNewVerison() {
		
		const optional<vector<uint8_t>> result = Web::HttpRequest(L"users.atw.hu", L"battlechicken/ts/downloads/SoundplayerPlugin_x64.ts3_plugin");
		if (!result) {
			return;
		}

		TCHAR tempDirectoryPath[MAX_PATH];
		GetTempPath(MAX_PATH, tempDirectoryPath);

		using namespace std::filesystem;

		path tempPath = path(tempDirectoryPath) / "SoundplayerPlugin_x64.ts3_plugin";

		ofstream out(tempPath, ios::binary);
		out.write((const char*)result->data(), result->size());
		out.close();


		ShellExecute(0, 0, tempPath.wstring().c_str(), 0, 0, SW_SHOW);

		
	}

	// defined in plugin.h
	extern "C" const char* ts3plugin_version();

	void SoundplayerApp::CheckForUpdates() {
		const optional<vector<uint8_t>> result = Web::HttpRequest(L"users.atw.hu", L"battlechicken/ts/version");

		if (!result) {
			return;
		}

		

		const CStringA versionOnServer = CStringA((const char*)result->data(), (int)result->size());
		const CStringA currentVersion = ts3plugin_version();

		if (currentVersion.Compare(versionOnServer) >= 0) {
			// we are newer or equal to the server
			return;
		}
		
		const CString title = L"Update available";
		const CString message = L"Newer version exists. Would you like to download it? -- you might have to quit TS after downloading";
		const int messageBoxResult = MessageBox(HWND(0), message, title, MB_YESNO);
		if (messageBoxResult == IDYES) {
			DownloadAndInstallNewVerison();
		}

	}

}

