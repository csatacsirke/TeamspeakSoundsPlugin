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
#include <Gui/TwitchIntegrationDialog.h>


#include <Audio/MP3Player.h>
#include <Audio/SignalProcessing.h>
#include <Audio/Steganography.h>
#include <Audio/wave.h>
#include <Audio/AudioProcessor.h>
#include <Audio/PitchFilter.h>

#include <Util/Config.h>

#include <Twitch/TwitchApi.h>

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
	void TwitchCliTest() {

		auto twitchState = make_shared<Twitch::TwitchState>();
		twitchState->accessToken = L"1hqjcgjx1b2g2fducxekdj0pqczugv";
		Twitch::CreateReward(*twitchState);
	}

	static CString GetEnv(const TCHAR* envVarName) {

		TCHAR* envVar;
		size_t size;
		errno_t result = _wdupenv_s(&envVar, &size, envVarName);
		if (result != 0) {
			ASSERT(0);
			return L"";
		}

		Finally finally([&] {
			free(envVar);
		});

		return envVar;
	}


	class CMyApp : public CWinApp {
		BOOL InitInstance() override {
			
			OpenConsole();

			//auto app = make_shared<SoundplayerApp>();
			
			//Twitch::ValidateToken(L"...");
			// https://id.twitch.tv/oauth2/validate
			//auto response = Http::HttpRequest(L"id.twitch.tv", L"oauth2/validate", {
			//	.useHttps=true,
			//	.headers = FormatString(L"Authorization: OAuth %s", L"..."),
			//	});

			//Twitch::StartUserLogin();
			//auto optAccessToken = Twitch::PollAccessToken(L"9dd3b0e245cb84761cd3cce85dded044");
			//
			//bool isTokenValid = false;
			//if (optAccessToken) {
			//	isTokenValid = Twitch::ValidateToken(*optAccessToken);
			//}


			//app->Shutdown();

			//auto twitchState = make_shared<Twitch::TwitchState>();
			//TwitchIntegrationDialog dialog(twitchState);
			//dialog.DoModal();
#if 1
			auto twitchState = make_shared<Twitch::TwitchState>();
			
			twitchState->session = GetEnv(L"TWITCH_SESSION");
			
			if (!Twitch::PollAccessToken(*twitchState)) {
				return FALSE;
			}



			auto optRewards = Twitch::GetRewards(*twitchState);
			if (!optRewards) {
				return FALSE;
			}
			auto& rewards = *optRewards;

			//bool didDelete0 = Twitch::DeleteReward(*twitchState, (CString)rewards["data"][0]["id"].get<std::string>().c_str());
			//bool didDelete1 = Twitch::DeleteReward(*twitchState, (CString)rewards["data"][1]["id"].get<std::string>().c_str());
			//Twitch::CreateReward(*twitchState);
			//std::string _rewards = rewards.dump();


			if (rewards["data"].size() == 0) {
				Twitch::CreateReward(*twitchState);
			}


			CString rewardId = (CString)rewards["data"][0]["id"].get<std::string>().c_str();
			//for (auto& rewardObject : rewards["data"]) {
			//	auto rewardTitle = rewardObject["title"].get<std::string>();
			//	if (rewardTitle == "Play sound") {
			//		rewardId = rewardObject["id"].get<std::string>().c_str();
			//	}
			//}

			nlohmann::json toUpdate;
			toUpdate["cost"] = 50000;
			toUpdate["title"] = "[TEST-ONLY] Play Sound";
			toUpdate["prompt"] = "Name of the sound to play?";
			const bool didUpdate = Twitch::UpdateReward(*twitchState, rewardId, toUpdate);


			auto optUnfullfilledRedemptions = GetUnfulfilledRedemptions(*twitchState, rewardId);
			if (!optUnfullfilledRedemptions) {
				return FALSE;
			}

			//auto str = optUnfullfilledRewards->dump();
			for (auto& redemption : *optUnfullfilledRedemptions) {
				ConfirmRewardRedemption(*twitchState, redemption);
			}

#endif
			//TwitchCliTest();
			return TRUE;
		}
	};


	// singleton
	CMyApp theApp;
}


#endif


