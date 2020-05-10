#include "stdafx.h"

#include "AudioFilter.h"

namespace TSPlugin {

	namespace AudioFilters {


		class PitchShiftFilter : public IAudioFilter {

		public:

			void ProcessData(OutputAudioData& dataToProcess) override {

			}
		};

		shared_ptr<IAudioFilter> CreatePitchShiftFilter() {
			return make_shared<PitchShiftFilter>();
		}
	}

} // namespace TSPlugin 

