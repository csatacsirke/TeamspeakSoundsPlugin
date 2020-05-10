#pragma once


#include "AudioFilter.h"

#include <vector>
#include <memory>

namespace TSPlugin {



	class AudioProcessor {

		
	public:

		AudioProcessor() = default;
		AudioProcessor(const vector<shared_ptr<IAudioFilter>>& filters);

		bool Process(short* samples, size_t sampleCount, int channels);

	public:
		vector<shared_ptr<IAudioFilter>> Filters;
		volatile bool enabled = false;

	private:

	};


}

