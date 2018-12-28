﻿#include "stdafx.h"

#include "SoundplayerApp.h"
#include "Wave\wave.h"
#include "Wave\AudioDecoder.h"
#include "Wave\Steganography.h"

#include "Gui\SettingsDialog.h"
#include "Gui\SoundFolderSelector.h"
#include <gui/AudioProcessorDialog.h>

#include "Util\TSSoundPlayer.h"
#include "Util\Util.h"


#include "afxdlgs.h"
#include <Mmsystem.h>
#include <atlpath.h>



#define USE_KEYBOARD_HOOK TRUE

/*
TODO LIST
első futtatásnál 0x707 hiba a lejátszásnál
normálisan beálíltani a lejátszás paramétereket, új soundpalyer osztály?
megnézni hogy lehet e használni a acquiredata függvényt
update jelzö
*backspace - ez mitnha meglenne
reload plugin ne fagyjon szét
*chatbe küldés
hangtorzitás :^)
*nem létező fájlnál ne pampogjon
hangfelvétel
A Sleep paraméterét normálisan kiszámolni ????
cachelni a reasamplezott cuccokat
a connectiont hozzárendelni az apphoz, mert az a szar simán változhat a az ipse két fület nyit T_T
	arra viszont vigyázni kell hogy a keyboardhook az singleton!!!!
az audiocache-nek kéne egy map a különböző méretekhez
a resample-t be kéne tenni az onlófasz eventbe

*/

using namespace std;
using namespace Global;


void SoundplayerApp::InitKeyboardHook() {

	localHookInstaller.Attach();

}


void SoundplayerApp::Init() {
	
#if USE_KEYBOARD_HOOK
	InitKeyboardHook();
#endif

	//CreateConsole();

	//PresenceBroadcaster broadcaster;
	//PresenceListener listener;
	//
	//TcpStreamer tcpStreamer;
	//TcpReceiver tcpReveiver;

	//PathAppend()
	//PathCchAppend()
	//CString path = CString(Global::pluginPath) + L"\\" + Global::config.defaultFileName;
	
}



/*
* Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
* Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
* This function is automatically called by the client after ts3plugin_init.
*/
void SoundplayerApp::InitHotkeys(struct PluginHotkey*** hotkeys) {
	*hotkeys = nullptr;
	/* Register hotkeys giving a keyword and a description.
	* The keyword will be later passed to ts3plugin_onHotkeyEvent to identify which hotkey was triggered.
	* The description is shown in the clients hotkey dialog. */

	//hotkeyHandler.Add(Hotkey::STOP, "Stop playback", [this]{StopPlayback();});
	//hotkeyHandler.Add(Hotkey::PLAY_QUEUED, "Play queued", [this] {PlayQueued(); });
	//hotkeyHandler.Add(Hotkey::REPLAY, "Replay", [this] {Replay(); });
	//hotkeyHandler.Add(Hotkey::PLAY_RANDOM, "Play random", [this] {PlayRandom(); });
	//
	//for(int i = 0; i < soundHotkeyCount; ++i) {
	//	CStringA hotkey;
	//	hotkey.Format(Hotkey::PLAY_PRESET_TEMPLATE, i);
	//
	//	CStringA title;
	//	title.Format("Play sound #%d", i);
	//
	//	hotkeyHandler.Add(hotkey, title, [this, i] {PlayPreset(i); });
	//	//CREATE_HOTKEY(Hotkey::REPLAY, "Replay");
	//}
	//
	//
	////Config::Get(ConfigKey::SoundHotkeyCount, soundHotkeyCount)
	//
	//hotkeyHandler.Configure(hotkeys);

	/* The client will call ts3plugin_freeMemory to release all allocated memory */
}

