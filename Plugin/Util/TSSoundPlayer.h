#pragma once

#if 0
#include <memory>

using namespace Global;
using namespace std;

// azé van ennek külön osztály, hogy ha valami hiba van
// akkor a destruktor meg tudjon hívódni, és minden felszabaduljon
class LocalSoundPlayer {
	uint64 playbackHandle;
	CString fileName;
public: 
	LocalSoundPlayer(CString fileName, bool startPlaying = false) {
		this->fileName = fileName;
		if(startPlaying) {
			PlayAsync();
		}
	}
	~LocalSoundPlayer() {
		PauseAsync();
		ts3Functions.closeWaveFileHandle(connection, playbackHandle);
	}

	void PlayAsync() {
		PauseAsync();
		ts3Functions.playWaveFileHandle(connection, ConvertUnicodeToUTF8(fileName), FALSE, &playbackHandle);
	}

	void PauseAsync() {
		ts3Functions.pauseWaveFileHandle(connection, playbackHandle, TRUE);
	}
};


class TSDevice {
private:

	TSDevice(CStringA id, CStringA description, int frequency, int channels) {
		this->id = id;
		this->description = description;
		this->frequency = frequency;
		this->channels = channels;

	}

public:

	void Register() {
		unsigned int error;
		//if((error = ts3Functions.registerCustomDevice(id, "Nice displayable wave device name", frequency, channels, frequency, channels)) != ERROR_ok) {
		if((error = ts3Functions.registerCustomDevice(id, "Nice displayable wave device name", frequency, channels, PLAYBACK_FREQUENCY, PLAYBACK_CHANNELS)) != ERROR_ok) {
			Log::Warning(CString("Error registering custom sound device: ") + Ts::ErrorToString(error));
		}
	}

	void Unregister() {
		unsigned int error;
		if((error = ts3Functions.unregisterCustomDevice(id) != ERROR_ok)) {
			Log::Warning(L"Error unregisterCustomDevice: " + Ts::ErrorToString(error));
		}
	}

	~TSDevice() {
		Unregister();
	}

	static shared_ptr<TSDevice> Create(CStringA id, CStringA description, int frequency, int channels) {
		shared_ptr<TSDevice> device(new TSDevice(id, description, frequency, channels));
		device->Register();
		return device;
	}
private:
	CStringA id;
	CStringA description;
	int frequency;
	int channels;
};



class TSSoundPlayer {
	// Teamspeak sound related config
	const char* myDeviceId = "BattlechickensId";
	//The client lib works at 48Khz internally.
	//It is therefore advisable to use the same for your project 
	// kell ez nekünk?
	//const int PLAYBACK_FREQUENCY = 48000;
	//const int PLAYBACK_CHANNELS = 2;

private:
	std::shared_ptr<WaveTrack> track;

private:

	char* voiceactivation_level;
	char* previousCaptureDeviceName;
	char* previousCaptureMode;
	//char* previousPlaybackDeviceName;
	//char* previousPlaybackMode;
public:



	TSSoundPlayer(std::shared_ptr<WaveTrack> track) {

		this->track = track;

		ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &voiceactivation_level);


		BOOL isDefault;
		ts3Functions.getCurrentCaptureDeviceName(connection, &previousCaptureDeviceName, &isDefault);
		ts3Functions.getCurrentCaptureMode(connection, &previousCaptureMode);

