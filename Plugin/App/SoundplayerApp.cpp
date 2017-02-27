#include "stdafx.h"

#include "SoundplayerApp.h"
#include "afxdlgs.h"
#include "Wave\wave.h"
#include "Wave\AudioDecoder.h"

#include "Gui\SettingsDialog.h"
#include "Gui\SoundFolderSelector.h"
#include <gui/AudioProcessorDialog.h>

#include "Util\TSSoundPlayer.h"

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

	pipeHandler.SetOnNewEntryListener([&](PipeHandler& pipeHandler) {
		KeyboardHook::KeyData keyData;

		while(pipeHandler.TryPop(keyData)) {
			OnKeyData(keyData);
		}
	});

	if(!pipeHandler.ListenPipe()) {
		//MessageBoxA(0, "ListenPipeload failed", 0, 0);
		Log::Error(L"ListenPipeload failed");
		return;
	}

}

void SoundplayerApp::Init() {
	CPath path = CString(Global::configPath);
	path.Append(Global::config.defaultFileName);
#if USE_KEYBOARD_HOOK
	InitKeyboardHook();
#endif

	//PathAppend()
	//PathCchAppend()
	//CString path = CString(Global::pluginPath) + L"\\" + Global::config.defaultFileName;
	
	Global::config.LoadFromFile(path);
}



/*
* Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
* Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
* This function is automatically called by the client after ts3plugin_init.
*/
void SoundplayerApp::InitHotkeys(struct PluginHotkey*** hotkeys) {
	/* Register hotkeys giving a keyword and a description.
	* The keyword will be later passed to ts3plugin_onHotkeyEvent to identify which hotkey was triggered.
	* The description is shown in the clients hotkey dialog. */

	hotkeyHandler.Add(Hotkey::STOP, "Stop playback", [this]{StopPlayback();});
	hotkeyHandler.Add(Hotkey::PLAY_QUEUED, "Play queued", [this] {PlayQueued(); });
	hotkeyHandler.Add(Hotkey::REPLAY, "Replay", [this] {Replay(); });
	hotkeyHandler.Add(Hotkey::PLAY_RANDOM, "Play random", [this] {PlayRandom(); });

	for(int i = 0; i < soundHotkeyCount; ++i) {
		CStringA hotkey;
		hotkey.Format(Hotkey::PLAY_PRESET_TEMPLATE, i);

		CStringA title;
		title.Format("Play sound #%d", i);

		hotkeyHandler.Add(hotkey, title, [this, i] {PlayPreset(i); });
		//CREATE_HOTKEY(Hotkey::REPLAY, "Replay");
	}


	//Config::Get(ConfigKey::SoundHotkeyCount, soundHotkeyCount)

	hotkeyHandler.Configure(hotkeys);

	/* The client will call ts3plugin_freeMemory to release all allocated memory */
}

void SoundplayerApp::OnHotkey(CStringA keyword) {
	hotkeyHandler.OnHotkeyEvent(keyword);
}



