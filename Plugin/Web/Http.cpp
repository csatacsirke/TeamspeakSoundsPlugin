#include "stdafx.h"
#include "Http.h"

#pragma comment(lib, "Winhttp.lib")
#include <winhttp.h>

namespace TSPlugin::Http {

	HttpResponse Error(const CString& internalError) {
		const int error = GetLastError();
		return HttpResponse{
			.internalError = internalError,
			.win32_error = error,
		};
	}

	HttpResponse HttpRequest(const CString& serverUrl, const CString& serverObject, const HttpOptions& options) {

		HINTERNET hSession = WinHttpOpen(
			L"A WinHTTP Example Program/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0
		);

		if (!hSession) {
			return Error(L"WinHttpOpen");
		}

		WinHttpSetTimeouts(
			hSession, 5, 5, 5, 5
		);


		Finally closeSession([&] {
			WinHttpCloseHandle(hSession);
		});

		HINTERNET hConnect = WinHttpConnect(hSession, serverUrl, INTERNET_DEFAULT_PORT, 0);
		if (!hConnect) {
			return Error(L"WinHttpConnect");
		}

		Finally closeConnection([&] {
			WinHttpCloseHandle(hConnect);
		});

		DWORD flags = 0;
		if (options.useHttps) {
			flags |= WINHTTP_FLAG_SECURE;
		}

		HINTERNET hRequest = WinHttpOpenRequest(hConnect, options.verb, serverObject, NULL, NULL, NULL, flags);
		Finally closeRequest([&] {
			WinHttpCloseHandle(hRequest);
		});

		//if (options.useHttps) {
		//	DWORD dwReqOpts = 0;
		//	DWORD dwSize = sizeof(DWORD);
		//	WinHttpSetOption(
		//		hRequest,
		//		WINHTTP_OPTION_SECURITY_FLAGS,
		//		&dwReqOpts,
		//		sizeof(DWORD));
		//}
		//

		const BOOL bSendRequestResult = WinHttpSendRequest(
			hRequest,
			options.headers,
			-1,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0
		);

		if (!bSendRequestResult) {
			return Error(L"WinHttpSendRequest");
		}

		const BOOL bReceiveResponseResult = WinHttpReceiveResponse(hRequest, NULL);
		if (!bReceiveResponseResult) {
			return Error(L"WinHttpReceiveResponse");
		}


		DWORD dwStatusCode = 0;
		DWORD dwSizeStatusCode = sizeof(dwStatusCode);

		WinHttpQueryHeaders(hRequest,
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&dwStatusCode, &dwSizeStatusCode, WINHTTP_NO_HEADER_INDEX);

		vector<uint8_t> buffer;
		DWORD dwSize = 0;
		DWORD totalDataDownloaded = 0;
		do {
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				return Error(L"WinHttpQueryDataAvailable");
			}

			buffer.resize(totalDataDownloaded + dwSize, 0);


			DWORD dwDownloaded;
			if (!WinHttpReadData(hRequest, (LPVOID)(buffer.data() + totalDataDownloaded), dwSize, &dwDownloaded)) {
				return Error(L"WinHttpReadData");
			}

			totalDataDownloaded += dwDownloaded;

		} while (dwSize > 0);


		return HttpResponse{
			.statusCode = dwStatusCode,
			.body = std::move(buffer),
		};
		//CString result = Utf8ToCString((const char*)buffer.data());


		//return CStringA((const char*)buffer.data(), buffer.size());
	}

	optional<vector<uint8_t>> SimpleHttpRequest(const CString& serverUrl, const CString& serverObject) {
		HINTERNET hSession = WinHttpOpen(
			L"A WinHTTP Example Program/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0
		);

		if (!hSession) {
			return nullopt;
		}

		Finally closeSession([&] {
			WinHttpCloseHandle(hSession);
		});

		HINTERNET hConnect = WinHttpConnect(hSession, serverUrl, INTERNET_DEFAULT_PORT, 0);
		if (!hConnect) {
			const int error = GetLastError();
			return nullopt;
		}

		Finally closeConnection([&] {
			WinHttpCloseHandle(hConnect);
		});

		HINTERNET hRequest = WinHttpOpenRequest(hConnect, GET, serverObject, NULL, NULL, NULL, 0);
		Finally closeRequest([&] {
			WinHttpCloseHandle(hRequest);
		});

		const BOOL bSendRequestResult = WinHttpSendRequest(
			hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0
		);
		if (!bSendRequestResult) {
			return nullopt;
		}

		const BOOL bReceiveResponseResult = WinHttpReceiveResponse(hRequest, NULL);
		if (!bReceiveResponseResult) {
			return nullopt;
		}


		vector<uint8_t> buffer;
		DWORD dwSize = 0;
		DWORD totalDataDownloaded = 0;
		do {
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
				return nullopt;
			}

			buffer.resize(totalDataDownloaded + dwSize, 0);


			DWORD dwDownloaded;
			if (!WinHttpReadData(hRequest, (LPVOID)(buffer.data() + totalDataDownloaded), dwSize, &dwDownloaded)) {
				return nullopt;
			}

			totalDataDownloaded += dwDownloaded;

		} while (dwSize > 0);


		return buffer;
		//CString result = Utf8ToCString((const char*)buffer.data());


		//return CStringA((const char*)buffer.data(), buffer.size());
	}

}
