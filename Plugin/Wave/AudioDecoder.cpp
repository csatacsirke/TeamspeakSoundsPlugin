#include "stdafx.h"

#include "AudioDecoder.h"
#include "AudioClipSample.h"


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
		throw Exception("GetHeader (audiodecoder) failed");
	}

	return header;
}

AudioDecoder::Buffer AudioDecoder::DecodeMillisecs(int msecAudioData) {
	Buffer buffer;

	HRESULT hr = S_OK;

	DWORD cbHeader = 0;         // Size of the WAVE file header, in bytes.
	DWORD cbAudioData = 0;      // Total bytes of PCM audio data written to the file.
	DWORD cbMaxAudioData = 0;



	// Calculate the maximum amount of audio to decode, in bytes.
	cbMaxAudioData = CalculateMaxAudioDataSize(pAudioType, cbHeader, msecAudioData);
	//cbMaxAudioData = 10 * 1024 * 1024;
	// Decode audio data to the file.
	buffer = GetNextChunk(cbMaxAudioData);

	
	//// Fix up the RIFF headers with the correct sizes.
	//if(SUCCEEDED(hr)) {
	//	hr = FixUpChunkSizes(hFile, cbHeader, cbAudioData);
	//}

	

	assert(SUCCEEDED(hr));

	return buffer;
}

// TODO rename
AudioDecoder::Buffer AudioDecoder::GetNextChunk( DWORD cbMaxAudioData) {
	HRESULT hr = S_OK;
	DWORD cbAudioData = 0;
	DWORD cbBuffer = 0;
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


		hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);

		if(FAILED(hr)) {
			break;
		}


		// Make sure not to exceed the specified maximum size.
		if(cbMaxAudioData - cbAudioData < cbBuffer) {
			cbBuffer = cbMaxAudioData - cbAudioData;
		}

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

		if(cbAudioData >= cbMaxAudioData) {
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

