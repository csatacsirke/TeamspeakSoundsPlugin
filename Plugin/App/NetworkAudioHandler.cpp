#include "stdafx.h"
#include "NetworkAudioHandler.h"

namespace TSPlugin {

	//NetworkAudioHandler::NetworkAudioHandler(const shared_ptr<AudioBuffer>& captureBuffer, const shared_ptr<AudioBuffer>& playbackBuffer) 
	//: CaptureBuffer(captureBuffer), PlaybackBuffer(playbackBuffer)
	//{
	//	// null
	//}

	shared_ptr<AudioBuffer> NetworkAudioHandler::GetCaptureBuffer() const {
		return CaptureBuffer;
	}


	shared_ptr<AudioBuffer> NetworkAudioHandler::GetPlaybackBuffer() const {
		return PlaybackBuffer;
	}


	void NetworkAudioHandler::Listen() {

	}


	void NetworkAudioHandler::TryConnect() {

	}

} // namespace TSPlugin 

