﻿/*
 * TeamSpeak 3 demo plugin
 *
 * Copyright (c) 2008-2015 TeamSpeak Systems GmbH
 */

#include "stdafx.h"


//#pragma comment(lib, "HookInstaller")

using namespace std;

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
//#include <Windows.h>
//#include <afxwin.h>
#endif

#include "pluginsdk/include/teamspeak/public_errors.h"
#include "pluginsdk/include/teamspeak/public_errors_rare.h"
#include "pluginsdk/include/teamspeak/public_definitions.h"
#include "pluginsdk/include/teamspeak/public_rare_definitions.h"
#include "pluginsdk/include/teamspeak/clientlib_publicdefinitions.h"
#include "pluginsdk/include/ts3_functions.h"
#include "plugin.h"

#include "Wave\wave.h"

#include "App\SoundplayerApp.h"


#include <atlpath.h>



#ifdef _WIN32
#define SLEEP(x) Sleep(x)
#else
#define SLEEP(x) usleep(x*1000)
#endif

const char* version = "17.03.24";


//#define AUDIO_PROCESS_SECONDS 10



using namespace TSPlugin;
using namespace Global;

//SoundplayerApp theApp;
std::unique_ptr<SoundplayerApp> theApp;

//int PlayWelcomeSound();

#define PLUGIN_API_VERSION 21



const static char* myDeviceId = "BattlechickensId";
//static anyID g_myId = 0;



#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif




/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if(!result) {
		const wchar_t* name = L"Soundplayer";
		if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = "Soundplayer";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Soundplayer";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
	return version;
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
	return "Battlechicken";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
	return "Me trying to write a TS plugin. If you see this remind me to change it kappa";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */



unsigned int error;
//	char *version;
//	char *identity;

static int    captureFrequency;
static int    captureChannels;
static short* captureBuffer;
static size_t buffer_size;
static int    captureBufferSamples;
static int    audioPeriodCounter;
static int    capturePeriodSize;




int ts3plugin_init() {
#if 0
	ShellExecuteA(NULL, "open", "https://www.youtube.com/watch?v=oHg5SJYRHA0", NULL, NULL, SW_SHOWNORMAL);
	return 0;
#endif


//#ifdef DEBUG
#if 1
#pragma warning( push )
#pragma warning( disable : 4996)
	if(GetKeyState(VK_CONTROL) < 0) {
		// A JÓ KURVA ANYÁD!
		// enélkül ha konzolra írsz egy ő betűt eltörik az egész konzol....
		std::locale::global(std::locale(""));

		static FILE* console = NULL;
		if(!console) {
			AllocConsole();
			console = freopen("CONOUT$", "w", stdout);
		} else {
			FreeConsole();
			fclose(console);
			console = NULL;
		}
		
	}
#pragma warning( pop )
#endif
	
	/* Your plugin init code here */
	printf("PLUGIN: init\n");

	/* Example on how to query application, resources and configuration paths from client */
	/* Note: Console client returns empty string for app and resources path */
	ts3Functions.getAppPath(appPath, PATH_BUFSIZE);
	ts3Functions.getResourcesPath(resourcesPath, PATH_BUFSIZE);
	ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, Global::pluginID);

	printf("PLUGIN: App path: %s\nResources path: %s\nConfig path: %s\nPlugin path: %s\n", appPath, resourcesPath, configPath, pluginPath);


	/* Read in the wave we are going to stream to the server */
	//if (!readWave(L"c:\\Program Files\\TeamSpeak 3 Client\\plugins\\welcome_to_teamspeak.wav", &captureFrequency, &captureChannels, &captureBuffer, &buffer_size, &captureBufferSamples)) {
	//	cout << "readWave failed";
	//	return 1;
	//}

	CPath path = CString(Global::configPath);
	path.Append(Global::config.defaultFileName);
	Global::config.LoadFromFile(path);

	

	theApp.reset(new SoundplayerApp());
	theApp->Init();

	return 0;  /* 0 = success, 1 = failure, -2 = failure but client will not show a "failed to load" warning */
	/* -2 is a very special case and should only be used if a plugin displays a dialog (e.g. overlay) asking the user to disable
	 * the plugin again, avoiding the show another dialog by the client telling the user the plugin failed to load.
	 * For normal case, if a plugin really failed to load because of an error, the correct return value is 1. */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
	/* Your plugin cleanup code here */
	printf("PLUGIN: shutdown\n");

	theApp = nullptr;

	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	printf("PLUGIN: offersConfigure\n");
	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
	//return PLUGIN_OFFERS_NO_CONFIGURE;  /* In this case ts3plugin_configure does not need to be implemented */

	// ó bazdmeg ha én azt tudnám hogy ez mi a faszt csinál
	//return PLUGIN_OFFERS_CONFIGURE_NEW_THREAD;

	// Ez azért kell hogy a showHotkeySetup müködjön
	return PLUGIN_OFFERS_CONFIGURE_QT_THREAD;
}


/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
	printf("PLUGIN: configure\n");


	theApp->OpenSettingsDialog(handle, qParentWidget);
	//SettingsDialog

	//ts3Functions.showHotkeySetup();

	//Sleep(10000);
	//ts3Functions.requestHotkeyInputDialog(pluginID, "keyword", true, qParentWidget);
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

