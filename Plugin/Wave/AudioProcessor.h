#pragma once


#include "AudioProcessing.h"

#include <vector>
#include <memory>

namespace TSPlugin {



	class AudioProcessor {

		
	public:

		AudioProcessor() = default;
		AudioProcessor(const vector<shared_ptr<IAudioFilter>>& filters);

		bool Process(short* samples, size_t sampleCount, int channels);

		//void Enable();
		//void Disable();
	public:
		vector<shared_ptr<IAudioFilter>> Filters;
		volatile bool enabled = false;

		// elv 20 ms-t kapunk
		//volatile int durationMs = 20;
		//volatile int windowLengthMicroSec = 200;
	private:

		//std::vector<short> sampleBuffer;
		//std::vector<short> previousSampleBuffer;
	};


}

