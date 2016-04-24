#include "stdafx.h"

#include "SoundplayerApp.h"
#include "afxdlgs.h"
#include "Wave\wave.h"
#include "Wave\AudioDecoder.h"
#include "Gui\SettingsDialog.h"
#include "Gui\SoundFolderSelector.h"

#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"

using namespace std;
using namespace Global;


SoundplayerApp::SoundplayerApp(/*TS3Functions& ts3Functions*/)/* : ts3Functions(ts3Functions)*/ {

}


SoundplayerApp::~SoundplayerApp() {
}
//
//void SoundplayerApp::SetConnectionHandle(uint64 connection) {
//	this->connection = connection;
//}
//
//uint64 SoundplayerApp::GetConnectionHandle() {
//	return this->connection;
//}

void SoundplayerApp::InitKeyboardHook() {
#if USE_KEYBOARDHOOK



	if(!hookInstaller.AttachDll()) {
		MessageBoxA(0, "KeyboardHook dll load failed", 0, 0);
		return;
	}


	pipeHandler.SetOnNewEntryListener([&](PipeHandler& pipeHandler) {
		KeyboardHook::KeyData keyData;

		while(pipeHandler.TryPop(keyData)) {
			OnKeyData(keyData);
		}
	});

	if(!pipeHandler.ListenPipe()) {
		MessageBoxA(0, "ListenPipeload failed", 0, 0);
		return;
	}

#endif

}

void SoundplayerApp::Init() {

	
	InitKeyboardHook();
	
	//ts3Functions.showHotkeySetup();

	Global::config.LoadFromFile(Global::config.defaultFileName);
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
	//if(keyword == Hotkey::PLAY_QUEUED) {
	//	PlayQueued();
	//}

	//if(keyword == Hotkey::STOP) {
	//	StopPlayback();
	//}

	//if(keyword == Hotkey::REPLAY) {
	//	Replay();
	//}
}



void SoundplayerApp::OnKeyData(const KeyboardHook::KeyData& keyData) {
	if(keyData.hookData.vkCode == VK_ESCAPE) {
		commandInProgress = false;
		inputBuffer = "";
		stop = true;
	}

	if(keyData.unicodeLiteral == CString("/")) {
		commandInProgress = true;
	} else {
		if(commandInProgress) {
			if(keyData.hookData.vkCode == VK_RETURN) {
				//CString _inputbuffer = this->inputBuffer;
				std::wcout << std::endl << ">>" << (const wchar_t*)inputBuffer << "<<" << std::endl;

				this->ProcessRegexCommand(inputBuffer);

				inputBuffer = "";

				if(wcout.fail()) {
					wcout.clear();
				}
				//std::thread messageThread([_inputbuffer] {
				//	MessageBox(0, _inputbuffer, 0, 0);
				//});
				commandInProgress = false;
			} else {
				inputBuffer += keyData.unicodeLiteral;
			}
		}
		
		
	}
}



void SoundplayerApp::AsyncOpenAndPlayFile_advanced() {

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
			try {
#if USE_WINDOWS_MEDIA_PACK_FEATURES

				PlayFile_advanced(fileName);
#endif // USE_WINDOWS_MEDIA_PACK_FEATURES

			} catch(Exception e) {
				Log::Error(e.errorMessage);
			}
			
			//MessageBox(0, fileName, 0, 0);
		}


	});
	soundPlayerThread.detach();

}


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

// azé van ennek külön osztály, hogy ha valami hiba van
// akkor a destruktor meg tudjon hívódni, és minden felszabaduljon
class LocalSoundPlayer {
	uint64 playbackHandle;
	CString fileName;
public:
	LocalSoundPlayer(CString fileName, bool startPlaying = true) {
		this->fileName = fileName;
		if(startPlaying) {
			Play();
		}
	}
	~LocalSoundPlayer() {
		Pause();
		ts3Functions.closeWaveFileHandle(connection, playbackHandle);
	}

	void Play() {
		Pause();
		ts3Functions.playWaveFileHandle(connection, ConvertUnicodeToUTF8(fileName), FALSE, &playbackHandle);
	}