/* Plugin command keyword. Return NULL or "" if not used. */
const char* ts3plugin_commandKeyword() {
	return "test";
}

/* Plugin processes console command. Return 0 if plugin handled the command, 1 if not handled. */
int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command) {
	char buf[COMMAND_BUFSIZE];
	char *s, *param1 = NULL, *param2 = NULL;
	int i = 0;
	enum { CMD_NONE = 0, CMD_JOIN, CMD_COMMAND, CMD_SERVERINFO, CMD_CHANNELINFO, CMD_AVATAR, CMD_ENABLEMENU, CMD_SUBSCRIBE, CMD_UNSUBSCRIBE, CMD_SUBSCRIBEALL, CMD_UNSUBSCRIBEALL } cmd = CMD_NONE;
#ifdef _WIN32
	char* context = NULL;
#endif

	printf("PLUGIN: process command: '%s'\n", command);

	_strcpy(buf, COMMAND_BUFSIZE, command);
#ifdef _WIN32
	s = strtok_s(buf, " ", &context);
#else
	s = strtok(buf, " ");
#endif
	while(s != NULL) {
		if(i == 0) {
			if(!strcmp(s, "join")) {
				cmd = CMD_JOIN;
			} else if(!strcmp(s, "command")) {
				cmd = CMD_COMMAND;
			} else if(!strcmp(s, "serverinfo")) {
				cmd = CMD_SERVERINFO;
			} else if(!strcmp(s, "channelinfo")) {
				cmd = CMD_CHANNELINFO;
			} else if(!strcmp(s, "avatar")) {
				cmd = CMD_AVATAR;
			} else if(!strcmp(s, "enablemenu")) {
				cmd = CMD_ENABLEMENU;
			} else if(!strcmp(s, "subscribe")) {
				cmd = CMD_SUBSCRIBE;
			} else if(!strcmp(s, "unsubscribe")) {
				cmd = CMD_UNSUBSCRIBE;
			} else if(!strcmp(s, "subscribeall")) {
				cmd = CMD_SUBSCRIBEALL;
			} else if(!strcmp(s, "unsubscribeall")) {
				cmd = CMD_UNSUBSCRIBEALL;
			}
		} else if(i == 1) {
			param1 = s;
		} else {
			param2 = s;
		}
#ifdef _WIN32
		s = strtok_s(NULL, " ", &context);
#else
		s = strtok(NULL, " ");
#endif
		i++;
	}

	switch(cmd) {
		case CMD_NONE:
			return 1;  /* Command not handled by plugin */
		case CMD_JOIN:  /* /test join <channelID> [optionalCannelPassword] */
			if(param1) {
				uint64 channelID = (uint64)atoi(param1);
				char* password = param2 ? param2 : "";
				char returnCode[RETURNCODE_BUFSIZE];
				anyID myID;
				/* Get own clientID */
				if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
					ts3Functions.logMessage("Error querying client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
					break;
				}

				/* Create return code for requestClientMove function call. If creation fails, returnCode will be NULL, which can be
				 * passed into the client functions meaning no return code is used.
				 * Note: To use return codes, the plugin needs to register a plugin ID using ts3plugin_registerPluginID */
				ts3Functions.createReturnCode(pluginID, returnCode, RETURNCODE_BUFSIZE);

				/* In a real world plugin, the returnCode should be remembered (e.g. in a dynamic STL vector, if it's a C++ plugin).
				 * onServerErrorEvent can then check the received returnCode, compare with the remembered ones and thus identify
				 * which function call has triggered the event and react accordingly. */

				/* Request joining specified channel using above created return code */
				if(ts3Functions.requestClientMove(serverConnectionHandlerID, myID, channelID, password, returnCode) != ERROR_ok) {
					ts3Functions.logMessage("Error requesting client move", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
				}
			} else {
				ts3Functions.printMessageToCurrentTab("Missing channel ID parameter.");
			}
			break;
		case CMD_COMMAND:  /* /test command <command> */
			if(param1) {
				/* Send plugin command to all clients in current channel. In this case targetIds is unused and can be NULL. */
				if(pluginID) {
					/* See ts3plugin_registerPluginID for how to obtain a pluginID */
					printf("PLUGIN: Sending plugin command to current channel: %s\n", param1);
					ts3Functions.sendPluginCommand(serverConnectionHandlerID, pluginID, param1, PluginCommandTarget_CURRENT_CHANNEL, NULL, NULL);
				} else {
					printf("PLUGIN: Failed to send plugin command, was not registered.\n");
				}
			} else {
				ts3Functions.printMessageToCurrentTab("Missing command parameter.");
			}
			break;
		case CMD_SERVERINFO: {  /* /test serverinfo */
			/* Query host, port and server password of current server tab.
			 * The password parameter can be NULL if the plugin does not want to receive the server password.
			 * Note: Server password is only available if the user has actually used it when connecting. If a user has
			 * connected with the permission to ignore passwords (b_virtualserver_join_ignore_password) and the password,
			 * was not entered, it will not be available.
			 * getServerConnectInfo returns 0 on success, 1 on error or if current server tab is disconnected. */
			char host[SERVERINFO_BUFSIZE];
			/*char password[SERVERINFO_BUFSIZE];*/
			char* password = NULL;  /* Don't receive server password */
			unsigned short port;
			if(!ts3Functions.getServerConnectInfo(serverConnectionHandlerID, host, &port, password, SERVERINFO_BUFSIZE)) {
				char msg[SERVERINFO_BUFSIZE];
				snprintf(msg, sizeof(msg), "Server Connect Info: %s:%d", host, port);
				ts3Functions.printMessageToCurrentTab(msg);
			} else {
				ts3Functions.printMessageToCurrentTab("No server connect info available.");
			}
			break;
		}
		case CMD_CHANNELINFO: {  /* /test channelinfo */
			/* Query channel path and password of current server tab.
			 * The password parameter can be NULL if the plugin does not want to receive the channel password.
			 * Note: Channel password is only available if the user has actually used it when entering the channel. If a user has
			 * entered a channel with the permission to ignore passwords (b_channel_join_ignore_password) and the password,
			 * was not entered, it will not be available.
			 * getChannelConnectInfo returns 0 on success, 1 on error or if current server tab is disconnected. */
			char path[CHANNELINFO_BUFSIZE];
			/*char password[CHANNELINFO_BUFSIZE];*/
			char* password = NULL;  /* Don't receive channel password */

			/* Get own clientID and channelID */
			anyID myID;
			uint64 myChannelID;
			if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
				ts3Functions.logMessage("Error querying client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				break;
			}
			/* Get own channel ID */
			if(ts3Functions.getChannelOfClient(serverConnectionHandlerID, myID, &myChannelID) != ERROR_ok) {
				ts3Functions.logMessage("Error querying channel ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				break;
			}

			/* Get channel connect info of own channel */
			if(!ts3Functions.getChannelConnectInfo(serverConnectionHandlerID, myChannelID, path, password, CHANNELINFO_BUFSIZE)) {
				char msg[CHANNELINFO_BUFSIZE];
				snprintf(msg, sizeof(msg), "Channel Connect Info: %s", path);
				ts3Functions.printMessageToCurrentTab(msg);
			} else {
				ts3Functions.printMessageToCurrentTab("No channel connect info available.");
			}
			break;
		}
		case CMD_AVATAR: {  /* /test avatar <clientID> */
			char avatarPath[PATH_BUFSIZE];
			anyID clientID = (anyID)atoi(param1);
			unsigned int error;

			memset(avatarPath, 0, PATH_BUFSIZE);
			error = ts3Functions.getAvatar(serverConnectionHandlerID, clientID, avatarPath, PATH_BUFSIZE);
			if(error == ERROR_ok) {  /* ERROR_ok means the client has an avatar set. */
				if(strlen(avatarPath)) {  /* Avatar path contains the full path to the avatar image in the TS3Client cache directory */
					printf("Avatar path: %s\n", avatarPath);
				} else { /* Empty avatar path means the client has an avatar but the image has not yet been cached. The TeamSpeak
						  * client will automatically start the download and call onAvatarUpdated when done */
					printf("Avatar not yet downloaded, waiting for onAvatarUpdated...\n");
				}
			} else if(error == ERROR_database_empty_result) {  /* Not an error, the client simply has no avatar set */
				printf("Client has no avatar\n");
			} else { /* Other error occured (invalid server connection handler ID, invalid client ID, file io error etc) */
				printf("Error getting avatar: %d\n", error);
			}
			break;
		}
		case CMD_ENABLEMENU:  /* /test enablemenu <menuID> <0|1> */
			if(param1) {
				int menuID = atoi(param1);
				int enable = param2 ? atoi(param2) : 0;
				ts3Functions.setPluginMenuEnabled(pluginID, menuID, enable);
			} else {
				ts3Functions.printMessageToCurrentTab("Usage is: /test enablemenu <menuID> <0|1>");
			}
			break;
		case CMD_SUBSCRIBE:  /* /test subscribe <channelID> */
			if(param1) {
				char returnCode[RETURNCODE_BUFSIZE];
				uint64 channelIDArray[2];
				channelIDArray[0] = (uint64)atoi(param1);
				channelIDArray[1] = 0;
				ts3Functions.createReturnCode(pluginID, returnCode, RETURNCODE_BUFSIZE);
				if(ts3Functions.requestChannelSubscribe(serverConnectionHandlerID, channelIDArray, returnCode) != ERROR_ok) {
					ts3Functions.logMessage("Error subscribing channel", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
				}
			}
			break;
		case CMD_UNSUBSCRIBE:  /* /test unsubscribe <channelID> */
			if(param1) {
				char returnCode[RETURNCODE_BUFSIZE];
				uint64 channelIDArray[2];
				channelIDArray[0] = (uint64)atoi(param1);
				channelIDArray[1] = 0;
				ts3Functions.createReturnCode(pluginID, returnCode, RETURNCODE_BUFSIZE);
				if(ts3Functions.requestChannelUnsubscribe(serverConnectionHandlerID, channelIDArray, NULL) != ERROR_ok) {
					ts3Functions.logMessage("Error unsubscribing channel", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
				}
			}
			break;
		case CMD_SUBSCRIBEALL: {  /* /test subscribeall */
			char returnCode[RETURNCODE_BUFSIZE];
			ts3Functions.createReturnCode(pluginID, returnCode, RETURNCODE_BUFSIZE);
			if(ts3Functions.requestChannelSubscribeAll(serverConnectionHandlerID, returnCode) != ERROR_ok) {
				ts3Functions.logMessage("Error subscribing channel", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
			}
			break;
		}
		case CMD_UNSUBSCRIBEALL: {  /* /test unsubscribeall */
			char returnCode[RETURNCODE_BUFSIZE];
			ts3Functions.createReturnCode(pluginID, returnCode, RETURNCODE_BUFSIZE);
			if(ts3Functions.requestChannelUnsubscribeAll(serverConnectionHandlerID, returnCode) != ERROR_ok) {
				ts3Functions.logMessage("Error subscribing channel", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
			}
			break;
		}
	}

	return 0;  /* Plugin handled command */
}

/* Client changed current server connection handler */
void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID) {
	connection = serverConnectionHandlerID;
	printf("PLUGIN: currentServerConnectionChanged %llu (%llu)\n", (long long unsigned int)serverConnectionHandlerID, (long long unsigned int)ts3Functions.getCurrentServerConnectionHandlerID());
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "Test plugin info";
}

/*
 * Dynamic content shown in the right column in the info frame. Memory for the data string needs to be allocated in this
 * function. The client will call ts3plugin_freeMemory once done with the string to release the allocated memory again.
 * Check the parameter "type" if you want to implement this feature only for specific item types. Set the parameter
 * "data" to NULL to have the client ignore the info data.
 */
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) {
	char* name;

	/* For demonstration purpose, display the name of the currently selected server, channel or client. */
	switch(type) {
		case PLUGIN_SERVER:
			if(ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &name) != ERROR_ok) {
				printf("Error getting virtual server name\n");
				return;
			}
			break;
		case PLUGIN_CHANNEL:
			if(ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, id, CHANNEL_NAME, &name) != ERROR_ok) {
				printf("Error getting channel name\n");
				return;
			}
			break;
		case PLUGIN_CLIENT:
			if(ts3Functions.getClientVariableAsString(serverConnectionHandlerID, (anyID)id, CLIENT_NICKNAME, &name) != ERROR_ok) {
				printf("Error getting client nickname\n");
				return;
			}
			break;
		default:
			printf("Invalid item type: %d\n", type);
			data = NULL;  /* Ignore */
			return;
	}


	CStringA infoData = theApp->GetPluginInfoData(id, type);

	const size_t allocatedSize = (infoData.GetLength() + 1);
	//*data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));  /* Must be allocated in the plugin! */
	*data = (char*)malloc(allocatedSize* sizeof(char));  /* Must be allocated in the plugin! */
	//snprintf(*data, INFODATA_BUFSIZE, "The nickname is [I]\"%s\"[/I]", name);  /* bbCode is supported. HTML is not supported */
	strcpy_s(*data, allocatedSize, infoData);

	ts3Functions.freeMemory(name);

	
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
	return 0;  /* 1 = request autoloaded, 0 = do not request autoload */
}



/*
 * Initialize plugin menus.
 * This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
 * Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
 * If plugin menus are not used by a plugin, do not implement this function or return NULL.
 */
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
	theApp->InitMenus(menuItems, menuIcon);
	
}

/* Helper function to create a hotkey */
static struct PluginHotkey* createHotkey(const char* keyword, const char* description) {
	struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
	_strcpy(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
	_strcpy(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
	return hotkey;
}

/* Some makros to make the code to create hotkeys a bit more readable */
#define BEGIN_CREATE_HOTKEYS(x) const size_t sz = x + 1; size_t n = 0; *hotkeys = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS (*hotkeys)[n++] = NULL; assert(n == sz);

/*
 * Initialize plugin hotkeys. If your plugin does not use this feature, this function can be omitted.
 * Hotkeys require ts3plugin_registerPluginID and ts3plugin_freeMemory to be implemented.
 * This function is automatically called by the client after ts3plugin_init.
 */
void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys) {
	/* Register hotkeys giving a keyword and a description.
	 * The keyword will be later passed to ts3plugin_onHotkeyEvent to identify which hotkey was triggered.
	 * The description is shown in the clients hotkey dialog. */
	//BEGIN_CREATE_HOTKEYS(3);  /* Create 3 hotkeys. Size must be correct for allocating memory. */
	//CREATE_HOTKEY("keyword_stop", "Stop playback");
	//CREATE_HOTKEY("keyword_2", "Test hotkey 2");
	//CREATE_HOTKEY("keyword_3", "Test hotkey 3");
	//END_CREATE_HOTKEYS;

	theApp->InitHotkeys(hotkeys);
	/* The client will call ts3plugin_freeMemory to release all allocated memory */
}

/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
	/* Some example code following to show how to use the information query functions. */

	// i can just hope that this is the correct way
	connection = serverConnectionHandlerID;
	//theApp->SetConnectionHandle(serverConnectionHandlerID);


	if(newStatus == STATUS_CONNECTION_ESTABLISHED) {  /* connection established and we have client and channels available */
		char* s;
		char msg[1024];
		anyID myID;
		uint64* ids;
		size_t i;
		unsigned int error;

		/* Print clientlib version */
		if(ts3Functions.getClientLibVersion(&s) == ERROR_ok) {
			printf("PLUGIN: Client lib version: %s\n", s);
			ts3Functions.freeMemory(s);  /* Release string */
		} else {
			ts3Functions.logMessage("Error querying client lib version", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}

		/* Write plugin name and version to log */
		snprintf(msg, sizeof(msg), "Plugin %s, Version %s, Author: %s", ts3plugin_name(), ts3plugin_version(), ts3plugin_author());
		ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);

		/* Print virtual server name */
		if((error = ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_NAME, &s)) != ERROR_ok) {
			if(error != ERROR_not_connected) {  /* Don't spam error in this case (failed to connect) */
				ts3Functions.logMessage("Error querying server name", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			}
			return;
		}
		printf("PLUGIN: Server name: %s\n", s);
		ts3Functions.freeMemory(s);

		/* Print virtual server welcome message */
		if(ts3Functions.getServerVariableAsString(serverConnectionHandlerID, VIRTUALSERVER_WELCOMEMESSAGE, &s) != ERROR_ok) {
			ts3Functions.logMessage("Error querying server welcome message", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}
		printf("PLUGIN: Server welcome message: %s\n", s);
		ts3Functions.freeMemory(s);  /* Release string */

		/* Print own client ID and nickname on this server */
		if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
			ts3Functions.logMessage("Error querying client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}
		if(ts3Functions.getClientSelfVariableAsString(serverConnectionHandlerID, CLIENT_NICKNAME, &s) != ERROR_ok) {
			ts3Functions.logMessage("Error querying client nickname", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}
		printf("PLUGIN: My client ID = %d, nickname = %s\n", myID, s);
		ts3Functions.freeMemory(s);

		//Global::connection = myID;
		//g_myId = myID;

		/* Print list of all channels on this server */
		if(ts3Functions.getChannelList(serverConnectionHandlerID, &ids) != ERROR_ok) {
			ts3Functions.logMessage("Error getting channel list", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}
		//printf("PLUGIN: Available channels:\n");
		for(i=0; ids[i]; i++) {
			/* Query channel name */
			if(ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, ids[i], CHANNEL_NAME, &s) != ERROR_ok) {
				ts3Functions.logMessage("Error querying channel name", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				return;
			}
			//printf("PLUGIN: Channel ID = %llu, name = %s\n", (long long unsigned int)ids[i], s);
			ts3Functions.freeMemory(s);
		}
		ts3Functions.freeMemory(ids);  /* Release array */

		/* Print list of existing server connection handlers */
		printf("PLUGIN: Existing server connection handlers:\n");
		if(ts3Functions.getServerConnectionHandlerList(&ids) != ERROR_ok) {
			ts3Functions.logMessage("Error getting server list", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return;
		}
		for(i=0; ids[i]; i++) {
			if((error = ts3Functions.getServerVariableAsString(ids[i], VIRTUALSERVER_NAME, &s)) != ERROR_ok) {
				if(error != ERROR_not_connected) {  /* Don't spam error in this case (failed to connect) */
					ts3Functions.logMessage("Error querying server name", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
				}
				continue;
			}
			printf("- %llu - %s\n", (long long unsigned int)ids[i], s);
			ts3Functions.freeMemory(s);
		}
		ts3Functions.freeMemory(ids);
	}
}

void ts3plugin_onNewChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID) {
}

void ts3plugin_onNewChannelCreatedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 channelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onDelChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onChannelMoveEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 newChannelParentID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onUpdateChannelEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}

void ts3plugin_onUpdateClientEvent(uint64 serverConnectionHandlerID, anyID clientID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
}



void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	//void SoundplayerApp::OnClientMoved(anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, CString moveMessage) {
	theApp->OnClientMoved(clientID, oldChannelID, newChannelID, visibility, CString(moveMessage));
}

void ts3plugin_onClientMoveSubscriptionEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility) {
}

void ts3plugin_onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* timeoutMessage) {
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID, const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
}

void ts3plugin_onClientIDsEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, anyID clientID, const char* clientName) {
}

void ts3plugin_onClientIDsFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerEditedEvent(uint64 serverConnectionHandlerID, anyID editerID, const char* editerName, const char* editerUniqueIdentifier) {
}

void ts3plugin_onServerUpdatedEvent(uint64 serverConnectionHandlerID) {
}

int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage) {
	printf("PLUGIN: onServerErrorEvent %llu %s %d %s\n", (long long unsigned int)serverConnectionHandlerID, errorMessage, error, (returnCode ? returnCode : ""));
	if(returnCode) {
		/* A plugin could now check the returnCode with previously (when calling a function) remembered returnCodes and react accordingly */
		/* In case of using a a plugin return code, the plugin can return:
		 * 0: Client will continue handling this error (print to chat tab)
		 * 1: Client will ignore this error, the plugin announces it has handled it */
		return 1;
	}
	return 0;  /* If no plugin return code was used, the return value of this function is ignored */
}

