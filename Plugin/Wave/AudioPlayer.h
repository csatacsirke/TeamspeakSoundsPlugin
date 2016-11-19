#pragma once


#include <Mmsystem.h>

class SmartWaveOutHandle {
	HWAVEOUT hWaveOut = NULL;
public:
	SmartWaveOutHandle() {}

	SmartWaveOutHandle(const HWAVEOUT& handle) {
		*this = handle;
	}

	SmartWaveOutHandle& operator = (const HWAVEOUT& handle) {
		if(handle == NULL) {
			throw Exception(GetLastError());
		}
		this->hWaveOut = handle;
		return *this;
	}

	~SmartWaveOutHandle() {
		if(hWaveOut != NULL) {
			// Reset before close (otherwise, waveOutClose will not work on playing buffer)
			waveOutReset(hWaveOut);
			// Close the waveOut
			waveOutClose(hWaveOut);
		}
	}

	operator HWAVEOUT() {
		return hWaveOut;
	}

	operator bool() {
		return hWaveOut != NULL;
	}

};


class AudioPlayer {
	WAVEFORMATEX pcmFormat;
	SmartWaveOutHandle hWaveOut;
public:
	AudioPlayer();
	AudioPlayer(WAVEFORMATEX pcmFormat);

	void SetPcmFormat(WAVEFORMATEX pcmFormat);
	void AddSamples(short* samples, size_t sampleCount);
	
	
};
