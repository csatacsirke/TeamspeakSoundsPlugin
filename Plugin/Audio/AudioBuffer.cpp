#include "stdafx.h"


#include "AudioBuffer.h"

namespace TSPlugin {

	void AudioBuffer::AddTrackToQueue(std::shared_ptr<WaveTrackPlaybackState> track) {
		std::unique_lock lock(mutex);
		trackQueue.push(track);
	}

	shared_ptr<WaveTrackPlaybackState> AudioBuffer::FetchTrack() {
		std::unique_lock lock(mutex);

		if (trackQueue.empty()) {
			return nullptr;
		}

		const shared_ptr<WaveTrackPlaybackState> track = trackQueue.front();

		if (track->EndOfTrack()) {
			trackQueue.pop();
			return nullptr;
			// nem törödünk azzal, hogy megnézzük van e másik, majd a kövi körben
			// ( most nincs kedvem elbaszni az idöt)
		}

		return track;
	}

	shared_ptr<WaveTrackPlaybackState> AudioBuffer::TryPopTrack() {
		std::unique_lock lock(mutex);

		if (trackQueue.empty()) {
			return nullptr;
		}

		shared_ptr<WaveTrackPlaybackState> track = trackQueue.front();
		trackQueue.pop();

		return track;
	}

	CachedAudioSample48k AudioBuffer::TryGetSamples(const uint64_t sampleCountForOneChannel, const int outputChannels) {
		shared_ptr<WaveTrackPlaybackState> playbackState = FetchTrack();
		if (!playbackState) {
			return nullptr;
		}

		auto track_guard = playbackState->GetTrack();
		const WaveTrack& track = *track_guard;


		const WAVEFORMATEX& format = track.format;

		const uint64_t frameLengthMillisecs = sampleCountForOneChannel * 1000 / outputFrequency;

		const uint64_t inputSampleCount = uint64_t(format.nSamplesPerSec) * format.nChannels * frameLengthMillisecs / 1000;
		const uint64_t outputSampleCount = sampleCountForOneChannel * outputChannels;


		//const uint8_t* start = track.data.data();
		//const uint8_t* const end = start + track.data.size();

		const DataSegment dataSegment = playbackState->GetNextDataSegment(inputSampleCount * sizeof(short));

		if (dataSegment.size == inputSampleCount * format.nChannels) {

			CachedAudioSample48k data = cache.GetNewBuffer(outputSampleCount);
			memset(data->data(), 0, GetDataSizeInBytes(*data));

			if (outputSampleCount != inputSampleCount) {
				SgnProc::Resample((const short*)dataSegment.start, inputSampleCount, format.nChannels, data->data(), data->size(), outputChannels);
			} else {
				memcpy(data->data(), dataSegment.start, GetDataSizeInBytes(*data));
			}
			return data;

		} else if (dataSegment.size > 0) {
			// overflow, ami nem egész 20ms adat
			CachedAudioSample48k data = cache.GetNewBuffer(outputSampleCount);
			memset(data->data(), 0, GetDataSizeInBytes(*data));

			const size_t inputOverflowSampleCount = dataSegment.size / sizeof(short);
			const size_t outputOverflowSampleCount = inputOverflowSampleCount * outputSampleCount / inputSampleCount;
			SgnProc::Resample((const short*)dataSegment.start, inputOverflowSampleCount, format.nChannels, data->data(), outputOverflowSampleCount, outputChannels);


			return data;

		}

		return nullptr;
	}

	void AudioBuffer::Clear() {
		std::unique_lock<std::mutex> lock(mutex);

		// wtf? miért nincs clear()??
		trackQueue = {};
	}

	bool AudioBuffer::IsEmpty() const {
		std::unique_lock<std::mutex> lock(mutex);

		return trackQueue.empty();
	}

	std::shared_ptr<const WaveTrack> AudioBuffer::GetCurrentTrack() {
		if (auto playbackState = FetchTrack()) {
			return playbackState->GetTrack();
		}
		return nullptr;
	}
} // namespace TSPlugin 
