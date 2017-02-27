#pragma once


#include <Wave\SignalProcessing.h>
#include <Wave\Wave.h>

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
private:
	void Initialize(size_t sampleCount) {

		if (!this->get()) {
			this->reset(new AudioSample48k(sampleCount));
		}

		// ha nem jó a méret, akkor nicns értelme a cachelésnek
		assert((*this)->size() == sampleCount && "AudioSample48k GetNewBuffer(size_t size) : más a méret...");
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

		CachedAudioSample48k buffer(this);
		availableBuffers.try_pop(buffer);

		occupiedBuffers.insert(buffer);
		managedBuffers.insert(buffer);


		return buffer;
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
		return (*currentOffset >= track->dataLength);
	}

	byte* GetNextDataSegment(size_t size) {
		byte* ptr = track->buffer.data() + *currentOffset;
		*currentOffset += size;
		return ptr;
	}
	
	//WaveTrackPtr& operator=(const WaveTrackPtr& other) = delete;
private:
	std::shared_ptr<WaveTrack> track;
	std::shared_ptr<size_t> currentOffset = std::shared_ptr<size_t>(new size_t(0));
	//size_t currentOffset = 0;

};


class AudioBuffer {
public:
	//int outputChannels = 1;
private:
	const int frameLengthMillisecs = 20;
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
	//	CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(newSampleCount);
	//	if (newSampleCount != sampleCount) {

	//		SgnProc::Resample(samples, sampleCount, channels, buffer->data(), buffer->size(), outputChannels);
	//		
	//	} else {
	//		memcpy(buffer->data(), samples, sampleCount * sizeof(short));

	//	}

	//	std::unique_lock<std::mutex> lock;
	//	buffers.push(buffer);
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
		if(trackQueue.empty()) {
			return nullptr;
		}

		const WaveTrackPtr& track = trackQueue.front();

		if(track.EndOfTrack()) {
			trackQueue.pop();
			return nullptr;
			// nem törödünk azzal, hogy megnézzük van e másik, majd a kövi körben
			// ( most nincs kedvem elbaszni az idöt)
		}

		return track;

	}

	//bool TryGetSamples20ms(_Out_ CachedAudioSample48k sample) {

	// hubazdmeg itt össze van baszva minden a channelek és a smaplecount zavarban
	// ha ez egyszer jó lesz, ez lesz az a függvény amihez még bottal se szabad hozzányulni
	// DRÁGA MARCI! C++ POINTERARITMETIKA LUXUS???
	CachedAudioSample48k TryGetSamples20ms(const int outputChannels) {
		WaveTrackPtr trackPtr = FetchTrack();
		if(!trackPtr) return false;

		const WaveTrack& track = trackPtr;
		

		const WAVEFORMATEX& header = track.header;


		const int64_t inputSampleCountOf20ms = header.nSamplesPerSec * header.nChannels * frameLengthMillisecs / 1000;
		const int64_t outputSampleCountof20ms = outputFrequency * outputChannels * frameLengthMillisecs / 1000;



		//const short* const start = (short*)track.buffer.data();
		
		const byte* start = track.buffer.data();
		//const short* const start = (short*)track.buffer.data();
		//const short* const end = start + header.nSamplesPerSec * header.nChannels;
		const byte* const end = start + track.dataLength;
		//const short* const end = start + track.dataLength;
		//const short* const end = start + track.numberOfSamples;

		const byte* offset = trackPtr.GetNextDataSegment(inputSampleCountOf20ms * sizeof(short));
		
		//const short* offset = (const short*)trackPtr.GetNextDataSegment(inputSampleCountOf20ms*sizeof(short));
		//for(offset = start; offset < end - inputSampleCountOf20ms; offset += inputSampleCountOf20ms) {
		if(end - offset > inputSampleCountOf20ms) {


			//CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(outputSampleCountof20ms);
			CachedAudioSample48k buffer = cache.GetNewBuffer(outputSampleCountof20ms);
			//AudioBufferCache cache;

			if(outputSampleCountof20ms != inputSampleCountOf20ms) {
				SgnProc::Resample((const short*)offset, inputSampleCountOf20ms, header.nChannels, buffer->data(), buffer->size(), outputChannels);

			} else {
				memcpy(buffer->data(), offset, GetDataSizeInBytes(*buffer));
				//memcpy(buffer->data(), offset, inputSampleCountOf20ms * sizeof(short) * outputChannels);
			}
			return buffer;
			//std::unique_lock<std::mutex> lock(mutex);
			//buffers.push(buffer);
		} else if(end - offset > 0) {
			// overflow, ami nem egész 20ms adat
			//CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(outputSampleCountof20ms);
			CachedAudioSample48k buffer = cache.GetNewBuffer(outputSampleCountof20ms);
			memset(buffer->data(), 0, GetDataSizeInBytes(*buffer));

			size_t inputOverflowSampleCount = (end - offset) / sizeof(short);
			size_t outputOverflowSampleCount = inputOverflowSampleCount * outputSampleCountof20ms / inputSampleCountOf20ms;
			SgnProc::Resample((const short*)offset, inputOverflowSampleCount, header.nChannels, buffer->data(), outputOverflowSampleCount, outputChannels);



			return buffer;
			//std::unique_lock<std::mutex> lock(mutex);
			//buffers.push(buffer);
		}

		return nullptr;
	}

	void Clear() {
		std::unique_lock<std::mutex> lock(mutex);

		trackQueue = std::queue<WaveTrackPtr>();
	}

	//void TryGetSamples20ms(_Out_ CachedAudioSample48k) {
	//void lofasz() {
	//	const WAVEFORMATEX& header = track.header;


	//	const int inputSampleCountOf20ms = header.nSamplesPerSec * header.nChannels * frameLengthMillisecs / 1000;
	//	const int outputSampleCountof20ms = outputFrequency * outputChannels * frameLengthMillisecs / 1000;

	//	const short* const start = (short*)track.buffer.data();
	//	const short* const end = start + track.numberOfSamples;

	//	const short* offset = start;
	//	for(offset = start; offset < end - inputSampleCountOf20ms; offset += inputSampleCountOf20ms) {

	//		
	//		CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(outputSampleCountof20ms);
	//		if(outputSampleCountof20ms != inputSampleCountOf20ms) {
	//			SgnProc::Resample(offset, inputSampleCountOf20ms, header.nChannels, buffer->data(), buffer->size(), outputChannels);

	//		} else {
	//			memcpy(buffer->data(), offset, GetDataSizeInBytes(*buffer));
	//			//memcpy(buffer->data(), offset, inputSampleCountOf20ms * sizeof(short) * outputChannels);
	//		}
	//		
	//		std::unique_lock<std::mutex> lock(mutex);
	//		buffers.push(buffer);
	//	}

	//	// overflow, ami nem egész 20ms adat
	//	if(end - offset > 0) {
	//		CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(outputSampleCountof20ms);
	//		memset(buffer->data(), 0, GetDataSizeInBytes(*buffer));

	//		size_t inputOverflowSampleCount = end - offset;
	//		size_t outputOverflowSampleCount = outputSampleCountof20ms * inputOverflowSampleCount / inputSampleCountOf20ms;
	//		SgnProc::Resample(offset, inputOverflowSampleCount, header.nChannels, buffer->data(), outputOverflowSampleCount, outputChannels);

	//		


	//		std::unique_lock<std::mutex> lock(mutex);
	//		buffers.push(buffer);
	//	}
	//	

	//}

};


