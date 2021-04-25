#include <stdafx.h>

#include "TwitchPubSub.h"
#include "TwitchApi.h"

#include "TwitchChat.h"

#include <Util/Log.h>
#include <Util/Util.h>

#include <nlohmann/json.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <regex>

namespace TSPlugin::TwitchPubSub {

    using namespace std;
    using namespace Twitch;
    using Json = nlohmann::json;


    namespace beast = boost::beast;         // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
    namespace net = boost::asio;            // from <boost/asio.hpp>
    using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>




    struct TwitchPubSub : public ITwitchPubSub {
        TwitchPubSub(ITwitchPubSubMessageHandler& handler, shared_ptr<TwitchState> state);
        ~TwitchPubSub();
        void Start() override;
        void Stop() override;

    private:
        bool Run();
        void HandleRead(const std::string_view message);
        void Authenticate();
        void Write(const Json& json);
    private:
        void Ping();
    private:
        ITwitchPubSubMessageHandler& handler;
        shared_ptr<TwitchState> twitchState;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ ioc };

        // The SSL context is required, and holds certificates
        boost::asio::ssl::context ssl_ctx{ boost::asio::ssl::context::tlsv12_client };

        // the websocket
        //std::shared_ptr<websocket::stream<tcp::socket>> ws = make_shared<websocket::stream<tcp::socket>>(ioc);
        std::shared_ptr<websocket::stream<beast::ssl_stream<tcp::socket>>> ws = make_shared<websocket::stream<beast::ssl_stream<tcp::socket>>>(ioc, ssl_ctx);


        std::thread backgroundThread;
        std::mutex _mutex;
    };

    std::shared_ptr<ITwitchPubSub> CreateTwitchPubSub(ITwitchPubSubMessageHandler& handler, shared_ptr<TwitchState> state) {
        ASSERT(state->validationInfo);
        if (!state->validationInfo) {
            return nullptr;
        }

        return std::make_shared<TwitchPubSub>(handler, state);
    }

    TwitchPubSub::TwitchPubSub(ITwitchPubSubMessageHandler& handler, shared_ptr<TwitchState> state) : handler(handler), twitchState(state) {
        // void
    }

    TwitchPubSub::~TwitchPubSub() {
        Stop();
    }

    void TwitchPubSub::Start() {
        // just in case
        Stop();

        std::lock_guard guard(_mutex);

        // need to copy, can't be sure if string_view is persistent
        backgroundThread = std::thread([this]{
            Run();
        });
    }

    void TwitchPubSub::Stop() {
        std::lock_guard guard(_mutex);


        if (ws->is_open()) {
            //ws->close(websocket::close_code::normal);
            ws->next_layer().next_layer().close();
        }

        if (backgroundThread.joinable()) {
            backgroundThread.join();
        }

    }

    void TwitchPubSub::HandleRead(const std::string_view message) {
        handler.OnTwitchPubSubMessage(message);

        const Json json = Json::parse(message);
        if (json["type"] == "PING") {
            Ping();
            return;
        }

        if (json["type"] == "reward-redeemed") {
            try {
                const RewardRedemption rewardRedemption = json["data"]["redemption"].get<RewardRedemption>();
                handler.OnTwitchChannelPointRedemption(rewardRedemption);
            } catch (Json::exception) {
                // ...
            }
            return;
        }

        return;
    }

    void TwitchPubSub::Write(const Json& json) {
        ASSERT(ws->is_open());
        std::string jsonString = json.dump();
        //cout << "write: " << jsonString << endl;
        ws->write(net::buffer(jsonString));
        ioc.run_one();
    }

    void TwitchPubSub::Ping() {
        Json json;
        json["type"] = "PING";
        Write(json);
    }

    void TwitchPubSub::Authenticate() {

        /*
        // Request from client to server
        {
          "type": "LISTEN",
          "nonce": "44h1k13746815ab1r2",
          "data": {
            "topics": ["channel-bits-events-v1.44322889"],
            "auth_token": "cfabdegwdoklmawdzdo98xt2fo512y"
          }
        }
        */
        // for channel points:
        // channel-points-channel-v1.<channel_id>

        auto validationInfo = twitchState->validationInfo;
        ASSERT(validationInfo);
        if (!validationInfo) {
            throw std::exception();
            //return;
        }
        
        {
            std::string userId = ConvertUnicodeToUTF8(validationInfo->userId).GetString();
            Json listenRequest;
            listenRequest["type"] = "LISTEN";
            //listenRequest["nonce"] = RandomNonce(); // optional
            listenRequest["data"]["topics"] = { format_string("channel-points-channel-v1.%s", userId.c_str()) };
            listenRequest["data"]["auth_token"] = ConvertUnicodeToUTF8(twitchState->accessToken);

            Write(listenRequest);
        }
        
    }

    bool TwitchPubSub::Run() {
        try {

            std::string host = "pubsub-edge.twitch.tv";
            //auto const port = "80";
            auto const port = "443";

            // Look up the domain name
            auto const results = resolver.resolve(host, port);

            // Make the connection on the IP address we get from a lookup
            auto ep = net::connect(ws->next_layer().next_layer(), results);

            {
                // Set SNI Hostname (many hosts need this to handshake successfully)
                if (!SSL_set_tlsext_host_name(ws->next_layer().native_handle(), host.c_str()))
                    throw beast::system_error(
                        beast::error_code(
                            static_cast<int>(::ERR_get_error()),
                            net::error::get_ssl_category()),
                        "Failed to set SNI Hostname");
            }


            // Update the host_ string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            host += ':' + std::to_string(ep.port());

            // ssl handshake
            ws->next_layer().handshake(boost::asio::ssl::stream_base::client);

            // Set a decorator to change the User-Agent of the handshake
            ws->set_option(websocket::stream_base::decorator(
                [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
            }));

            // Perform the websocket handshake
            ws->handshake(host, "/");

            Ping();
            Authenticate();

            // Send the message
            //ws->write(net::buffer(std::string(text)));

            // This buffer will hold the incoming message
            while (ws->is_open()) {
                // Read a message into our buffer
                beast::flat_buffer buffer;
                ws->read(buffer);
                HandleRead(std::string_view((const char*)buffer.data().data(), buffer.data().size()));
            }

            // Close the WebSocket connection
            ws->close(websocket::close_code::normal);


            // If we get here then the connection is closed gracefully

        } catch (std::exception const& e) {
            //std::cout << "Error: " << e.what() << std::endl;
            Log::Warning(CString("Websocket error: ") + CString(e.what()));
            return false;
        }

        return true;
    }


}

