#pragma once

#include <stdint.h>
#include <pluginsdk/include/ts3_functions.h>


#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

#define PATH_BUFSIZE 512

/*The client lib works at 48Khz internally.
It is therefore advisable to use the same for your project */
#define PLAYBACK_FREQUENCY 48000
#define PLAYBACK_CHANNELS 2



namespace TSPlugin::Global {
	extern struct TS3Functions ts3Functions;
	extern uint64_t connection;
	extern char* pluginID;
	//extern anyID myID;

	extern char appPath[PATH_BUFSIZE];
	extern char resourcesPath[PATH_BUFSIZE];
	extern char configPath[PATH_BUFSIZE];
	extern char pluginPath[PATH_BUFSIZE];

}
