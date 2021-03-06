#include "stdafx.h"
#include "PipeHandler.h"

#include <Util/Log.h>
#include <Util/Util.h>

#include <vector>
#include <thread>

namespace TSPlugin {

	PipeHandler::PipeHandler(void) {
	}


	PipeHandler::~PipeHandler(void) {
		Stop();
	}

	// ez m�g nem t�k�letes, de c�lnak megfelel :(
	bool PipeHandler::ListenPipe(CString pipeName) {

		if (!mutex.try_lock()) {
			return false;
		}

		this->stop = false;

		// ezt mi�rt is raktam ide?
		//std::condition_variable wait;

		std::thread listenerThread([&, pipeName]() {
			RunPipe(pipeName);
		});

		listenerThread.detach();

		return true;
	}

	void PipeHandler::Stop() {
		stop = true;

		// itt mi a fasz t�rt�nt?
		// ez vagy szar ahogy volt vagy useless
		//mutex.try_lock();
		//mutex.unlock();
	}

	void PipeHandler::SetOnNewEntryListener(std::function<void(PipeHandler&)> callback) {
		this->OnNewEntry = callback;
	}

	bool PipeHandler::TryPop(_Out_ KeyboardHook::KeyData& keyData) {
		auto result = queue.try_pop(keyData);
		return result;
	}


	//void PipeHandler::Push(const KBDLLHOOKSTRUCT& hookStruct) {
	void PipeHandler::Push(KeyboardHook::KeyData keyData) {
		queue.push(keyData);
	}


	// TODO ha egyszer kurvasok id�d lesz, debugold ki
	// hogy mi�rt continue kell oda �s mi�rt nem lehet 
	// norm�lisan lekezelni a kurva hib�t....
	BOOL PipeHandler::RunPipe(CString pipeName) {

		while (!this->stop) {
			// Create a pipe 
			HANDLE pipe = CreateNamedPipe(
				pipeName, // name of the pipe
				PIPE_ACCESS_INBOUND, // 1-way pipe -- recv only
				PIPE_TYPE_BYTE, // send data as a uint8_t stream
				1, // only allow 1 instance of this pipe
				0, // no outbound buffer
				1024 * 5, // inbound buffer
				0, // use default wait time
				NULL // use default security attributes
			);

			if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
				int asd = GetLastError();
				//MessageBox(0,TEXT("Failed to create inbound pipe instance."),0,0);
				Log::Error(L"Failed to create inbound pipe instance.");
				// look up error code here using GetLastError()
				return FALSE;
			}

			// This call blocks until a client process connects to the pipe
			BOOL result = ConnectNamedPipe(pipe, NULL);
			if (!result) {
				// look up error code here using GetLastError()
				int error = GetLastError();
				CloseHandle(pipe); // close the pipe
				//MessageBox(0,TEXT("Failed to make connection on named pipe."),0,0);
				//std::wcout << L"ConnectNamedPipe error " << error << std::endl;
				Log::Warning(CString(L"ConnectNamedPipe error ") + ToString(error));
				//return FALSE;
				continue;
			}


			while (!this->stop) {

				//KBDLLHOOKSTRUCT hookStruct;
				DWORD nReadBytes = 0;
				//BOOL result = ReadFile(
				//	pipe,
				//	&hookStruct, // the data from the pipe will be put here
				//	sizeof(hookStruct), // number of bytes allocated
				//	&nReadBytes, // this will store number of bytes actually read
				//	NULL // not using overlapped IO
				//	);


				KeyboardHook::KeyData keyData;
				volatile BOOL result = ReadFile(
					pipe,
					&keyData, // the data from the pipe will be put here
					sizeof(keyData), // number of bytes allocated
					&nReadBytes, // this will store number of bytes actually read
					NULL // not using overlapped IO
				);

				volatile const auto debug___ = sizeof(keyData);

				if (result != FALSE && nReadBytes == sizeof(keyData)) {

					CString str = keyData.unicodeLiteral;
					//
					//if(str.GetLength() > 0) {
					//	std::wcout << L">" << (const wchar_t*)keyData.unicodeLiteral;
					//	std::wcout << std::endl;
					//	if(std::wcout.fail()) {
					//		std::wcout.clear();
					//	}
					//} else {
					//	std::wcout << L"0";
					//}


					this->Push(keyData);
					// callback method
					if (this->OnNewEntry) {
						// TODO antihat�kony
						//std::thread callback([&]() {
						OnNewEntry(*this);
						//});
						//callback.detach();
					}
				} else {
					int error = GetLastError();
					if (error != ERROR_BROKEN_PIPE) {
						Log::Warning(CString(L"Pipe read error " + ToString(error)));
					}

					break;
					//std::wcout << L"Pipe read error " << error << std::endl;
					//std::wcout << std::endl << "result: " << result << " nBytes: " << nReadBytes << "size: " << sizeof(hookStruct) ;
					//MessageBox(0,TEXT("Unexpected exception @ readnamedpipe"),0,0);
					//break;
				}
			}

			// Close the pipe (automatically disconnects client too)
			//Log::Warning(L"PipeHandler: closing pipe");
			CloseHandle(pipe);
		}
		return TRUE;
	}

}
