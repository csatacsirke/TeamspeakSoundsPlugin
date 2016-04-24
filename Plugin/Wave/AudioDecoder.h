#pragma once

#if USE_WINDOWS_MEDIA_PACK_FEATURES

//#include "wave.h"
#include <mmreg.h> // for WAVEFORMATEX


class AudioDecoder {
	struct IMFSourceReader *pReader = NULL;
	struct IMFMediaType *pAudioType = NULL;    // Represents the PCM audio format.

	
public:
	//typedef std::unique_ptr<std::vector<byte>> Buffer;
	class Buffer : public std::unique_ptr<std::vector<byte>> {
	public:
		Buffer() {
			std::unique_ptr<std::vector<byte>>::reset(new std::vector<byte>());
		}
		void Append(void* data, size_t length) {
			size_t offset = (*this)->size();
			(*this)->resize(offset + length);
			memcpy((*this)->data() + offset, data, length);
		}
		void Truncate(size_t size, Buffer& overflow) {
			if((*this)->size() > size) {
				size_t overflowSize = (*this)->size() - size;
				overflow.Append(this->get()->data() + size, overflowSize);
				this->get()->resize(size);
			}
		}
	};
private:
	Buffer overflow;
	WAVEFORMATEX header;
public:

	AudioDecoder(CString fileName);
	~AudioDecoder();
	WAVEFORMATEX GetHeader();
	Buffer DecodeMillisecs(int millis);

private:
	Buffer AudioDecoder::GetNextChunk(DWORD cbMaxAudioData);
	WAVEFORMATEX CreateHeader();
};


#endif // USE_WINDOWS_MEDIA_PACK_FEATURES