void ts3plugin_onServerStopEvent(uint64 serverConnectionHandlerID, const char* shutdownMessage) {
	cout << "server stopped" << endl;
}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored) {
	printf("PLUGIN: onTextMessageEvent %llu %d %d %s %s %d\n", (long long unsigned int)serverConnectionHandlerID, targetMode, fromID, fromName, message, ffIgnored);

	/* Friend/Foe manager has ignored the message, so ignore here as well. */
	if(ffIgnored) {
		return 0; /* Client will ignore the message anyways, so return value here doesn't matter */
	}

#if 0
	{
		/* Example code: Autoreply to sender */
		/* Disabled because quite annoying, but should give you some ideas what is possible here */
		/* Careful, when two clients use this, they will get banned quickly... */
		anyID myID;
		if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {
			ts3Functions.logMessage("Error querying own client id", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			return 0;
		}
		if(fromID != myID) {  /* Don't reply when source is own client */
			if(ts3Functions.requestSendPrivateTextMsg(serverConnectionHandlerID, "Text message back!", fromID, NULL) != ERROR_ok) {
				ts3Functions.logMessage("Error requesting send text message", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
			}
		}
	}
#endif

	return 0;  /* 0 = handle normally, 1 = client will ignore the text message */
}

void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID) {
	/* Demonstrate usage of getClientDisplayName */
	char name[512];
	if(ts3Functions.getClientDisplayName(serverConnectionHandlerID, clientID, name, 512) == ERROR_ok) {
		if(status == STATUS_TALKING) {
			//printf("--> %s starts talking\n", name);
		} else {
			//printf("--> %s stops talking\n", name);
		}
	}
}

void ts3plugin_onConnectionInfoEvent(uint64 serverConnectionHandlerID, anyID clientID) {
	cout << "connection info: " << serverConnectionHandlerID << " " << clientID << endl;
}

void ts3plugin_onServerConnectionInfoEvent(uint64 serverConnectionHandlerID) {
	cout << "server connection info: " << serverConnectionHandlerID << endl;
}

void ts3plugin_onChannelSubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelSubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelUnsubscribeEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelUnsubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelDescriptionUpdateEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onChannelPasswordChangedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onPlaybackShutdownCompleteEvent(uint64 serverConnectionHandlerID) {
	//cout << "onPlaybackShutdownCompleteEvent" << endl;
}

void ts3plugin_onSoundDeviceListChangedEvent(const char* modeID, int playOrCap) {
	//cout << "onSoundDeviceListChangedEvent" << endl;
}

void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels) {
	theApp->OnEditPlaybackVoiceDataEvent(clientID, samples, sampleCount, channels);
	//cout << "onEditPlaybackVoiceDataEvent" << endl;
}

