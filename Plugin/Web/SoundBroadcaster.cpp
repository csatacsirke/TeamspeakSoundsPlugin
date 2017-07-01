#include "stdafx.h"
#include "SoundBroadcaster.h"

#include <WinSock2.h>

// global singleton
class WSAHandler {
	WSADATA wsaData;
public:
	WSAHandler() {

		WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	~WSAHandler() {

		WSACleanup();
	}
private:
	static WSAHandler wsaHandlerInstance;
};

// Global singleton
WSAHandler WSAHandler::wsaHandlerInstance;


//
//
//
//
//
//SoundBroadcaster::SoundBroadcaster() {
//
//}
//
//
//SoundBroadcaster::~SoundBroadcaster() {
//	closesocket(sock);
//}
//
//void SoundBroadcaster::Init() {
//
//	sock = socket(AF_INET, SOCK_DGRAM, 0);
//	if(sock == INVALID_SOCKET) {
//		throw Exception("socket creation");
//	}
//
//	BOOL enabled = TRUE;
//	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(BOOL)) < 0) {
//		throw Exception("broadcast options");
//	}
//
//	
//	Sender_addr.sin_family = AF_INET;
//	Sender_addr.sin_port = htons(SoundBroadcaster::port);
//	Sender_addr.sin_addr.s_addr = INADDR_BROADCAST;
//	//Sender_addr.sin_addr.s_addr = inet_addr("192.168"); 
//
//	//struct sockaddr_in Recv_addr;
//	//Recv_addr.sin_family = AF_INET;
//	//Recv_addr.sin_port = htons(SoundBroadcaster::port);
//	//Recv_addr.sin_addr.s_addr = INADDR_ANY;
//	//
//	//if(bind(sock, (sockaddr*)&Recv_addr, sizeof(Recv_addr)) < 0) {
//	//	throw Exception("bind");
//	//}
//
//
//
//}
//
//struct UdpPacket {
//	
//	std::vector<byte> buffer;
//	UdpPacket(CachedAudioSample48k sample) {
//		size_t size = sample.GetDataSize();
//		buffer.resize(sample.GetDataSize() + sizeof(size));
//		
//		memcpy(buffer.data(), &size, sizeof(size));
//		memcpy(buffer.data() + sizeof(size), sample.GetData(), size);
//	}
//
//	const char* GetData() {
//		return (const char*)buffer.data();
//	}
//
//	size_t GetDataSize() {
//		return buffer.size();
//	}
//
//	
//
//};
//
//void SoundBroadcaster::SendPacket(CachedAudioSample48k sample) {
//	
//	UdpPacket packet(sample);
//	if(sendto(sock, packet.GetData(), (int)packet.GetDataSize(), 0, (sockaddr *)&Sender_addr, sizeof(Sender_addr)) < 0) {
//		throw Exception("Broadcasting failed.");
//	}
//
//	
//}
//
//
//void SoundReceiver::Init() {
//
//	sock = socket(AF_INET, SOCK_DGRAM, 0);
//	if(sock == INVALID_SOCKET) {
//		throw Exception("socket creation");
//	}
//
//	BOOL enabled = TRUE;
//	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&enabled, sizeof(BOOL)) < 0) {
//		throw Exception("broadcast options");
//	}
//
//
//	//Sender_addr.sin_family = AF_INET;
//	//Sender_addr.sin_port = htons(SoundBroadcaster::port);
//	//Sender_addr.sin_addr.s_addr = INADDR_BROADCAST;
//	//Sender_addr.sin_addr.s_addr = inet_addr("192.168"); 
//
//	struct sockaddr_in Recv_addr;
//	Recv_addr.sin_family = AF_INET;
//	Recv_addr.sin_port = htons(SoundBroadcaster::port);
//	Recv_addr.sin_addr.s_addr = INADDR_ANY;
//	
//
//	if(bind(sock, (sockaddr*)&Recv_addr, sizeof(Recv_addr)) < 0) {
//		throw Exception("bind");
//	}
//
//}
//
//CachedAudioSample48k SoundReceiver::ReceivePacket() {
//	size_t size;
//	recvfrom(sock, (char*)&size, sizeof(size), MSG_PEEK, nullptr, 0);
//	// és mi a fasz van, ha több ember broadcastol egyszerre?p
//	//static_assert(false); // ez szar
//	return nullptr;
//}
//
//
//
//
//
