#include "stdafx.h"


#include "NetworkAudioHandler.h"

//#include <WinSock2.h>
#include <asio.hpp>

#include <array>
#include <random>



namespace TSPlugin {

	//NetworkAudioHandler::NetworkAudioHandler(const shared_ptr<AudioBuffer>& captureBuffer, const shared_ptr<AudioBuffer>& playbackBuffer) 
	//: CaptureBuffer(captureBuffer), PlaybackBuffer(playbackBuffer)
	//{
	//	// null
	//}

	static const int UDP_PORT = 8889;
	static const int TCP_PORT = 8891;
	//static const int TCP_LOCAL_PORT = 8892;
	
	static uint64_t GenerateRandomUint64() {
		std::random_device rd;
		std::mt19937 mt(rd());
		const uint64_t random_number = uint64_t(mt()) + (uint64_t(mt()) << 4*8);
		return random_number;
	}

	uint64_t MAGIC_NUMBER = 0x5345341253425334;
	struct UdpQuery {
		uint64_t magic_number = 0;
		uint64_t random_number = 0;
	};

	//static bool operator==(const UdpQuery& a, const UdpQuery& b) {
	//	return (a.magic_number == b.magic_number && a.random_number == )
	//}

	const UdpQuery UDP_QUERY_MSG = { MAGIC_NUMBER, GenerateRandomUint64() };

	//static const std::string UDP_MSG_QUERY = "Are you there?";
	//static const std::string UDP_MSG_REPONSE = "Here I am";

	//using namespace asio;
	using asio::ip::udp;
	using asio::ip::tcp;
	using asio::ip::address;



	class NetworkAudioHandlerImpl : public NetworkAudioHandler {
		asio::io_service io_service;
		volatile bool run = true;
	public:
		void StartService() override;
		void Stop() override;

		~NetworkAudioHandlerImpl() override;
	private:

		void ListenUdp();
		void BroadcastUdp();

		void ConnectAndTransmitAudio(const asio::ip::address& address);
		void ListenForAudioTransmission();
		//NetworkAudioHandler(const shared_ptr<AudioBuffer>& captureBuffer, const shared_ptr<AudioBuffer>& playbackBuffer);

		void LoopTaskInBackground(const function<void()>& task);

		bool IsAddressLocalhost(const asio::ip::address& address);
	};




	shared_ptr<NetworkAudioHandler> NetworkAudioHandler::Create() {
		return make_shared<NetworkAudioHandlerImpl>();
	}

	NetworkAudioHandlerImpl::~NetworkAudioHandlerImpl() {
		Stop();
	}


	void NetworkAudioHandlerImpl::LoopTaskInBackground(const function<void()>& task) {
		std::thread([=] {
			while (run) {
				task();
				std::this_thread::sleep_for(1s);
			}
		}).detach();
	}


	void NetworkAudioHandlerImpl::StartService() {

		LoopTaskInBackground([&] {
			ListenUdp();
		});
		

		LoopTaskInBackground([&] {
			ListenForAudioTransmission();
		});

		LoopTaskInBackground([&] {
			BroadcastUdp();
		});


	}

	void NetworkAudioHandlerImpl::Stop() {
		run = false;
		io_service.stop();
	}


	bool NetworkAudioHandlerImpl::IsAddressLocalhost(const asio::ip::address& address) {
		
		try {

			
			asio::ip::tcp::resolver resolver(io_service);
			asio::ip::tcp::resolver::query query(address.to_string().c_str(), "80");
			//asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
			auto results = resolver.resolve(query);
			for (auto& result : results) {
				Log::Debug(FormatString(L"Local address: %S", result.endpoint().address().to_string().c_str()));
				if (result.endpoint().address() == address) {
					return true;
				}
			}

		} catch (const asio::system_error& error) {
			if (error.code().value() != 10004) {
				CString exceptionMessage(error.what());
				Log::Debug(exceptionMessage);
				assert(0);
			}
		}
		
		return false;
	}
	

	void NetworkAudioHandlerImpl::ListenUdp() {
		try {
			
			asio::ip::udp::socket socket(io_service);

			socket.open(udp::v4());
			socket.bind(udp::endpoint(asio::ip::address_v4::any(), UDP_PORT));
			
			
			udp::endpoint remote_endpoint;
			//std::string recv_buffer(100, '\0');
			

			//auto handle_receive = [&](const asio::error_code& error, size_t bytes_transferred) {
			//	if (!error) {
			//		Log::Debug(CString(recv_buffer.c_str()));
			//		if (recv_buffer == UDP_MSG_QUERY) {
			//			std::string address = remote_endpoint.address().to_string();
			//			std::thread([address, this] {
			//				ConnectAndTransmitAudio(address);
			//			}).detach();
			//		}
			//	}
			//};

			
			//socket.async_receive_from(
			//	asio::buffer(recv_buffer),
			//	remote_endpoint,
			//	handle_receive
			//);

			UdpQuery recv_buffer;

			const size_t received_size = socket.receive_from(
				asio::buffer(&recv_buffer, sizeof recv_buffer),
				remote_endpoint
			);

			//recv_buffer.resize(received_size);

			//Log::Debug(FormatString(L"Connecting to remote address: %S", remote_endpoint.address().to_string().c_str()));

			

			//!remote_endpoint.address().is_loopback();
			//const bool canConnectToThisAddress = !IsAddressLocalhost(remote_endpoint.address());

			//Log::Debug(FormatString(L"magic number: 0x%16llx random number: 0x%16llx", recv_buffer.magic_number, recv_buffer.random_number));

			if (recv_buffer.magic_number == UDP_QUERY_MSG.magic_number) {

				const bool canConnectToThisAddress = recv_buffer.random_number != UDP_QUERY_MSG.random_number;

				if (canConnectToThisAddress) {

					ConnectAndTransmitAudio(remote_endpoint.address());
				} else {
					//Log::Debug(L"Loopback address responded");
				}

				//std::string address = remote_endpoint.address().to_string();
				//std::thread([address, this] {
				//	ConnectAndTransmitAudio(address);
				//}).detach();
			}


			//io_service.run();
		} catch (const asio::system_error& error) {
			if (error.code().value() != 10004) {
				CString exceptionMessage(error.what());
				Log::Debug(exceptionMessage);
				assert(0);
			}
		}
		
	}

