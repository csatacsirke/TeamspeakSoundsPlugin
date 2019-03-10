#include "stdafx.h"
#include "AudioPlayer.h"


#include <stdio.h>
#include <assert.h>
#include <mmreg.h>
#include <msacm.h>
#include <wmsdk.h>

#pragma comment(lib, "msacm32.lib") 
#pragma comment(lib, "wmvcore.lib") 
#pragma comment(lib, "winmm.lib") 









namespace TSPlugin {








	// Define output format
	static WAVEFORMATEX defaultPcmFormat = {
		WAVE_FORMAT_PCM, // WORD        wFormatTag;         /* format type */
		2,     // WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
		48000,    // DWORD       nSamplesPerSec;     /* sample rate */
		4 * 48000,   // DWORD       nAvgBytesPerSec;    /* for buffer estimation */
		4,     // WORD        nBlockAlign;        /* block size of data */
		16,     // WORD        wBitsPerSample;     /* number of bits per sample of mono data */
		0,     // WORD        cbSize;             /* the count in bytes of the size of */
	};

	AudioPlayer::AudioPlayer() : AudioPlayer(defaultPcmFormat) {
		NULL;
	}

	AudioPlayer::AudioPlayer(WAVEFORMATEX pcmFormat) {
		SetPcmFormat(pcmFormat);
	}

	template<class T>
	bool EqualsBitwise(T& a, T& b) {
		return (memcmp(&a, &b, sizeof(T)) == 0);
	}

	void AudioPlayer::SetPcmFormat(WAVEFORMATEX pcmFormat) {
		if (EqualsBitwise(this->pcmFormat, pcmFormat)) {
			// nothing to be done ^^
			return;
		}


		this->pcmFormat = pcmFormat;

		HWAVEOUT hWaveOut;
		MMRESULT succeeded = waveOutOpen(&hWaveOut, WAVE_MAPPER, &pcmFormat, NULL, 0, CALLBACK_NULL);
		if (succeeded) {
			this->hWaveOut = hWaveOut;
		}

	}

	void AudioPlayer::AddSamples(short* samples, size_t sampleCount) {
		static WAVEHDR WaveHDR = { (char*)samples,  (DWORD)sampleCount * sizeof(*samples) };
		waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
		waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));

	}


	//
	//void Play() {
	//	static WAVEHDR WaveHDR = { (LPSTR)soundBuffer,  bufferLength };
	//
	//
	//	waveOutOpen(&hWaveOut, WAVE_MAPPER, &pcmFormat, NULL, 0, CALLBACK_NULL);
	//	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	//	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	//	// Reset before close (otherwise, waveOutClose will not work on playing buffer)
	//	waveOutReset(hWaveOut);
	//	// Close the waveOut
	//	waveOutClose(hWaveOut);
	//}


}