void SoundplayerApp::OnKeyData(const KeyboardHook::KeyData& keyData) {
	if(keyData.hookData.vkCode == VK_ESCAPE) {
		commandInProgress = false;
		inputBuffer = "";
		stop = true;

		// TODO

		audioBufferForCapture.Clear();
		audioBufferForPlayback.Clear();

		return;
	}

	if(keyData.hookData.vkCode == VK_BACK) {
		if(inputBuffer.GetLength() > 0) {
			inputBuffer.Truncate(inputBuffer.GetLength() - 1);
		}
		return;
	}

	
	CString unicodeLiteral = keyData.unicodeLiteral;
	if(unicodeLiteral == CString("/")) {
		commandInProgress = true;
		inputBuffer = "";
	} else {
		if(commandInProgress) {
			if(keyData.hookData.vkCode == VK_RETURN) {
				
				std::wcout << std::endl << ">>" << (const wchar_t*)inputBuffer << "<<" << std::endl;

				this->ProcessCommand(inputBuffer);

				inputBuffer = "";

				if(wcout.fail()) {
					wcout.clear();
				}

				commandInProgress = false;
			} else {
				inputBuffer += unicodeLiteral;
				//Log::Debug(unicodeLiteral);
			}
		}
		
		
	}
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


#if OLD_VERSION

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


void SoundplayerApp::PlayPreset(int ordinal) {
	//assert(0 && "nincs megirva 2");
	CStringA key;
	key.Format(Hotkey::PLAY_PRESET_TEMPLATE, ordinal);
	CString file;
	if(Global::config.TryGet(CString(key), file)) {
		AsyncPlayFile(file);
	}

}

void SoundplayerApp::PlayRandom() {
	CString folder;
	if(Global::config.TryGet(ConfigKey::SoundFolder, folder)) {
		vector<CString> files;

		if(folder.Right(1) != "\\" && folder.Right(1) != "/") {
			folder += "\\";
		}

		ListFilesInDirectory(_Out_ files, folder);

		srand((unsigned int)time(0));
		int random = rand() % files.size();

		AsyncPlayFile(folder + files[random]);
	}
}


//CString SoundplayerApp::GetLikelyFileName(CString str) {
bool SoundplayerApp::GetLikelyFileName(_Out_ CString& result, CString str) {
	CString directory = L"";
	bool tryAgain = false;
	do {
		tryAgain = false;
		directory = Global::config.Get(ConfigKey::SoundFolder, L"");
		if(!DirectoryExists(directory)) {
			SoundFolderSelector dialog;
			auto result = dialog.DoModal();
			if(result == IDOK) {
				tryAgain = true;
			} else {
				//return L""; // TODO
				return false;
			}
		}
	} while(tryAgain);


	if(directory.Right(1) != "\\" && directory.Right(1) != "/") {
		directory += "\\";
	}


	vector<CString> files;
	ListFilesInDirectory(_Out_ files, directory);

	int hits = 0;

	for(auto& file : files) {
		if( EqualsIgnoreCaseAndWhitespace(file.Left(str.GetLength()), str) ) {
		//if(file.Left(str.GetLength()).MakeLower() == str.MakeLower()) {
			//return directory + file;
			result = directory + file;
			++hits;
		}
	}

	if(hits > 1) {
		Log::Warning(L"Multiple hits");
	}

	if(hits < 1) {
		Log::Warning(L"Zero hits");
	}

	return (hits == 1);
}


bool SoundplayerApp::TryEnqueueFileFromCommand(CString str) {
	CString queuePrefix = L"q ";
	
	if(StartsWith(str, queuePrefix)) {
		CString command = str.Right(str.GetLength() - queuePrefix.GetLength());
		CString fileName;
		if(GetLikelyFileName(fileName, command)) {
			playlist.push(fileName);
			return true;
		}
	}
	return false;
}




void SoundplayerApp::ProcessCommand(CString str) {
	if(TryEnqueueFileFromCommand(str)) {
		return;
	}

	CString fileName;
	if(GetLikelyFileName(fileName, str)) {
		AsyncPlayFile(fileName);
	} else {
		PlaySound(
			(LPCWSTR)SND_ALIAS_SYSTEMEXCLAMATION,
			GetModuleHandle(0),
			SND_ALIAS_ID
		);
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
	uint64 channelId;
	ts3Functions.getChannelOfClient(Global::connection, Global::myID, &channelId);
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

#if OLD_VERSION
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

void SoundplayerApp::OnEditCapturedVoiceDataEvent(short* samples, int sampleCount, int channels, int* edited) {

	//if(GetKeyState(VK_CONTROL) < 0) {
	//	ts3Functions.ts3client_setLocalTestMode(Global::connection, 1);
	//} else {
	//	ts3Functions.ts3client_setLocalTestMode(Global::connection, 0);
	//	
	//}
	


	//unsigned int ts3client_setLocalTestMode(serverConnectionHandlerID, status);
#if OLD_VERSION
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
	CachedAudioSample48k playbackSamples = audioBufferForCapture.TryGetSamples20ms(channels);
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
	
	
}


void SoundplayerApp::OnEditMixedPlaybackVoiceDataEvent(short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
#if OLD_VERSION
	
	//audioBufferForPlayback.outputChannels = channels;
	
	
	//CachedAudioSample48k playbackSamples;
	
	//bool success = audioBufferForPlayback.TryGetSamples20ms(playbackSamples);
	CachedAudioSample48k playbackSamples = audioBufferForPlayback.TryGetSamples20ms(channels);
	
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



// stolen from someone's soundboard solution on github

//// TODO ezt kéne megcsinálni
//bool TalkStateManager::setTalkState(uint64 scHandlerID, talk_state_e state) {
//	logDebug("TSMGR: Setting talk state of %ull to %s, previous was %s",
//		(unsigned long long)scHandlerID, toString(state), toString(previousTalkState));
//
//	if(scHandlerID == 0 || state == TS_INVALID)
//		return false;
//
//	bool va = state == TS_PTT_WITH_VA || state == TS_VOICE_ACTIVATION;
//	bool in = state == TS_CONT_TRANS || state == TS_VOICE_ACTIVATION;
//
//	if(checkError(ts3Functions.setPreProcessorConfigValue(
//		scHandlerID, "vad", va ? "true" : "false"), "Error toggling vad"))
//		return false;
//
//	if(checkError(ts3Functions.setClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED,
//		in ? INPUT_ACTIVE : INPUT_DEACTIVATED), "Error toggling input"))
//		return false;
//
//	ts3Functions.flushClientSelfUpdates(scHandlerID, NULL);
//	currentTalkState = state;
//	return true;
//}
//
