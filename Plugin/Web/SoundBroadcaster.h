#pragma once

#include <Wave\AudioBuffer.h>
#include <Strsafe.h>
#include <Ws2tcpip.h>

class SoundBroadcaster
{
	SOCKET sock;
	struct sockaddr_in Sender_addr;
public:
	static const short port = 27016;

	SoundBroadcaster();
	~SoundBroadcaster();

	void Init();


	void SendPacket(CachedAudioSample48k sample);
};


class SoundReceiver {
	SOCKET sock;
public:
	void Init();

	CachedAudioSample48k ReceivePacket();
};




struct RawUdpPacket {
	static const size_t maxPacketSize = 100;

	TCHAR buffer[maxPacketSize];

	RawUdpPacket() {
		NULL;
	}

	RawUdpPacket(CString message) {
		StringCbCopy(buffer, maxPacketSize, message);
	}

	CString ToString() {
		return CString(buffer);
	}

};

class UdpBroadcaster
{
	SOCKET sock;
	struct sockaddr_in Sender_addr;
	
public:
	

	//virtual short Port() = NULL;

	UdpBroadcaster() {
		Init();
	}
	~UdpBroadcaster() {
		closesocket(sock);
	}

	void Init() {

		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock == INVALID_SOCKET) {
			Log::Warning(L"socket creation");
			//throw Exception("socket creation");
		}

		BOOL enabled = TRUE;
		if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(BOOL)) < 0) {
			Log::Warning(L"broadcast options");
			//throw Exception("broadcast options");
		}

	}



	void BroadcastMessage(CString message, short port) {


		Sender_addr.sin_family = AF_INET;
		Sender_addr.sin_port = htons(port);
		Sender_addr.sin_addr.s_addr = INADDR_BROADCAST;

		RawUdpPacket packet(message);

		const int asdf = sizeof(packet);
		if(sendto(sock, (const char*)&packet, sizeof(packet), 0, (sockaddr *)&Sender_addr, sizeof(Sender_addr)) < 0) {
			Log::Warning(L"UdpBroadcaster::BroadcastMessage - sendto failed");
			//throw Exception("UdpB failed.");
		}
	}

	
};

class UdpReceiver {
	SOCKET udpSocket;
	std::mutex listenMutex;
public:


	UdpReceiver() {
		//Init();
		NULL;
	}
	~UdpReceiver() {
		Close();
		std::unique_lock<std::mutex> closeLock(listenMutex);
	}


	struct Packet {
		CString message;
		sockaddr_in sender;
		int result;
		int errorCode;
	};





	void ListenAsyc(short port) {
		//Close(); // ez mi a kurva anyjáért volt itt?

		
		//Init();
		std::thread listenerThread([&, port] {
			std::unique_lock<std::mutex> listenLock(listenMutex);


			udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
			if(udpSocket == INVALID_SOCKET) {
				Log::Warning(L"socket creation");
			}

			BOOL enabled = TRUE;
			if(setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(BOOL)) < 0) {
				Log::Warning(L"broadcast options");
			}


			//Sender_addr.sin_family = AF_INET;
			//Sender_addr.sin_port = htons(SoundBroadcaster::port);
			//Sender_addr.sin_addr.s_addr = INADDR_BROADCAST;
			//Sender_addr.sin_addr.s_addr = inet_addr("192.168"); 

			struct sockaddr_in Recv_addr;
			Recv_addr.sin_family = AF_INET;
			Recv_addr.sin_port = htons(port);
			Recv_addr.sin_addr.s_addr = INADDR_ANY;


			if(::bind(udpSocket, (sockaddr*)&Recv_addr, sizeof(Recv_addr)) < 0) {
				Log::Warning(L"bind");
				//throw Exception("bind");
			}

			Packet packet;
			while(true) {
				bool success = ReceivePacket(packet);
				if(!success) {
					break;
				}

				OnPacketReceived(packet);

			}
		});


		listenerThread.detach();

	}

	

	bool ReceivePacket(Packet& packet) {


		RawUdpPacket rawPacket;
		int senderSize = sizeof(packet.sender);
		int result = recvfrom(udpSocket, (char*)&rawPacket, sizeof(rawPacket), 0, (sockaddr*)&packet.sender, &senderSize);
		int debug = ntohs(packet.sender.sin_port);
		if(result == SOCKET_ERROR) {
			int errorCode = WSAGetLastError();
			Log::Warning(L"UdpReceiver::RevievePacket - revfrom failed");
			//message = L"recvfrom failed. (this is not the actual message)";
			return false;
		} else {
			packet.message = rawPacket.ToString();
			return true;
		}



		//RawUdpPacket packet;
		//recvfrom(sock, (char*)&packet, sizeof(packet), 0, nullptr, 0);
		//Packet packet;
		//packet.ReceiveFrom(sock);
		//return packet;
		//return packet.ToString();
	}

	virtual void OnPacketReceived(Packet& packet) = NULL;

