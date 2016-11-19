#pragma once


#define _USE_MATH_DEFINES
#include <math.h>

namespace SgnProc {

	inline double Hann(int n, int N) {
		return 0.5*(1 - cos(2*M_PI*n / (N-1)));
	}

	inline short SampleLinear(const std::vector<short>& samples, double t) {
		//return samples[Clamp<size_t>((size_t)round(t * (samples.size() - 1)), 0, samples.size())];
	}
	

	void MultiplyByHann(_Out_ std::vector<short>& result, const std::vector<short>& original) {
		result.resize(original.size());



	}


}

