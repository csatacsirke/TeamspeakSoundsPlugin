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
	extern AudioBufferCache audioBufferCache;
	//extern AudioBufferCache audioBufferCacheForCapture;
	//extern AudioBufferCache audioBufferCacheForPlayback;

}









class AudioBuffer {
public:
	int outputChannels = 1;
private:
	const int frameLengthMillisecs = 20;
	const int outputFrequency = 48000;
	
	std::mutex mutex;
	//std::queue<std::shared_ptr<std::vector<short>>> buffers;
	std::queue<CachedAudioSample48k> buffers;


public:

	AudioBuffer(int channels = 1) {
		this->outputChannels = channels;
	}


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
	bool TryGetSamples20ms(OUT CachedAudioSample48k& sample) {
		std::unique_lock<std::mutex> lock;


		if (buffers.size() > 0) {
			sample = buffers.front();
			buffers.pop();
			return true;
		}
		return false;
	}

	void AddSamples(WaveTrack& track) {
		const WAVEFORMATEX& header = track.header;


		const int inputSampleCountOf20ms = header.nSamplesPerSec * header.nChannels * frameLengthMillisecs / 1000;
		const int outputSampleCountof20ms = outputFrequency * outputChannels * frameLengthMillisecs / 1000;

		const short* const start = (short*)track.buffer.data();
		const short* const end = start + track.numberOfSamples;

		const short* offset = start;
		for(offset = start; offset < end - inputSampleCountOf20ms; offset += inputSampleCountOf20ms) {

			
			CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(outputSampleCountof20ms);
			if(outputSampleCountof20ms != inputSampleCountOf20ms) {
				SgnProc::Resample(offset, inputSampleCountOf20ms, header.nChannels, buffer->data(), buffer->size(), outputChannels);

			} else {
				memcpy(buffer->data(), offset, GetDataSizeInBytes(*buffer));
				//memcpy(buffer->data(), offset, inputSampleCountOf20ms * sizeof(short) * outputChannels);
			}
			
			std::unique_lock<std::mutex> lock;
			buffers.push(buffer);
		}

		// overflow, ami nem egész 20ms adat
		if(end - offset > 0) {
			CachedAudioSample48k buffer = Global::audioBufferCache.GetNewBuffer(outputSampleCountof20ms);
			memset(buffer->data(), 0, GetDataSizeInBytes(*buffer));

			size_t inputOverflowSampleCount = end - offset;
			size_t outputOverflowSampleCount = outputSampleCountof20ms * inputOverflowSampleCount / inputSampleCountOf20ms;
			SgnProc::Resample(offset, inputOverflowSampleCount, header.nChannels, buffer->data(), outputOverflowSampleCount, outputChannels);

			


			std::unique_lock<std::mutex> lock;
			buffers.push(buffer);
		}
		

	}

};


