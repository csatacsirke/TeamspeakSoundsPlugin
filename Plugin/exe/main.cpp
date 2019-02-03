#include "stdafx.h"

#ifndef _WINDLL

//#import "msxml6.dll"
//#import <Winhttp.lib>

#include "http.h"
#include <Wave\MP3Player.h>
#include <Wave\SignalProcessing.h>
#include <Web/SoundBroadcaster.h>
#include <Gui/NetworkDialog.h>
#include <conio.h>
#include <stdio.h>
#include <Wave/Steganography.h>


using namespace std;

void wmain() {

	AllocConsole();
	FILE* console_write = freopen("CONIN$", "r", stdin);
	FILE* console_read = freopen("CONOUT$", "w", stdout);
	
	const size_t size = 8000;
	short data[size];


	CStringA secret_orig = "lofasz";
	Steganography::WriteSecret(data, size, secret_orig);


	CStringA secret_result = Steganography::ReadSecret(data, size);

	bool success = secret_result == secret_orig;

	std::cout << success ? "jo" : "nem";



	getch();
}



void wmain__() {


	AllocConsole();
	FILE* console_write = freopen("CONIN$", "r", stdin);
	FILE* console_read = freopen("CONOUT$", "w", stdout);
	

	const short port = 27017;
	PresenceBroadcaster broadcaster;
	PresenceListener listener;

	listener.OnPresenceQuery = [&](PresenceListener::Packet& packet) {
		broadcaster.RespondToQuery(port);
	};


	listener.OnPresenceResponse = [&](PresenceListener::Packet& packet) {
		sockaddr_in& sender = (sockaddr_in&)packet.sender;

		TCHAR ip_str[INET_ADDRSTRLEN];
		
		InetNtop(AF_INET, &(sender.sin_addr), ip_str, INET_ADDRSTRLEN);

		std::wcout << ip_str << ":" << sender.sin_port << " " << (const wchar_t*)packet.message << std::endl;
	};

	listener.ListenAsyc(port);

	//gethostbyaddr("localhost", )



	const short tcpPort = 27018;

	TcpStreamer tcpStreamer;
	tcpStreamer.Listen(tcpPort);
	Sleep(100);



	TcpReceiver tcpReveiver;

	tcpReveiver.Connect("127.0.0.1", tcpPort, [&] (TcpReceiver::Packet packet){
		std::cout << packet->data() << endl;
	});
	


	Sleep(500);
	char* msg = "lofasz";
	tcpStreamer.SendToAllClients(msg, strlen(msg) + 1);



	NetworkDialog networkDialog;

	networkDialog.DoModal();


	//SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//if(clientSocket == INVALID_SOCKET) {
	//	int errorCode = WSAGetLastError();
	//}



	//sockaddr_in serverAddress;
	//serverAddress.sin_family = AF_INET;
	////serverAddress.sin_addr.s_addr = inet_addr("localhost");
	//serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	//serverAddress.sin_port = htons(tcpPort);
	//
	//int result = ::connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	//if(result != 0) {
	//	int errorCode = WSAGetLastError();
	//	int asd = 43;
	//}

	//Sleep(40);

	//const char* msg = "lofasz";
	//tcpStreamer.SendToAllClients((void*)msg, strlen(msg)+1);

	//char buffer[200];
	//result = ::recv(clientSocket, buffer, sizeof(buffer), 0);
	//if(SOCKET_ERROR == 0) {
	//	int errorCode = WSAGetLastError();
	//	int asd = 43;
	//} else {
	//	std::cout << buffer;
	//}


	//std::thread t1([]{
	//	while(GetKeyState(VK_ESCAPE) >= 0) Sleep(20);
	//});
	//

	//std::thread t2([&] {
	//	
	//});

	while(GetKeyState(VK_ESCAPE) >= 0) {
		//int ch = getchar();
		int ch = _getch();

		if(ch == VK_ESCAPE) break;

		if(ch == ' ') {
			broadcaster.QueryDevices(port);
			cout << "Sending query...." << endl;
		}

		if(ch == 'm') {

			tcpStreamer.SendToAllClients(msg, strlen(msg) + 1);
			cout << "sending message..." << endl;
		}
	}

	cout << "press esc" << endl;

	//t1.join();
	//t2.join();


}

