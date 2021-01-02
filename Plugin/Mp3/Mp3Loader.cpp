#include "stdafx.h"

#include <mfreadwrite.h>
#include <mfapi.h>

#include <Wave/wave.h>

namespace TSPlugin {

	namespace Mp3 {


		//-------------------------------------------------------------------
		// ConfigureAudioStream
		//
		// Selects an audio stream from the source file, and configures the
		// stream to deliver decoded PCM audio.
		//-------------------------------------------------------------------
		static HRESULT ConfigureAudioStreamForRawOutput(
			IMFSourceReader* pReader,   // Pointer to the source reader.
			IMFMediaType** ppPCMAudio   // Receives the audio format.
		) {
			HRESULT hr = S_OK;

			CComPtr<IMFMediaType> pUncompressedAudioType = NULL;
			CComPtr<IMFMediaType> pPartialType = NULL;

			// Create a partial media type that specifies uncompressed PCM audio.

			hr = MFCreateMediaType(&pPartialType);

			if (SUCCEEDED(hr)) {
				hr = pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			}

			if (SUCCEEDED(hr)) {
				hr = pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
			}

			// Set this type on the source reader. The source reader will
			// load the necessary decoder.
			if (SUCCEEDED(hr)) {
				hr = pReader->SetCurrentMediaType(
					(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
					NULL,
					pPartialType
				);
			}

			// Get the complete uncompressed format.
			if (SUCCEEDED(hr)) {
				hr = pReader->GetCurrentMediaType(
					(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
					&pUncompressedAudioType
				);
			}

			// Ensure the stream is selected.
			if (SUCCEEDED(hr)) {
				hr = pReader->SetStreamSelection(
					(DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
					TRUE
				);
			}

			// Return the PCM format to the caller.
			if (SUCCEEDED(hr)) {
				*ppPCMAudio = pUncompressedAudioType;
				(*ppPCMAudio)->AddRef();
			}

			return hr;
		}


		static HRESULT ReadAllDataFromSource(IMFSourceReader* pReader) {
			HRESULT hr = S_OK;
			DWORD cbAudioData = 0;
			DWORD cbBuffer = 0;
			BYTE* pAudioData = NULL;

			CComPtr<IMFSample> pSample = NULL;
			CComPtr<IMFMediaBuffer> pBuffer = NULL;

			// Get audio samples from the source reader.
			while (true) {
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

				if (FAILED(hr)) {
					break;
				}

				if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
					printf("Type change - not supported by WAVE file format.\n");
					break;
				}
				if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
					printf("End of input file.\n");
					break;
				}

				if (pSample == NULL) {
					printf("No sample\n");
					continue;
				}

				// Get a pointer to the audio data in the sample.

				hr = pSample->ConvertToContiguousBuffer(&pBuffer);

				if (FAILED(hr)) {
					break;
				}


				hr = pBuffer->Lock(&pAudioData, NULL, &cbBuffer);

				if (FAILED(hr)) {
					break;
				}


				// Write this data to the output file.
				hr = WriteToFile(hFile, pAudioData, cbBuffer);

				if (FAILED(hr)) {
					break;
				}

				// Unlock the buffer.
				hr = pBuffer->Unlock();
				pAudioData = NULL;

				if (FAILED(hr)) {
					break;
				}

				// Update running total of audio data.
				cbAudioData += cbBuffer;

			}

			if (pAudioData) {
				pBuffer->Unlock();
			}

			return hr;
		}

		HRESULT LoadMp3(LPCWSTR wszSourceFile) {

			HRESULT hr = S_OK;

			// Initialize the COM library.
			hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

			// Intialize the Media Foundation platform.
			if (SUCCEEDED(hr)) {
				hr = MFStartup(MF_VERSION);
			}


			CComPtr<IMFSourceReader> pReader = NULL;
			// Create the source reader to read the input file.
			if (SUCCEEDED(hr)) {
				hr = MFCreateSourceReaderFromURL(
					wszSourceFile,
					NULL,
					&pReader
				);
			}


			// Represents the PCM audio format.
			CComPtr<IMFMediaType> pAudioType = NULL;
			if (SUCCEEDED(hr)) {
				// Configure the source reader to get uncompressed PCM audio from the source file.
				hr = ConfigureAudioStreamForRawOutput(pReader, &pAudioType);
			}



			
			CComHeapPtr<WAVEFORMATEX> pWav;
			if (SUCCEEDED(hr)) {
				UINT32 cbFormat = 0;
				hr = MFCreateWaveFormatExFromMFMediaType(
					pAudioType,
					&pWav,
					&cbFormat
				);
			}

			auto track = make_shared<WaveTrack>();
			track->format = *pWav;

			return hr;
		}

		std::shared_ptr<WaveTrack> LoadMp3File(const fs::path& path) {

		}
	}

}