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
				channelMap[i] = i;
				//channelMap.push_back(i);
			}
		} else {
			for(int i = 0; i < outputChannels; ++i) {
				channelMap[i] = 0;
				//channelMap.push_back(0);
			}
		}

		assert(outputChannels == channelMap.size());


		for(int channel = 0; channel < outputChannels; ++channel) {
			for(int i = 0; i < outputCount / outputChannels; ++i) {
				float index = float(i)*float(sampleCount) / float(outputCount);
				float integer_part;
				float t = modf(index, &integer_part);
				int index1 = int(index);

				short sample1 = samples[index1 / channels * channels * outputChannels + channelMap[channel]];
				short sample2 = samples[(index1+1) / channels * channels * outputChannels + channelMap[channel]];

				output[i*outputChannels + channel] = (1-t) * sample1 + t * sample2;

				//output[i*outputChannels + channel] = samples[i*sampleCount / channels / outputCount * channels * outputChannels + channelMap[channel]];
				//output[i*outputChannels + channel] = samples[i*sampleCount / outputCount * outputChannels / channels + channelMap[channel]];
			}
		}

	}

	static void Resample__(const short* samples, size_t sampleCount, int channels, short* output, size_t outputCount, int outputChannels) {
		// TODO optimize
		// TODO test
		std::vector<int> channelMap(outputChannels);
		if(channels == outputChannels) {
			for(int i = 0; i < outputChannels; ++i) {
				channelMap[i] = i;
				//channelMap.push_back(i);
			}
		} else {
			for(int i = 0; i < outputChannels; ++i) {
				channelMap[i] = 0;
				//channelMap.push_back(0);
			}
		}

		assert(outputChannels == channelMap.size());


		for(int channel = 0; channel < outputChannels; ++channel) {
			for(int i = 0; i < outputCount / outputChannels; ++i) {
				output[i*outputChannels + channel] = samples[i*sampleCount / channels / outputCount * channels * outputChannels + channelMap[channel]];
				//output[i*outputChannels + channel] = samples[i*sampleCount / outputCount * outputChannels / channels + channelMap[channel]];
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
			// TODO rendes hangeröallitas
			s1[i] += MulDiv(s2[i], 1, 2);
		}
	}


}

