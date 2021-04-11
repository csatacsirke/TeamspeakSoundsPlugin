#pragma once

#include <memory>
#include <string>

namespace TSPlugin::TwitchChat {

    struct ITwitchMessageHandler {
        virtual void OnTwitchMessage(const std::string& channel, const std::string& sender, const std::string& message) = 0;
        virtual ~ITwitchMessageHandler() = default;
    };

    struct ITwitchChatReader {
        virtual void Start(ITwitchMessageHandler& handler, const std::string_view channel, const std::string_view password) = 0;
        virtual void Stop() = 0;
        virtual ~ITwitchChatReader() = default;
    };

    std::shared_ptr<ITwitchChatReader> CreateTwitchChatReader();

}