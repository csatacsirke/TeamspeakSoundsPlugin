#pragma once

#include "NetworkAudioHandler.h"

#include <asio.hpp>


#include <array>
#include <future>
#include <random>
#include <list>



namespace TSPlugin {

	using asio::ip::udp;
	using asio::ip::tcp;
	using asio::ip::address;



	class NetworkAudioHandlerImpl : public NetworkAudioHandler {
		asio::io_service io_service;
		volatile bool run = true;
		list<shared_future<void>> taskResults;

		tcp::acceptor acceptor = tcp::acceptor(io_service);
		asio::ip::udp::socket udp_receiver_socket = asio::ip::udp::socket(io_service);
	public:
		void StartService() override;
		void Stop() override;

		~NetworkAudioHandlerImpl() override;
	private:

		void ListenUdp();
		void BroadcastUdp();
		void ListenForAudioTransmission();

		void ConnectAndTransmitAudio(const asio::ip::address& address);
		//NetworkAudioHandler(const shared_ptr<AudioBuffer>& captureBuffer, const shared_ptr<AudioBuffer>& playbackBuffer);

		void LoopTaskInBackground(const function<void()>& task);

		bool IsAddressLocalhost(const asio::ip::address& address);
	};

} // namespace TSPlugin