void SoundplayerApp::OnHotkey(CStringA keyword) {
	//hotkeyHandler.OnHotkeyEvent(keyword);
}
//
//
//class QuickVoiceChatHandler {
//	vector<CString> keys;
//	bool inProgress = false;
//public:
//
//
//	function<void(vector<CString>& keys)> OnCommandCompleted;
//
//public:
//	void Process(const KeyboardHook::KeyData& keyData) {
//		if(inProgress) {
//			if(CString("1234567890").Find(keyData.unicodeLiteral) >= 0) {
//				keys.push_back(keyData.unicodeLiteral);
//				if(keys.size() >= 2) {
//					OnCommandCompleted ? OnCommandCompleted(keys) : NULL;
//					keys.clear();
//				}
//			} else {
//				inProgress = false;
//				keys.clear();
//			}
//
//
//		} else {
//			if(CString(keyData.unicodeLiteral).CompareNoCase(L"v") == 0) {
//				inProgress = true;
//			}
//		}
//	}
//
//
//};
//


HookResult SoundplayerApp::OnKeyData(const KeyboardHook::KeyData& keyData) {

	Finally finally = [&] {
		UpdateObserverDialog();
	};


	if (inputHandler.TryConsumeEvent(keyData) == HookResult::ConsumeEvent) {
		return HookResult::ConsumeEvent;
	}

	if (quickSoundHandler.TryConsumeEvent(keyData) == HookResult::ConsumeEvent) {
		return HookResult::ConsumeEvent;
	}

	return HookResult::PassEvent;
}



//HookResult SoundplayerApp::OnKeyData(const KeyboardHook::KeyData& keyData) {
//
//	Finally finally = [&]{
//		UpdateObserverDialog();
//	};
//
//
//
//
//	if (keyData.hookData.vkCode == VK_ESCAPE) {
//		stop = true;
//
//		audioBufferForCapture.Clear();
//		audioBufferForPlayback.Clear();
//	}
//
//
//
//	if(keyData.hookData.vkCode == VK_ESCAPE) {
//
//		if (!commandInProgress) {
//			return HookResult::PassEvent;
//		}
//
//		commandInProgress = false;
//		inputBuffer = "";
//		stop = true;
//
//		// TODO
//
//		return HookResult::ConsumeEvent;
//	}
//
//	if(keyData.hookData.vkCode == VK_BACK) {
//		if(inputBuffer.GetLength() > 0) {
//			inputBuffer.Truncate(inputBuffer.GetLength() - 1);
//		}
//		return commandInProgress ? HookResult::ConsumeEvent : HookResult::PassEvent;
//	}
//
//	//quickVoiceChatHandler.OnCommandCompleted = [&](vector<CString>& keys) {
//	//
//	//	commandInProgress = false;
//	//	inputBuffer = "";
//	//};
//	//
//	//quickVoiceChatHandler.process(keyData);
//	
//	CString unicodeLiteral = keyData.unicodeLiteral;
//	if(unicodeLiteral == CString("/")) {
//		commandInProgress = true;
//		inputBuffer = "";
//	} else {
//		if(commandInProgress) {
//			if(keyData.hookData.vkCode == VK_RETURN) {
//				
//				_Log(inputBuffer);
//
//				ProcessCommand(inputBuffer);
//
//				inputBuffer = "";
//
//
//				commandInProgress = false;
//			} else {
//				inputBuffer += unicodeLiteral;
//				//Log::Debug(unicodeLiteral);
//			}
//		}
//	}
//
//	//if (TryPlayQuickSound(inputBuffer)) {
//	//	return;
//	//}
//
//	return commandInProgress ? HookResult::ConsumeEvent : HookResult::PassEvent;
//
//	//SetScrollLockState();
//}

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
	menuHandler.Add("Open observer...", [&] { this->OpenObserverDialog(); });

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


