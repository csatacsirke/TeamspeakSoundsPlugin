#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"



#include "wave.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma warning( disable: 4267 )

char riff[4] = { 'R', 'I', 'F', 'F' };
char wave[4] = { 'W', 'A', 'V', 'E' };
char fmt[4]  = { 'f', 'm', 't', ' ' };
char dat[4]  = { 'd', 'a', 't', 'a' };

void writeWave(const char* filename, int freq, int channels, short* buffer, int samples) {
	struct WaveHeader wh;
	int i;
	int elemWritten;
	FILE *f;

	for(i=0; i<4; i++) {
		wh.riffId[i] = riff[i];
		wh.riffType[i] = wave[i];
		wh.fmtId[i] = fmt[i];
		wh.dataId[i] = dat[i];
	}

	/* Format chunk */
	wh.fmtLen = 16;
	wh.formatTag = 1;  /* PCM */
	wh.channels = channels;  
	wh.samplesPerSec =  freq;
	wh.avgBytesPerSec = freq * channels * sizeof(short);
	wh.blockAlign = channels * sizeof(short);
	wh.bitsPerSample = sizeof(short)*8;
	wh.dataLen = samples * channels * sizeof(short);
	wh.len = 36 + wh.dataLen;

	f = fopen(filename,"wb");
	if (!f) {
		printf("error: could not write wave\n");
		return;
	}

	elemWritten = fwrite(&wh, sizeof(wh), 1, f);
	if (elemWritten) elemWritten = fwrite(buffer, wh.dataLen, 1, f);
	fclose(f);
	
	if (!elemWritten){
		printf("error: could not write wave\n");
	}
}




//int readWave(const wchar_t* filename, int* freq, int* channels, short** buffer, size_t* buffer_size, int* samples) {
//int readWave(const wchar_t* filename, _Out_ int& freq, _Out_ int& channels, short** buffer, size_t* buffer_size, int* samples) {
std::unique_ptr<WaveTrack> readWave(const wchar_t* filename) {

	std::unique_ptr<WaveTrack> result(new WaveTrack);

	struct WaveHeader wh;
	FILE *f;
	int i;
	int elemsRead;

	memset(&wh, 0, sizeof(wh));
	
	f = _wfopen(filename, L"rb");
	if (!f) {
		wprintf(L"error: could not open wave %s\n",filename);
		return 0;
	}

	fread(&wh, sizeof(wh), 1, f);
	
	for(i=0; i<4; i++) {
		if ((wh.riffId[i] != riff[i]) ||
			(wh.riffType[i] != wave[i]) ||
			(wh.fmtId[i] != fmt[i]) ||
			(wh.dataId[i] != dat[i])){
				goto closeError;
		}
	}
	// Format chunk
	if (wh.fmtLen != 16) goto closeError;
	if (wh.formatTag != 1) goto closeError;
	int channels = wh.channels;
	result->channels = wh.channels;
	if (channels <1 || channels >2) goto closeError;
	//*freq = wh.samplesPerSec;
	result->frequency = wh.samplesPerSec;
	if (wh.blockAlign != channels * sizeof(short)) goto closeError;
	result->numberOfSamples = wh.dataLen / (channels * sizeof(short));



	// what? what's wrong with short files...?
	//if (*samples < *freq) {
	//	fclose(f);
	//	printf("error: wave file is too short\n");
	//	return 0;
	//}
	
	result->buffer.resize(wh.dataLen);
	short* buffer = (short*)result->buffer.data();
	//(*buffer) = (short*) malloc(wh.dataLen);
	//*buffer_size = wh.dataLen;
	//if (!*buffer){
	//	printf("error: could not allocate memory for wave\n");
	//	return 0;
	//}

	elemsRead = fread(buffer, wh.dataLen, 1, f);
	fclose(f);
	if (elemsRead != 1){
		printf("error: reading wave file\n");
		return 0;
	}

	return result;

closeError:
	fclose(f);
	wprintf(L"error: invalid wave file %s\n",filename);
	return 0;
}