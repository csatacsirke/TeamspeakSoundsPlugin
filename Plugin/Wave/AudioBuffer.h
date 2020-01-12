#pragma once


#include <Wave\SignalProcessing.h>
#include <Wave\Wave.h>

#include <Util/Util.h>


namespace TSPlugin {
	typedef std::vector<short> AudioSample48k;



	class IAudioBufferCache {
	public:
		virtual void Notify(class CachedAudioSample48k& sample) = NULL;
	};

	//stypedef std::shared_ptr<AudioSample48k> CachedAudioSample48k;


	class CachedAudioSample48k : public std::shared_ptr<AudioSample48k> {
		class IAudioBufferCache* cache = NULL;
		bool used = false;
		friend class AudioBufferCache;

	public:
		CachedAudioSample48k(class IAudioBufferCache* cache = NULL) {
			this->cache = cache;
		}

		~CachedAudioSample48k() {

			if (std::shared_ptr<AudioSample48k>::use_count() == 2) {
				if (cache && used) {
					cache->Notify(*this);
				}
			}
		}

		operator bool() {
			return std::shared_ptr<AudioSample48k>::operator bool();
		}

		const char* GetData() {
			return (const char*)this->get()->data();
		}

		size_t GetDataSize() {
			return GetDataSizeInBytes(*this->get());
		}
	private:
		void Initialize(size_t sampleCount) {

			if (!this->get()) {
				this->reset(new AudioSample48k(sampleCount));
			}
			//assert(sampleCount == 960); // defaq?
			// ha nem jó a méret, akkor nicns értelme a cachelésnek
			//assert((*this)->size() == sampleCount && "AudioSample48k GetNewBuffer(size_t size) : más a méret...");
			// UPDATE - pl ha megállítod debugban vagy másért belaggol, akkor lehet más méretű a buffer, de amugy nem fordul elő sürün

			if ((*this)->size() != sampleCount) {
				(*this)->resize(sampleCount);
			}

			this->used = true;
		}

		void Uninitialize() {
			this->used = false;
		}

	};

	// TODO különbözõ méretekre is jó legyen
	// Annyi lenne a lényege a cuccnak, hogy mikor a bufferekkel zsonglörködünk akkor ne kelljen 
	// pár kilobájtos buffereket ujra-ujra allokálni amikor ugyis mindegyik ugyanakkor meretu
	class AudioBufferCache : public IAudioBufferCache {
		const int bufferCountWarningLimit = 30000;
		std::set<CachedAudioSample48k> managedBuffers;
		std::set<CachedAudioSample48k> occupiedBuffers;
		concurrency::concurrent_queue<CachedAudioSample48k> availableBuffers;

		std::mutex mutex;

		friend class CachedAudioSample48k;
	public:

		CachedAudioSample48k GetNewBuffer(size_t sampleCount) {
			CachedAudioSample48k sample = GetNewUninitializedBuffer();

			sample.Initialize(sampleCount);

			return sample;
		}

	private:
		CachedAudioSample48k GetNewUninitializedBuffer() {
			std::unique_lock<std::mutex>(mutex);

			SelfTest();

			CachedAudioSample48k data(this);
			availableBuffers.try_pop(data);

			occupiedBuffers.insert(data);
			managedBuffers.insert(data);


			return data;
		}


		void Notify(CachedAudioSample48k& sample) override {
			std::unique_lock<std::mutex>(mutex);


			sample.Uninitialize();


			occupiedBuffers.erase(sample);
			availableBuffers.push(sample);
		}

		void SelfTest() {
			assert(availableBuffers.unsafe_size() + occupiedBuffers.size() < bufferCountWarningLimit);
		}

	};

	namespace Global {
		//extern AudioBufferCache audioBufferCache;
		//extern AudioBufferCache audioBufferCacheForCapture;
		//extern AudioBufferCache audioBufferCacheForPlayback;

	}



	struct DataSegment {
		uint8_t* start;
		size_t size;
	};


	struct WaveTrackPtr {

		WaveTrackPtr(std::shared_ptr<WaveTrack> track = NULL) : track(track) {
			NULL;
		}

		operator const WaveTrack&() const {
			return *track;
		}

		operator bool() const {
			return track.operator bool();
		}

		bool EndOfTrack() const {
			return (*currentOffset >= track->data.size());
		}