protected:

	//void Init() {


	//}

	void Close() {
		closesocket(udpSocket);
	}

};




namespace UdpMessages {
	static const CString IWannaStream("i wanna stream");
	static const CString IAmHere("i am here");
	static const CString WhoIsHere("who is here");
}

class PresenceBroadcaster : public UdpBroadcaster {

	//static const int port = 27017;
	

public:


	void QueryDevices(short port) {
		BroadcastMessage(UdpMessages::WhoIsHere, port);
	}

	void RespondToQuery(short port) {
		BroadcastMessage(UdpMessages::IAmHere, port);
	}



};

class PresenceListener : public UdpReceiver {

	
	//std::mutex listenMutex;
public:
	PresenceListener() {
		//ListenAsyc();
		NULL;
	}

	void OnPacketReceived(Packet& packet) override {
		if(packet.message == UdpMessages::WhoIsHere) {
			if(OnPresenceQuery) {
				OnPresenceQuery(packet);
			}
		}

		if(packet.message == UdpMessages::IAmHere) {
			if(OnPresenceResponse) {
				OnPresenceResponse(packet);
			}
		}
	}
	//

	//void ListenAsyc() {
	//	//Close(); // ez mi a kurva anyjáért volt itt?

	//	std::thread listenerThread([&] {
	//		std::unique_lock<std::mutex> listenMutex;

	//		Packet packet;
	//		while(true) {
	//			bool success = ReceivePacket(packet);
	//			if(!success) {
	//				break;
	//			}

	//			if(packet.message == UdpMessages::WhoIsHere) {
	//				if(OnPresenceQuery) {
	//					OnPresenceQuery(packet);
	//				}
	//			}

	//			if(packet.message == UdpMessages::IAmHere) {
	//				if(OnPresenceResponse) {
	//					OnPresenceResponse(packet);
	//				}
	//			}
	//		}
	//	});
	//	

	//	listenerThread.detach();
	//	

	//	
	//}

	//void Close() {
	//	CloseHandle(handle);
	//	handle = NULL;
	//}

	std::function<void(Packet& packet)> OnPresenceQuery;
	std::function<void(Packet& packet)> OnPresenceResponse;
};



class TcpStreamer {
	//int address;
	//short port;

	SOCKET listenerSocket = INVALID_SOCKET;
	std::vector<SOCKET> clientSockets;
	SOCKET serverSocket = INVALID_SOCKET;
	std::mutex listenMutex;
public:

	TcpStreamer() {
		NULL;
	}


	~TcpStreamer() {
		closesocket(listenerSocket);
		std::unique_lock<std::mutex> listenLock(listenMutex);
	}


	void Listen(short port) {
		//this->port = port;

		// from cppreference.com
		// [&, i]{}; // ok: by-reference capture, except i is captured by copy
		// The reaching scope of a local lambda expression is the set of enclosing
		// scopes up to and including the innermost enclosing function and its parameters.
		// TODO "listenerSocket" ref szerint van átadva ami veszélyes lehet, de csaknem....
		std::thread listenerThread([&, port] {
			std::unique_lock<std::mutex> listenLock(listenMutex);

			listenerSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(listenerSocket == INVALID_SOCKET) {
				throw Exception("socket creation");
			}

			struct sockaddr_in Recv_addr;
			Recv_addr.sin_family = AF_INET;
			Recv_addr.sin_port = htons(port);
			Recv_addr.sin_addr.s_addr = INADDR_ANY;

			int result = ::bind(listenerSocket, (sockaddr*)&Recv_addr, sizeof(Recv_addr));

			if(result == SOCKET_ERROR) {
				throw Exception("tcp bind failed");
			}

			::listen(listenerSocket, SOMAXCONN);

			while(true) {
				//sockaddr_in clientAddress;
				sockaddr clientAddress;
				SOCKET clientSocket = ::accept(listenerSocket, &clientAddress, nullptr);

				if(INVALID_SOCKET == clientSocket) {
					// TODO Terminate() fv
					int errorCode = WSAGetLastError();
					CString errorMsg = L"Accpet failed: " + ToString(errorCode);
					Log::Warning(errorMsg);
					break;
				}
				Log::Debug(L"Accepted a client");
				clientSockets.push_back(clientSocket);

			}
		});
		listenerThread.detach();

	}

	void SendToAllClients(void* data, size_t size) {
		for(SOCKET clientSocket : clientSockets) {
			int result;

			result = send(clientSocket, (char*)&size, sizeof(size), NULL);
			if(result == SOCKET_ERROR) {
				int errorCode = WSAGetLastError();
				CString errorMsg = ToString(errorCode);
				Log::Error(L"SendToAllClients failed" + errorMsg);
			}



			result = send(clientSocket, (char*)data, size, NULL);
			if(result == SOCKET_ERROR) {
				int errorCode = WSAGetLastError();
				CString errorMsg = ToString(errorCode);
				Log::Error(L"SendToAllClients failed(2)" + errorMsg);
			}
		}
		

	}


};


