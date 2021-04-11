#include "stdafx.h"
#include "TwitchChat.h"
#include <Util/Log.h>
#include <Util/Util.h>
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket client, synchronous
//
//------------------------------------------------------------------------------


#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <regex>


namespace TSPlugin::TwitchChat {

    // quick way to get token:
    // https://twitchapps.com/tmi/

    namespace beast = boost::beast;         // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
    namespace net = boost::asio;            // from <boost/asio.hpp>
    using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

    struct TwitchChatReader : public ITwitchChatReader {
        void Start(ITwitchMessageHandler& handler, const std::string_view channel, const std::string_view password) override;
        void Stop() override;

    private:
        bool Run(ITwitchMessageHandler& handler, const std::string_view channel, const std::string_view password);
        TwitchResponse HandleRead(ITwitchMessageHandler& handler, const std::string_view message);
        void Authenticate(const std::string_view channel, const std::string_view password);
        void Write(const std::string_view message);
    private:

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ ioc };

        // the websocket
        std::shared_ptr<websocket::stream<tcp::socket>> ws = make_shared<websocket::stream<tcp::socket>>(ioc);

        std::thread backgroundThread;

        std::mutex _mutex;
    };

    std::shared_ptr<ITwitchChatReader> CreateTwitchChatReader() {
        return std::make_shared<TwitchChatReader>();
    }

    void TwitchChatReader::Start(ITwitchMessageHandler& handler, const std::string_view channel, const std::string_view password) {
        std::lock_guard guard(_mutex);

        // need to copy, can't be sure if string_view is persistent
        backgroundThread = std::thread([this, &handler, channel=std::string(channel), password=std::string(password)] {
            Run(handler, channel, password);
        });
    }

    void TwitchChatReader::Stop() {
        std::lock_guard guard(_mutex);

        
        if (ws->is_open()) {
            //ws->close(websocket::close_code::normal);
            ws->next_layer().close();
        }

        if(backgroundThread.joinable()) {
            backgroundThread.join();
        }
        
    }

    TwitchResponse TwitchChatReader::HandleRead(ITwitchMessageHandler& handler, const std::string_view message) {
        if (message.starts_with("PING")) {
            ws->write(net::buffer("PONG :tmi.twitch.tv"));
        }

        /*
        battlechicken!battlechicken@battlechicken.tmi.twitch.tv PRIVMSG #bogeczki :dont even mind me, i'm a bot
        [D]>
        [D]:bogeczki!bogeczki@bogeczki.tmi.twitch.tv PRIVMSG #bogeczki :valami
        [D]>
        [D]:bapetta!bapetta@bapetta.tmi.twitch.tv PRIVMSG #bogeczki :valami
        [D]>
        [D]:bogeczki!bogeczki@bogeczki.tmi.twitch.tv PRIVMSG #bogeczki :xD
        [D]>
        [D]:bogeczki!bogeczki@bogeczki.tmi.twitch.tv PRIVMSG #bogeczki :hello there

        */

        Log::Debug(CString(L">") + Utf8ToCString(message.data()));
        //std::cout << ">" << message << endl;

        //Log::Debug(CString(L">") + Utf8ToCString(str.data()));

        std::regex re(R"""(.*!(\w+)@.*PRIVMSG #(\w+) :(.*))""");
        std::match_results<const char*> captures;

        if (std::regex_search(message.data(), captures, re)) {
        //if (std::regex_match(message.data(), captures, re)) {

            const std::string speaker = captures[1];
            const std::string channel = captures[2];
            const std::string message = captures[3];

            return handler.OnTwitchMessage(channel, speaker, message);
        }

        return {};
    }

    void TwitchChatReader::Write(const std::string_view message) {
        ws->write(net::buffer(message));
    }

    void TwitchChatReader::Authenticate(const std::string_view channel, const std::string_view password) {

        std::string message_auth;
        message_auth += "PASS ";
        message_auth += password;
        Write(message_auth);
        


        std::string message_nick;
        message_nick += "NICK ";
        message_nick += "battlechickenchatbot";
        message_nick += std::to_string(time(0));
        
        Write(message_nick);


        std::string message_join;
        message_join += "JOIN ";
        message_join += channel;
        Write(message_join);
        
    }

    bool TwitchChatReader::Run(ITwitchMessageHandler& handler, const std::string_view channel, const std::string_view password) {
        try {

            std::string host = "irc-ws.chat.twitch.tv";
            auto const port = "80";

            // Look up the domain name
            auto const results = resolver.resolve(host, port);

            // Make the connection on the IP address we get from a lookup
            auto ep = net::connect(ws->next_layer(), results);

            // Update the host_ string. This will provide the value of the
            // Host HTTP header during the WebSocket handshake.
            // See https://tools.ietf.org/html/rfc7230#section-5.4
            host += ':' + std::to_string(ep.port());

            // Set a decorator to change the User-Agent of the handshake
            ws->set_option(websocket::stream_base::decorator(
                [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
            }));

            // Perform the websocket handshake
            ws->handshake(host, "/");


            Authenticate(channel, password);

            // Send the message
            //ws->write(net::buffer(std::string(text)));

            // This buffer will hold the incoming message
            while (ws->is_open()) {
                // Read a message into our buffer
                beast::flat_buffer buffer;
                ws->read(buffer);
                const TwitchResponse response = HandleRead(handler, std::string_view((const char*)buffer.data().data(), buffer.data().size()));

                if (response.chatResponseText) {
                    // PRIVMSG #<channel> :This is a sample message
                    Write(format_string("PRIVMSG %s :%s", channel.data(), response.chatResponseText->c_str()));
                }
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
