#include "stdafx.h"

#include "OnlineMicrophone.h"


OnlineMicrophone::OnlineMicrophone() {
	listenerSocket = ListenerSocket::Create(27015);
	listenerThread.reset(new std::thread([&] {
		Listen();
	}));
}

OnlineMicrophone::~OnlineMicrophone() {
	if(listenerThread) {
		listenerThread->join();
	}

	if(readerThread) {
		readerThread->join();
	}
}

void OnlineMicrophone::OnConnect(std::shared_ptr<ClientSocket> clientSocket) {
	if(!clientSocket) {
		this->clientSocket = clientSocket;
		readerThread.reset(new std::thread([&] {
			std::shared_ptr<std::vector<byte>> buffer;
			while(clientSocket->Receive(*buffer)) {
				this->buffers.push(buffer);
			}
		}));
	}
}

void OnlineMicrophone::Listen() {
	listenerSocket->Listen([&](std::shared_ptr<ClientSocket> clientSocket) {
		OnConnect(clientSocket);
	});
}

bool OnlineMicrophone::TryGetSamples(std::vector<byte>& buffer) {
	if(!buffers.empty()) {
		buffer.swap(*buffers.front());
		buffers.pop();
		return true;
	}

	return false;
}

