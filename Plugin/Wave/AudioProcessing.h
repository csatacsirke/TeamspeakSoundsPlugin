#pragma once

#include "SignalProcessing.h"


#include <vector>
#include <cassert>
#include <type_traits>


namespace TSPlugin {


	//void PostProcessWaveTrack(class WaveTrack& waveTrack);
	float CalculateMaxVolume(const class WaveTrack& waveTrack);



	template<typename PtrType>
	struct AudioDataT {
		const PtrType Samples;
		const size_t SampleCount;
		const int ChannelCount;

		typedef typename std::add_const<PtrType>::type ConstPtrType;

		operator AudioDataT<ConstPtrType> () const {
			return { Samples, SampleCount, ChannelCount };
		}
	};

	typedef AudioDataT<short*> AudioData;
	typedef AudioDataT<const short*> InputAudioData;
	typedef AudioData OutputAudioData;

	class FilterBuffer {

		const size_t CachedSampleCount = 0;

		int ChannelCount = 0;
		std::vector<short> Samples;
		size_t CurrentWindowSampleCount = 0;
	public:
		FilterBuffer(size_t filterSize) : CachedSampleCount(filterSize) {
			// NULL
		}

		InputAudioData GetData() {
			return InputAudioData{ CurrentWindowStart(), CurrentWindowSampleCount, ChannelCount };
		}

		void SetCurrentData(const AudioData& data) {
			CurrentWindowSampleCount = data.SampleCount;

			const size_t requiredBufferSize = (CachedSampleCount + data.SampleCount) * data.ChannelCount;


			if (data.ChannelCount != ChannelCount) {
				ChannelCount = data.ChannelCount;
				Samples.resize(requiredBufferSize, 0);
			}

			if (Samples.size() != requiredBufferSize) {
				Samples.resize(requiredBufferSize);
				// ez az ág nincs rendesen lekezelve
				assert(0);
			}


			std::rotate(BufferBegin(), BufferBegin() + CurrentWindowSampleCount * ChannelCount, BufferEnd());
			memcpy(CurrentWindowStart(), data.Samples, sizeof(*data.Samples)*data.SampleCount*data.ChannelCount);
		}


		short* BufferBegin() {
			return Samples.data();
		}

		short* BufferEnd() {
			assert(Samples.data() + Samples.size() == Samples.data() + (CachedSampleCount + CurrentWindowSampleCount) * ChannelCount);
			return Samples.data() + Samples.size();
		}

		short* CurrentWindowStart() {
			return BufferBegin() + CachedSampleCount * ChannelCount;
		}

		int GetChannelCount() const {
			return ChannelCount;
		}


	};


	class SampleIterator {

		short* Ptr;
		size_t Stride;
	public:
		SampleIterator(short* ptr, size_t stride) {
			Ptr = ptr;
			Stride = stride;
		}

		short& operator*() {
			return *Ptr;
		}

		const short& operator*() const {
			return *Ptr;
		}

		SampleIterator& operator++() {
			Ptr += Stride;
			return *this;
		}

		SampleIterator& operator--() {
			Ptr -= Stride;
			return *this;
		}

		SampleIterator& operator+=(int delta) {
			Ptr += delta * Stride;
			return *this;
		}

		SampleIterator& operator-=(int delta) {
			Ptr -= delta * Stride;
			return *this;
		}

		SampleIterator operator+(int delta) {
			SampleIterator result(*this);
			result += delta;
			return result;
		}

		SampleIterator operator-(int delta) {
			SampleIterator result(*this);
			result -= delta;
			return result;
		}

		bool operator ==(const SampleIterator& other) {
			return Ptr == other.Ptr;
		}

		bool operator !=(const SampleIterator& other) {
			return !(*this == other);
		}



	};




	template<class DataType>
	struct ChannelT {


		DataType& Data;
		int ChannelIndex = 0;


		ChannelT(const ChannelT<DataType>& other) = default;

		size_t SampleCount() const {
			return Data.SampleCount;
		}

		auto SamplePtrForIndex(int index) {
			return Data.Samples + Data.ChannelCount * index + ChannelIndex;
		}

		auto& SampleForIndex(int index) {
			return *SamplePtrForIndex(index);
		}


		auto& operator[](int index) {
			return SampleForIndex(index);
		}

		SampleIterator begin() {
			return SampleIterator(SamplePtrForIndex(0), Data.ChannelCount);
		}

		SampleIterator end() {
			return SampleIterator(SamplePtrForIndex((int)SampleCount), Data.ChannelCount);
		}

		ChannelT<DataType>& operator++() {
			++ChannelIndex;
			return *this;
		}


		bool operator!= (const ChannelT<DataType>& other) const {
			return ChannelIndex != other.ChannelIndex;
		}

		auto& operator*() const {
			return *this;
		}


		auto& operator*() {
			return *this;
		}




	};

	typedef ChannelT<InputAudioData> InputChannel;
	typedef ChannelT<OutputAudioData> OutputChannel;


	template<class DataType>
	struct ChannelIterator {

	};

	template<class DataType>
	struct ChannelEnumerator {

		DataType& Data;
		typedef ChannelT<DataType> iterator;

		auto begin() {
			return iterator{ Data, 0 };
		}

		auto end() {
			return iterator{ Data, Data.ChannelCount };
		}

	};

