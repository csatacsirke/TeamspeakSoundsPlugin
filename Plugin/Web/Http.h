#pragma once

#include <Util/Util.h>

namespace TSPlugin::Http {

	using namespace std;

	static const CString GET = L"GET";
	static const CString POST = L"POST";
	static const CString PATCH = L"PATCH";
	static const CString DELETE = L"DELETE";


	struct HttpOptions {
		CString verb = GET;
		bool useHttps = true;
		CString headers = L"";
		// can be binary
		std::string body;
		// .php?key=value
		std::map<CString, CString> queryParameters;
	};

	struct HttpResponse {
		DWORD statusCode = 0;
		vector<uint8_t> body;
		CString internalError;
		int win32_error = 0;
	};


	HttpResponse HttpRequest(const CString& serverUrl, const CString& serverObject, const HttpOptions& options);
	optional<vector<uint8_t>> SimpleHttpRequest(const CString& serverUrl, const CString& serverObject);

}
