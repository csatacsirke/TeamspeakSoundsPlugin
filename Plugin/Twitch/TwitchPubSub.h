#pragma once

#include "TwitchApi.h"

#include <memory>
#include <string>
#include <optional>


namespace TSPlugin::TwitchPubSub {

    using RewardRedemption = Twitch::RewardRedemption;

    struct ITwitchPubSubMessageHandler {
        virtual void OnTwitchChannelPointRedemption(const RewardRedemption& rewardRedemption) = 0;
        virtual void OnTwitchPubSubMessage(std::string_view json) {}; // not pure virtual
    };

    struct ITwitchPubSub {
        virtual void Start() = 0;
        virtual void Stop() = 0;
    };

    std::shared_ptr<ITwitchPubSub> CreateTwitchPubSub(ITwitchPubSubMessageHandler& handler, shared_ptr<Twitch::TwitchState> state);
}

