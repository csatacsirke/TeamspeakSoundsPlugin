#include "stdafx.h"

#include "AudioProcessor.h"
#include "AudioProcessing.h"


namespace TSPlugin {

	
	AudioProcessor::AudioProcessor(const vector<shared_ptr<IAudioFilter>>& filters) : Filters(filters) {
		// NULL
	}

	bool AudioProcessor::Process(
		short* samples,
		size_t sampleCount,
		int channels
	) {

		if (!enabled) {
			return false;
		}


		if (Filters.size() == 0) {
			return false;
		}


		for (auto& filter : Filters) {
			AudioData data{ samples, sampleCount, channels };
			filter->ProcessData(data);
		}

		return true;
	}




}