//
//void wmain() {
//
//	const short input[] = {1, -1 ,2 ,-2, 3, -3,  4, -4, 5, -5, 6, -6 , 7, -7 };
//	const int count = sizeof input / sizeof(short);
//
//
//	//short* output = new short[count];
//	//outputCount = 
//
//	//short output[] = { -1, -1, -1, -1, -1 , -1 };
//	short output[30];
//	for(short& s : output) {
//		s = -1;
//	}
//	//memset(output, -1, sizeof(output), );
//
//	SgnProc::Resample(input, count, 2, output, 22, 2);
//
//
//	for(short& s : output) {
//		s = -1;
//	}
//	SgnProc::Resample(input, count, 2, output, 22, 1);
//
//
//	int asdf = 234 + 43;
//
//}
//#include <HttpRequest.idl>
//
//void wmain() {
//
//
//	MP3Player player;
//
//	// Open the mp3 from a file...
//	CString fileName = L"d:/Documents/AudioRaw/Zilean voice Hungarian.mp3";
//	player.OpenFromFile(fileName);
//	// or From a memory location!
//	//player.OpenFromMemory(ptrToMP3Song, bytesLength);
//
//	player.Play();
//
//
//	MessageBoxA(0, 0, 0, 0);
//	player.Close();
//
//	return;
//
//	try {
//
//		//GET https ://www.googleapis.com/drive/v2/files?corpus=DEFAULT&key={YOUR_API_KEY}
//		//CString apiKey = L"AIzaSyBaej6ehWdKsFbN6REzFA2cXrBXU21e7S4";
//		//CString apiKey = L" AIzaSyA8XQeh5ZEqc4z64uY5g_H1luwLEbW4qjs";
//		CString apiKey = L"w7a6BOxi_m5hLnIJZYT9gVoO";
//		//CString url = L"www.googleapis.com/drive/v2/files?corpus=DOMAIN&key=" + apiKey;
//		//CString url = L"www.googleapis.com/drive/v2/files";
//		CString url = L"www.googleapis.com";
//		CString object = L"drive/v2/files?corpus=DOMAIN&key=" + apiKey;
//
//		//"http://users.atw.hu/battlechicken/ts/downloads/szarhazi.wav"
//
//
//		url = L"users.atw.hu";
//		object = L"battlechicken/ts/downloads/szarhazi.wav";
//
//
//		Web::Http http(url);
//
//		Web::Buffer buffer = http.Get(object);
//
//		MessageBoxA(0, (char*)buffer.Data(), 0, 0);
//		
//
//	} catch(Exception e) {
//		MessageBox(0, e.errorMessage == L"" ? ToString((int)e.errorCode) : e.errorMessage , 0, 0);
//	}
//	
//}

