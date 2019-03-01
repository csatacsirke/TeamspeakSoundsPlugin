#pragma once

#include "SignalProcessing.h"


#include <vector>
#include <cassert>

namespace AudioProcessing {

	template<typename PtrType>
	struct AudioDataT {
		const PtrType Samples;
		const size_t SampleCount;
		const int ChannelCount;
	};

	typedef AudioDataT<short*> AudioData;
	typedef AudioDataT<const short*> InputAudioData;
	typedef AudioData OutputAudioData;

	//struct OutputAudioData {
	//	short* const Samples;
	//	size_t const SampleCount;
	//	int const ChannelCount;
	//};

	class FilterBuffer {

		//vector<short> previousSamples;
		const size_t FilterSize;

		int ChannelCount = 0;
		std::vector<short> Samples;
		size_t CurrentWindowSampleCount = 0;
		//short* CurrentWindowStart = nullptr;
		//size_t CurrentWindowSampleCount = 0;
		//short* Offset = nullptr;
	public:
		FilterBuffer(size_t filterSize) : FilterSize(filterSize) {
			// NULL
		}

		InputAudioData GetData() {
			return InputAudioData{ CurrentWindowStart(), CurrentWindowSampleCount, ChannelCount };
		}

		void SetCurrentData(const AudioData& data) {
			CurrentWindowSampleCount = data.SampleCount;

			const size_t requiredBufferSize = (FilterSize + data.SampleCount) * data.ChannelCount;


			if (data.ChannelCount != ChannelCount) {
				ChannelCount = data.ChannelCount;
				Samples.resize(requiredBufferSize, 0);
			}

			if (Samples.size() != requiredBufferSize) {
				Samples.resize(requiredBufferSize);
				// ez az ág nincs rendesen lekezelve
				assert(0);
			}


			std::rotate(BufferBegin(), CurrentWindowStart(), BufferEnd());
			memcpy(CurrentWindowStart(), data.Samples, sizeof(*data.Samples)*data.SampleCount*data.ChannelCount);
		}


		//void SetCurrentData(const short* samples, int sampleCount, int channels) {
		//	CurrentWindowSampleCount = sampleCount;

		//	const size_t requiredBufferSize = (FilterSize + sampleCount) * channels;


		//	if (channels != ChannelCount) {
		//		ChannelCount = channels;
		//		Samples.resize(requiredBufferSize, 0);
		//	}

		//	if (Samples.size() != requiredBufferSize) {
		//		Samples.resize(requiredBufferSize);
		//		// ez az ág nincs rendesen lekezelve
		//		assert(0);
		//	}


		//	std::rotate(BufferBegin(), CurrentWindowStart(), BufferEnd());
		//	memcpy(CurrentWindowStart(), samples, sizeof(*samples)*sampleCount*channels);
		//}

		short* BufferBegin() {
			return Samples.data();
		}

		short* BufferEnd() {
			return Samples.data() + Samples.size();
		}

		short* CurrentWindowStart() {
			return BufferBegin() + CurrentWindowSampleCount * ChannelCount;
			//short* bufferStart = Samples.data();
			//return bufferStart + ChannelCount * FilterSize;
		}

		int GetChannelCount() const{
			return ChannelCount;
		}

		size_t SampleCount() const {
			return Samples.size() / ChannelCount;
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

	void f() {
		SampleIterator s1 = { nullptr, 0 };
		SampleIterator s2 = { nullptr, 0 };

		s1 != s2;
	}


	//class ChannelT {
	//	FilterBuffer& Buffer;
	//	const int ChannelIndex;


	//	const size_t& FilterSize = Buffer.FilterSize;
	//	const int& ChannelCount = Buffer.ChannelCount;
	//public:
	//	ChannelT(FilterBuffer& buffer, int channelIndex) : Buffer(buffer), ChannelIndex(channelIndex) {
	//		// NULL
	//	}

	//	short* SamplePtrForIndex(const size_t index) {
	//		return Buffer.BufferBegin() + Buffer.ChannelCount * index + ChannelIndex;
	//	}

	//	short& SampleForIndex(const size_t index) {
	//		return *SamplePtrForIndex(index);
	//	}


	//	short& operator[](const size_t index) {
	//		return SampleForIndex(index);
	//	}

	//	SampleIterator begin() {
	//		return SampleIterator(SamplePtrForIndex(FilterSize), ChannelCount);
	//	}

	//	SampleIterator end() {
	//		return SampleIterator(SamplePtrForIndex(Buffer.SampleCount()), ChannelCount);
	//	}



	//};


	template<class DataType>
	struct ChannelT {
		
		//const int ChannelCount;
		//short* const Samples;
		//const size_t SampleCount;

		const DataType& Data;
		const int ChannelIndex;

	
	
		ChannelT(DataType& data, int channelIndex)
			: Data(data), ChannelIndex(channelIndex)
		{
			// NULL
		}

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

	};

	typedef ChannelT<InputAudioData> InputChannel;
	typedef ChannelT<OutputAudioData> OutputChannel;

	template<class DataType>
	class ChannelIterator {
		std::vector<ChannelT<DataType>> channels;
	public:

		typedef decltype(channels.begin()) iterator;

		ChannelIterator(DataType& data) {
			channels.reserve(data.ChannelCount);
			for (int channelIndex = 0; channelIndex < data.ChannelCount; ++channelIndex) {
				channels.emplace_back(data, channelIndex);
			}
		}

		ChannelIterator(short* samples, size_t sampleCount, int channelCount) {
			channels.reserve(channelCount);
			for (int channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
				channels.emplace_back(samples, sampleCount, channelCount, channelIndex);
			}
		}

		auto begin() {
			return channels.begin();
		}

		auto end() {
			return channels.end();
		}

	};

	template<class DataType>
	ChannelIterator<DataType> Channels(DataType& data) {
		return ChannelIterator<DataType>(data);
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
			container1(container1), container2(container2)
		{
			it1 = container1.begin();
			it2 = container2.begin();
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






	class Filter {

		FilterBuffer buffer = FilterBuffer(4);
	public:
		void ProcessData(OutputAudioData& dataToProcess) {
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

		short ProcessSampleForIndex(int sampleIndex, InputChannel& inputChannel) {
			return inputChannel[sampleIndex] - inputChannel[sampleIndex - 1];
		}

		//void ProcessChannel(ChannelT& channel) {
		//	
		//
		//	for (auto it = channel.begin(); ++it != channel.end(); ++it) {
//		//		ProcessSample(it);
		//	}
		//}

		//short ProcessSample(size_t sampleIndex, const I& channel) {
		//	//*sampleIterator = (*sampleIterator - *(sampleIterator - 2)*2);
		//}

	};


	template<typename Ptr>
	void a(const Ptr p) {
		*p = 42;
	}

	void test() {
		short array[] = { 1, 2, 3, 4, 54 };
		short *ptr = (short*)array;
		a<short*>(ptr);
	}

} // namespace AudioProcessing