	template<class DataType>
	ChannelEnumerator<DataType> Channels(DataType& data) {
		return ChannelEnumerator<DataType>{data};
	}


#pragma region Zip

	struct ZipIteratorEnd {};

	template<class C1, class C2>
	struct ZippedElement {
		typename C1::iterator it1;
		typename C2::iterator it2;


		auto First() const {
			return *it1;
		}

		auto Second() const {
			return *it2;
		}

	};

	template<class C1, class C2>
	class ZipIterator : private ZippedElement<C1, C2> {

		C1& container1;
		C2& container2;
	public:

		ZipIterator(C1& container1, C2& container2) :
			container1(container1), container2(container2), ZippedElement<C1, C2>{container1.begin(), container2.begin()}
		{
			// NULL
		}


		const ZippedElement& operator*() {
			return *this;
		}


		ZipIterator& operator++() {
			if (it1 != container1.end()) {
				++it1;
			}

			if (it2 != container2.end()) {
				++it2;
			}

			return *this;
		}

		bool AreBothIteratorsValid() const {
			return it1 != container1.end() && it2 != container2.end();
		}

		bool operator !=(const ZipIteratorEnd&) const {
			return AreBothIteratorsValid();
		}
	};

	template<class C1, class C2>
	struct ZipWrapper {
		typedef ZipIterator<C1, C2> iterator;

		C1& container1;
		C2& container2;

		iterator begin() {
			return iterator(container1, container2);
		}

		ZipIteratorEnd end() {
			return ZipIteratorEnd{};
		}

	};


	template<class C1, class C2>
	ZipWrapper<C1, C2> Zip(C1& container1, C2& container2) {
		return ZipWrapper<C1, C2>{container1, container2};
	}

#pragma endregion Zip


	class IAudioFilter {
	public:
		virtual void ProcessData(OutputAudioData& dataToProcess) = 0;
		virtual ~IAudioFilter() = default;
	};



	class FilterBase : public IAudioFilter {

		FilterBuffer buffer;
	public:
		FilterBase(size_t cachedSampleCount) : buffer(cachedSampleCount) {
			// NULL
		}


		virtual short ProcessSampleForIndex(int sampleIndex, InputChannel& inputChannel) = 0;

		void ProcessData(OutputAudioData& dataToProcess) override {
			buffer.SetCurrentData(dataToProcess);

			auto bufferedData = buffer.GetData();

			// BUG WARNING! DO NOT OPTIMIZE! 
			// IF these are temp variables like:
			// Zip(Channels(dataToProcess), Channels(bufferedData))
			// it will cause a heap corruption
			// Reason: Channel returns a struct that contains a vector,
			// whose begin() is called, but it's destroyed before this happens
			// need to find a proper solution for this
			auto channelIterator1 = Channels(dataToProcess);
			auto channelIterator2 = Channels(bufferedData);

			for (auto& channelPair : Zip(channelIterator1, channelIterator2)) {
				OutputChannel& outputChannel = channelPair.First();
				InputChannel& inputChannel = channelPair.Second();
				ProcessChannel(outputChannel, inputChannel);
			}

		}

		void ProcessChannel(OutputChannel& outputChannel, InputChannel& inputChannel) {
			for (int sampleIndex = 0; sampleIndex < outputChannel.SampleCount(); ++sampleIndex) {
				outputChannel.SampleForIndex(sampleIndex) = ProcessSampleForIndex(sampleIndex, inputChannel);
			}
		}



	};

	static inline std::vector<short> SplitChannels(const InputAudioData& inputData) {
		std::vector<short> splitData(inputData.SampleCount*inputData.ChannelCount);

		for (int channelIndex = 0; channelIndex < inputData.ChannelCount; ++channelIndex) {
			for (int sampleIndex = 0; sampleIndex < inputData.SampleCount; ++sampleIndex) {
				const size_t sourceIndex = sampleIndex * inputData.ChannelCount + channelIndex;
				const size_t destIndex = sampleIndex + inputData.SampleCount * channelIndex;
				splitData[destIndex] = inputData.Samples[sourceIndex];
			}
		}
		return splitData;
	}

	//static inline void SplitChannelsInPlace(AudioData& data) {
	//	for (int sampleIndex = data.SampleCount - 1; sampleIndex >= 0 ; --sampleIndex) {
	//	//for (int sampleIndex = 0; sampleIndex < data.SampleCount; ++sampleIndex) {
	//		for (int channelIndex = 0; channelIndex < data.ChannelCount; ++channelIndex) {
	//		//for (int channelIndex = data.ChannelCount - 1; channelIndex >=0; --channelIndex) {
	//			short& originalSample = data.Samples[sampleIndex*data.ChannelCount + channelIndex];
	//			short& newSample = data.Samples[sampleIndex + data.SampleCount * channelIndex];
	//			std::swap(originalSample, newSample);
	//		}
	//	}
	//}

	class Filter : public FilterBase {
	public:

		Filter() : FilterBase(20) {

		}

		short ProcessSampleForIndex(int sampleIndex, InputChannel& inputChannel) override {
			return inputChannel[sampleIndex] / 2;
			//return (inputChannel[sampleIndex] - inputChannel[sampleIndex - 2])/2;
		}

	};



	// :)

}

