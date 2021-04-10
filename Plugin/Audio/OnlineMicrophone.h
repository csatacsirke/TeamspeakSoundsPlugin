#pragma once



#include <Util\NetUtil.h>

#include <stdint.h>

namespace TSPlugin {

	using namespace Net;

	class OnlineMicrophone {
		Wsa wsa;
		std::shared_ptr<ListenerSocket> listenerSocket;
		std::unique_ptr<std::thread> listenerThread;
		std::shared_ptr<ClientSocket> clientSocket;
		std::unique_ptr<std::thread> readerThread;

		std::queue<std::shared_ptr<std::vector<uint8_t>>> buffers;
	public:
		OnlineMicrophone();
		~OnlineMicrophone();

		bool TryGetSamples(std::vector<uint8_t>& buffer);

	private:
		void Listen();
		void OnConnect(std::shared_ptr<ClientSocket> clientSocket);
	};


}