	void NetworkAudioHandlerImpl::BroadcastUdp() {
		try {
			asio::ip::udp::socket socket(io_service);
			socket.open(udp::v4());

			asio::socket_base::broadcast option(true);
			socket.set_option(option);

			

			udp::endpoint remote_endpoint(asio::ip::address_v4::broadcast(), UDP_PORT);
			//socket.bind(remote_endpoint);

			//asio::const_buffer buffer(UDP_MSG_QUERY.data(), UDP_MSG_QUERY.size());
			asio::const_buffer buffer(&UDP_QUERY_MSG, sizeof UDP_QUERY_MSG);
			socket.send_to(buffer, remote_endpoint);



		} catch (asio::system_error& error) {
			if (error.code().value() != 10004) {
				CString exceptionMessage(error.what());
				Log::Debug(exceptionMessage);
				assert(0);
			}
		}
	}

	void NetworkAudioHandlerImpl::ConnectAndTransmitAudio(const asio::ip::address& address) {

		try {
			Log::Debug(FormatString(L"Connecting to remote address: %S", address.to_string().c_str()));

			asio::ip::tcp::endpoint remote_endpoint(address, TCP_PORT);
			//asio::ip::tcp::endpoint local_endpoint(asio::ip::address_v4::any(), TCP_LOCAL_PORT);
			asio::ip::tcp::endpoint local_endpoint;

			asio::ip::tcp::socket socket(io_service);

			socket.open(tcp::v4());
			socket.bind(local_endpoint);

			socket.connect(remote_endpoint);


			Log::Debug(FormatString(L"Successfully connected to remote address: %S", address.to_string().c_str()));

			while (true) {
				auto track = outboundAudioBuffer->TryPopTrack();
				if (track) {


					Log::Debug(FormatString(L"Sending TCP data with size: %ld", track->data.size()));

					//const uint64_t headerSize = sizeof(track->format);
					//socket.send(asio::const_buffer(&headerSize, sizeof(headerSize)));

					//socket.send(asio::const_buffer(&track->format, sizeof(track->format)));
					asio::write(socket, asio::const_buffer(&track->format, sizeof(track->format)));

					const uint64_t dataSize = track->data.size();
					//socket.send(asio::const_buffer(&dataSize, sizeof(dataSize)));
					//socket.send(asio::const_buffer(track->data.data(), track->data.size()));

					asio::write(socket, asio::const_buffer(&dataSize, sizeof(dataSize)));
					asio::write(socket, asio::const_buffer(track->data.data(), track->data.size()));

					//Log::Debug(CString("Sent data with size: ") + ToString(track->data.size()));
				} else {
					std::this_thread::sleep_for(0.5s);
				}
			}

		} catch (asio::system_error& error) {
			if (error.code().value() != 10004) {
				CString exceptionMessage(error.what());
				Log::Debug(exceptionMessage);
				//assert(0);
			}
		}
	}


	void NetworkAudioHandlerImpl::ListenForAudioTransmission() {
		
		try {
			asio::ip::tcp::endpoint remote_endpoint(asio::ip::address_v4::any(), 0);
			asio::ip::tcp::endpoint local_endpoint(asio::ip::address_v4::any(), 0);

			
			tcp::acceptor acceptor(io_service);
			acceptor.open(tcp::v4());
			acceptor.bind(local_endpoint);
			acceptor.listen();


			asio::ip::tcp::socket socket(io_service);
			acceptor.accept(socket, remote_endpoint);

			Log::Debug(CString("Accepted: ") + CString(remote_endpoint.address().to_string().c_str()));

			while (true) {
				//vector<uint8_t> data;
				//socket.read_some(data);

				//uint64_t headerSize;
				//socket.receive(asio::mutable_buffer(&headerSize, sizeof(headerSize)));

				WAVEFORMATEX format;
				//socket.receive(asio::mutable_buffer(&format, sizeof(format)));
				asio::read(socket, asio::mutable_buffer(&format, sizeof(format)));
				

				uint64_t dataSize;
				//socket.receive(asio::mutable_buffer(&dataSize, sizeof(dataSize)));
				asio::read(socket, asio::mutable_buffer(&dataSize, sizeof(dataSize)));

				vector<uint8_t> data(dataSize);
				//socket.receive(asio::mutable_buffer(data.data(), data.size()));
				asio::read(socket, asio::mutable_buffer(data.data(), data.size()));


				Log::Debug(FormatString(L"Received TCP data with size: %ld", data.size()));


				auto track = WaveTrack::MakeFromData(format, std::move(data));
				inboundAudioBuffer->AddSamples(track);

				//shared_ptr<WaveTrack> track = make_shared<WaveTrack>();
				//track->data = std::move(data);
				//
				//PlaybackBuffer->AddSamples(track);
			}
			


		} catch (asio::system_error& error) {
			if (error.code().value() != 10004) {
				CString exceptionMessage(error.what());
				Log::Debug(exceptionMessage);
				assert(0);
			}
			
		}

		
	}

} // namespace TSPlugin 

