#pragma once




class AudioProcessor {
	
public:
	bool Process(short* samples, int sampleCount, int channels);

	//void Enable();
	//void Disable();
public:
	volatile bool enabled = false;

	// elv 20 ms-t kapunk
	volatile int durationMs = 20;
	volatile int windowLengthMicroSec = 200;
private:
	
	std::vector<short> sampleBuffer;
	std::vector<short> previousSampleBuffer;
};

