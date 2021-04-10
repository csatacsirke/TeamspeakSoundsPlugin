#include "stdafx.h"


#if USE_WINDOWS_MEDIA_PACK_FEATURES

#include "AudioDecoder.h"
#include "AudioClipSample.h"



//#include "wave.h"
#include <mmreg.h> // for WAVEFORMATEX


class AudioDecoder {
	struct IMFSourceReader* pReader = NULL;
	struct IMFMediaType* pAudioType = NULL;    // Represents the PCM audio format.


public:
	//typedef std::unique_ptr<std::vector<uint8_t>> Buffer;
	class Buffer : public std::unique_ptr<std::vector<uint8_t>> {
	public:
		Buffer() {
			std::unique_ptr<std::vector<uint8_t>>::reset(new std::vector<uint8_t>());
		}
		void Append(void* data, size_t length) {
			size_t offset = (*this)->size();
			(*this)->resize(offset + length);
			memcpy((*this)->data() + offset, data, length);
		}
		void Truncate(size_t size, Buffer& overflow) {
			if ((*this)->size() > size) {
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
	Buffer GetNextChunk(DWORD cbMaxAudioData);
	WAVEFORMATEX CreateHeader();
};



AudioDecoder::AudioDecoder(CString fileName) {

	(void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	//if(argc != 3) {
	//	printf("arguments: input_file output_file.wav\n");
	//	return 1;
	//}

	//const WCHAR *wszSourceFile = argv[1];
	//const WCHAR *wszTargetFile = argv[2];

	// TODO

	//const LONG MAX_AUDIO_DURATION_MSEC = 5000; // 5 seconds 
	//const LONG MAX_AUDIO_DURATION_MSEC = 20; // 5 seconds 

	HRESULT hr = S_OK;

	


	// Initialize the COM library.
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Intialize the Media Foundation platform.
	if(SUCCEEDED(hr)) {
		hr = MFStartup(MF_VERSION);
	}

	// Create the source reader to read the input file.
	if(SUCCEEDED(hr)) {
		hr = MFCreateSourceReaderFromURL(
			fileName,
			NULL,
			&pReader
			);

		if(FAILED(hr)) {
			printf("Error opening input file: %S\n", (const wchar_t*)fileName);
		}
	}


	//HANDLE hFile = INVALID_HANDLE_VALUE;
	//// Open the output file for writing.
	//if(SUCCEEDED(hr)) {
	//	hFile = CreateFile(
	//		wszTargetFile,
	//		GENERIC_WRITE,
	//		FILE_SHARE_READ,
	//		NULL,
	//		CREATE_ALWAYS,
	//		0,
	//		NULL
	//		);

	//	if(hFile == INVALID_HANDLE_VALUE) {
	//		hr = HRESULT_FROM_WIN32(GetLastError());

	//		printf("Cannot create output file: %S\n", wszTargetFile, hr);
	//	}
	//}

	// Write the WAVE file.
	//if(SUCCEEDED(hr)) {
	//	hr = WriteWaveFile(pReader, hFile, MAX_AUDIO_DURATION_MSEC);
	//}

	

										// Configure the source reader to get uncompressed PCM audio from the source file.

	hr = ConfigureAudioStream(pReader, &pAudioType);

	if(FAILED(hr)) {
		printf("Failed, hr = 0x%X\n", hr);
	}

	// Clean up.
	//if(hFile != INVALID_HANDLE_VALUE) {
	//	CloseHandle(hFile);
	//}

	this->header = CreateHeader();

	assert(hr == S_OK);
	//return SUCCEEDED(hr) ? 0 : 1;
}


AudioDecoder::~AudioDecoder() {
	SafeRelease(&pAudioType);
	SafeRelease(&pReader);
	MFShutdown();
	CoUninitialize();
}

WAVEFORMATEX AudioDecoder::GetHeader() {
	return this->header;
}


WAVEFORMATEX AudioDecoder::CreateHeader() {
	HRESULT hr = S_OK;
	UINT32 cbFormat = 0;

	
	WAVEFORMATEX *pWav = NULL;

	//*pcbWritten = 0;

	// Convert the PCM audio format into a WAVEFORMATEX structure.
	hr = MFCreateWaveFormatExFromMFMediaType(
		pAudioType,
		&pWav,
		&cbFormat
		);

	WAVEFORMATEX header;

	if(SUCCEEDED(hr)) {
		header = *pWav;
	} 
	

	assert(pWav);

	CoTaskMemFree(pWav);
	//return hr;

	// Write the WAVE file header.
	//if(SUCCEEDED(hr)) {
		//hr = WriteWaveHeader(hFile, pAudioType, &cbHeader);
	//}

	//assert(0);
	
	if(FAILED(hr)) {
		throw std::exception("GetHeader (audiodecoder) failed");
	}

	return header;
}

AudioDecoder::Buffer AudioDecoder::DecodeMillisecs(int msecAudioData) {
	

	//HRESULT hr = S_OK;

	DWORD cbHeader = 0;         // Size of the WAVE file header, in bytes.
	//DWORD cbAudioData = 0;      // Total bytes of PCM audio data written to the file.
	DWORD cbMaxAudioData = 0;



	// Calculate the maximum amount of audio to decode, in bytes.
	// GY.K.: átváltás másodpercböl bájtba....
	cbMaxAudioData = CalculateMaxAudioDataSize(pAudioType, cbHeader, msecAudioData);
	
	// Decode audio data to the file.
	Buffer buffer;
	//if(overflow->size() > 0) {
	//	if(overflow->size() >= cbMaxAudioData) {
	//		// ha maradt annyi az elözö körböl, hogy még egy kört fel lehet vele tölteni

	//		buffer.swap(overflow);
	//		buffer.Truncate(cbMaxAudioData, overflow);

	//	} else {
	//		// ha maradt valamennyi, de kell még hozzáolvasni

	//		buffer.swap(overflow);
	//		Buffer nextChunk = GetNextChunk(cbMaxAudioData);
	//		buffer.Append(nextChunk->data(), nextChunk->size());
	//		buffer.Truncate(cbMaxAudioData, overflow);

	//	}
	//} else {
	//	// ha üres volt az overflow
	//	buffer = GetNextChunk(cbMaxAudioData);
	//	buffer.Truncate(cbMaxAudioData, overflow);
	//}
	
	buffer.swap(overflow);
	while(true) {
		if(buffer->size() < cbMaxAudioData) {
			Buffer nextChunk = GetNextChunk(cbMaxAudioData);
			if(nextChunk->size() == 0) {
				break;
			}
			buffer.Append(nextChunk->data(), nextChunk->size());
		} else {
			buffer.Truncate(cbMaxAudioData, overflow);
			break;
		}
	}

	//bool eof = false;
	/*while(buffer->size < cbMaxAudioData && !eof) {
		buffer.swap(overflow);
		if(buffer->size <= cbMaxAudioData)
	}*/




	
	//// Fix up the RIFF headers with the correct sizes.
	//if(SUCCEEDED(hr)) {
	//	hr = FixUpChunkSizes(hFile, cbHeader, cbAudioData);
	//}

	

	//assert(SUCCEEDED(hr));

	return buffer;
}

// TODO rename
//AudioDecoder::Buffer AudioDecoder::GetNextChunk( DWORD cbMaxAudioData) {

AudioDecoder::Buffer AudioDecoder::GetNextChunk(DWORD cbMinAudioData) {
	HRESULT hr = S_OK;
	DWORD cbAudioData = 0;
	
	BYTE *pAudioData = NULL;

	IMFSample *pSample = NULL;
	IMFMediaBuffer *pBuffer = NULL;

	Buffer buffer;

	// Get audio samples from the source reader.
	while(true) {
		DWORD dwFlags = 0;

		// Read the next sample.
		hr = pReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			NULL,
			&dwFlags,
			NULL,
			&pSample
			);

		if(FAILED(hr)) {
			break;
		}

		if(dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
			printf("Type change - not supported by WAVE file format.\n");
			break;
		}
		if(dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			printf("End of input file.\n");
			break;
		}

		if(pSample == NULL) {
			printf("No sample\n");
			continue;
		}

		// Get a pointer to the audio data in the sample.

		hr = pSample->ConvertToContiguousBuffer(&pBuffer);

		if(FAILED(hr)) {
			break;
		}

		DWORD cbBuffer = 0;
		hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);

		if(FAILED(hr)) {
			break;
		}

		// TODO ezt máshogy mert ez el van baszva...
		// Make sure not to exceed the specified maximum size.
		//if(cbMaxAudioData - cbAudioData < cbBuffer) {
		//	cbBuffer = cbMaxAudioData - cbAudioData;
		//}

		//// Write this data to the output file.
		//hr = WriteToFile(hFile, pAudioData, cbBuffer);
		
		//buffer->resize(cbBuffer);
		//memcpy(buffer->data(), pAudioData, cbBuffer);
		buffer.Append(pAudioData, cbBuffer);

		if(FAILED(hr)) {
			break;
		}

		// Unlock the buffer.
		hr = pBuffer->Unlock();
		pAudioData = NULL;

		if(FAILED(hr)) {
			break;
		}

		// Update running total of audio data.
		cbAudioData += cbBuffer;

		if(cbAudioData >= cbMinAudioData) {
			break;
		}

		SafeRelease(&pSample);
		SafeRelease(&pBuffer);
	}

	if(SUCCEEDED(hr)) {
		printf("Wrote %d bytes of audio data.\n", cbAudioData);

		//*pcbDataWritten = cbAudioData;
	}

	if(pAudioData) {
		pBuffer->Unlock();
	}

	SafeRelease(&pBuffer);
	SafeRelease(&pSample);
	//return hr;

	return buffer;
}

#endif // USE_WINDOWS_MEDIA_PACK_FEATURES