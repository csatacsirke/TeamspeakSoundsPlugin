#pragma once


class AudioProcessor {
	bool enabled = false;
public:
	bool Process(short* samples, int sampleCount, int channels);

	void Enable();
	void Disable();

};


