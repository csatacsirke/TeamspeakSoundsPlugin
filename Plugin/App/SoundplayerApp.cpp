#include "stdafx.h"
#include "SoundplayerApp.h"
#include "afxdlgs.h"
#include "Wave\wave.h"

#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"

using namespace std;

//#define USE_KEYBOARDHOOK TRUE
#define USE_KEYBOARDHOOK FALSE


SoundplayerApp::SoundplayerApp(TS3Functions& ts3Functions) : ts3Functions(ts3Functions) {

}


SoundplayerApp::~SoundplayerApp() {
}

void SoundplayerApp::SetConnectionHandle(uint64 connection) {
	this->connection = connection;
}

uint64 SoundplayerApp::GetConnectionHandle() {
	return this->connection;
}

void SoundplayerApp::InitKeyboardHook() {
#if USE_KEYBOARDHOOK



	if(!hookInstaller.AttachDll()) {
		MessageBoxA(0, "KeyboardHook dll load failed", 0, 0);
		return;
	}


	pipeHandler.SetOnNewEntryListener([&](PipeHandler& pipeHandler) {
		PipeHandler::KeyData keyData;

		while(pipeHandler.TryPop(keyData)) {
			std::wcout << keyData.unicodeLiteral.GetString();
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

	// A JÓ KURVA ANYÁD!
	// enélkül ha konzolra írsz egy ő betűt eltörik az egész konzol....
	std::locale::global(std::locale(""));

	InitKeyboardHook();


}

void SoundplayerApp::OnKeyData(const PipeHandler::KeyData& keyData) {
	if(keyData.unicodeLiteral == L"/") {
		commandInProgress = true;
	} else {
		if(commandInProgress) {
			if(keyData.hookData.vkCode == VK_RETURN) {
				//CString _inputbuffer = this->inputBuffer;
				std::wcout << std::endl << ">>" << inputBuffer << "<<" << std::endl;
				inputBuffer == "";
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

void SoundplayerApp::PlayFile(CString fileName) {
	std::unique_lock<std::mutex> lock(playerLock);
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
		char* errormsg;
		if(ts3Functions.getErrorMessage(error, &errormsg) == ERROR_ok) {
			printf("Error registering custom sound device: %s\n", errormsg);
			ts3Functions.freeMemory(errormsg);
			MessageBoxA(0, "Error registering custom sound device", 0, 0);
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

//	int error;
	if((error = ts3Functions.openCaptureDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
		printf("Error opening capture device: 0x%x\n", error);
		return;
	} else {
		cout << "\tdevice id: " << myDeviceId << endl;
	}



	cout << "buffer size: " << track->buffer.size() << endl;


	//ts3Functions.activateCaptureDevice(connection);


	int capturePeriodSize = (track->frequency * 20) / 1000;

	int captureAudioOffset = 0;
	//for(audioPeriodCounter = 0; audioPeriodCounter < 50 * AUDIO_PROCESS_SECONDS; ++audioPeriodCounter) { /*50*20=1000*/
	while( true ) {

		// make sure we dont stream past the end of our wave sample 
		if(captureAudioOffset + capturePeriodSize > track->numberOfSamples ) {
			captureAudioOffset = 0;
			break;
		}

		//SLEEP(20);
		Sleep(20);

		/* stream capture data to the client lib */
		if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->channels, capturePeriodSize)) != ERROR_ok) {
			printf("Failed to get stream capture data: %d\n", error);
			return;
		}


		/*update buffer offsets */
		captureAudioOffset += capturePeriodSize;
	}


	//if((error = ts3Functions.initiateGracefulPlaybackShutdown(connection) != ERROR_ok)) {
	//	printf("Error initiateGracefulPlaybackShutdown: 0x%x\n", error);
	//	//return 1;
	//}
	
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