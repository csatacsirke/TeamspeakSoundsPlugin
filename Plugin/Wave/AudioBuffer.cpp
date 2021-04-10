#include "stdafx.h"


#include "AudioBuffer.h"

namespace TSPlugin {

	void TSPlugin::AudioBuffer::AddTrackToQueue(std::shared_ptr<WaveTrackPlaybackState> track) {
		std::unique_lock<std::mutex> lock(mutex);
		trackQueue.push(track);
	}

	shared_ptr<WaveTrackPlaybackState> TSPlugin::AudioBuffer::FetchTrack() {
		std::unique_lock<std::mutex> lock(mutex);

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

	shared_ptr<WaveTrackPlaybackState> TSPlugin::AudioBuffer::TryPopTrack() {
		std::unique_lock<std::mutex> lock(mutex);

		if (trackQueue.empty()) {
			return nullptr;
		}

		shared_ptr<WaveTrackPlaybackState> track = trackQueue.front();
		trackQueue.pop();

		return track;
	}

	CachedAudioSample48k TSPlugin::AudioBuffer::TryGetSamples(const int sampleCountForOneChannel, const int outputChannels) {
		shared_ptr<WaveTrackPlaybackState> playbackState = FetchTrack();
		if (!playbackState) {
			return nullptr;
		}

		const WaveTrack& track = *playbackState->GetTrack();


		const WAVEFORMATEX& format = track.format;

		const int frameLengthMillisecs = sampleCountForOneChannel * 1000 / outputFrequency;

		const int64_t inputSampleCount = format.nSamplesPerSec * format.nChannels * frameLengthMillisecs / 1000;
		const int64_t outputSampleCount = sampleCountForOneChannel * outputChannels;


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

	void TSPlugin::AudioBuffer::Clear() {
		std::unique_lock<std::mutex> lock(mutex);

		// wtf? miért nincs clear()??
		trackQueue = {};
	}

	bool TSPlugin::AudioBuffer::IsEmpty() const {
		return trackQueue.empty();
	}
} // namespace TSPlugin 
