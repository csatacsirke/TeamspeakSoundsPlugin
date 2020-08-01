#include "stdafx.h"

#include "AudioFilter.h"
#include "PitchShiftFilter.h"

namespace TSPlugin {

	namespace AudioFilters {

		shared_ptr<IAudioFilter> CreatePitchShiftFilter() {
			return make_shared<PitchShiftFilter>();
		}
	}

} // namespace TSPlugin 

