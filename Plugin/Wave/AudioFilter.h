#pragma once

#include "AudioFilterTypes.h"

namespace TSPlugin {

	class IAudioFilter {
	public:
		virtual void ProcessData(OutputAudioData& dataToProcess) = 0;
		virtual ~IAudioFilter() = default;
	};

	namespace AudioFilters {
		shared_ptr<IAudioFilter> CreatePitchShiftFilter();
	}
	

} // namespace TSPlugin 
