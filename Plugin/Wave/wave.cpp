#include "stdafx.h"



#include "wave.h"
#include <Util/Config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>

#pragma warning( disable: 4267 )


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


	bool WaveHeader::ReadFrom(std::istream& stream) {

		stream.read(riffId, sizeof riffId);
		stream.read((char*)&len, sizeof len);
		stream.read(riffType, sizeof riffType);
		stream.read(fmtId, sizeof fmtId);

		// hogyan gányoljunk, bálintmarcival S01E01
		// kratulálok... el is basztad mikor kivontál egymsából két int* ot 
		// söt, még azt is hogy az utolsó utáni offsetet kell kivonni
		// vagy legalább hozzáadni az uolsó méretét...
		stream.read((char*)&fmtLen, sizeof fmtLen);
		std::vector<char> data;
		data.resize(fmtLen);
		stream.read(data.data(), fmtLen);
		// két címet vonunk ki egymásból
		//size_t ourChunkLength = ((char*)&bitsPerSample - (char*)&formatTag);
		size_t ourChunkLength = ((char*)dataId - (char*)&formatTag);

		if (data.size() < ourChunkLength) {
			Log::Error(L"WaveHeader::ReadFrom: data.size() < ourChuckLength");
			return false;
		}
		memcpy(&formatTag, data.data(), ourChunkLength);

		stream.read(dataId, sizeof dataId);
		stream.read((char*)&dataLen, sizeof dataLen);


		char riff[4] = { 'R', 'I', 'F', 'F' };
		char wave[4] = { 'W', 'A', 'V', 'E' };
		char fmt[4] = { 'f', 'm', 't', ' ' };
		char dat[4] = { 'd', 'a', 't', 'a' };


		for (int i = 0; i < 4; i++) {
			if ((riffId[i] != riff[i]) ||
				(riffType[i] != wave[i]) ||
				(fmtId[i] != fmt[i]) ||
				(dataId[i] != dat[i])
				) {
				Log::Error(L"WaveHeader::ReadFrom: different wav magic number");
				return false;
			}
		}

		//if(fmtLen != 16) return false;
		if (formatTag != 1) {
			Log::Error(L"WaveHeader::ReadFrom: formatTag != 1");
			return false;
		}
		if (channels < 1 || channels >2) {
			Log::Error(L"WaveHeader::ReadFrom: channels < 1 || channels >2");
			return false;
		}
		//if(blockAlign != channels * sizeof(short)) {
		if (blockAlign != channels * this->bitsPerSample / 8) {
			Log::Error(L"WaveHeader::ReadFrom: blockAlign != channels * this->bitsPerSample / 8");

			return false;
		}

		return true;
	}


	WAVEFORMATEX WaveHeader::ToWaveFormatEx() {
		WAVEFORMATEX header;

		header.wFormatTag = WAVE_FORMAT_PCM;
		header.nChannels = this->channels;
		header.nSamplesPerSec = this->samplesPerSec;
		header.nAvgBytesPerSec = this->avgBytesPerSec;
		header.nBlockAlign = this->blockAlign;
		header.wBitsPerSample = this->bitsPerSample;
		header.cbSize = 0; // ennek utána kéne nézni *


		//*cbSize
		// 
		//	Size, in bytes, of extra format information appended to the end 
		//	of the WAVEFORMATEX structure.This information can be used by non 
		//	- PCM formats to store extra attributes for the wFormatTag.
		//	If no extra information is required by the wFormatTag, this member
		//	must be set to 0. For WAVE_FORMAT_PCM formats(and only WAVE_FORMAT_PCM formats),
		//	this member is ignored.When this structure is included 
		//	in a WAVEFORMATEXTENSIBLE structure, this value must be at least 22.


		return header;
	}

	bool WaveTrack::ReadHeader(std::istream& stream) {

		WaveHeader header;
		header.ReadFrom(stream);
		this->header = header.ToWaveFormatEx();
		numberOfSamples = header.dataLen / (header.channels * sizeof(short));
		dataLength = header.dataLen;

		return true;
	}


	bool WaveTrack::ReadData(std::istream& stream) {

		Log::Debug(L"Data Length :" + ToString(dataLength));

		data.resize(dataLength, 0);




		stream.read((char*)data.data(), dataLength);


		//short* data = (short*)result->data.data();

		if (header.wBitsPerSample == 8) {
			header.wBitsPerSample = 16;
			std::vector<uint8_t> buffer16Bit(data.size() * 2, 0);

			for (int i = 0; i < data.size(); ++i) {
				// >implying that its stored in little endian
				buffer16Bit[2 * i] = data[i];
				buffer16Bit[2 * i + 1] = 0;
			}
			std::swap(buffer16Bit, data);
		}

		if (stream.fail()) {
			Log::Warning(L"(warning) ReadData: in.fail()");
		}



		// Add smooth ending, so there will be no clicking sound because of the abrupt ending
		const int fadeoutMs = 20;
		const size_t fadeoutSampleCount = header.nSamplesPerSec / (1000 / fadeoutMs);
		const size_t fadeoutSize = fadeoutSampleCount * header.nChannels * sizeof(short);
		const size_t originalSize = data.size();
		data.resize(originalSize + fadeoutSize);

		if (header.nChannels == 1 && data.size() > sizeof(short)) {

			short* const start = (short*)(data.data() + originalSize);
			short* const end = (short*)(data.data() + data.size());

			const short lastSample = *(end - 1);
			const int64_t sampleCount = end - start;
			int64_t currentIndex = 0;

			for (short* it = start; it < end; ++it, ++currentIndex) {
				short& currentSample = *it;
				currentSample = (short)(currentSample * (sampleCount - currentIndex) / sampleCount);
			}
		}






		return true;
	}


	static float CalculateMaxVolume(const class WaveTrack& waveTrack) {

		const auto& header = waveTrack.header;

		const int bytesPerSample = header.wBitsPerSample / 8;
		const uint8_t* data = waveTrack.data.data();
		const size_t dataSize = waveTrack.data.size();

		float absMaxSample = 0;

		if (bytesPerSample == 1) {
			for (size_t offset = 0; offset < dataSize; offset += bytesPerSample) {
				const int sample = *reinterpret_cast<const uint8_t*>(data + offset);
				const float sampleAsFloat = float(sample) / float(1 << 8);
				absMaxSample = std::max<float>(abs(sampleAsFloat), absMaxSample);
			}
		} else if (bytesPerSample == 2) {
			for (size_t offset = 0; offset < dataSize; offset += bytesPerSample) {
				const int sample = *reinterpret_cast<const short*>(data + offset);
				const float sampleAsFloat = float(sample) / float(1 << 16);
				absMaxSample = std::max<float>(abs(sampleAsFloat), absMaxSample);
			}
		} else {
			ASSERT(0);
			return 1.0f;
		}

		return absMaxSample;
	}


	void WaveTrack::FillMetadata() {
		metadata.maxVolume = CalculateMaxVolume(*this);
	}


	void WaveTrack::NormalizeVolume() {
		
		const float maxVolume = CalculateMaxVolume(*this);
		const float targetVolume = float(_wtof(Global::config.Get(ConfigKey::NormalizeVolume)));
		const BOOL normalizeVolume = _wtoi(Global::config.Get(ConfigKey::NormalizeVolume));
		const float targetNormalizedVolume = float(_wtof(Global::config.Get(ConfigKey::TargetNormalizedVolume)));

		const float multiplier = normalizeVolume ? targetNormalizedVolume / maxVolume * targetVolume : targetVolume;

		short* dataStart = (short*)data.data();
		short* dataEnd = (short*)(data.data() + data.size());
		for (short* data = dataStart; data != dataEnd; ++data) {
			*data = short(float(*data) * multiplier);
		}

	}
	

	std::shared_ptr<WaveTrack> WaveTrack::LoadWaveFile(const wchar_t* fileName) {

		std::shared_ptr<WaveTrack> result(new WaveTrack);



		std::ifstream in(fileName, std::ifstream::binary);

		if (!in) {
			Log::Error(L"LoadWaveFile: failed to open stream");
			return NULL;
		}


		if (!result->ReadHeader(in)) {
			return NULL;
		}

		if (!result->ReadData(in)) {
			return NULL;
		}

		in.close();


		//result->FillMetadata();

		
		result->NormalizeVolume();
		

		return result;

	}



	static char chunk_id_riff[4] = { 'R', 'I', 'F', 'F' };
	static char chunk_id_wave[4] = { 'W', 'A', 'V', 'E' };
	static char chunk_id_fmt[4] = { 'f', 'm', 't', ' ' };
	static char chunk_id_data[4] = { 'd', 'a', 't', 'a' };

	bool WaveTrack::Save(const CString& fileName) {
		std::ofstream out(fileName, std::ofstream::binary);

		if (!out) {
			return false;
		}



		uint32_t headerChunkSize = sizeof(header);
		uint32_t dataChunkSize = data.size();
		uint32_t mainChunkSize = sizeof(chunk_id_wave) + headerChunkSize + dataChunkSize;


		out.write(chunk_id_riff, sizeof(chunk_id_riff));
		out.write((const char*)&mainChunkSize, sizeof(mainChunkSize));


		out.write((const char*)&chunk_id_wave, sizeof(chunk_id_wave));

		out.write((const char*)&chunk_id_fmt, sizeof(chunk_id_fmt));
		out.write((const char*)&headerChunkSize, sizeof(headerChunkSize));
		out.write((const char*)&header, sizeof(header));


		out.write(chunk_id_data, sizeof(chunk_id_data));
		out.write((const char*)&dataChunkSize, sizeof(dataChunkSize));
		out.write((const char*)data.data(), data.size());

		return true;
	}

}