class TcpReceiver {

	SOCKET clientSocket;
	std::mutex connectMutex;
public:
	typedef std::shared_ptr<std::vector<byte>> Packet;

	TcpReceiver() {

	}

	~TcpReceiver() {
		closesocket(clientSocket);
		// megvárjuk, hogy vége legyen a threadnek
		std::unique_lock<std::mutex> connectLock(connectMutex);
	}


	void Connect(CStringA host, int port, std::function<void(Packet data)> callback) {


		std::thread connectThread([=] {
			

			std::unique_lock<std::mutex> connectLock(connectMutex);


			clientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(clientSocket == INVALID_SOCKET) {
				int errorCode = WSAGetLastError();
				Log::Warning(L"TcpReceiver::Connect - socket: " + ToString(errorCode));
				return;
			}



			sockaddr_in serverAddress;
			serverAddress.sin_family = AF_INET;
			//serverAddress.sin_addr.s_addr = inet_addr("localhost");
			serverAddress.sin_addr.s_addr = inet_addr(host);
			serverAddress.sin_port = htons(port);

			int result = ::connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
			if(result != 0) {
				int errorCode = WSAGetLastError();
				Log::Warning(L"TcpReceiver::Connect - conenct: " + ToString(errorCode));
				return;
			}


			bool success = true;
			
			do {
				Packet data = Receive();
				if(data) {
					if(callback) {
						callback(data);
					}
				} else {
					Log::Warning(L"TcpReceiver::Connect - Receive");
					success = false;
				}

			} while(success);



		});
		connectThread.detach();

		

	}
private:
	Packet Receive() {
		size_t size;
		int result;

		result = ::recv(clientSocket, (char*)&size, sizeof(size), 0);
		if(SOCKET_ERROR == result) {
			int errorCode = WSAGetLastError();
			Log::Warning(L"TcpReceiver::Receive - recv: " + ToString(errorCode));
			return nullptr;
		}

		Packet buffer(new std::vector<byte>(size));

		result = ::recv(clientSocket, (char*)buffer->data(), buffer->size(), 0);
		if(SOCKET_ERROR == result) {
			int errorCode = WSAGetLastError();
			Log::Warning(L"TcpReceiver::Receive - recv(2): " + ToString(errorCode));
			return nullptr;
		}
		return buffer;
	}


};

//
//
//class NetworkHandler {
//	PresenceBroadcaster broadcaster;
//	PresenceListener udpListener;
//
//	TcpStreamer tcpStreamer;
//	TcpReceiver tcpReveiver;
//
//
//	const short udpPort = 27017;
//	const short tcpPort = 27018;
//public:
//	typedef IN_ADDR DeviceInfo;
//
//	std::set<DeviceInfo> availableAddresses;
//
//	std::function<void()> OnNewDevice;
//
//public:
//	void Init() {
//		udpListener.ListenAsyc(udpPort);
//		tcpStreamer.Listen(tcpPort);
//
//		// ez igy elég gagyisztikus... nem biztos hogy kell ehhez callback...
//		udpListener.OnPresenceQuery = [&](PresenceListener::Packet& packet) {
//			broadcaster.RespondToQuery(udpPort);
//		};
//
//
//		udpListener.OnPresenceResponse = [&](PresenceListener::Packet& packet) {
//			sockaddr_in& sender = (sockaddr_in&)packet.sender;
//			//TCHAR ip_str[INET_ADDRSTRLEN];
//			//InetNtop(AF_INET, &(sender.sin_addr), ip_str, INET_ADDRSTRLEN);
//			//std::wcout << ip_str << ":" << sender.sin_port << " " << (const wchar_t*)packet.message << std::endl;
//
//			IN_ADDR newAddress = packet.sender.sin_addr;
//			auto result = availableAddresses.insert(newAddress);
//			// The pair::second element in the pair is set to true if a new element was inserted or false if an equivalent element already existed.
//			if(result.second) {
//				if(OnNewDevice) {
//					OnNewDevice();
//				}
//			}
//		};
//
//		
//	}
//
//	void RefreshDevices() {
//		broadcaster.QueryDevices(udpPort);
//	}
//
//};



////
//
//namespace UdpMessages {
//	static const CString unmute("unmute");
//	static const CString remute("remute");
//}
//
//class UnmuteBroadcaster : UdpBroadcaster {
//	
//	static const int port = 27017;
//	int Port() override {
//		return port;
//	}
//
//public:
//	
//	void UnmuteMe() {
//		BroadcastMessage(UdpMessages::unmute);
//	}
//
//	void RemuteMe() {
//		BroadcastMessage(UdpMessages::remute);
//	}
//
//
//};
//
//class UnmuteReceiver : UdpReceiver{
//
//};




