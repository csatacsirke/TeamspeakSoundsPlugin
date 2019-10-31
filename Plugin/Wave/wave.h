#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <optional>

#include <Mmsystem.h>

namespace TSPlugin {


	//  NE HASZNÁLD! EZ SZAR!!!! HA MÁZLID VAN MüKÖDIK, DE KURVÁRA NEM VALID!
	struct WaveHeader {

		// (.cpp ben)
		bool ReadFrom(std::istream& stream);
		WAVEFORMATEX ToWaveFormatEx();
	public:
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


	class WaveTrack {
	private:
		WaveTrack() {};
		optional<WaveHeader> ReadHeader(std::istream& stream);
		bool ReadData(const WaveHeader& header, std::istream& stream);
	public:

		//WaveHeader header;
		WAVEFORMATEX format;
		std::vector<uint8_t> data;


		//int numberOfSamples;
		//size_t dataLength;

		struct Metadata {
			float maxVolume = 1.0f;
		} metadata;
	public:
		static std::shared_ptr<WaveTrack> MakeFromData(const WAVEFORMATEX& format, std::vector<uint8_t>&& data);

		static std::shared_ptr<WaveTrack> LoadWaveFile(const wchar_t* fileName);
		static std::shared_ptr<WaveTrack> LoadWaveFile(std::istream& stream);
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

