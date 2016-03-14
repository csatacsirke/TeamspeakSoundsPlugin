#include "stdafx.h"
#include "PipeHandler.h"

#include <vector>

PipeHandler::PipeHandler(void)
{
}


PipeHandler::~PipeHandler(void)
{
	Stop();
}

// ez még nem tökéletes, de célnak megfelel :(
bool PipeHandler::ListenPipe(CString pipeName) {
	if( !mutex.try_lock() ) {
		return false;
	}
	
	this->stop = false;

	std::condition_variable wait;

	std::thread listenThread([&, pipeName]() {;
		RunPipe(pipeName);
	});

	listenThread.detach();
	
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
	return queue.try_pop(keyData);
}


void PipeHandler::Push(const KBDLLHOOKSTRUCT& hookStruct) {
	KeyData keyData(hookStruct);
	queue.push(keyData);
}


// TODO ha egyszer kurvasok idöd lesz, debugold ki
// hogy miért continue kell oda és miért nem lehet 
// normálisan lekezelni a kurva hibát....
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
			int asd = GetLastError();
			CloseHandle(pipe); // close the pipe
			//MessageBox(0,TEXT("Failed to make connection on named pipe."),0,0);
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
					OnNewEntry(*this);
				}
			} else {
				int error = GetLastError();
				//MessageBox(0,TEXT("Unexpected exception @ readnamedpipe"),0,0);
				break;
			}
		}

		// Close the pipe (automatically disconnects client too)
		CloseHandle(pipe);
	}
	return TRUE;
}

