#pragma once

#include "AudioFilter.h"

namespace TSPlugin {

	class PitchShiftFilter : public IAudioFilter {

	public:

		void ProcessData(OutputAudioData& dataToProcess) override;
	};



} // namespace TSPlugin 