void ts3plugin_onEditPostProcessVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
	//cout << "onEditPostProcessVoiceDataEvent" << endl;
	

}

void ts3plugin_onEditMixedPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask) {
	theApp->OnEditMixedPlaybackVoiceDataEvent(samples, sampleCount, channels, channelSpeakerArray, channelFillMask);
	//cout << "ts3plugin_onEditMixedPlaybackVoiceDataEvent" << endl;
}

void ts3plugin_onEditCapturedVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited) {
	//cout << "ts3plugin_onEditCapturedVoiceDataEvent" << endl;
	theApp->OnEditCapturedVoiceDataEvent(samples, sampleCount, channels, edited);
}

void ts3plugin_onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume) {
}

void ts3plugin_onCustom3dRolloffCalculationWaveEvent(uint64 serverConnectionHandlerID, uint64 waveHandle, float distance, float* volume) {
}

void ts3plugin_onUserLoggingMessageEvent(const char* logMessage, int logLevel, const char* logChannel, uint64 logID, const char* logTime, const char* completeLogString) {
}

/* Clientlib rare */

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID, const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage) {
}

int ts3plugin_onClientPokeEvent(uint64 serverConnectionHandlerID, anyID fromClientID, const char* pokerName, const char* pokerUniqueIdentity, const char* message, int ffIgnored) {
	anyID myID;

	printf("PLUGIN onClientPokeEvent: %llu %d %s %s %d\n", (long long unsigned int)serverConnectionHandlerID, fromClientID, pokerName, message, ffIgnored);

	/* Check if the Friend/Foe manager has already blocked this poke */
	if(ffIgnored) {
		return 0;  /* Client will block anyways, doesn't matter what we return */
	}

	/* Example code: Send text message back to poking client */
	if(ts3Functions.getClientID(serverConnectionHandlerID, &myID) != ERROR_ok) {  /* Get own client ID */
		ts3Functions.logMessage("Error querying own client id", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
		return 0;
	}
	if(fromClientID != myID) {  /* Don't reply when source is own client */
		if(ts3Functions.requestSendPrivateTextMsg(serverConnectionHandlerID, "Received your poke!", fromClientID, NULL) != ERROR_ok) {
			ts3Functions.logMessage("Error requesting send text message", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
		}
	}

	return 0;  /* 0 = handle normally, 1 = client will ignore the poke */
}

void ts3plugin_onClientSelfVariableUpdateEvent(uint64 serverConnectionHandlerID, int flag, const char* oldValue, const char* newValue) {
}

void ts3plugin_onFileListEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path, const char* name, uint64 size, uint64 datetime, int type, uint64 incompletesize, const char* returnCode) {
}

void ts3plugin_onFileListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* path) {
}

