#pragma once

#include <Audio/Wave.h>


namespace TSPlugin {

	struct DataSegment {
		const uint8_t* start;
		size_t size;
	};


	struct WaveTrackPlaybackState {

		explicit WaveTrackPlaybackState(std::shared_ptr<WaveTrack> track) : track(track) {
			NULL;
		}

		WaveTrackPlaybackState(const WaveTrackPlaybackState& other) = delete;

		bool EndOfTrack() const {
			return (currentOffset >= track->data.size());
		}

		DataSegment GetNextDataSegment(size_t requestedSize) {
			const uint8_t* ptr = track->data.data() + currentOffset;

			const size_t remainingDataSize = track->data.size() > currentOffset ? track->data.size() - currentOffset : 0;

			const size_t segmentSize = std::min<size_t>(requestedSize, remainingDataSize);

			currentOffset += requestedSize;

			return { ptr, segmentSize };
		}

		std::shared_ptr<const WaveTrack> GetTrack() const {
			return track;
		}

	private:
		const std::shared_ptr<const WaveTrack> track;
		size_t currentOffset = 0;
	};


} // TSPlugin 
