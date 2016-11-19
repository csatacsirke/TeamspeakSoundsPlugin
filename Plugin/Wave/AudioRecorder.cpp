#include "stdafx.h"

#include "AudioRecorder.h"



void AudioRecorder::SetSize(size_t size) {
	std::unique_lock<std::mutex> lock(mutex);
	tempBuffer.Resize(size);
}

void AudioRecorder::AddSamples(short* samples, size_t size) {
	std::unique_lock<std::mutex> lock(mutex);
	tempBuffer.AddData(samples, size);
	if(isRecording) {
		recordBuffer.insert(recordBuffer.end(), samples, samples + size);
	}
}

void AudioRecorder::StartRecording() {
	std::unique_lock<std::mutex> lock(mutex);
	isRecording = true;
	std::vector<short> buffer;
	tempBuffer.GetData(buffer);
	recordBuffer.insert(recordBuffer.end(), buffer.data(), buffer.data() + buffer.size());
	tempBuffer.Clear();
}

void AudioRecorder::StopRecording() {
	isRecording = false;
}

void AudioRecorder::Clear() {
	std::unique_lock<std::mutex> lock(mutex);
	recordBuffer.clear();
}

void AudioRecorder::GetDataAndClear(std::vector<short>& targetBuffer) {
	std::unique_lock<std::mutex> lock(mutex);

	std::swap(targetBuffer, recordBuffer);
	recordBuffer.clear();
}

//void AudioRecorder::SaveWav(CString fileName) {
//	std::unique_lock<std::mutex> lock(mutex);
//
//}

