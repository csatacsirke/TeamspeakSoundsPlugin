#pragma once

#include <Wave/AudioBuffer.h>

#include <memory>

namespace TSPlugin {
	using namespace std;

	class NetworkAudioHandler {
		shared_ptr<AudioBuffer> CaptureBuffer = make_shared<AudioBuffer>();
		shared_ptr<AudioBuffer> PlaybackBuffer = make_shared<AudioBuffer>();
	public:
		shared_ptr<AudioBuffer> GetCaptureBuffer() const;
		shared_ptr<AudioBuffer> GetPlaybackBuffer() const;

		void Listen();
		void TryConnect();
		//NetworkAudioHandler(const shared_ptr<AudioBuffer>& captureBuffer, const shared_ptr<AudioBuffer>& playbackBuffer);

	};

} // namespace TSPlugin 

