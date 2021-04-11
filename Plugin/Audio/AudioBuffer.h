#pragma once


#include <Audio/SignalProcessing.h>
#include <Audio/Wave.h>
#include <Audio/WaveTrackPlaybackState.h>

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



	class AudioBuffer {
	public:

		void AddTrackToQueue(std::shared_ptr<WaveTrackPlaybackState> track);

		CachedAudioSample48k TryGetSamples(const uint64_t sampleCountForOneChannel, const int outputChannels);

		void Clear();
		bool IsEmpty() const;
		std::shared_ptr<const WaveTrack> GetCurrentTrack();

		shared_ptr<WaveTrackPlaybackState> TryPopTrack();

	private:
		shared_ptr<WaveTrackPlaybackState> FetchTrack();


	private:
		const int outputFrequency = 48000;

		mutable std::mutex mutex;
		std::queue<shared_ptr<WaveTrackPlaybackState>> trackQueue;
		AudioBufferCache cache;

	};

}
