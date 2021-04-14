#include "stdafx.h"

#ifndef _WINDLL

//#import "msxml6.dll"
//#import <Winhttp.lib>

//#include "http.h"
#include <App/SoundplayerApp.h>

//#include <Web/SoundBroadcaster.h>
#include <Web/Http.h>

#include <Gui/NetworkDialog.h>
#include <Gui/AudioProcessorDialog.h>
#include <Gui/OverlayWindow.h>


#include <Audio/MP3Player.h>
#include <Audio/SignalProcessing.h>
#include <Audio/Steganography.h>
#include <Audio/wave.h>
#include <Audio/AudioProcessor.h>
#include <Audio/PitchFilter.h>

#include <Util/Config.h>

#include <Twitch/TwitchLogin.h>

#include <conio.h>
#include <stdio.h>

namespace TSPlugin {

	using namespace std;


	//void processingtest() {

	//	// Mivel ez nem egy generált mfc alkalmazás nincs minden
	//	// alapból inicializálva, és assert-et dob a dialog konstruktor
	//	// ha ez nincs itt
	//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//	CFileDialog openDialog(TRUE);


	//	if (openDialog.DoModal() != IDOK) {
	//		return;
	//	}
	//	//CString fileName = dialog.GetFileName();
	//	CString fileName = openDialog.GetPathName();

	//	//MessageBox(0, fileName, 0, 0);

	//	std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(fileName);

	//	

	//	AudioProcessor audioPorcessor({ make_shared<PitchFilter>() });


	//	//AudioProcessorDialog dlg(audioPorcessor);
	//	//dlg.DoModal();

	//	//MessageBoxA(0, audioPorcessor.enabled ? "igen" : "nem", 0, 0);

	//	audioPorcessor.enabled = true;



	//	audioPorcessor.Process((short*)track->data.data(), track->numberOfSamples, track->header.nChannels);


	//	CFileDialog saveDialog(TRUE);

	//	if (openDialog.DoModal() != IDOK) {
	//		return;
	//	}

	//	CString outputFileName = openDialog.GetPathName();

	//	//CString resultFileName = CString() + L"d:/Documents/temp/wave/test" + ToString(time(NULL)) + L".wav";

	//	track->Save(outputFileName);

	//	ShellExecute(0, 0, outputFileName, 0, 0, SW_SHOW);

	//}

	//static void OverlayTest() {
	//	auto overlayWindow = OverlayWindow::GetInstance();
	//	
	//	MSG msg;
	//	BOOL bRet;
	//	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
	//		if (bRet == -1) {
	//			// handle the error and possibly exit
	//		} else {
	//			TranslateMessage(&msg);
	//			DispatchMessage(&msg);
	//		}
	//	}
	//}
	



	void NormalizeTest() {

		// Mivel ez nem egy generált mfc alkalmazás nincs minden
		// alapból inicializálva, és assert-et dob a dialog konstruktor
		// ha ez nincs itt
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CFileDialog openDialog(TRUE);


		if (openDialog.DoModal() != IDOK) {
			return;
		}
		CString fileName = openDialog.GetPathName();

		auto entries = Global::config.MakeCopyOfEntries();
		entries[ConfigKeys::NormalizeVolume] = "0";
		Global::config.SetEntries(entries);


		std::shared_ptr<WaveTrack> track = WaveTrack::LoadWaveFile(fileName.GetString());


		float volume1 = CalculateMaxVolume_Perceptive(*track);
		float volume2 = CalculateMaxVolume_Absolute(*track);

	}


	class CMyApp : public CWinApp {
		BOOL InitInstance() override {
			
			OpenConsole();

			auto app = make_shared<SoundplayerApp>();
			
			//Twitch::ValidateToken(L"...");
			// https://id.twitch.tv/oauth2/validate
			auto response = Http::HttpRequest(L"id.twitch.tv", L"oauth2/validate", {
				.useHttps=true,
				.headers = FormatString(L"Authorization: OAuth %s", L"..."),
				});

			app->Shutdown();

			return TRUE;
		}
	};

	CMyApp theApp;



}


#endif


