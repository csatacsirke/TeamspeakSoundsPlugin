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

		// ha nem j� a m�ret, akkor nicns �rtelme a cachel�snek
		assert((*this)->size() == sampleCount && "AudioSample48k GetNewBuffer(size_t size) : m�s a m�ret...");
		if ((*this)->size() != sampleCount) {
			(*this)->resize(sampleCount);
		}

		this->used = true;
	}

	void Uninitialize() {
		this->used = false;
	}

};

// TODO k�l�nb�z� m�retekre is j� legyen
// Annyi lenne a l�nyege a cuccnak, hogy mikor a bufferekkel zsongl�rk�d�nk akkor ne kelljen 
// p�r kilob�jtos buffereket ujra-ujra allok�lni amikor ugyis mindegyik ugyanakkor meretu
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
		byte* ptr = track->data.data() + *currentOffset;
		*currentOffset += size;
		return ptr;
	}
	
	//WaveTrackPtr& operator=(const WaveTrackPtr& other) = delete;
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


	// 20 ms adatot v�r
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
		if(trackQueue.empty()) {
			return nullptr;
		}

		const WaveTrackPtr& track = trackQueue.front();

		if(track.EndOfTrack()) {
			trackQueue.pop();
			return nullptr;
			// nem t�r�d�nk azzal, hogy megn�zz�k van e m�sik, majd a k�vi k�rben
			// ( most nincs kedvem elbaszni az id�t)
		}

		return track;

	}

	//bool TryGetSamples20ms(_Out_ CachedAudioSample48k sample) {

	// hubazdmeg itt �ssze van baszva minden a channelek �s a smaplecount zavarban
	// ha ez egyszer j� lesz, ez lesz az a f�ggv�ny amihez m�g bottal se szabad hozz�nyulni
	// DR�GA MARCI! C++ POINTERARITMETIKA LUXUS???
	CachedAudioSample48k TryGetSamples(const int sampleCountForOneChannel, const int outputChannels) {
		WaveTrackPtr trackPtr = FetchTrack();
		if(!trackPtr) return false;

		const WaveTrack& track = trackPtr;
		

		const WAVEFORMATEX& header = track.header;

		const int frameLengthMillisecs = sampleCountForOneChannel * 1000 / outputFrequency;

		const int64_t inputSampleCount = header.nSamplesPerSec * header.nChannels * frameLengthMillisecs / 1000;
		//const int64_t outputSampleCountof= outputFrequency * outputChannels * frameLengthMillisecs / 1000;
		const int64_t outputSampleCount = sampleCountForOneChannel * outputChannels;



		//const short* const start = (short*)track.data.data();
		
		const byte* start = track.data.data();
		//const short* const start = (short*)track.data.data();
		//const short* const end = start + header.nSamplesPerSec * header.nChannels;
		const byte* const end = start + track.dataLength;
		//const short* const end = start + track.dataLength;
		//const short* const end = start + track.numberOfSamples;

		const byte* offset = trackPtr.GetNextDataSegment(inputSampleCount* sizeof(short));
		
		//const short* offset = (const short*)trackPtr.GetNextDataSegment(inputSampleCount*sizeof(short));
		//for(offset = start; offset < end - inputSampleCount; offset += inputSampleCount) {
		if(end - offset > inputSampleCount) {


			//CachedAudioSample48k data = Global::audioBufferCache.GetNewBuffer(outputSampleCount);
			CachedAudioSample48k data = cache.GetNewBuffer(outputSampleCount);
			memset(data->data(), 0, GetDataSizeInBytes(*data));
			//AudioBufferCache cache;

			if(outputSampleCount != inputSampleCount) {
				SgnProc::Resample((const short*)offset, inputSampleCount, header.nChannels, data->data(), data->size(), outputChannels);

			} else {
				memcpy(data->data(), offset, GetDataSizeInBytes(*data));
				//memcpy(data->data(), offset, inputSampleCount * sizeof(short) * outputChannels);
			}
			return data;
			//std::unique_lock<std::mutex> lock(mutex);
			//buffers.push(data);
		} else if(end - offset > 0) {
			// overflow, ami nem eg�sz 20ms adat
			//CachedAudioSample48k data = Global::audioBufferCache.GetNewBuffer(outputSampleCount);
			CachedAudioSample48k data = cache.GetNewBuffer(outputSampleCount);
			memset(data->data(), 0, GetDataSizeInBytes(*data));

			size_t inputOverflowSampleCount = (end - offset) / sizeof(short);
			size_t outputOverflowSampleCount = inputOverflowSampleCount * outputSampleCount / inputSampleCount;
			SgnProc::Resample((const short*)offset, inputOverflowSampleCount, header.nChannels, data->data(), outputOverflowSampleCount, outputChannels);



			return data;
			//std::unique_lock<std::mutex> lock(mutex);
			//buffers.push(data);
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


	//	const int inputSampleCount = header.nSamplesPerSec * header.nChannels * frameLengthMillisecs / 1000;
	//	const int outputSampleCount = outputFrequency * outputChannels * frameLengthMillisecs / 1000;

	//	const short* const start = (short*)track.data.data();
	//	const short* const end = start + track.numberOfSamples;

	//	const short* offset = start;
	//	for(offset = start; offset < end - inputSampleCount; offset += inputSampleCount) {

	//		
	//		CachedAudioSample48k data = Global::audioBufferCache.GetNewBuffer(outputSampleCount);
	//		if(outputSampleCount != inputSampleCount) {
	//			SgnProc::Resample(offset, inputSampleCount, header.nChannels, data->data(), data->size(), outputChannels);

	//		} else {
	//			memcpy(data->data(), offset, GetDataSizeInBytes(*data));
	//			//memcpy(data->data(), offset, inputSampleCount * sizeof(short) * outputChannels);
	//		}
	//		
	//		std::unique_lock<std::mutex> lock(mutex);
	//		buffers.push(data);
	//	}

	//	// overflow, ami nem eg�sz 20ms adat
	//	if(end - offset > 0) {
	//		CachedAudioSample48k data = Global::audioBufferCache.GetNewBuffer(outputSampleCount);
	//		memset(data->data(), 0, GetDataSizeInBytes(*data));

	//		size_t inputOverflowSampleCount = end - offset;
	//		size_t outputOverflowSampleCount = outputSampleCount * inputOverflowSampleCount / inputSampleCount;
	//		SgnProc::Resample(offset, inputOverflowSampleCount, header.nChannels, data->data(), outputOverflowSampleCount, outputChannels);

	//		


	//		std::unique_lock<std::mutex> lock(mutex);
	//		buffers.push(data);
	//	}
	//	

	//}

};


