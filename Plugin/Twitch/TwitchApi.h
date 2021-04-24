#pragma once

#include <Util/Atomic.h>

#include <nlohmann/json.hpp>

#include <optional>
#include <memory>
#include <mutex>



namespace TSPlugin::Twitch {

	using namespace std;
	using Json = nlohmann::json;



	struct ValidationInfo {
		CString clientId;
		CString clientName;
		CString userId;
		int expiresIn;
	};


	struct TwitchState {
		Atomic<CString> session;
		Atomic<CString> accessToken;
		std::shared_ptr<const ValidationInfo> validationInfo;
	};


	// returns session
	bool StartUserLogin(TwitchState& twitchState);
	// returns access token
	bool PollAccessToken(TwitchState& twitchState);
	// returns access token
	bool RefreshAccessToken(TwitchState& twitchState);

	bool ValidateAccessToken(TwitchState& twitchState);



	struct Reward {
		std::string id;
		std::string title;
	};

	struct RewardRedemption {
		std::string user_name;
		std::string id;
		std::string user_input;
		Reward reward;
	};


	bool CreateReward(TwitchState& twitchState);
	bool UpdateReward(TwitchState& twitchState, const CString& rewardId, const Json& params);
	bool DeleteReward(TwitchState& twitchState, const CString& rewardId);
	optional<Json> GetRewards(TwitchState& twitchState);


	optional<vector<RewardRedemption>> GetUnfulfilledRedemptions(TwitchState& twitchState, const CString& rewardId);

	bool ConfirmRewardRedemption(TwitchState& twitchState, const RewardRedemption& redemption);


};

