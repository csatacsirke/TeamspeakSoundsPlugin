#pragma once

#include "wave.h"


namespace TSPlugin {

	namespace MediaFoundation {
		std::shared_ptr<WaveTrack> LoadAudioFile(const fs::path& path);
	}
}
