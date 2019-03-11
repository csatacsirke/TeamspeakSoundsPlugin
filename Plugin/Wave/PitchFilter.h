#pragma once

#include "AudioProcessing.h"
#include "FourierUtils.h"

namespace TSPlugin {

	using namespace Fourier;



	const int echoSampleCount = 48000;

	class PitchFilter : public FilterBase {
	public:


		PitchFilter() : FilterBase(echoSampleCount) {

		}

		short ProcessSampleForIndex(int sampleIndex, InputChannel& inputChannel) override {
			return inputChannel[sampleIndex] + inputChannel[sampleIndex - echoSampleCount / 2] / 2 + inputChannel[sampleIndex - (echoSampleCount - 1)] / 3;
			//return (inputChannel[sampleIndex] - inputChannel[sampleIndex - 2])/2;
		}

	};




} // namespace TSPlugin


