#pragma once

#include "ts3_functions.h"

#include <HookInstaller\Hook\KeyboardHookInstaller.h>
#include <HookInstaller\Hook\PipeHandler.h>


class SoundplayerApp  {
	KeyboardHookInstaller hookInstaller;
	PipeHandler pipeHandler;
	TS3Functions& ts3Functions;
public:
	

	SoundplayerApp(TS3Functions& ts3Functions);
	~SoundplayerApp();

	

};

