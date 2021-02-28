#pragma once


#pragma comment(lib, "Winhttp.lib")
#include <winhttp.h>
#include <afx.h>

#include <Util/Util.h>

namespace TSPlugin {

	namespace Web {

		static const CString GET = L"GET";
		static const CString POST = L"POST";

		//enum Exception {
		//	NullHandleException, 
		//	FalseReturnException, 
		//	WinHttpReadDataException, 
		//	WinHttpQueryDataAvailableException
		//};

		//class SmartBoolResult {
		//public:
		//	SmartBoolResult(bool value) {
		//		*this = value;
		//	}

		//	SmartBoolResult(BOOL value) {
		//		*this = value;
		//	}

		//	SmartBoolResult& operator = (const bool& value) {
		//		if(value == false) {
		//			throw Exception(GetLastError());
		//		}
		//		return *this;
		//	}

		//	SmartBoolResult& operator = (const BOOL& value) {
		//		if(value == FALSE) {
		//			throw Exception(GetLastError());
		//		}
		//		return *this;
		//	}

		//	operator bool() {
		//		return true;
		//	}

		//	operator BOOL() {
		//		return TRUE;
		//	}
		//};

		//
		//class SmartHandle {
		//	HINTERNET handle = NULL;
		//public:
		//	SmartHandle() {}

		//	SmartHandle(const HINTERNET& handle) {
		//		*this = handle;
		//	}

		//	SmartHandle& operator = (const HINTERNET& handle) {
		//		if(handle == NULL) {
		//			throw Exception(GetLastError());
		//		}
		//		this->handle = handle;
		//		return *this;
		//	}

		//	~SmartHandle() {
		//		if(handle != NULL) {
		//			WinHttpCloseHandle(handle);
		//		}
		//	}

		//	operator HINTERNET() {
		//		return handle;
		//	}

		//	operator bool() {
		//		return handle != NULL;
		//	}

		//};

		optional<vector<uint8_t>> HttpRequest(const CString& serverUrl, const CString& serverObject) {
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

		//class Http {

		//	HINTERNET hSession;
		//	HINTERNET hConnect;
		//public:
		//	Http(CString url) {
		//		hSession = WinHttpOpen(
		//			L"A WinHTTP Example Program/1.0",
		//			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		//			WINHTTP_NO_PROXY_NAME,
		//			WINHTTP_NO_PROXY_BYPASS, 0
		//		);

		//		hConnect = WinHttpConnect(hSession, url, INTERNET_DEFAULT_PORT, 0);



		//	}

		//	Buffer Get(CString object) {


		//		SmartHandle hRequest = WinHttpOpenRequest(hConnect, GET, object, NULL, NULL, NULL, 0);;

		//		SmartBoolResult bResults = WinHttpSendRequest(
		//			hRequest,
		//			WINHTTP_NO_ADDITIONAL_HEADERS,
		//			0,
		//			WINHTTP_NO_REQUEST_DATA,
		//			0,
		//			0,
		//			0
		//		);


		//		bResults = WinHttpReceiveResponse(hRequest, NULL);


		//		// Keep checking for data until there is nothing left.

		//		Buffer buffer;

		//		DWORD dwSize = 0;
		//		DWORD dwDownloaded = 0;
		//		//LPSTR pszOutBuffer;
		//		do {
		//			// Check for available data.
		//			dwSize = 0;
		//			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
		//				throw Exception(GetLastError());
		//			}

		//			std::vector<uint8_t> chunkBuffer(dwSize + 1);
		//			if (!WinHttpReadData(hRequest, (LPVOID)chunkBuffer.data(), dwSize, &dwDownloaded)) {
		//				throw Exception(GetLastError());
		//			}

		//			buffer.Append(chunkBuffer.data(), dwSize);

		//		} while (dwSize > 0);


		//		return buffer;
		//	}


		//};

	}



}