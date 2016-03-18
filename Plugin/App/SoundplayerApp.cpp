#include "stdafx.h"
#include "SoundplayerApp.h"


SoundplayerApp::SoundplayerApp(TS3Functions& ts3Functions) : ts3Functions(ts3Functions) {

}


SoundplayerApp::~SoundplayerApp() {
}


void SoundplayerApp::Init() {

	// A JÓ KURVA ANYÁD!
	// enélkül ha konzolra írsz egy ő betűt eltörik az egész konzol....
	std::locale::global(std::locale(""));

	if(!hookInstaller.AttachDll()) {
		MessageBoxA(0, "KeyboardHook dll load failed", 0, 0);
		return;
	}


	pipeHandler.SetOnNewEntryListener([&](PipeHandler& pipeHandler) {
		PipeHandler::KeyData keyData;

		while(pipeHandler.TryPop(keyData)) {
			std::wcout << keyData.unicodeLiteral.GetString();
		}
	});

	if(!pipeHandler.ListenPipe()) {
		MessageBoxA(0, "ListenPipeload failed", 0, 0);
		return;
	} 

}


