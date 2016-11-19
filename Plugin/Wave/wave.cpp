#include "stdafx.h"



#include "wave.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma warning( disable: 4267 )

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
	std::vector<char> buffer;
	buffer.resize(fmtLen);
	stream.read(buffer.data(), fmtLen);
	// két címet vonunk ki egymásból
	//size_t ourChunkLength = ((char*)&bitsPerSample - (char*)&formatTag);
	size_t ourChunkLength = ((char*)dataId - (char*)&formatTag);
	
	if(buffer.size() < ourChunkLength) {
		Log::Error(L"WaveHeader::ReadFrom: buffer.size() < ourChuckLength");
		return false;
	}
	memcpy(&formatTag, buffer.data(), ourChunkLength);

	stream.read(dataId, sizeof dataId);
	stream.read((char*)&dataLen, sizeof dataLen);


	char riff[4] = { 'R', 'I', 'F', 'F' };
	char wave[4] = { 'W', 'A', 'V', 'E' };
	char fmt[4] = { 'f', 'm', 't', ' ' };
	char dat[4] = { 'd', 'a', 't', 'a' };


	for(int i = 0; i < 4; i++) {
		if((riffId[i] != riff[i]) ||
			(riffType[i] != wave[i]) ||
			(fmtId[i] != fmt[i]) ||
			(dataId[i] != dat[i])
			) {
			Log::Error(L"WaveHeader::ReadFrom: different wav magic number");
			return false;
		}
	}

	//if(fmtLen != 16) return false;
	if(formatTag != 1) {
		Log::Error(L"WaveHeader::ReadFrom: formatTag != 1");
		return false;
	}
	if(channels < 1 || channels >2) {
		Log::Error(L"WaveHeader::ReadFrom: channels < 1 || channels >2");
		return false;
	}
	//if(blockAlign != channels * sizeof(short)) {
	if(blockAlign != channels * this->bitsPerSample / 8) {
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

	
	buffer.resize(dataLength);


	Log::Debug(L"Data Length :" + ToString(dataLength));

	stream.read((char*)buffer.data(), dataLength);


	//short* buffer = (short*)result->buffer.data();

	if(header.wBitsPerSample == 8) {
		std::vector<byte> buffer16Bit(buffer.size() * 2);

		for(int i = 0; i < buffer.size(); ++i) {
			// >implying that its stored in little endian
			buffer16Bit[2 * i] = buffer[i];
			buffer16Bit[2 * i + 1] = 0;
		}
		std::swap(buffer16Bit, buffer);
	}

	if(stream.fail()) {
		Log::Warning(L"(warning) ReadData: in.fail()");
		//Log::Warning(L"LoadWaveFile: in.fail()");
		//Log::Error(L"LoadWaveFile: in.fail()");
		//return NULL;
	}

	return true;
}


std::shared_ptr<WaveTrack> WaveTrack::LoadWaveFile(const wchar_t* fileName) {

	std::shared_ptr<WaveTrack> result(new WaveTrack);

	

	std::ifstream in(fileName, std::ifstream::binary);
	
	if(!in) {
		Log::Error(L"LoadWaveFile: failed to open stream");
		return NULL;
	}


	if(!result->ReadHeader(in)) {
		return NULL;
	}

	if(!result->ReadData(in)) {
		return NULL;
	}
	
	in.close();

	return result;

}



static char riff[4] = { 'R', 'I', 'F', 'F' };
static char wave[4] = { 'W', 'A', 'V', 'E' };
static char fmt[4] = { 'f', 'm', 't', ' ' };
static char dat[4] = { 'd', 'a', 't', 'a' };


//
////int readWave(const wchar_t* filename, int* freq, int* channels, short** buffer, size_t* buffer_size, int* samples) {
////int readWave(const wchar_t* filename, _Out_ int& freq, _Out_ int& channels, short** buffer, size_t* buffer_size, int* samples) {
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
//	result->buffer.resize(wh.dataLen);
//	short* buffer = (short*)result->buffer.data();
//	//(*buffer) = (short*) malloc(wh.dataLen);
//	//*buffer_size = wh.dataLen;
//	//if (!*buffer){
//	//	printf("error: could not allocate memory for wave\n");
//	//	return 0;
//	//}
//
//	elemsRead = fread(buffer, wh.dataLen, 1, f);
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


void WaveTrack::Save(CString fileName) {
	struct WaveHeader wh;
	int i;
	int elemWritten;
	FILE *f;

	for(i = 0; i < 4; i++) {
		wh.riffId[i] = riff[i];
		wh.riffType[i] = wave[i];
		wh.fmtId[i] = fmt[i];
		wh.dataId[i] = dat[i];
	}

	/* Format chunk */
	wh.fmtLen = 16;
	wh.formatTag = 1;  /* PCM */
	wh.channels = header.nChannels;
	wh.samplesPerSec = header.nSamplesPerSec;
	wh.avgBytesPerSec = header.nSamplesPerSec * header.nChannels * sizeof(short);
	wh.blockAlign = header.nChannels * sizeof(short);
	wh.bitsPerSample = sizeof(short) * 8;
	wh.dataLen = this->numberOfSamples * header.nChannels * sizeof(short);
	wh.len = 36 + wh.dataLen;

	f = _wfopen(fileName, L"wb");
	if(!f) {
		//printf("error: could not write wave\n");
		Log::Error(L"error: could not write wave");
		return;
	}

	elemWritten = fwrite(&wh, sizeof(wh), 1, f);
	if(elemWritten) elemWritten = fwrite(buffer.data(), wh.dataLen, 1, f);
	fclose(f);

	if(!elemWritten) {
		//printf("error: could not write wave\n");
		Log::Error(L"error: could not write wave");
	}
}

//void writeWave(const char* filename, int freq, int channels, short* buffer, int samples) {
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
//	if(elemWritten) elemWritten = fwrite(buffer, wh.dataLen, 1, f);
//	fclose(f);
//
//	if(!elemWritten) {
//		printf("error: could not write wave\n");
//	}
//}
//
//
