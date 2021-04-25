#include "stdafx.h"

#include "TwitchApi.h"


#include <Util/Util.h>
#include <Web/Http.h>


#include <optional>
#include <string>
#include <functional>


namespace TSPlugin::Twitch {

	const Http::HttpOptions HttpsGet {
		.verb = Http::GET,
		.useHttps = true
	};

	bool ValidateAccessToken(TwitchState& twitchState) {
		// curl -H "Authorization: OAuth --token--" https://id.twitch.tv/oauth2/validate
		using namespace Http;

		HttpOptions options{
			.verb = GET,
			.useHttps = true,
			.headers = FormatString(L"Authorization: OAuth %s", twitchState.accessToken.Copy().GetString()),
		};
		/*
		{
			"client_id": "wbmytr93xzw8zbg0p1izqyzzc5mbiz",
			"login": "twitchdev",
			"scopes": [
				"channel:read:subscriptions"
			],
			"user_id": "141981764",
			"expires_in": 5520838
		}
		*/
		auto result = HttpRequest(L"id.twitch.tv", L"oauth2/validate", options);
		if (result.statusCode != 200) {
			return false;
		}

		try {
			auto jsonObject = nlohmann::json::parse(result.body);

			twitchState.validationInfo = std::make_shared<ValidationInfo>(ValidationInfo{
				.clientId = Utf8ToCString(jsonObject["client_id"].get<string>().c_str()),
				.clientName = Utf8ToCString(jsonObject["login"].get<string>().c_str()),
				.userId = Utf8ToCString(jsonObject["user_id"].get<string>().c_str()),
				.expiresIn = jsonObject["expires_in"].get<int>(),
			});
		} catch (nlohmann::json::type_error&) {
			return false;
		}

		return true;
	}

	bool PollAccessToken(TwitchState& twitchState) {
		using namespace Http;

		const CString serverObject = FormatString(L"twitch/poll_access_token.php?session=%s", twitchState.session.Copy().GetString());
		auto result = HttpRequest(L"battlechicken.hu", serverObject, HttpsGet);
		if (result.statusCode != 200) {
			return false;
		}

		twitchState.accessToken = Utf8ToCString(CStringA((const char*)result.body.data(), (int)result.body.size())).Trim();

		if (!Twitch::ValidateAccessToken(twitchState)) {
			if (!Twitch::RefreshAccessToken(twitchState)) {
				return FALSE;
			}

			if (!Twitch::ValidateAccessToken(twitchState)) {
				return FALSE;
			}
		}

		return true; 
	}

	bool RefreshAccessToken(TwitchState& twitchState) {
		using namespace Http;

		const CString serverObject = FormatString(L"twitch/refresh_session?session=%s", twitchState.session.Copy().GetString());
		auto result = HttpRequest(L"battlechicken.hu", serverObject, HttpsGet);
		if (result.statusCode != 200) {
			return false;
		}

		twitchState.accessToken = Utf8ToCString(CStringA((const char*)result.body.data(), (int)result.body.size())).Trim();

		return true;
	}



	bool StartUserLogin(TwitchState& twitchState) {
		using namespace Http;

		// http://battlechicken.hu/twitch/get_session.php
		auto result = HttpRequest(L"battlechicken.hu", L"twitch/start_session.php", HttpsGet);
		if (result.statusCode != 200) {
			return false;
		}

		const CStringA session_utf8((const char*)result.body.data(), (int)result.body.size());
		const CString session = Utf8ToCString(session_utf8).Trim();

		twitchState.session = session;

		const CString url = FormatString(L"https://battlechicken.hu/twitch?session=%s", session.GetString());

		::ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);

