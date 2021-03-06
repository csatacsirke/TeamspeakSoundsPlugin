#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <optional>

#include <Mmsystem.h>

namespace TSPlugin {


	class WaveTrack {
	public:

		WAVEFORMATEX format;
		std::vector<uint8_t> data;

		struct Metadata {
			float maxVolume = 1.0f;
			std::optional<CString> fileName;
			std::optional<CString> comment;
		} metadata;
	public:
		static std::shared_ptr<WaveTrack> MakeFromData(const WAVEFORMATEX& format, std::vector<uint8_t>&& data);

		static std::shared_ptr<WaveTrack> LoadWaveFile(const fs::path& path);
		static std::shared_ptr<WaveTrack> LoadWaveFileFromStream(std::istream& stream);

		bool Save(const CString& fileName);

	private:
		void FillMetadata();
	};


	// helper functions
	float CalculateMaxVolume_Perceptive(const class WaveTrack& waveTrack);
	float CalculateMaxVolume_Absolute(const class WaveTrack& waveTrack);

}