void ts3plugin_onFileInfoEvent(uint64 serverConnectionHandlerID, uint64 channelID, const char* name, uint64 size, uint64 datetime) {
}

void ts3plugin_onServerGroupListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, const char* name, int type, int iconID, int saveDB) {
}

void ts3plugin_onServerGroupListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
}

void ts3plugin_onServerGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onServerGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID) {
}

void ts3plugin_onServerGroupClientListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, uint64 clientDatabaseID, const char* clientNameIdentifier, const char* clientUniqueID) {
}

void ts3plugin_onChannelGroupListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, const char* name, int type, int iconID, int saveDB) {
}

void ts3plugin_onChannelGroupListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onChannelGroupPermListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelGroupPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID) {
}

void ts3plugin_onChannelPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID) {
}

void ts3plugin_onClientPermListEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID) {
}

void ts3plugin_onChannelClientPermListEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onChannelClientPermListFinishedEvent(uint64 serverConnectionHandlerID, uint64 channelID, uint64 clientDatabaseID) {
}

void ts3plugin_onClientChannelGroupChangedEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, uint64 channelID, anyID clientID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

int ts3plugin_onServerPermissionErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, unsigned int failedPermissionID) {
	return 0;  /* See onServerErrorEvent for return code description */
}

void ts3plugin_onPermissionListGroupEndIDEvent(uint64 serverConnectionHandlerID, unsigned int groupEndID) {
}

