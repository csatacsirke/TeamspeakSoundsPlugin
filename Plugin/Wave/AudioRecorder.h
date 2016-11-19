#pragma once

#include <Util\RingBuffer.h>


class AudioRecorder {
	RingBuffer<short> tempBuffer;
	std::vector<short> recordBuffer;
	bool isRecording = false;
	std::mutex mutex;
public:


	void SetSize(size_t size);
	void AddSamples(short* samples, size_t size);

	void StartRecording();
	void StopRecording();
	void Clear();
	void GetDataAndClear(std::vector<short>& targetBuffer);


	//void SaveWav(CString fileName);


};