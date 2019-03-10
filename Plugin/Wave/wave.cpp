#include "stdafx.h"



#include "wave.h"

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
			//Log::Warning(L"LoadWaveFile: in.fail()");
			//Log::Error(L"LoadWaveFile: in.fail()");
			//return NULL;
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

	//
	////int readWave(const wchar_t* filename, int* freq, int* channels, short** data, size_t* buffer_size, int* samples) {
	////int readWave(const wchar_t* filename, _Out_ int& freq, _Out_ int& channels, short** data, size_t* buffer_size, int* samples) {
	//std::unique_ptr<WaveTrack> readWave(const wchar_t* filename) {
	//
	//	std::unique_ptr<WaveTrack> result(new WaveTrack);
	//
	//	struct WaveHeader wh;
	//	FILE *f;
	//	int i;
	//	int elemsRead;
	//
	//	memset(&wh, 0, sizeof(wh));
	//	
	//	f = _wfopen(filename, L"rb");
	//	if (!f) {
	//		wprintf(L"error: could not open wave %s\n",filename);
	//		return 0;
	//	}
	//
	//	fread(&wh, sizeof(wh), 1, f);
	//	
	//	for(i=0; i<4; i++) {
	//		if ((wh.riffId[i] != riff[i]) ||
	//			(wh.riffType[i] != wave[i]) ||
	//			(wh.fmtId[i] != fmt[i]) ||
	//			(wh.dataId[i] != dat[i])){
	//				goto closeError;
	//		}
	//	}
	//	// Format chunk
	//	if (wh.fmtLen != 16) goto closeError;
	//	if (wh.formatTag != 1) goto closeError;
	//	int channels = wh.channels;
	//	result->channels = wh.channels;
	//	if (channels <1 || channels >2) goto closeError;
	//	//*freq = wh.samplesPerSec;
	//	result->frequency = wh.samplesPerSec;
	//	if (wh.blockAlign != channels * sizeof(short)) goto closeError;
	//	result->numberOfSamples = wh.dataLen / (channels * sizeof(short));
	//
	//
	//
	//	// what? what's wrong with short files...?
	//	//if (*samples < *freq) {
	//	//	fclose(f);
	//	//	printf("error: wave file is too short\n");
	//	//	return 0;
	//	//}
	//	
	//	result->data.resize(wh.dataLen);
	//	short* data = (short*)result->data.data();
	//	//(*data) = (short*) malloc(wh.dataLen);
	//	//*buffer_size = wh.dataLen;
	//	//if (!*data){
	//	//	printf("error: could not allocate memory for wave\n");
	//	//	return 0;
	//	//}
	//
	//	elemsRead = fread(data, wh.dataLen, 1, f);
	//	fclose(f);
	//	if (elemsRead != 1){
	//		//printf("error: reading wave file\n");
	//		Log::Error(L"error: reading wave file");
	//		return 0;
	//	}
	//
	//	return result;
	//
	//closeError:
	//	fclose(f);
	//	//wprintf(L"error: invalid wave file %s\n",filename);
	//	Log::Error(L"error: invalid wave file :" + CString(filename));
	//	return 0;
	//}
	//
	//
	//
	//
	//void WaveTrack::Save(CString fileName) {
	//	struct WaveHeader wh;
	//	int i;
	//	int elemWritten;
	//	FILE *f;
	//
	//	for(i = 0; i < 4; i++) {
	//		wh.riffId[i] = riff[i];
	//		wh.riffType[i] = wave[i];
	//		wh.fmtId[i] = fmt[i];
	//		wh.dataId[i] = dat[i];
	//	}
	//
	//	/* Format chunk */
	//	wh.fmtLen = 16;
	//	wh.formatTag = 1;  /* PCM */
	//	wh.channels = header.nChannels;
	//	wh.samplesPerSec = header.nSamplesPerSec;
	//	wh.avgBytesPerSec = header.nSamplesPerSec * header.nChannels * sizeof(short);
	//	wh.blockAlign = header.nChannels * sizeof(short);
	//	wh.bitsPerSample = sizeof(short) * 8;
	//	wh.dataLen = this->numberOfSamples * header.nChannels * sizeof(short);
	//	wh.len = 36 + wh.dataLen;
	//
	//	f = _wfopen(fileName, L"wb");
	//	if(!f) {
	//		//printf("error: could not write wave\n");
	//		Log::Error(L"error: could not write wave");
	//		return;
	//	}
	//
	//	elemWritten = fwrite(&wh, sizeof(wh), 1, f);
	//	if(elemWritten) elemWritten = fwrite(data.data(), wh.dataLen, 1, f);
	//	fclose(f);
	//
	//	if(!elemWritten) {
	//		//printf("error: could not write wave\n");
	//		Log::Error(L"error: could not write wave");
	//	}
	//}

	//void writeWave(const char* filename, int freq, int channels, short* data, int samples) {
	//	struct WaveHeader wh;
	//	int i;
	//	int elemWritten;
	//	FILE *f;
	//
	//	for(i = 0; i<4; i++) {
	//		wh.riffId[i] = riff[i];
	//		wh.riffType[i] = wave[i];
	//		wh.fmtId[i] = fmt[i];
	//		wh.dataId[i] = dat[i];
	//	}
	//
	//	/* Format chunk */
	//	wh.fmtLen = 16;
	//	wh.formatTag = 1;  /* PCM */
	//	wh.channels = channels;
	//	wh.samplesPerSec = freq;
	//	wh.avgBytesPerSec = freq * channels * sizeof(short);
	//	wh.blockAlign = channels * sizeof(short);
	//	wh.bitsPerSample = sizeof(short) * 8;
	//	wh.dataLen = samples * channels * sizeof(short);
	//	wh.len = 36 + wh.dataLen;
	//
	//	f = fopen(filename, "wb");
	//	if(!f) {
	//		printf("error: could not write wave\n");
	//		return;
	//	}
	//
	//	elemWritten = fwrite(&wh, sizeof(wh), 1, f);
	//	if(elemWritten) elemWritten = fwrite(data, wh.dataLen, 1, f);
	//	fclose(f);
	//
	//	if(!elemWritten) {
	//		printf("error: could not write wave\n");
	//	}
	//}
	//
	//
}

