#pragma once


#pragma comment(lib, "Winhttp.lib")
#include <winhttp.h>

namespace Web {

	static const CString GET = L"GET";
	static const CString POST = L"POST";

	//enum Exception {
	//	NullHandleException, 
	//	FalseReturnException, 
	//	WinHttpReadDataException, 
	//	WinHttpQueryDataAvailableException
	//};

	class SmartBoolResult {
	public:
		SmartBoolResult(bool value) {
			*this = value;
		}

		SmartBoolResult(BOOL value) {
			*this = value;
		}

		SmartBoolResult& operator = (const bool& value) {
			if(value == false) {
				throw Exception(GetLastError());
			}
			return *this;
		}

		SmartBoolResult& operator = (const BOOL& value) {
			if(value == FALSE) {
				throw Exception(GetLastError());
			}
			return *this;
		}

		operator bool() {
			return true;
		}

		operator BOOL() {
			return TRUE;
		}
	};

	
	class SmartHandle {
		HINTERNET handle = NULL;
	public:
		SmartHandle() {}

		SmartHandle(const HINTERNET& handle) {
			*this = handle;
		}

		SmartHandle& operator = (const HINTERNET& handle) {
			if(handle == NULL) {
				throw Exception(GetLastError());
			}
			this->handle = handle;
			return *this;
		}

		~SmartHandle() {
			if(handle != NULL) {
				WinHttpCloseHandle(handle);
			}
		}

		operator HINTERNET() {
			return handle;
		}

		operator bool() {
			return handle != NULL;
		}

	};


	class Http {

		SmartHandle hSession;
		SmartHandle hConnect;
	public:
		Http(CString url) {
			hSession = WinHttpOpen(
				L"A WinHTTP Example Program/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0
			);

			hConnect = WinHttpConnect(hSession, url, INTERNET_DEFAULT_PORT, 0);

			

		}

		Buffer Get(CString object) {

			
			SmartHandle hRequest = WinHttpOpenRequest(hConnect, GET, object, NULL, NULL, NULL, 0);;

			SmartBoolResult bResults = WinHttpSendRequest(
				hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS,
				0, 
				WINHTTP_NO_REQUEST_DATA, 
				0,
				0,
				0
			);

			
			bResults = WinHttpReceiveResponse(hRequest, NULL);
			

			// Keep checking for data until there is nothing left.

			Buffer buffer;

			DWORD dwSize = 0;
			DWORD dwDownloaded = 0;
			//LPSTR pszOutBuffer;
			do {
				// Check for available data.
				dwSize = 0;
				if(!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
					throw Exception(GetLastError());
					//printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
				}
				
				std::vector<uint8_t> chunkBuffer(dwSize + 1);
				if(!WinHttpReadData(hRequest, (LPVOID)chunkBuffer.data(), dwSize, &dwDownloaded)) {
					throw Exception(GetLastError());
				}
				
				buffer.Append(chunkBuffer.data(), dwSize);

				// Allocate space for the buffer.
				//pszOutBuffer = new char[dwSize + 1];
				//if(!pszOutBuffer) {
				//	printf("Out of memory\n");
				//	dwSize = 0;
				//} else {
				//	// Read the data.
				//	ZeroMemory(pszOutBuffer, dwSize + 1);

				//	if(!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
				//		printf("Error %u in WinHttpReadData.\n", GetLastError());
				//	} else {
				//		printf("%s", pszOutBuffer);

				//	}

				//	// Free the memory allocated to the buffer.
				//	delete[] pszOutBuffer;
				//}
			} while(dwSize > 0);


			return buffer;
		}

		
	};

}