		//ts3Functions.getCurrentPlaybackDeviceName(connection, &previousPlaybackDeviceName, &isDefault);
		//ts3Functions.getCurrentPlayBackMode(connection, &previousPlaybackMode);

	}



	void Play(volatile bool& stop) {


		//TSDevice device(myDeviceId, "Nice displayable device name 2", track->header.nSamplesPerSec, track->header.nChannels);
		shared_ptr<TSDevice> ts3Device = TSDevice::Create(myDeviceId, "Nice displayable device name 2", track->header.nSamplesPerSec, track->header.nChannels);



		ts3Functions.closeCaptureDevice(connection);
		//ts3Functions.closePlaybackDevice(connection);

		unsigned int error;
		if((error = ts3Functions.openCaptureDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
			Log::Warning(CString(L"Error opening capture device: ") + Ts::ErrorToString(error));
			return;
		}

		//if((error = ts3Functions.openPlaybackDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
		//	Log::Warning(CString(L"Error opening playback device: ") + TsErrorToString(error));
		//	return;
		//}

		int capturePeriodSize = (track->header.nSamplesPerSec * 20) / 1000;

		int captureAudioOffset = 0;
		//for(audioPeriodCounter = 0; audioPeriodCounter < 50 * AUDIO_PROCESS_SECONDS; ++audioPeriodCounter) { /*50*20=1000*/

		DWORD nextPlannedUpdate = GetTickCount();

		while(true) {

			//Sleep(Relu(nextPlannedUpdate - GetTickCount()));
			Sleep(20);
			nextPlannedUpdate += 20;


			// make sure we dont stream past the end of our wave sample 
			if(captureAudioOffset + capturePeriodSize > track->numberOfSamples || stop) {
				captureAudioOffset = 0;
				break;
			}

			unsigned int error;
			// stream capture data to the client lib 
			if((error = ts3Functions.processCustomCaptureData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->header.nChannels, capturePeriodSize)) != ERROR_ok) {
				Log::Warning(CString("Failed to get stream capture data: ") + Ts::ErrorToString(error));
				return;
			}
			//#if 1
			//			if((error = ts3Functions.acquireCustomPlaybackData(myDeviceId, ((short*)track->buffer.data()) + captureAudioOffset*track->channels, capturePeriodSize)) != ERROR_ok) {
			//				Log::Warning(CString("acquireCustomPlaybackData failed: ") + TsErrorToString(error));
			//			}
			//#endif		
			// update buffer offsets 
			captureAudioOffset += capturePeriodSize;

		}


		if((error = ts3Functions.closeCaptureDevice(connection) != ERROR_ok)) {
			Log::Warning(CString("Error closeCaptureDevice: ") + Ts::ErrorToString(error));
		}

		//if((error = ts3Functions.closePlaybackDevice(connection) != ERROR_ok)) {
		//	Log::Warning(CString("Error closePlaybackDevice: ") + TsErrorToString(error));
		//}

		//if((error = ts3Functions.initiateGracefulPlaybackShutdown(connection) != ERROR_ok)) {
		//	printf("Error initiateGracefulPlaybackShutdown: 0x%x\n", error);
		//	//return 1;
		//}

		//Sleep(20);

	}


	~TSSoundPlayer() {

		unsigned int error;

		//if((error = ts3Functions.unregisterCustomDevice(myDeviceId) != ERROR_ok)) {
		//	Log::Warning(CString(L"Error unregisterCustomDevice: ") + TsErrorToString(error));
		//	
		//	//printf("Error unregisterCustomDevice: 0x%x\n", error);
		//	//return 1;
		//}

		if((error = ts3Functions.openCaptureDevice(connection, previousCaptureMode, previousCaptureDeviceName)) != ERROR_ok) {
			//printf("Error (re)opening capture device: 0x%x\n", error);
			Log::Warning(CString(L"Error (re)opening capture device: ") + Ts::ErrorToString(error));
			return;
		} else {
			//cout << "\tnew device id: " << previousDeviceName << endl;
			Log::Debug(L"\tnew device id : " + CString(previousCaptureDeviceName));
		}

		//if((error = ts3Functions.openPlaybackDevice(connection, previousPlaybackMode, previousPlaybackDeviceName)) != ERROR_ok) {
		//	Log::Warning(CString(L"Error (re)opening playback device: ") + TsErrorToString(error));
		//	return;
		//}

		//if((error = ts3Functions.activateCaptureDevice(connection) ) != ERROR_ok) {
		//	printf("Error activating capture device: 0x%x\n", error);
		//	return error;
		//} 


		ts3Functions.setPreProcessorConfigValue(connection, "voiceactivation_level", voiceactivation_level);
		ts3Functions.freeMemory(voiceactivation_level);
	}

};

#endif

