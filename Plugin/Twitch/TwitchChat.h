#pragma once

#include <memory>
#include <string>

namespace TSPlugin::TwitchChat {

    struct IHandler {
        virtual void OnTwitchMessage(const std::string& channel, const std::string& sender, const std::string& message) = 0;
        virtual ~IHandler() = default;
    };

    struct ITwitchChatReader {
        virtual void Start(IHandler& handler, const std::string_view channel, const std::string_view password) = 0;
        virtual void Stop() = 0;
        virtual ~ITwitchChatReader() = default;
    };

    std::shared_ptr<ITwitchChatReader> CreateTwitchChatReader();

}