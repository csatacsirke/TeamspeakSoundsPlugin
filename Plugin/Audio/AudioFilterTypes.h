#pragma once


#include <cassert>
#include <vector>


namespace TSPlugin {



	template<typename PtrType>
	struct AudioDataT {
		const PtrType Samples;
		const size_t SampleCount;
		const int ChannelCount;

		typedef typename std::add_const<PtrType>::type ConstPtrType;

		operator AudioDataT<ConstPtrType>() const {
			return { Samples, SampleCount, ChannelCount };
		}
	};

	typedef AudioDataT<short*> AudioData;
	typedef AudioDataT<const short*> InputAudioData;
	typedef AudioData OutputAudioData;


} // namespace TSPlugin 
