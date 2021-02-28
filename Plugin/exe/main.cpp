#include "stdafx.h"

#ifndef _WINDLL

//#import "msxml6.dll"
//#import <Winhttp.lib>

//#include "http.h"
#include <App/SoundplayerApp.h>

#include <Web/SoundBroadcaster.h>

#include <Gui/NetworkDialog.h>
#include <Gui/AudioProcessorDialog.h>
#include <Gui/OverlayWindow.h>


#include <Wave\MP3Player.h>
#include <Wave\SignalProcessing.h>
#include <Wave/Steganography.h>
#include <Wave/wave.h>
#include <Wave/AudioProcessor.h>
#include <Wave/PitchFilter.h>

#include <Util/Config.h>

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
			
			CreateConsole();

			auto app = make_shared<SoundplayerApp>();
			app->Init();


			std::string line;
			while (std::getline(std::cin, line)) {
				if (line == "exit") {
					break;
				}
			}

			app->Shutdown();

			return TRUE;
		}
	};

	CMyApp theApp;



}


#endif


