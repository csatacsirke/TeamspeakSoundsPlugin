#pragma once

#include "SignalProcessing.h"


#include <vector>
#include <type_traits>


namespace TSPlugin {


	//void PostProcessWaveTrack(class WaveTrack& waveTrack);
	//float CalculateMaxVolume(const class WaveTrack& waveTrack);



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




}

