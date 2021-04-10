#pragma once

#include <Audio/AudioBuffer.h>

#include <memory>

namespace TSPlugin {
	using namespace std;

	class NetworkAudioHandler {
	protected:
		shared_ptr<AudioBuffer> outboundAudioBuffer = make_shared<AudioBuffer>();
		shared_ptr<AudioBuffer> inboundAudioBuffer = make_shared<AudioBuffer>();

	public:

		virtual ~NetworkAudioHandler() = default;

		const shared_ptr<AudioBuffer>& GetOutboundAudioBuffer() { return outboundAudioBuffer; }
		const shared_ptr<AudioBuffer>& GetInboundAudioBuffer() { return inboundAudioBuffer; }

		virtual void StartService() = 0;
		virtual void Stop() = 0;

		static shared_ptr<NetworkAudioHandler> Create();
	};

} // namespace TSPlugin 

