#pragma once

#include "AudioProcessing.h"
#include "FourierUtils.h"

namespace AudioProcessing {

	using namespace Fourier;



	class Filter : public FilterBase {
	public:

		const int echoSampleCount = 48000;

		Filter() : FilterBase(echoSampleCount) {

		}

		short ProcessSampleForIndex(int sampleIndex, InputChannel& inputChannel) override {
			return inputChannel[sampleIndex] + inputChannel[- echoSampleCount / 2] + inputChannel[ - (echoSampleCount - 1)];
			//return (inputChannel[sampleIndex] - inputChannel[sampleIndex - 2])/2;
		}

	};



} // namespace AudioProcessing 


