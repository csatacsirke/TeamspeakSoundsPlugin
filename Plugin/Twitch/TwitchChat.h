#pragma once

#include <memory>
#include <string>
#include <optional>

namespace TSPlugin::TwitchChat {

    struct TwitchResponse {
        std::optional<std::string> chatResponseText;
    };

    struct ITwitchMessageHandler {
        virtual TwitchResponse OnTwitchMessage(const std::string& channel, const std::string& sender, const std::string& message) = 0;
        virtual ~ITwitchMessageHandler() = default;
    };

    struct ITwitchChatReader {
        virtual void Start(ITwitchMessageHandler& handler, const std::string_view channel, const std::string_view password) = 0;
        virtual void Stop() = 0;
        virtual void SendChannelMessage(const char* message) = 0;
        virtual ~ITwitchChatReader() = default;
    };

    std::shared_ptr<ITwitchChatReader> CreateTwitchChatReader();

}