#pragma once

#include "AudioFilterTypes.h"

namespace TSPlugin {

	class FilterBuffer {

		const size_t CachedSampleCount = 0;

		int ChannelCount = 0;
		std::vector<short> Samples;
		size_t CurrentWindowSampleCount = 0;
	public:
		FilterBuffer(size_t filterSize) : CachedSampleCount(filterSize) {
			// NULL
		}

		InputAudioData GetData() {
			return InputAudioData{ CurrentWindowStart(), CurrentWindowSampleCount, ChannelCount };
		}

		void SetCurrentData(const AudioData& data) {
			CurrentWindowSampleCount = data.SampleCount;

			const size_t requiredBufferSize = (CachedSampleCount + data.SampleCount) * data.ChannelCount;


			if (data.ChannelCount != ChannelCount) {
				ChannelCount = data.ChannelCount;
				Samples.resize(requiredBufferSize, 0);
			}

			if (Samples.size() != requiredBufferSize) {
				Samples.resize(requiredBufferSize);
				// ez az ág nincs rendesen lekezelve
				assert(0);
			}


			std::rotate(BufferBegin(), BufferBegin() + CurrentWindowSampleCount * ChannelCount, BufferEnd());
			memcpy(CurrentWindowStart(), data.Samples, sizeof(*data.Samples)*data.SampleCount*data.ChannelCount);
		}


		short* BufferBegin() {
			return Samples.data();
		}

		short* BufferEnd() {
			assert(Samples.data() + Samples.size() == Samples.data() + (CachedSampleCount + CurrentWindowSampleCount) * ChannelCount);
			return Samples.data() + Samples.size();
		}

		short* CurrentWindowStart() {
			return BufferBegin() + CachedSampleCount * ChannelCount;
		}

		int GetChannelCount() const {
			return ChannelCount;
		}


	};

} // namespace TSPlugin 