//
//void SoundplayerApp::AsyncOpenAndPlayFile_advanced() {
//
//	std::thread soundPlayerThread([&]() {
//		// Mivel ez nem egy generált mfc alkalmazás nincs minden
//		// alapból inicializálva, és assert-et dob a dialog konstruktor
//		// ha ez nincs itt
//		AFX_MANAGE_STATE(AfxGetStaticModuleState());
//		CFileDialog dialog(TRUE);
//
//		auto result = dialog.DoModal();
//		if(result == IDOK) {
//			//CString fileName = dialog.GetFileName();
//			CString fileName = dialog.GetPathName();
//			try {
//#if USE_WINDOWS_MEDIA_PACK_FEATURES
//
//				PlayFile_advanced(fileName);
//#endif // USE_WINDOWS_MEDIA_PACK_FEATURES
//
//			} catch(Exception e) {
//				Log::Error(e.errorMessage);
//			}
//			
//			//MessageBox(0, fileName, 0, 0);
//		}
//
//
//	});
//	soundPlayerThread.detach();
//
//}


void SoundplayerApp::AsyncOpenAndPlayFile() {

	std::thread soundPlayerThread([&]() {
		// Mivel ez nem egy generált mfc alkalmazás nincs minden
		// alapból inicializálva, és assert-et dob a dialog konstruktor
		// ha ez nincs itt
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CFileDialog dialog(TRUE);
		
		auto result = dialog.DoModal();
		if( result == IDOK ) {
			//CString fileName = dialog.GetFileName();
			CString fileName = dialog.GetPathName();
			PlayFile(fileName);
			//MessageBox(0, fileName, 0, 0);
		}
		

	});
	soundPlayerThread.detach();

}


#if NEW_SOUND_PROCESSING_VERSION

void SoundplayerApp::PlayFile(CString fileName) {

	if (fileName.Find(L".mp3") >= 0) {
		return;
	}

	if(!PathFileExists(fileName)) {
		return;
	}

#ifdef DEBUG 
	if(playerLock.try_lock()) {
		playerLock.unlock();
	} else {
		Log::Warning(L"SoundplayerApp::PlayFile(CString fileName) is locked.");
	}
#endif
	std::unique_lock<std::mutex> lock(playerLock);
	this->stop = false;

	//SendFileNameToChat(fileName);

	this->lastFile = fileName; // csak nem akad össze...

	std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(fileName);

	
	if(!track) {
		Log::Warning(L"LoadWaveFile failed");
		return;
	}

	//void AddSamples(short* samples, size_t sampleCount, int channels)
	// TODO EZ IGY GECIRONDA
	//const int sampleCountOf20ms = outputFrequency*outputChannels*frameLengthMillisecs / 1000;
	//const int sampleCountOf20ms = track->header.nSamplesPerSec * track->header.nChannels * 20 / 1000;

	//audioBuffer.AddSamples((short*)track->buffer.data(), track->numberOfSamples, track->header.nChannels);
	//audioBuffer.AddSamples((short*)track->buffer.data(), track->header);

	audioBufferForCapture.AddSamples(track);
	audioBufferForPlayback.AddSamples(track);

	

}

#else 
void SoundplayerApp::PlayFile(CString fileName) {

	if(!PathFileExists(fileName)) {
		return;
	}
	
#ifdef DEBUG 
	if(playerLock.try_lock()) {
		playerLock.unlock();
	} else {
		Log::Warning(L"SoundplayerApp::PlayFile(CString fileName) is locked.");
	}
#endif
	std::unique_lock<std::mutex> lock(playerLock);
	this->stop = false;

	SendFileNameToChat(fileName);

	this->lastFile = fileName; // csak nem akad össze...

	std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(fileName);
	

	if(!track) {
		Log::Warning(L"LoadWaveFile failed");
		return;
	}

	//wprintf(L"playing %s  \n", (const wchar_t*)fileName);
	Log::Debug(L"Playing: " + fileName);

	LocalSoundPlayer localPlayer(fileName);

	TSSoundPlayer player(track);

	// async call
	localPlayer.PlayAsync();

	// sync call
	player.Play(stop);


	Log::Debug(L"    finished playing sound ");

}

#endif


void SoundplayerApp::StopPlayback() {
	stop = true;
}


void SoundplayerApp::AsyncPlayFile(CString fileName) {
	std::thread t([this, fileName] {
		PlayFile(fileName);
	});
	t.detach();
}


