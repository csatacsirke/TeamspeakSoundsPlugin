#include "stdafx.h"
#include "PipeHandler.h"

#include <vector>
#include <thread>

PipeHandler::PipeHandler(void)
{
}


PipeHandler::~PipeHandler(void)
{
	Stop();
}

// ez m�g nem t�k�letes, de c�lnak megfelel :(
bool PipeHandler::ListenPipe(CString pipeName) {
	
	if( !mutex.try_lock() ) {
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
	mutex.try_lock();
	mutex.unlock();
}

void PipeHandler::SetOnNewEntryListener(std::function<void(PipeHandler&)> callback) {
	this->OnNewEntry = callback;
}

bool PipeHandler::TryPop(_Out_ KeyData& keyData) {
	auto result = queue.try_pop(keyData);
	return result;
}


void PipeHandler::Push(const KBDLLHOOKSTRUCT& hookStruct) {
	KeyData keyData = KeyData::CreateFromHookData(hookStruct);
	queue.push(keyData);
}


// TODO ha egyszer kurvasok id�d lesz, debugold ki
// hogy mi�rt continue kell oda �s mi�rt nem lehet 
// norm�lisan lekezelni a kurva hib�t....
BOOL PipeHandler::RunPipe(CString pipeName) {
	
	while(!this->stop){
		// Create a pipe 
		HANDLE pipe = CreateNamedPipe(
			pipeName, // name of the pipe
			PIPE_ACCESS_INBOUND, // 1-way pipe -- recv only
			PIPE_TYPE_BYTE, // send data as a byte stream
			1, // only allow 1 instance of this pipe
			0, // no outbound buffer
			1024*5, // inbound buffer
			0, // use default wait time
			NULL // use default security attributes
			);

		if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
			int asd = GetLastError();
			MessageBox(0,TEXT("Failed to create inbound pipe instance."),0,0);
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
			std::wcout << L"ConnectNamedPipe error " << error << std::endl;
			//return FALSE;
			continue;
		}


		while(!this->stop) {

			KBDLLHOOKSTRUCT hookStruct;
			DWORD nReadBytes;
			BOOL result = ReadFile(
				pipe,
				&hookStruct, // the data from the pipe will be put here
				sizeof(hookStruct), // number of bytes allocated
				&nReadBytes, // this will store number of bytes actually read
				NULL // not using overlapped IO
				);

			if( result!=FALSE && nReadBytes == sizeof(hookStruct) ){
				this->Push(hookStruct);
				// callback method
				if(this->OnNewEntry) {
					// TODO antihat�kony
					//std::thread callback([&]() {
						OnNewEntry(*this);
					//});
					//callback.detach();
				}
			} else {
				int error = GetLastError();
				//std::wcout << L"Pipe read error " << error << std::endl;
				//std::wcout << std::endl << "result: " << result << " nBytes: " << nReadBytes << "size: " << sizeof(hookStruct) ;
				//MessageBox(0,TEXT("Unexpected exception @ readnamedpipe"),0,0);
				break;
			}
		}

		// Close the pipe (automatically disconnects client too)
		CloseHandle(pipe);
	}
	return TRUE;
}
