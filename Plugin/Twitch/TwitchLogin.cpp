#include "stdafx.h"

#include "TwitchLogin.h"


#include <Util/Util.h>
#include <Web/Http.h>


#include <optional>
#include <string>
#include <functional>


namespace TSPlugin::Twitch {

	void ValidateToken(const wchar_t* token) {
		// curl -H "Authorization: OAuth --token--" https://id.twitch.tv/oauth2/validate
		using namespace Http;

		HttpOptions options{
			.verb = GET,
			.useHttps = true,
			.headers = {
				//FormatString(L"Authorization: OAuth %s", token),
			},
		};

		auto result = HttpRequest(L"id.twitch.tv", L"oauth2/validate");

		int a = 42;
		//return result;
	}



	void Login() {
		::ShellExecuteW(NULL, L"open", L"https://battlechicken.hu/twitch", NULL, NULL, SW_SHOWNORMAL);
	}


};