void SoundplayerApp::AsyncEnqueueFile() {

	std::thread soundPlayerThread([&]() {
		// Mivel ez nem egy generált mfc alkalmazás nincs minden
		// alapból inicializálva, és assert-et dob a dialog konstruktor
		// ha ez nincs itt
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CFileDialog dialog(TRUE);

		auto result = dialog.DoModal();
		if(result == IDOK) {
			//CString fileName = dialog.GetFileName();
			CString fileName = dialog.GetPathName();
			playlist.push(fileName);
		}
	});
	soundPlayerThread.detach();
}

void SoundplayerApp::PlayQueued() {
	CString fileName;
	if(playlist.try_pop(fileName)) {
		AsyncPlayFile(fileName);
	}
}

void SoundplayerApp::Replay() {
	if(lastFile.GetLength() > 0) {
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

//
//void SoundplayerApp::PlayPreset(int ordinal) {
//	//assert(0 && "nincs megirva 2");
//	CStringA key;
//	key.Format(Hotkey::PLAY_PRESET_TEMPLATE, ordinal);
//	CString file;
//	if(Global::config.TryGet(CString(key), file)) {
//		AsyncPlayFile(file);
//	}
//
//}

void SoundplayerApp::PlayRandom() {
	CString folder;
	if(Global::config.TryGet(ConfigKey::SoundFolder, folder)) {
		

		if(folder.Right(1) != "\\" && folder.Right(1) != "/") {
			folder += "\\";
		}

		vector<CString> files = ListFilesInDirectory(folder);

		srand((unsigned int)time(0));
		int random = rand() % files.size();

		AsyncPlayFile(folder + files[random]);
	}
}

unique_ptr<CString> SoundplayerApp::TryGetSoundsDirectory() {
	
	while (true) {

		CString directory = Global::config.Get(ConfigKey::SoundFolder, L"");
		if (DirectoryExists(directory)) {
			return make_unique<CString>(directory);
		} else {
			SoundFolderSelector dialog;
			auto result = dialog.DoModal();
			if (result == IDOK) {
				continue;
			} else {
				return nullptr;
			}
		}
	}

	return nullptr;
}



unique_ptr<CString> SoundplayerApp::TryGetLikelyFileName(const CString& inputString) {

	vector<CString> possibleFiles = GetPossibleFiles(inputString);
	if (possibleFiles.size() == 1) {
		return as_unqiue(possibleFiles.front());
	}

	return nullptr;
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
	inputObserverDialog = make_unique<InputObserverDialog>();
	//inputObserverDialog.ShowWindow(TRUE);
}

//
////CString SoundplayerApp::GetLikelyFileName(CString str) {
//bool SoundplayerApp::GetLikelyFileName(_Out_ CString& result, CString str) {
//	CString directory = L"";
//	bool tryAgain = false;
//	do {
//		tryAgain = false;
//		directory = Global::config.Get(ConfigKey::SoundFolder, L"");
//		if(!DirectoryExists(directory)) {
//			SoundFolderSelector dialog;
//			auto result = dialog.DoModal();
//			if(result == IDOK) {
//				tryAgain = true;
//			} else {
//				//return L""; // TODO
//				return false;
//			}
//		}
//	} while(tryAgain);
//
//
//	if(directory.Right(1) != "\\" && directory.Right(1) != "/") {
//		directory += "\\";
//	}
//
//
//	vector<CString> files;
//	ListFilesInDirectory(_Out_ files, directory);
//
//	int hits = 0;
//
//	for(auto& file : files) {
//		if( EqualsIgnoreCaseAndWhitespace(file.Left(str.GetLength()), str) ) {
//		//if(file.Left(str.GetLength()).MakeLower() == str.MakeLower()) {
//			//return directory + file;
//			result = directory + file;
//			++hits;
//		}
//	}
//
//	if(hits > 1) {
//		Log::Warning(L"Multiple hits");
//	}
//
//	if(hits < 1) {
//		Log::Warning(L"Zero hits");
//	}
//
//	return (hits == 1);
//}

std::vector<CString> SoundplayerApp::GetPossibleFiles(const CString & inputString) {

	if (inputString.GetLength() == 0) {
		return std::vector<CString>();
	}
	
	unique_ptr<CString> directoryOrNull = TryGetSoundsDirectory();
	if (!directoryOrNull) {
		return std::vector<CString>();
	}

	CString directory = *directoryOrNull;


	if (directory.Right(1) != "\\" && directory.Right(1) != "/") {
		directory += "\\";
	}


	vector<CString> files = ListFilesInDirectory(directory);


	std::vector<CString> results;
	
	for (auto& file : files) {
		if (EqualsIgnoreCaseAndWhitespace(file.Left(inputString.GetLength()), inputString)) {
			//if(file.Left(str.GetLength()).MakeLower() == str.MakeLower()) {
				//return directory + file;
			CString result = directory + file;
			results.push_back(result);
		}
	}

	return results;
}


bool SoundplayerApp::TryEnqueueFileFromCommand(CString str) {
	CString queuePrefix = L"q ";
	
	if(StartsWith(str, queuePrefix)) {
		CString command = str.Right(str.GetLength() - queuePrefix.GetLength());
		CString fileName;
		if(auto fileName = TryGetLikelyFileName(command)) {
			playlist.push(*fileName);
		}

		return true;
	}
	return false;
}

void SoundplayerApp::OpenDeveloperConsole() {
	CreateConsole();
	QuickSoundsFileSystem fs;
}


//
//bool SoundplayerApp::TryPlayQuickSound(CString str) {
//
//	
//	//quicksoundhandler.
//	
//	if(str.GetLength() != 3) return false;
//	if(str[0] != L'V' && str[0] != L'v') return false;
//	if(!iswdigit(str[1]) ) return false;
//	if(!iswdigit(str[2])) return false;
//
//	
//	int n = _wtoi(static_cast<const wchar_t*>(str) + 1);
//
//	CString dir1 = ToString(n / 10);
//	CString dir2 = ToString(n % 10);
//	
//	//CString path = 
//	CPath directory = Global::config.Get(ConfigKey::SoundFolder, L"");
//	directory.Append(L"cod");
//	directory.Append(dir1);
//	directory.Append(dir2);
//	
//	if(DirectoryExists(directory)) {
//		CPath file = directory;
//		file += PickRandomFile(directory);
//		AsyncPlayFile(file);
//		return true;
//	}
//	return false;
//}


void SoundplayerApp::ProcessCommand(const CString& inputString) {


	if(TryEnqueueFileFromCommand(inputString)) {
		return;
	}

	
	if(auto fileName = TryGetLikelyFileName(inputString)) {
		AsyncPlayFile(*fileName);
	} else {
		PlayAlarmSound();
	}
	
	
}

void SoundplayerApp::UpdateObserverDialog() {
	if (inputObserverDialog) {
		vector<CString> possibleFiles = GetPossibleFiles(inputHandler.GetBuffer());
		inputObserverDialog->SetFiles(possibleFiles);
	}

}



void SoundplayerApp::SendFileNameToChat(CString path) {
	// TODO ide kéne idözito h pl max 10 secenként 1x küldj be, mert láárka kifagy töle...
	const UINT minDelay = 5 * 1000;
	static UINT lastMessageTime = 0;
	UINT time = GetTickCount();


	CString fileName = FileNameFromPath(path);
	CString message = L"Playing: " + fileName;
	if(time - lastMessageTime > minDelay) {
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

//
//// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-keybd_event
//static void SetScrollLock(BOOL bState) {
//	BYTE keyState[256];
//
//	GetKeyboardState((LPBYTE)&keyState);
//	if ((bState && !(keyState[VK_NUMLOCK] & 1)) ||
//		(!bState && (keyState[VK_NUMLOCK] & 1))) {
//		// Simulate a key press
//		keybd_event(VK_NUMLOCK,
//			0x45,
//			KEYEVENTF_EXTENDEDKEY | 0,
//			0);
//
//		// Simulate a key release
//		keybd_event(VK_NUMLOCK,
//			0x45,
//			KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
//			0);
//	}
//
//}


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


		if(!soundPlaying) {
			soundPlaying = true;
			defaultVadState = Ts::GetPreProcessorConfigValue(Ts::VoiceActivation);
			Ts::SetPreProcessorConfigValue(Ts::VoiceActivation, Ts::False);

			//ts3Functions.flushClientSelfUpdates(Global::connection, NULL);
			// ezt lehet hogy célszerubb nem baszogatni
			//Ts::GetClientSelfVariableAsInt(CLIENT_INPUT_DEACTIVATED);
		}



	}

	void ResetMicrophone() {
		if(soundPlaying) {
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


static const CStringA testSecret = "looofasz";




void SoundplayerApp::OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited) {

	//if(GetKeyState(VK_CONTROL) < 0) {
	//	ts3Functions.ts3client_setLocalTestMode(Global::connection, 1);
	//} else {
	//	ts3Functions.ts3client_setLocalTestMode(Global::connection, 0);
	//	
	//}
	


	//unsigned int ts3client_setLocalTestMode(serverConnectionHandlerID, status);
#if NEW_SOUND_PROCESSING_VERSION
	assert(channels == 1);

	//static std::vector<byte> buffer;
	//if(onlineMicrophone.TryGetSamples(buffer)) {
	//	memcpy(samples, buffer.data(), buffer.size()*sizeof(short));
	//}


	//if(onlineMicrophone.TryGetSamples(buffer)) {
	//	memcpy(samples, buffer.data(), buffer.size() * sizeof(short));
	//}
	
	//pcmFormat
	//audioPlayer.SetPcmFormat(pcmFormat);
	//audioPlayer.AddSamples(samples, sampleCount);


	//CachedAudioSample48k playbackSamples;
	//bool success = audioBufferForCapture.TryGetSamples20ms(playbackSamples);
	CachedAudioSample48k playbackSamples = audioBufferForCapture.TryGetSamples(sampleCount, channels);
	if(playbackSamples) {
		
		// hát ezt lehet hogy nem ide kéne rakni :D dehát lófasz
		tsVoiceHandler.ForceEnableMicrophone();

		if(!(*edited &= 2)) {
			// ha nincs küldendö adat
			memset(samples, 0, sizeof(short)*sampleCount*channels);
		}

		assert(sampleCount*channels == playbackSamples->size());
		if(sampleCount*channels != playbackSamples->size()) {
			Log::Warning(L"if(sampleCount != playbackSamples->size()) {");
		}
		SgnProc::Mix(samples, playbackSamples->data(), sampleCount);
		//*edited |= 2;
		*edited |= 1;
	} else { 
		tsVoiceHandler.ResetMicrophone();
		*edited &= ~1;
	}
#endif
	//If the sound data will be send, (*edited | 2) is true.
	//If the sound data is changed, set bit 1 (*edited |= 1).
	//If the sound should not be send, clear bit 2. (*edited &= ~2)

	//if(*edited & 0x2) {
	//	bool enabled = audioProcessor.Process(samples, sampleCount, channels);

	//	if(enabled) {
	//		*edited |= 1;
	//	} else {
	//		*edited &= ~1;
	//	}
	//}
	

	if(steganographyEnabled) {
		Steganography::WriteSecret(samples, sampleCount*channels, testSecret);
		ONCE(Log::Debug(CString("Channels - write") + ToString(channels)));
	}
	
}



void SoundplayerApp::OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
#if NEW_SOUND_PROCESSING_VERSION
	
	//audioBufferForPlayback.outputChannels = channels;
	
	
	//CachedAudioSample48k playbackSamples;
	
	//bool success = audioBufferForPlayback.TryGetSamples20ms(playbackSamples);
	CachedAudioSample48k playbackSamples = audioBufferForPlayback.TryGetSamples(sampleCount, channels);
	
	//assert(sampleCount == playbackSamples->size() && "Ha ezt látod akkor ne ijedj meg..., hosszu történet....majd egyszer kijavitom");

	


	
	if(playbackSamples) {

		if(!(*channelFillMask & 0x3)) {
			*channelFillMask |= 3;
			memset(samples, 0, sampleCount*channels*sizeof(short));
		}
		// ez is gecikulturált lett.... TODO
		

		if(sampleCount*channels == playbackSamples->size()) {
			SgnProc::Mix(samples, playbackSamples->data(), sampleCount*channels);
		} else {
			static int asdf = 0;
			if(!asdf) {
				asdf = 1;
				Log::Error(L".......... anyád");
			}
		}
		
	}

	
#endif
}

void SoundplayerApp::OnEditPlaybackVoiceDataEvent(anyID clientID, short* samples, int sampleCount, int channels) {
	ONCE(Log::Debug(CString("Channels - read") + ToString(channels)));
	if(steganographyEnabled) {
		CStringA receivedSecret = Steganography::ReadSecret(samples, sampleCount*channels);
		bool success = receivedSecret == testSecret;

		if(success ) {
			ONCE(MessageBoxA(0, "megvan+", 0, 0));
		}
		//Steganography::WriteSecret(samples, sampleCount*channels, testSecret);
	}
}

#ifdef _DEBUG
#define MARCI_VERZIO TRUE
#else
#define MARCI_VERZIO FALSE
#endif

void SoundplayerApp::OnClientMoved(anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, CString moveMessage) {

	// ez csak az én gépemen legyen
	if(!MARCI_VERZIO) return;

	//if (clientID == Global::connection) {
	anyID myID;
	// ezt a globalozást kurva gyorsan meg kéne szüntetni
	ts3Functions.getClientID(Global::connection, &myID);

	if(clientID == myID) {
		cout << "Moved to channel id " << newChannelID << endl;
		cout << moveMessage << endl;

	}

	char name[256];
	ts3Functions.getClientDisplayName(Global::connection, clientID, name, sizeof(name));

	uint64 clientChannelId;
	ts3Functions.getChannelOfClient(Global::connection, clientID, &clientChannelId);

	uint64 ownChannelId;
	ts3Functions.getChannelOfClient(Global::connection, myID, &ownChannelId);

	if(ownChannelId == clientChannelId) {
		Log::Debug(CString(name));
	} else {
		Log::Debug(CString("masik channel: ") + CString(name));
	}

	bool nameMatchHodi = (strcmp(name, "Hodi") == 0);
	bool nameMatchTomi = (strcmp(name, "Ugyis") == 0);
	if(ownChannelId == clientChannelId) {
		std::thread playAndSleepThread = std::thread([=] {
			
			if(nameMatchHodi) {
				Sleep(500);
				if(auto fileName = TryGetLikelyFileName(L"szar")) {
					AsyncPlayFile(*fileName);
				}
			}

			if(nameMatchTomi) {
				Sleep(500);
				if(auto fileName = TryGetLikelyFileName(L"itt a ku")) {
					AsyncPlayFile(*fileName);
				}
			}
		});
		playAndSleepThread.detach();

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

	//bool commandWasInProgressBeforeProcessing = commandInProgress;

	return OnKeyData(keyData);

	//if (OnKeyData(keyData) == HookResult::ConsumeEvent) {
	//	return HookResult::ConsumeEvent;
	//}
	//
	//
	////bool shouldConsumeEvent = commandWasInProgressBeforeProcessing || commandInProgress;
	//
	//return shouldConsumeEvent ? HookResult::ConsumeEvent : HookResult::PassEvent;
}

void SoundplayerApp::OnMessage(const CString& message) {
	std::wcout << message << std::endl;
}

void SoundplayerApp::OnCommand(const CString& command) {
	ProcessCommand(command);
}

void SoundplayerApp::OnQuickSoundMatch(const CString& path) {
	AsyncPlayFile(path);
}









