#pragma once

#include <Util/Util.h>

namespace TSPlugin::Http {

	using namespace std;

	static const CString GET = L"GET";
	static const CString POST = L"POST";


	struct HttpOptions {
		CString verb = GET;
		bool useHttps = false;
		CString headers = L"";
	};

	struct HttpResponse {
		DWORD statusCode = 0;
		vector<uint8_t> body;
		CString internalError;
		int win32_error = 0;
	};


	HttpResponse HttpRequest(const CString& serverUrl, const CString& serverObject, const HttpOptions& options = {});
	optional<vector<uint8_t>> SimpleHttpRequest(const CString& serverUrl, const CString& serverObject);

}
