#include "stdafx.h"

#include "Globals.h"

namespace TSPlugin::Global {


	struct TS3Functions ts3Functions;
	uint64_t connection = 0;
	char* pluginID = NULL;
	//anyID myID;


	char appPath[PATH_BUFSIZE];
	char resourcesPath[PATH_BUFSIZE];
	char configPath[PATH_BUFSIZE];
	char pluginPath[PATH_BUFSIZE];

}

