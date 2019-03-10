#pragma once


#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


namespace TSPlugin {

	namespace Net {

		class Wsa {
			WSADATA wsaData;
			int iResult;
		public:
			Wsa() {
				// Initialize Winsock
				iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
				if (iResult != 0) {
					Log::Error(L"WSAStartup failed with error: " + ToString(iResult));
				}
			}

			~Wsa() {
				WSACleanup();
			}

		};


		class ClientSocket {
			SOCKET clientSocket;
		public:
			ClientSocket(SOCKET clientSocket) {
				this->clientSocket = clientSocket;
			}

			~ClientSocket() {
				shutdown(clientSocket, SD_SEND);
			}

			bool Receive(std::vector<uint8_t>& buffer) {
				uint64_t packetSize;
				int iResult = recv(clientSocket, (char*)&packetSize, sizeof(packetSize), 0);
				packetSize = ntohll(packetSize);
				if (iResult > 0) {
					buffer.resize(packetSize);
					iResult = recv(clientSocket, (char*)buffer.data(), (int)packetSize, 0);
					return (iResult == packetSize);
				}
				return false;
			}

		};

		class ListenerSocket {
			SOCKET listenerSocket;

			ListenerSocket(CString port) {
				ADDRINFOT hints;


				ZeroMemory(&hints, sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				hints.ai_flags = AI_PASSIVE;

				int iResult;


				// Resolve the server address and port
				//iResult = getaddrinfo(NULL, port, &hints, &result);
				ADDRINFOT* result = NULL;
				iResult = GetAddrInfo(NULL, port, &hints, &result);

				if (iResult != 0) {
					throw Exception(L"getaddrinfo failed with error: " + ToString(iResult));
				}



				// Create a SOCKET for connecting to server
				listenerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
				if (listenerSocket == INVALID_SOCKET) {
					throw Exception(L"socket failed with error: " + ToString(WSAGetLastError()));
				}

				// Setup the TCP listening socket
				iResult = ::bind(listenerSocket, result->ai_addr, (int)result->ai_addrlen);
				if (iResult == SOCKET_ERROR) {
					FreeAddrInfo(result);
					closesocket(listenerSocket);
					throw Exception(L"bind failed with error: " + ToString(WSAGetLastError()));
				}

				FreeAddrInfo(result);

				iResult = listen(listenerSocket, SOMAXCONN);
				if (iResult == SOCKET_ERROR) {
					closesocket(listenerSocket);
					throw Exception(L"listen failed with error: " + ToString(WSAGetLastError()));
				}
			}
		public:

			void Close() {
				closesocket(listenerSocket);
			}

			~ListenerSocket() {
				Close();
			}

		public:
			void Listen(std::function<void(std::shared_ptr<ClientSocket>)> OnNewConnectionCallback) {

				while (true) {
					// Accept a client socket
					SOCKET socket = accept(listenerSocket, NULL, NULL);
					if (socket == INVALID_SOCKET) {
						//throw Exception(L"accept failed with error: " + ToString(WSAGetLastError()));
						Log::Warning(L"accept failed with error: " + ToString(WSAGetLastError()));
						break;
					} else {
						OnNewConnectionCallback(std::shared_ptr<ClientSocket>(new ClientSocket(socket)));
					}
				}
			}

			static std::shared_ptr<ListenerSocket> Create(short port) {
				return Create(ToString(port));
			}

			static std::shared_ptr<ListenerSocket> Create(CString port) {

				try {
					std::shared_ptr<ListenerSocket> listenerSocket(new ListenerSocket(port));
					return listenerSocket;
				} catch (Exception e) {
					Log::Error(e.errorMessage);
					return NULL;
				}

			}

		};

	}

}