void ts3plugin_onPermissionListEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, const char* permissionName, const char* permissionDescription) {
}

void ts3plugin_onPermissionListFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onPermissionOverviewEvent(uint64 serverConnectionHandlerID, uint64 clientDatabaseID, uint64 channelID, int overviewType, uint64 overviewID1, uint64 overviewID2, unsigned int permissionID, int permissionValue, int permissionNegated, int permissionSkip) {
}

void ts3plugin_onPermissionOverviewFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
}

void ts3plugin_onClientNeededPermissionsEvent(uint64 serverConnectionHandlerID, unsigned int permissionID, int permissionValue) {
}

void ts3plugin_onClientNeededPermissionsFinishedEvent(uint64 serverConnectionHandlerID) {
}

void ts3plugin_onFileTransferStatusEvent(anyID transferID, unsigned int status, const char* statusMessage, uint64 remotefileSize, uint64 serverConnectionHandlerID) {
}

void ts3plugin_onClientChatClosedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {
}

void ts3plugin_onClientChatComposingEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientUniqueIdentity) {
}

void ts3plugin_onServerLogEvent(uint64 serverConnectionHandlerID, const char* logMsg) {
}

void ts3plugin_onServerLogFinishedEvent(uint64 serverConnectionHandlerID, uint64 lastPos, uint64 fileSize) {
}

