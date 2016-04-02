#pragma once




#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>

HRESULT WriteWaveFile(IMFSourceReader*, HANDLE, LONG);
HRESULT ConfigureAudioStream(IMFSourceReader*, IMFMediaType**);
HRESULT WriteWaveHeader(HANDLE, IMFMediaType*, DWORD*);
DWORD   CalculateMaxAudioDataSize(IMFMediaType*, DWORD, DWORD);
HRESULT WriteWaveData(HANDLE, IMFSourceReader*, DWORD, DWORD*);
HRESULT FixUpChunkSizes(HANDLE, DWORD, DWORD);
HRESULT WriteToFile(HANDLE, void*, DWORD);

template <class T> void SafeRelease(T **ppT) {
	if(*ppT) {
		(*ppT)->Release();
		*ppT = NULL;
	}
}
