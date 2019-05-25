#pragma once

#include "AudioProcessing.h"
#include "FourierUtils.h"

namespace TSPlugin {

	using namespace Fourier;

	const double compressionRatio = 2.0;

	const int samplesPerSec = 48000;
	const double windowLengthInSeconds = 20.0 / 1000.0;
	const size_t sampleCountPerWindow = size_t(samplesPerSec * windowLengthInSeconds);


	const size_t echoSampleCount = 48000;

	class EchoFilter : public FilterBase {
	public:


		EchoFilter() : FilterBase(samplesPerSec) {

		}

		short ProcessSampleForIndex(int sampleIndex, InputChannel& inputChannel) override {
			return inputChannel[sampleIndex] + inputChannel[sampleIndex - echoSampleCount / 2] / 2 + inputChannel[sampleIndex - (echoSampleCount - 1)] / 3;
			//return (inputChannel[sampleIndex] - inputChannel[sampleIndex - 2])/2;
		}

	};


	class PitchFilter : public IAudioFilter {
	public:


		void ProcessData(OutputAudioData& dataToProcess) override {
			SplitChannels(dataToProcess);
		}

	};





} // namespace TSPlugin