void ts3plugin_onMessageListEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, uint64 timestamp, int flagRead) {
}

void ts3plugin_onMessageGetEvent(uint64 serverConnectionHandlerID, uint64 messageID, const char* fromClientUniqueIdentity, const char* subject, const char* message, uint64 timestamp) {
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
}

void ts3plugin_onClientNamefromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
}

void ts3plugin_onClientNamefromDBIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
}

void ts3plugin_onComplainListEvent(uint64 serverConnectionHandlerID, uint64 targetClientDatabaseID, const char* targetClientNickName, uint64 fromClientDatabaseID, const char* fromClientNickName, const char* complainReason, uint64 timestamp) {
}

void ts3plugin_onBanListEvent(uint64 serverConnectionHandlerID, uint64 banid, const char* ip, const char* name, const char* uid, uint64 creationTime, uint64 durationTime, const char* invokerName,
							  uint64 invokercldbid, const char* invokeruid, const char* reason, int numberOfEnforcements, const char* lastNickName) {
}

void ts3plugin_onClientServerQueryLoginPasswordEvent(uint64 serverConnectionHandlerID, const char* loginPassword) {
}

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand) {
	printf("ON PLUGIN COMMAND: %s %s\n", pluginName, pluginCommand);
}

void ts3plugin_onIncomingClientQueryEvent(uint64 serverConnectionHandlerID, const char* commandText) {
}

void ts3plugin_onServerTemporaryPasswordListEvent(uint64 serverConnectionHandlerID, const char* clientNickname, const char* uniqueClientIdentifier, const char* description, const char* password, uint64 timestampStart, uint64 timestampEnd, uint64 targetChannelID, const char* targetChannelPW) {
}

/* Client UI callbacks */

/*
 * Called from client when an avatar image has been downloaded to or deleted from cache.
 * This callback can be called spontaneously or in response to ts3Functions.getAvatar()
 */