		DataSegment GetNextDataSegment(size_t requestedSize) {
			uint8_t* ptr = track->data.data() + *currentOffset;
			
			const size_t remainingDataSize = track->data.size() > *currentOffset ? track->data.size() - *currentOffset : 0;

			const size_t segmentSize = std::min<size_t>(requestedSize, remainingDataSize);

			*currentOffset += requestedSize;

			return { ptr, segmentSize };
		}

		//WaveTrackPtr& operator=(const WaveTrackPtr& other) = delete;
		std::shared_ptr<WaveTrack> GetTrack() const {
			return track;
		}
	private:
		std::shared_ptr<WaveTrack> track;
		std::shared_ptr<size_t> currentOffset = std::make_shared<size_t>(0);
		//size_t currentOffset = 0;

	};


	class AudioBuffer {
	public:
		//int outputChannels = 1;
	private:
		//const int frameLengthMillisecs = 20;
		const int outputFrequency = 48000;

		std::mutex mutex;
		//std::queue<std::shared_ptr<std::vector<short>>> buffers;
		//std::queue<CachedAudioSample48k> buffers;
		std::queue<WaveTrackPtr> trackQueue;
		AudioBufferCache cache;

	public:

		//AudioBuffer(int channels = 1) {
		//	this->outputChannels = channels;
		//}


		// 20 ms adatot vár
		//void AddSamples20ms(short* samples, size_t sampleCount, int channels) {
		//	
		//	//ASSERT(channels == 1);
		//	int newSampleCount = outputFrequency*outputChannels*frameLengthMillisecs / 1000;
		//	CachedAudioSample48k data = Global::audioBufferCache.GetNewBuffer(newSampleCount);
		//	if (newSampleCount != sampleCount) {

		//		SgnProc::Resample(samples, sampleCount, channels, data->data(), data->size(), outputChannels);
		//		
		//	} else {
		//		memcpy(data->data(), samples, sampleCount * sizeof(short));

		//	}

		//	std::unique_lock<std::mutex> lock;
		//	buffers.push(data);
		//}

		// ms adatot ad vissza 48khz sample freq.en
		//bool TryGetSamples20ms(_Out_ CachedAudioSample48k& sample) {
		//	std::unique_lock<std::mutex> lock;


		//	if (buffers.size() > 0) {
		//		sample = buffers.front();
		//		buffers.pop();
		//		return true;
		//	}
		//	return false;
		//}

		void AddSamples(std::shared_ptr<WaveTrack> track) {
			std::unique_lock<std::mutex> lock(mutex);
			trackQueue.push(track);
		}

		WaveTrackPtr FetchTrack() {
			std::unique_lock<std::mutex> lock(mutex);

			if (trackQueue.empty()) {
				return nullptr;
			}

			const WaveTrackPtr& track = trackQueue.front();

			if (track.EndOfTrack()) {
				trackQueue.pop();
				return nullptr;
				// nem törödünk azzal, hogy megnézzük van e másik, majd a kövi körben
				// ( most nincs kedvem elbaszni az idöt)
			}

			return track;
		}

		shared_ptr<WaveTrack> TryPopTrack() {
			std::unique_lock<std::mutex> lock(mutex);

			if (trackQueue.empty()) {
				return nullptr;
			}

			shared_ptr<WaveTrack> track = trackQueue.front().GetTrack();
			trackQueue.pop();

			return track;
		}

		CachedAudioSample48k TryGetSamples(const int sampleCountForOneChannel, const int outputChannels) {
			WaveTrackPtr trackPtr = FetchTrack();
			if (!trackPtr) return false;

			const WaveTrack& track = trackPtr;


			const WAVEFORMATEX& format = track.format;

			const int frameLengthMillisecs = sampleCountForOneChannel * 1000 / outputFrequency;

			const int64_t inputSampleCount = format.nSamplesPerSec * format.nChannels * frameLengthMillisecs / 1000;
			const int64_t outputSampleCount = sampleCountForOneChannel * outputChannels;


			//const uint8_t* start = track.data.data();
			//const uint8_t* const end = start + track.data.size();

			const DataSegment dataSegment = trackPtr.GetNextDataSegment(inputSampleCount * sizeof(short));

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

		void Clear() {
			std::unique_lock<std::mutex> lock(mutex);

			trackQueue = std::queue<WaveTrackPtr>();
		}


		bool IsEmpty() const {
			return trackQueue.empty();
		}

	};

}
