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

}