	void Pause() {
		ts3Functions.pauseWaveFileHandle(connection, playbackHandle, TRUE);
	}
};
//
//class RemoteSoundPlayer {
//	uint64 playbackHandle;
//	CString fileName;
//public:
//	RemoteSoundPlayer(CString fileName, bool startPlaying = true) {
//		this->fileName = fileName;
//		if(startPlaying) {
//			Play();
//		}
//	}
//
//	~RemoteSoundPlayer() {
//		Pause();
//		ts3Functions.closeWaveFileHandle(connection, playbackHandle);
//	}
//
//	void Play() {
//		Pause();
//		ts3Functions.playWaveFileHandle(connection, ConvertUnicodeToUTF8(fileName), FALSE, &playbackHandle);
//	}
//
//	void Pause() {
//		ts3Functions.pauseWaveFileHandle(connection, playbackHandle, TRUE);
//	}
//};

void SoundplayerApp::PlayFile(CString fileName) {

	std::unique_lock<std::mutex> lock(playerLock);
	this->stop = false;

	lastFile = fileName; // csak nem akad össze...


	//uint64 scHandlerID;

	// TODO epic memóriaszivárgás!

	//int    captureFrequency;
	//int    captureChannels;
	//short* captureBuffer;
	//size_t buffer_size;
	//int    captureBufferSamples;
	////int    audioPeriodCounter;
	//int    capturePeriodSize;

	std::unique_ptr<WaveTrack> track = readWave(fileName);
	
	if( !track ) {
		//if(!readWave(fileName, &captureFrequency, &captureChannels, &captureBuffer, &buffer_size, &captureBufferSamples)) {
		cout << "readWave failed";
		return;
	}


	unsigned int error;
	if((error = ts3Functions.registerCustomDevice(myDeviceId, "Nice displayable wave device name", track->frequency, track->channels, PLAYBACK_FREQUENCY, PLAYBACK_CHANNELS)) != ERROR_ok) {
	//if((error = ts3Functions.registerCustomDevice(myDeviceId, "Nice displayable wave device name", track->frequency, track->channels, track->frequency, track->channels)) != ERROR_ok) {
		char* errormsg;
		if(ts3Functions.getErrorMessage(error, &errormsg) == ERROR_ok) {
			printf("Error registering custom sound device: %s\n", errormsg);
			ts3Functions.freeMemory(errormsg);
			MessageBoxA(0, "Error registering custom sound device", 0, 0);
			goto cleanup;
		}
	}

	char* result_before;
	ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &result_before);

	wprintf(L"\nplaying %s  \n", (const wchar_t*)fileName);


	char* previousDeviceName;
	char* previousMode;
	BOOL isDefault;
	ts3Functions.getCurrentCaptureDeviceName(connection, &previousDeviceName, &isDefault);
	ts3Functions.getCurrentCaptureMode(connection, &previousMode);


	ts3Functions.closeCaptureDevice(connection);
	if((error = ts3Functions.openCaptureDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
		printf("Error opening capture device: 0x%x\n", error);
		goto cleanup;
	} else {
		cout << "\tdevice id: " << myDeviceId << endl;
	}

	
	//ts3Functions.playWaveFile(connection, ConvertUnicodeToUTF8(fileName));
	

	//ts3Functions.closePlaybackDevice(connection);
	//if((error = ts3Functions.openPlaybackDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
	//	printf("Error opening capture device: 0x%x\n", error);
	//} else {
	//	cout << "\tPlayback device id: " << myDeviceId << endl;
	//}


	

	//if((error = ts3Functions.openPlaybackDevice(connection, "", "")) != ERROR_ok) {
	//	printf("Error opening capture device: 0x%x\n", error);
	//} else {
	//	cout << "\tPlayback device id: " << myDeviceId << endl;
	//}


	cout << "buffer size: " << track->buffer.size() << endl;
	{
		LocalSoundPlayer localSoundPlayer(fileName);


		int capturePeriodSize = (track->frequency * 20) / 1000;

		int captureAudioOffset = 0;
		//for(audioPeriodCounter = 0; audioPeriodCounter < 50 * AUDIO_PROCESS_SECONDS; ++audioPeriodCounter) { /*50*20=1000*/
		while(true) {

			// make sure we dont stream past the end of our wave sample 
			if(captureAudioOffset + capturePeriodSize > track->numberOfSamples || this->stop) {
				captureAudioOffset = 0;
				break;
			}

			//SLEEP(20);
			Sleep(20);
			//Sleep(10);

			/* stream capture data to the client lib */
			if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->channels, capturePeriodSize)) != ERROR_ok) {
				printf("Failed to get stream capture data: %d\n", error);
				return;
			}

			//if((error = ts3Functions.acquireCustomPlaybackData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->channels, capturePeriodSize)) != ERROR_ok) {
			//	printf("acquireCustomPlaybackData failed: %d\n", error);
			//	//return;
			//}


			/*update buffer offsets */
			captureAudioOffset += capturePeriodSize;
		}


		//if((error = ts3Functions.initiateGracefulPlaybackShutdown(connection) != ERROR_ok)) {
		//	printf("Error initiateGracefulPlaybackShutdown: 0x%x\n", error);
		//	//return 1;
		//}

		Sleep(20);
	}

	cleanup:

	if((error = ts3Functions.closeCaptureDevice(connection) != ERROR_ok)) {
		printf("Error closeCaptureDevice: 0x%x\n", error);
		//return 1;
	}



	if((error = ts3Functions.unregisterCustomDevice(myDeviceId) != ERROR_ok)) {
		printf("Error unregisterCustomDevice: 0x%x\n", error);
		//return 1;
	}




	if((error = ts3Functions.openCaptureDevice(connection, previousMode, previousDeviceName)) != ERROR_ok) {
		printf("Error (re)opening capture device: 0x%x\n", error);
		return;
	} else {
		cout << "\tnew device id: " << previousDeviceName << endl;
	}

	//if((error = ts3Functions.activateCaptureDevice(connection) ) != ERROR_ok) {
	//	printf("Error activating capture device: 0x%x\n", error);
	//	return error;
	//} 

	char* result_after;
	ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &result_after);

	//cout << "results:" << std::endl;
	//cout << result_before << std::endl;
	//cout << result_after << std::endl;
	//cout << "" << std::endl;

	// ts resets it to 0 db unless we do this :'(
	ts3Functions.setPreProcessorConfigValue(connection, "voiceactivation_level", result_before);

	ts3Functions.freeMemory(result_before);
	ts3Functions.freeMemory(result_after);

	/*
	If the capture device for a given server connection handler has been deactivated by the Client
	Lib, the flag CLIENT_INPUT_HARDWARE will be set. This can be queried with the function
	ts3client_getClientSelfVariableAsInt.
	*/


	printf("\n    finished playing sound \n");

}

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

