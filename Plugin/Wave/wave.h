#pragma once


//  NE HASZN�LD! EZ SZAR!!!! HA M�ZLID VAN M�K�DIK, DE KURV�RA NEM VALID!
struct WaveHeader {
	// Riff chunk
	char riffId[4];  // 'RIFF'
	unsigned int len;
	char riffType[4];  // 'WAVE'

	// Format chunk
	char fmtId[4];  // 'fmt '
	unsigned int fmtLen;
	unsigned short formatTag;
	unsigned short channels;
	unsigned int samplesPerSec;
	unsigned int avgBytesPerSec;
	unsigned short blockAlign;
	unsigned short bitsPerSample;

	// Data chunk
	char dataId[4];  // 'data'
	unsigned int dataLen;
};


void writeWave(const char* filename, int freq, int channels, short* buffer, int samples);


struct WaveTrack {
	int frequency;
	int channels;

	std::vector<byte> buffer;
	int numberOfSamples;
};

std::unique_ptr<WaveTrack> readWave(const wchar_t* filename);

//this reads a 16 bit 1 or 2 channel wave file. returns 0 on error, 1 on success
//int readWave(const wchar_t* filename, int* freq, int* channels, short** buffer, size_t* buffer_size, int* samples);
//int readWave(const char* filename, int* freq, int* channels, short** buffer, int* samples);