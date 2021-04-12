#include "stdafx.h"
#include "TwitchRewards.h"

#include <App/Globals.h>
#include <Util/Util.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>


namespace TSPlugin::TwitchRewards {

    const char* TWITCH_CLIENT_ID = "ht0rnyz1dwsiwne2vtcecmdyciqsp5";
    /*
     
    Creates a Custom Reward on a channel.
    Authentication

    Query parameter broadcaster_id must match the user_id in the User-Access token
    Authorization

    Requires OAuth Scope: channel:manage:redemptions
    URL

    POST https://api.twitch.tv/helix/channel_points/custom_rewards
    Pagination

    None.
    Required Query Parameter
    Parameter 	Type 	Description
    broadcaster_id 	string 	Provided broadcaster_id must match the user_id in the auth token.

    curl -X POST 'https://api.twitch.tv/helix/channel_points/custom_rewards?broadcaster_id=274637212' \
        -H 'client-id: gx2pv4208cff0ig9ou7nk3riccffxt' \
        -H 'Authorization: Bearer vjxv3i0l4zxru966wsnwji51tmpkj2' \
        -H 'Content-Type: application/json' \
        -d '{
            "title":"game analysis 1v1",
            "cost":50000
        }'
    */
    bool Create() {
        boost::system::error_code ec;
        using namespace boost::asio;

        // what we need
        io_service svc;
        ssl::context ctx(ssl::context::method::sslv23_client);
        ssl::stream<ip::tcp::socket> ssock(svc, ctx);
        ssock.lowest_layer().connect({ {}, 8087 }); // http://localhost:8087 for test
        ssock.handshake(ssl::stream_base::handshake_type::client);

        // send request
        std::string request("GET /newGame?name=david HTTP/1.1\r\n\r\n");
        boost::asio::write(ssock, buffer(request));

        // read response
        std::string response;

        do {
            char buf[1024];
            size_t bytes_transferred = ssock.read_some(buffer(buf), ec);
            if (!ec) response.append(buf, buf + bytes_transferred);
        } while (!ec);

        // print and exit
        std::cout << "Response received: '" << response << "'\n";

        return true;
    }


    bool RegisterMagnetLink() {
        const wchar_t* urlSchemeName = L"ts-twitch-integration";

        LSTATUS result = ERROR_SUCCESS;

        HKEY hUrlSchemeKey;
        if (result == ERROR_SUCCESS) {
            result = RegCreateKeyW(HKEY_CLASSES_ROOT, urlSchemeName, &hUrlSchemeKey);
        }

        if (result == ERROR_SUCCESS) {
            result = RegSetValueW(hUrlSchemeKey, nullptr, REG_SZ, L"Twitch ts integration plugin", 0);
        }

        HKEY hCommandKey;
        if (result == ERROR_SUCCESS) {
            result = RegCreateKeyW(hUrlSchemeKey, L"shell\\open\\command", &hCommandKey);
        }

        if (result == ERROR_SUCCESS) {
            const CString command = FormatString(L"echo \"%%1\" > \"%s\\%s\"", Global::configPath, L"oauth_token.txt");
            result = RegSetValueW(hCommandKey, nullptr, REG_SZ, command, 0);
        }

        return result == ERROR_SUCCESS;
    }
}
