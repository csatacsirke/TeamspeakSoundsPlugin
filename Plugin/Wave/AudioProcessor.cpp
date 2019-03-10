#include "stdafx.h"

#include "AudioProcessor.h"
#include "AudioProcessing.h"


namespace TSPlugin {

	using namespace AudioProcessing;

	Filter filter;

	bool AudioProcessor::Process(
		short* samples,
		size_t sampleCount,
		int channels
	) {

		if (!enabled) return false;


		filter.ProcessData(AudioData{ samples, sampleCount, channels });




		//auto start = GetTickCount();


		//const int windowLength21342 = sampleCount / durationMs / 1000 * windowLengthMicroSec;
		////const int windowLength = 4600;
		//const int windowLength = sampleCount / 2;

		//if(sampleBuffer.size() != sampleCount) {
		//	sampleBuffer.resize(sampleCount*channels);
		//}

		//int windowOffset = 0;
		//while(true) {
		//	for(int i = 0; i < windowLength; ++i) {
		//		if(windowOffset + windowLength + i >= sampleBuffer.size()) {
		//			goto end;
		//		}

		//		const short newSample = (samples[windowOffset + 2 * i] + samples[windowOffset + 2 * i + channels]) / 2;
		//		sampleBuffer[windowOffset + i] = newSample;
		//		sampleBuffer[windowOffset + windowLength + i] = newSample;

		//	}
		//	windowOffset += windowLength;
		//}

		//// label
		//end:

		//memcpy(samples, sampleBuffer.data(), sizeof(short)*sampleCount*channels);

		//auto end = GetTickCount();

		//std::wcout.width(4);
		//std::wcout << int(end - start) << L"\r";

		//std::swap(sampleBuffer, previousSampleBuffer);

		return true;
	}

	//
	//void AudioProcessor::Enable() {
	//	enabled = true;
	//}
	//
	//void AudioProcessor::Disable() {
	//	enabled = false;
	//}


}

