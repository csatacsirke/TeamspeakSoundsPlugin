#pragma once


#define _USE_MATH_DEFINES
#include <math.h>

namespace SgnProc {

	static void Resample(const short* samples, size_t sampleCount, int channels, short* output, size_t outputCount, int outputChannels) {
		// TODO optimize
		// TODO test
		std::vector<int> channelMap(outputChannels);
		if(channels == outputChannels) {
			for(int i = 0; i < outputChannels; ++i) {
				channelMap.push_back(i);
			}
		} else {
			for(int i = 0; i < outputChannels; ++i) {
				channelMap.push_back(0);
			}
		}

		for(int channel = 0; channel < outputChannels; ++channel) {
			for(int i = 0; i < outputCount; ++i) {
				output[i*outputChannels + channel] = samples[i*sampleCount / outputCount*channelMap.size() + channelMap[channel]];
			}
		}
		
	}

	static inline double Hann(int n, int N) {
		return 0.5*(1 - cos(2*M_PI*n / (N-1)));
	}

	static inline short SampleLinear(const std::vector<short>& samples, double t) {
		//return samples[Clamp<size_t>((size_t)round(t * (samples.size() - 1)), 0, samples.size())];
	}
	

	//static void MultiplyByHann(_Out_ std::vector<short>& result, const std::vector<short>& original) {
	//	result.resize(original.size());

	//}


	static inline void Mix(short* s1, const short* s2, size_t length) {
		for(int i = 0; i < length; ++i) {
			s1[i] += s2[i];
		}
	}


}