void ts3plugin_onAvatarUpdated(uint64 serverConnectionHandlerID, anyID clientID, const char* avatarPath) {
	/* If avatarPath is NULL, the avatar got deleted */
	/* If not NULL, avatarPath contains the path to the avatar file in the TS3Client cache */
	if(avatarPath != NULL) {
		printf("onAvatarUpdated: %llu %d %s\n", (long long unsigned int)serverConnectionHandlerID, clientID, avatarPath);
	} else {
		printf("onAvatarUpdated: %llu %d - deleted\n", (long long unsigned int)serverConnectionHandlerID, clientID);
	}
}

/*
 * Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
 * 
 * Parameters:
 * - serverConnectionHandlerID: ID of the current server tab
 * - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
 * - menuItemID: Id used when creating the menu item
 * - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
 */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID) {
	theApp->OnMenuItemEvent(type, menuItemID, selectedItemID);
	
}

/* This function is called if a plugin hotkey was pressed. Omit if hotkeys are unused. */
void ts3plugin_onHotkeyEvent(const char* keyword) {
	printf("PLUGIN: Hotkey event: %s\n", keyword);
	theApp->OnHotkey(keyword);
	/* Identify the hotkey by keyword ("keyword_1", "keyword_2" or "keyword_3" in this example) and handle here... */
}

/* Called when recording a hotkey has finished after calling ts3Functions.requestHotkeyInputDialog */
void ts3plugin_onHotkeyRecordedEvent(const char* keyword, const char* key) {
}

/* Called when client custom nickname changed */
void ts3plugin_onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName, const char* uniqueClientIdentifier) {
}

//
//
//int PlayWelcomeSound() {
//	//uint64 scHandlerID;
//
//	char* result_before;
//	ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &result_before);
//
//	printf("\n    playing welcome sound \n");
//
//
//	char* previousDeviceName;
//	char* previousMode;
//	BOOL isDefault;
//	ts3Functions.getCurrentCaptureDeviceName(connection, &previousDeviceName, &isDefault);
//	ts3Functions.getCurrentCaptureMode(connection, &previousMode);
//	
//
//	ts3Functions.closeCaptureDevice(connection);
//
//	int error;
//	if ((error = ts3Functions.openCaptureDevice(connection, "custom", myDeviceId)) != ERROR_ok) {
//		printf("Error opening capture device: 0x%x\n", error);
//		return error;
//	} else {
//		cout << "\tdevice id: " << myDeviceId << endl;
//	}
//
//
//	
//	cout << "buffer size: " << buffer_size << endl;
//
//
//	ts3Functions.activateCaptureDevice(connection);
//
//
//	capturePeriodSize = (captureFrequency * 20) / 1000;
//
//	int captureAudioOffset = 0;
//	for (audioPeriodCounter = 0; audioPeriodCounter < 50 * AUDIO_PROCESS_SECONDS; ++audioPeriodCounter) { /*50*20=1000*/
//
//
//		/* make sure we dont stream past the end of our wave sample */
//		if (captureAudioOffset + capturePeriodSize > captureBufferSamples) {
//			captureAudioOffset = 0;
//			break;
//		}
//
//		// TODO: thread?
//		SLEEP(20);
//
//		/* stream capture data to the client lib */
//		if ((error = ts3Functions.processCustomCaptureData(myDeviceId, captureBuffer + captureAudioOffset*captureChannels, capturePeriodSize)) != ERROR_ok) {
//			printf("Failed to get stream capture data: %d\n", error);
//			return 1;
//		}
//
//
//		/*update buffer offsets */
//		captureAudioOffset += capturePeriodSize;
//	}
//
//
//	if ((error = ts3Functions.closeCaptureDevice(connection) != ERROR_ok)) {
//		printf("Error closeCaptureDevice: 0x%x\n", error);
//		//return 1;
//	}
//
//
//
//
//	if((error = ts3Functions.openCaptureDevice(connection, previousMode, previousDeviceName)) != ERROR_ok) {
//		printf("Error opening capture device: 0x%x\n", error);
//		return error;
//	} else {
//		cout << "\tnew device id: " << previousDeviceName << endl;
//	}
//
//	//if((error = ts3Functions.activateCaptureDevice(connection) ) != ERROR_ok) {
//	//	printf("Error activating capture device: 0x%x\n", error);
//	//	return error;
//	//} 
//
//	char* result_after;
//	ts3Functions.getPreProcessorConfigValue(connection, "voiceactivation_level", &result_after);
//
//	//cout << "results:" << std::endl;
//	//cout << result_before << std::endl;
//	//cout << result_after << std::endl;
//	//cout << "" << std::endl;
//
//	// ts resets it to 0 db unless we do this :'(
//	ts3Functions.setPreProcessorConfigValue(connection, "voiceactivation_level", result_before);
//
//	ts3Functions.freeMemory(result_before);
//	ts3Functions.freeMemory(result_after);
//
//	/*
//	If the capture device for a given server connection handler has been deactivated by the Client
//Lib, the flag CLIENT_INPUT_HARDWARE will be set. This can be queried with the function
//ts3client_getClientSelfVariableAsInt.
//	*/
//
//
//	printf("\n    finished playing welcome sound \n");
//	
//
//	return 0;
//}


