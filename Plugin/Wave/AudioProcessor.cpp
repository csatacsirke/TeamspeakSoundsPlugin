#include "stdafx.h"

#include "AudioProcessor.h"


bool AudioProcessor::Process(
	short* samples, 
	int sampleCount, 
	int channels
) {
	if(!enabled) return false;
	Log::Debug(L"u");


	return true;
}

void AudioProcessor::Enable() {
	enabled = true;
}

void AudioProcessor::Disable() {
	enabled = false;
}