		return true;
	}

	CString SerializeHeaders(const vector<CString>& headers) {
		CString serializedHeader;
		for (const CString& header : headers) {
			if (serializedHeader.GetLength() > 0) {
				serializedHeader += L"\r\n";
			}
			serializedHeader += header;
		}
		return serializedHeader;
	}

	vector<uint8_t> CStringToUtf8Bytes(const wchar_t* text) {
		const CStringA utf8Text = ConvertUnicodeToUTF8(text);
		vector<uint8_t> buffer(utf8Text.GetLength());
		memcpy(buffer.data(), utf8Text.GetString(), buffer.size());
		return buffer;
	}

	bool UpdateReward(TwitchState& twitchState, const CString& rewardId, const Json& params) {

		auto validationInfo = twitchState.validationInfo;
		if (!validationInfo) {
			return false;
		}
		/*
		curl -X PATCH 'https://api.twitch.tv/helix/channel_points/custom_rewards?broadcaster_id=274637212&id=92af127c-7326-4483-a52b-b0da0be61c01' \
			-H 'client-id: gx2pv4208cff0ig9ou7nk3riccffxt' \
			-H 'Authorization: Bearer vjxv3i0l4zxru966wsnwji51tmpkj2' \
			-H 'Content-Type: application/json' \
			-d '{
						  "is_enabled": false
			 }'
		*/

		/*
		field                                 Required   Type       Description
		-------------------------------------------------------------------------------------------------
		title 	                                Yes 	string 	    The title of the reward
		prompt 	                                No  	string 	    The prompt for the viewer when they are redeeming the reward
		cost 	                                Yes 	integer 	The cost of the reward
		is_enabled 	                            No   	Boolean 	Is the reward currently enabled, if false the reward won’t show up to viewers. Defaults true
		background_color 	                    No 	    string 	    Custom background color for the reward. Format: Hex with # prefix. Example: #00E5CB.
		is_user_input_required 	                No 	    Boolean 	Does the user need to enter information when redeeming the reward. Defaults false
		is_max_per_stream_enabled 	            No 	    Boolean 	Whether a maximum per stream is enabled. Defaults to false.
		max_per_stream 	                        No 	    integer 	The maximum number per stream if enabled
		is_max_per_user_per_stream_enabled  	No 	    Boolean 	Whether a maximum per user per stream is enabled. Defaults to false.
		max_per_user_per_stream 	            No 	    integer 	The maximum number per user per stream if enabled
		is_global_cooldown_enabled 	            No 	    Boolean 	Whether a cooldown is enabled. Defaults to false.
		global_cooldown_seconds 	            No 	    integer 	The cooldown in seconds if enabled
		should_redemptions_skip_request_queue 	No 	    Boolean 	Should redemptions be set to FULFILLED status immediately when redeemed and skip the request
																	queue instead of the normal UNFULFILLED status. Defaults false

		*/


		const vector<CString> headers = {
			FormatString(L"client-id: %s", validationInfo->clientId.GetString()),
			FormatString(L"Authorization: Bearer %s", twitchState.accessToken.Copy()),
			L"Content-Type: application/json",
		};

		Http::HttpOptions options{
			.verb = Http::PATCH,
			.useHttps = true,
			.headers = SerializeHeaders(headers),
			.body = params.dump(),
			.queryParameters = {
				{L"broadcaster_id", validationInfo->userId},
				{L"id", rewardId},
			}
		};
		/*
		Response Codes
		HTTP Code 	Meaning
		200 	OK: A list of the Custom Rewards is returned
		400 	Bad Request: Query Parameter missing or invalid
		401 	Unauthenticated: Missing/invalid Token
		403 	Forbidden: Channel Points are not available for the broadcaster
		500 	Internal Server Error: Something bad happened on our side
		*/
		auto result = HttpRequest(L"api.twitch.tv", L"helix/channel_points/custom_rewards", options);
		std::string response_body((const char*)result.body.data(), result.body.size());
		if (result.statusCode != 200) {
			return false;
		}

		return true;
	}

	bool DeleteReward(TwitchState& twitchState, const CString& rewardId) {
		auto validationInfo = twitchState.validationInfo;
		if (!validationInfo) {
			return false;
		}
		/*
		curl -X DELETE 'https://api.twitch.tv/helix/channel_points/custom_rewards?broadcaster_id=274637212&id=b045196d-9ce7-4a27-a9b9-279ed341ab28' \
			-H 'Client-Id: gx2pv4208cff0ig9ou7nk3riccffxt' \
			-H 'Authorization: Bearer vjxv3i0l4zxru966wsnwji51tmpkj2'
		*/


		const vector<CString> headers = {
			FormatString(L"client-id: %s", validationInfo->clientId.GetString()),
			FormatString(L"Authorization: Bearer %s", twitchState.accessToken.Copy()),
		};

		Http::HttpOptions options{
			.verb = Http::DELETE,
			.useHttps = true,
			.headers = SerializeHeaders(headers),
			.queryParameters = {
				{L"broadcaster_id", validationInfo->userId},
				{L"id", rewardId},
			}
		};
		
		auto result = HttpRequest(L"api.twitch.tv", L"helix/channel_points/custom_rewards", options);
		std::string response_body((const char*)result.body.data(), result.body.size());
		if (result.statusCode != 200) {
			return false;
		}

		return true;
	}

	bool CreateReward(TwitchState& twitchState) {
		// POST https://api.twitch.tv/helix/channel_points/custom_rewards

		auto validationInfo = twitchState.validationInfo;
		if (!validationInfo) {
			return false;
		}
		/*
		curl -X POST 'https://api.twitch.tv/helix/channel_points/custom_rewards?broadcaster_id=274637212' \
		-H 'client-id: gx2pv4208cff0ig9ou7nk3riccffxt' \
		-H 'Authorization: Bearer vjxv3i0l4zxru966wsnwji51tmpkj2' \
		-H 'Content-Type: application/json' \
		-d '{
			"title":"game analysis 1v1",
			"cost":50000
		}'
		*/

		/*
		field                                 Required   Type       Description
		-------------------------------------------------------------------------------------------------
		title 	                                Yes 	string 	    The title of the reward
		prompt 	                                No  	string 	    The prompt for the viewer when they are redeeming the reward
		cost 	                                Yes 	integer 	The cost of the reward
		is_enabled 	                            No   	Boolean 	Is the reward currently enabled, if false the reward won’t show up to viewers. Defaults true
		background_color 	                    No 	    string 	    Custom background color for the reward. Format: Hex with # prefix. Example: #00E5CB.
		is_user_input_required 	                No 	    Boolean 	Does the user need to enter information when redeeming the reward. Defaults false
		is_max_per_stream_enabled 	            No 	    Boolean 	Whether a maximum per stream is enabled. Defaults to false.
		max_per_stream 	                        No 	    integer 	The maximum number per stream if enabled
		is_max_per_user_per_stream_enabled  	No 	    Boolean 	Whether a maximum per user per stream is enabled. Defaults to false.
		max_per_user_per_stream 	            No 	    integer 	The maximum number per user per stream if enabled
		is_global_cooldown_enabled 	            No 	    Boolean 	Whether a cooldown is enabled. Defaults to false.
		global_cooldown_seconds 	            No 	    integer 	The cooldown in seconds if enabled
		should_redemptions_skip_request_queue 	No 	    Boolean 	Should redemptions be set to FULFILLED status immediately when redeemed and skip the request 
																	queue instead of the normal UNFULFILLED status. Defaults false

		*/

		Json bodyJson;
		bodyJson["title"] = "Play sound";
		bodyJson["cost"] = 5000;
		bodyJson["is_global_cooldown_enabled"] = true;
		bodyJson["global_cooldown_seconds"] = 10;
		bodyJson["is_user_input_required"] = true;
		bodyJson["prompt"] = "Name of the sound to play?";


		const vector<CString> headers = {
			FormatString(L"client-id: %s", validationInfo->clientId.GetString()),
			FormatString(L"Authorization: Bearer %s", twitchState.accessToken.Copy()),
			L"Content-Type: application/json",
		};

		Http::HttpOptions options{
			.verb = Http::POST,
			.useHttps = true,
			.headers = SerializeHeaders(headers),
			.body = bodyJson.dump(),
			.queryParameters = {{L"broadcaster_id", validationInfo->userId}}
		};
		/*
		Response Codes
		HTTP Code 	Meaning
		200 	OK: A list of the Custom Rewards is returned
		400 	Bad Request: Query Parameter missing or invalid
		401 	Unauthenticated: Missing/invalid Token
		403 	Forbidden: Channel Points are not available for the broadcaster
		500 	Internal Server Error: Something bad happened on our side
		*/
		auto result = HttpRequest(L"api.twitch.tv", L"helix/channel_points/custom_rewards", options);
		std::string response_body((const char*)result.body.data(), result.body.size());
		if (result.statusCode != 200) {
			return false;
		}

		return true;
	}


	optional<Json> GetRewards(TwitchState& twitchState) {
		// POST https://api.twitch.tv/helix/channel_points/custom_rewards

		auto validationInfo = twitchState.validationInfo;
		if (!validationInfo) {
			return nullopt;
		}
		
		const vector<CString> headers = {
			FormatString(L"client-id: %s", validationInfo->clientId),
			FormatString(L"Authorization: Bearer %s", twitchState.accessToken.Copy()),
			L"Content-Type: application/json",
		};

		Http::HttpOptions options{
			.verb = Http::GET,
			.useHttps = true,
			.headers = SerializeHeaders(headers),
			.queryParameters = {
				{L"broadcaster_id", validationInfo->userId},
				{L"only_manageable_rewards", L"true"},
			}
		};
		/*
		Response Codes
		HTTP Code 	Meaning
		200 	OK: A list of the Custom Rewards is returned
		400 	Bad Request: Query Parameter missing or invalid
		401 	Unauthenticated: Missing/invalid Token
		403 	Forbidden: Channel Points are not available for the broadcaster
		500 	Internal Server Error: Something bad happened on our side
		*/
		auto result = HttpRequest(L"api.twitch.tv", L"helix/channel_points/custom_rewards", options);
		std::string response_body((const char*)result.body.data(), result.body.size());
		if (result.statusCode != 200) {
			return nullopt;
		}

		try {
			return Json::parse(result.body);
		} catch (Json::exception) {
			return nullopt;
		}
		
	}



	//optional<vector<RewardRedemption>> _GetUnfulfilledRedemptions(TwitchState& twitchState, const CString& rewardId) {
	//	try {
	//		auto json = GetUnfulfilledRedemptions(twitchState, rewardId);
	//		if (!json) {
	//			return nullopt;
	//		}

	//		vector<RewardRedemption> redemptions;


	//		return redemptions;
	//	} catch (Json::parse_error) {
	//		return nullopt;
	//	}
	//}


	optional<vector<RewardRedemption>> GetUnfulfilledRedemptions(TwitchState& twitchState, const CString& rewardId) {
		/*
		curl -X GET 'https://api.twitch.tv/helix/channel_points/custom_rewards/redemptions?broadcaster_id=274637212&reward_id=92af127c-7326-4483-a52b-b0da0be61c01&status=CANCELED' \
			-H 'Client-Id: gx2pv4208cff0ig9ou7nk3riccffxt' \
			-H 'Authorization: Bearer vjxv3i0l4zxru966wsnwji51tmpkj2'
		*/

		auto validationInfo = twitchState.validationInfo;
		if (!validationInfo) {
			return nullopt;
		}

		const map<CString, CString> queryParameters = {
			{L"broadcaster_id", validationInfo->userId}, 
			{L"reward_id", rewardId},
			{L"status", L"UNFULFILLED"},
		};

		const vector<CString> headers = {
			FormatString(L"client-id: %s", validationInfo->clientId),
			FormatString(L"Authorization: Bearer %s", twitchState.accessToken.Copy()),
			L"Content-Type: application/json",
		};

		Http::HttpOptions options{
			.verb = Http::GET,
			.useHttps = true,
			.headers = SerializeHeaders(headers),
			.queryParameters = queryParameters
		};

		auto result = HttpRequest(L"api.twitch.tv", L"helix/channel_points/custom_rewards/redemptions", options);
		std::string response_body((const char*)result.body.data(), result.body.size());
		if (result.statusCode != 200) {
			return nullopt;
		}

		try {
			vector<RewardRedemption> redemptions;
			Json::parse(result.body)["data"].get_to(redemptions);
			return redemptions;
			//return Json::parse(result.body);
		} catch (Json::exception) {
			return nullopt;
		}
	}

	bool ConfirmRewardRedemption(TwitchState& twitchState, const RewardRedemption& redemption) {
		/*
		curl --X PATCH 'https://api.twitch.tv/helix/channel_points/custom_rewards/redemptions?broadcaster_id=274637212&reward_id=92af127c-7326-4483-a52b-b0da0be61c01&id=17fa2df1-ad76-4804-bfa5-a40ef63efe63' \
			-H 'client-id: gx2pv4208cff0ig9ou7nk3riccffxt' \
			-H 'Authorization: Bearer vjxv3i0l4zxru966wsnwji51tmpkj2' \
			-H 'Content-Type: application/json' \
			-d '{
				"status":"CANCELED"
			}'
		*/


		auto validationInfo = twitchState.validationInfo;
		if (!validationInfo) {
			return false;
		}

		const map<CString, CString> queryParameters = {
			{L"broadcaster_id", validationInfo->userId},
			{L"id", Utf8ToCString(redemption.id.c_str())},
			{L"reward_id", Utf8ToCString(redemption.reward.id.c_str())},
		};

		const vector<CString> headers = {
			FormatString(L"client-id: %s", validationInfo->clientId),
			FormatString(L"Authorization: Bearer %s", twitchState.accessToken.Copy()),
			L"Content-Type: application/json",
		};

		Json bodyJson;
		bodyJson["status"] = "CANCELED"; // TODO: FULFILLED

		Http::HttpOptions options{
			.verb = Http::PATCH,
			.useHttps = true,
			.headers = SerializeHeaders(headers),
			.body = bodyJson.dump().c_str(),
			.queryParameters = queryParameters,
		};

		auto result = HttpRequest(L"api.twitch.tv", L"helix/channel_points/custom_rewards/redemptions", options);
		std::string response_body((const char*)result.body.data(), result.body.size());
		if (result.statusCode != 200) {
			return false;
		}

		return true;
	}
};
