#pragma once

#include <iostream>
#include <vector>
#include <memory>

#include <Mmsystem.h>

namespace TSPlugin {

	class WaveTrack {
	private:
		WaveTrack() {};
		bool ReadHeader(std::istream& stream);
		bool ReadData(std::istream& stream);
	public:

		//WaveHeader header;
		WAVEFORMATEX header;


		std::vector<uint8_t> data;
		int numberOfSamples;
		size_t dataLength;

		struct Metadata {
			float maxVolume = 1.0f;
		} metadata;
	public:
		static std::shared_ptr<WaveTrack> LoadWaveFile(const wchar_t* fileName);
		bool Save(const CString& fileName);

	private:
		void FillMetadata();
		void NormalizeVolume();
	};



	//void writeWave(const char* filename, int freq, int channels, short* buffer, int samples);





	//std::unique_ptr<WaveTrack> readWave(const wchar_t* filename);

	//this reads a 16 bit 1 or 2 channel wave file. returns 0 on error, 1 on success
	//int readWave(const wchar_t* filename, int* freq, int* channels, short** buffer, size_t* buffer_size, int* samples);
	//int readWave(const char* filename, int* freq, int* channels, short** buffer, int* samples);


}