// basic-http-client.cpp
//#include <packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn.2.8.0\>
//
//#include <http_client.h>
//#include <filestream.h>
//#include <iostream>
//#include <sstream>
//
//using namespace web::http;
//using namespace web::http::client;
//
//// Creates an HTTP request and prints the length of the response stream.
//pplx::task<void> HTTPStreamingAsync() {
//	http_client client(L"http://www.fourthcoffee.com");
//
//	// Make the request and asynchronously process the response. 
//	return client.request(methods::GET).then([](http_response response) {
//		// Print the status code.
//		std::wostringstream ss;
//		ss << L"Server returned returned status code " << response.status_code() << L'.' << std::endl;
//		std::wcout << ss.str();
//
//		// TODO: Perform actions here reading from the response stream.
//		auto bodyStream = response.body();
//
//		// In this example, we print the length of the response to the console.
//		ss.str(std::wstring());
//		ss << L"Content length is " << response.headers().content_length() << L" bytes." << std::endl;
//		std::wcout << ss.str();
//	});
//
//	/* Sample output:
//	Server returned returned status code 200.
//	Content length is 63803 bytes.
//	*/
//}
//
//// Builds an HTTP request that uses custom header values.
//pplx::task<void> HTTPRequestCustomHeadersAsync() {
//	http_client client(L"http://www.fourthcoffee.com");
//
//	// Manually build up an HTTP request with header and request URI.
//	http_request request(methods::GET);
//	request.headers().add(L"MyHeaderField", L"MyHeaderValue");
//	request.set_request_uri(L"requestpath");
//	return client.request(request).then([](http_response response) {
//		// Print the status code.
//		std::wostringstream ss;
//		ss << L"Server returned returned status code " << response.status_code() << L"." << std::endl;
//		std::wcout << ss.str();
//	});
//
//	/* Sample output:
//	Server returned returned status code 200.
//	*/
//}
//
//// Upload a file to an HTTP server.
//pplx::task<void> UploadFileToHttpServerAsync() {
//	using concurrency::streams::file_stream;
//	using concurrency::streams::basic_istream;
//
//	// To run this example, you must have a file named myfile.txt in the current folder. 
//	// Alternatively, you can use the following code to create a stream from a text string. 
//	// std::string s("abcdefg");
//	// auto ss = concurrency::streams::stringstream::open_istream(s); 
//
//	// Open stream to file. 
//	return file_stream<unsigned char>::open_istream(L"myfile.txt").then([](pplx::task<basic_istream<unsigned char>> previousTask) {
//		try {
//			auto fileStream = previousTask.get();
//
//			// Make HTTP request with the file stream as the body.
//			http_client client(L"http://www.fourthcoffee.com");
//			return client.request(methods::PUT, L"myfile", fileStream).then([fileStream](pplx::task<http_response> previousTask) {
//				fileStream.close();
//
//				std::wostringstream ss;
//				try {
//					auto response = previousTask.get();
//					ss << L"Server returned returned status code " << response.status_code() << L"." << std::endl;
//				} catch(const http_exception& e) {
//					ss << e.what() << std::endl;
//				}
//				std::wcout << ss.str();
//			});
//		} catch(const std::system_error& e) {
//			std::wostringstream ss;
//			ss << e.what() << std::endl;
//			std::wcout << ss.str();
//
//			// Return an empty task. 
//			return pplx::task_from_result();
//		}
//	});
//
//	/* Sample output:
//	The request must be resent
//	*/
//}
//
//
//
//int wmain() {
//	// This example uses the task::wait method to ensure that async operations complete before the app exits.  
//	// In most apps, you typically dont wait for async operations to complete.
//
//	std::wcout << L"Calling HTTPStreamingAsync..." << std::endl;
//	HTTPStreamingAsync().wait();
//
//	std::wcout << L"Calling HTTPRequestCustomHeadersAsync..." << std::endl;
//	HTTPRequestCustomHeadersAsync().wait();
//
//	std::wcout << L"Calling UploadFileToHttpServerAsync..." << std::endl;
//	UploadFileToHttpServerAsync().wait();
//}



//#include <App/SoundplayerApp.h>
#include <Wave/wave.h>
#include <Wave/AudioProcessor.h>
#include <Gui/AudioProcessorDialog.h>

//#pragma comment(lib, "Release\\cpprest140_2_8.lib")


void processingtest() {

	// Mivel ez nem egy generált mfc alkalmazás nincs minden
	// alapból inicializálva, és assert-et dob a dialog konstruktor
	// ha ez nincs itt
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CFileDialog openDialog(TRUE);
	
	if (openDialog.DoModal() != IDOK) {
		return;
	}
	//CString fileName = dialog.GetFileName();
	CString fileName = openDialog.GetPathName();

	//MessageBox(0, fileName, 0, 0);

	std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(fileName);

	AudioProcessor audioPorcessor;


	//AudioProcessorDialog dlg(audioPorcessor);
	//dlg.DoModal();

	//MessageBoxA(0, audioPorcessor.enabled ? "igen" : "nem", 0, 0);

	audioPorcessor.enabled = false;



	audioPorcessor.Process((short*)track->data.data(), track->numberOfSamples, track->header.nChannels);


	CFileDialog saveDialog(TRUE);

	if (openDialog.DoModal() != IDOK) {
		return;
	}

	CString outputFileName = openDialog.GetPathName();

	//CString resultFileName = CString() + L"d:/Documents/temp/wave/test" + ToString(time(NULL)) + L".wav";

	track->Save(outputFileName);
}



class CMyApp : public CWinApp {
	BOOL InitInstance() override {


		processingtest();
		//wmain();

		//return TRUE;
		//MessageBoxA(0, "lofasz", 0, 0);
		//CString fileName = L"d:/Documents/AudioEdited/A pofadat befogod.wav";



		return TRUE;
	}
};

CMyApp theApp;


#endif