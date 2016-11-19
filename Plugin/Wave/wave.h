#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include <Mmsystem.h>

class WaveTrack {
private:
	WaveTrack() {};
	bool ReadHeader(std::istream& stream);
	bool ReadData(std::istream& stream);
public:
	
	//WaveHeader header;
	WAVEFORMATEX header;


	std::vector<byte> buffer;
	int numberOfSamples;
	size_t dataLength;
public:
	static std::shared_ptr<WaveTrack> LoadWaveFile(const wchar_t* fileName);
	void Save(CString fileName);
	
};



//void writeWave(const char* filename, int freq, int channels, short* buffer, int samples);





//std::unique_ptr<WaveTrack> readWave(const wchar_t* filename);

//this reads a 16 bit 1 or 2 channel wave file. returns 0 on error, 1 on success
//int readWave(const wchar_t* filename, int* freq, int* channels, short** buffer, size_t* buffer_size, int* samples);
//int readWave(const char* filename, int* freq, int* channels, short** buffer, int* samples);



