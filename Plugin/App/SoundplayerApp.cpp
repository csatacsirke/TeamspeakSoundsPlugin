#include "stdafx.h"
#include "SoundplayerApp.h"


SoundplayerApp::SoundplayerApp(TS3Functions& ts3Functions) : ts3Functions(ts3Functions) {
	pipeHandler.SetOnNewEntryListener([&](PipeHandler& pipeHandler) {
		PipeHandler::KeyData keyData;
		std::wcout << L"jott:";
		while (pipeHandler.TryPop(keyData)) {
			std::wcout << keyData.unicodeLiteral;
		}
	});

	pipeHandler.ListenPipe();

	hookInstaller.AttachDll();
}


SoundplayerApp::~SoundplayerApp() {
}