#if USE_WINDOWS_MEDIA_PACK_FEATURES


void SoundplayerApp::PlayFile_advanced(CString fileName) {
	std::unique_lock<std::mutex> lock(playerLock);
	this->stop = false;

	AudioDecoder decoder(fileName);

	WAVEFORMATEX header = decoder.GetHeader();

	printf("channels: %d", header.nChannels);

	unsigned int error;
	if((error = ts3Functions.registerCustomDevice(myDeviceId, "Nice displayable wave device name", header.nSamplesPerSec, header.nChannels, PLAYBACK_FREQUENCY, PLAYBACK_CHANNELS)) != ERROR_ok) {
	//if((error = ts3Functions.registerCustomDevice(myDeviceId, "Nice displayable wave device name", track->frequency, track->channels, PLAYBACK_FREQUENCY, PLAYBACK_CHANNELS)) != ERROR_ok) {
	//if((error = ts3Functions.registerCustomDevice(myDeviceId, "Nice displayable wave device name", track->frequency, track->channels, track->frequency, track->channels)) != ERROR_ok) {
		char* errormsg;
		if(ts3Functions.getErrorMessage(error, &errormsg) == ERROR_ok) {
			printf("Error registering custom sound device: %s\n", errormsg);
			ts3Functions.freeMemory(errormsg);
			MessageBoxA(0, "Error registering custom sound device", 0, 0);
			return;
		}
	}

	char* result_before;
	ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &result_before);

	wprintf(L"\n(advanced)playing %s  \n", (const wchar_t*)fileName);


	char* previousDeviceName;
	char* previousMode;
	BOOL isDefault;
	ts3Functions.getCurrentCaptureDeviceName(connection, &previousDeviceName, &isDefault);
	ts3Functions.getCurrentCaptureMode(connection, &previousMode);


	ts3Functions.closeCaptureDevice(connection);
	if((error = ts3Functions.openCaptureDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
		printf("Error opening capture device: 0x%x\n", error);
		return;
	} else {
		cout << "\tdevice id: " << myDeviceId << endl;
	}



	LocalSoundPlayer localSoundPlayer(fileName, false);


	size_t capturePeriodSize = (header.nSamplesPerSec * 20) / 1000;

	
	size_t debug_size = 0;
	//size_t captureAudioOffset = 0;
	//for(audioPeriodCounter = 0; audioPeriodCounter < 50 * AUDIO_PROCESS_SECONDS; ++audioPeriodCounter) { /*50*20=1000*/
	while(true) {
		AudioDecoder::Buffer buffer = decoder.DecodeMillisecs(20);


		debug_size += buffer->size();
		// make sure we dont stream past the end of our wave sample 
		if( buffer->size() == 0 || this->stop) {
		//if(captureAudioOffset + capturePeriodSize > buffer->size() || this->stop) {
			//captureAudioOffset = 0;
			break;
		}

		//SLEEP(20);
		//Sleep(20);
		

		/* stream capture data to the client lib */
		//if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->channels, capturePeriodSize)) != ERROR_ok) {
		//if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)buffer->data()) + captureAudioOffset*header.nChannels, (int)capturePeriodSize)) != ERROR_ok) {
		//if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)buffer->data()), (int)capturePeriodSize*header.nChannels)) != ERROR_ok) {
		if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)buffer->data()), (int)capturePeriodSize*header.nChannels)) != ERROR_ok) {
			printf("Failed to get stream capture data: %d\n", error);
			return;
		}

		Sleep(20);

		//if((error = ts3Functions.acquireCustomPlaybackData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->channels, capturePeriodSize)) != ERROR_ok) {
		//	printf("acquireCustomPlaybackData failed: %d\n", error);
		//	//return;
		//}


		/*update buffer offsets */
		//captureAudioOffset += capturePeriodSize;
	}

	Log::Write(CString("size: ") + ToString((int)debug_size), Log::Level_Warning);

	Sleep(20);

	if((error = ts3Functions.closeCaptureDevice(connection) != ERROR_ok)) {
		printf("Error closeCaptureDevice: 0x%x\n", error);
		//return 1;
	}



	if((error = ts3Functions.unregisterCustomDevice(myDeviceId) != ERROR_ok)) {
		printf("Error unregisterCustomDevice: 0x%x\n", error);
		//return 1;
	}




	if((error = ts3Functions.openCaptureDevice(connection, previousMode, previousDeviceName)) != ERROR_ok) {
		printf("Error opening capture device: 0x%x\n", error);
		return;
	} else {
		cout << "\tnew device id: " << previousDeviceName << endl;
	}

	char* result_after;
	ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &result_after);

	// ts resets it to 0 db unless we do this :'(
	ts3Functions.setPreProcessorConfigValue(connection, "voiceactivation_level", result_before);

	ts3Functions.freeMemory(result_before);
	ts3Functions.freeMemory(result_after);


	printf("\n    finished playing sound \n");

}

#endif // USE_WINDOWS_MEDIA_PACK_FEATURES


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


CString SoundplayerApp::GetLikelyFileName(CString str) {
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
				return L""; // TODO
			}
		}
	} while(tryAgain);

	vector<CString> files;
	ListFilesInDirectory(_Out_ files, directory);

	for(auto& file : files) {
		if(file.Left(str.GetLength()).MakeLower() == str.MakeLower()) {
			return directory + file;
			//AsyncPlayFile(directory + file);
		}
	}
	return L"";
}


// TODO ez aztán név bazmeg...
void SoundplayerApp::ProcessRegexCommand(CString str) {
	CString fileName = GetLikelyFileName(str);
	if(fileName.GetLength() > 0) {
		AsyncPlayFile(fileName);
	}
	
	
}




