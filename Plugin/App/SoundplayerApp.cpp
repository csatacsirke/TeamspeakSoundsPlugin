#include "stdafx.h"
#include "SoundplayerApp.h"


SoundplayerApp::SoundplayerApp(TS3Functions& ts3Functions) : ts3Functions(ts3Functions) {

}


SoundplayerApp::~SoundplayerApp() {
}


void SoundplayerApp::Init() {

	if(!hookInstaller.AttachDll()) {
		MessageBoxA(0, "KeyboardHook dll load failed", 0, 0);
		return;
	}


	pipeHandler.SetOnNewEntryListener([&](PipeHandler& pipeHandler) {
		PipeHandler::KeyData keyData;
		std::wcout << L"jott:";
		while(pipeHandler.TryPop(keyData)) {
			std::wcout << keyData.unicodeLiteral;
		}
	});

	if(!pipeHandler.ListenPipe()) {
		MessageBoxA(0, "ListenPipeload failed", 0, 0);
		return;
	} 

}


