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

		if (!enabled) return false;


		for (auto& filter : Filters) {
			filter->ProcessData(AudioData{ samples, sampleCount, channels });
		}



		return true;
	}

}

