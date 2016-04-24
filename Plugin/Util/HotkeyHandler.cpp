#include "stdafx.h"

#include "HotkeyHandler.h"


void HotkeyHandler::Add(CStringA key, CStringA title, std::function<void()> callback) {
	HotkeyRecord record = { title, callback };
	hotkeys.insert(std::make_pair(key, record));
}

void HotkeyHandler::OnHotkeyEvent(CStringA key) {
	auto it = hotkeys.find(key);
	if(it != hotkeys.end()) {
		auto callback = (*it).second.callback;
		callback();
	}
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

void HotkeyHandler::Configure(struct PluginHotkey*** hotkeys) {
	// TODO névütközést megoldani szebbre
	size_t size = this->hotkeys.size();

	/* Create x hotkeys. Size must be correct for allocating memory. */
	BEGIN_CREATE_HOTKEYS(size);

	for(auto& record : this->hotkeys) {
		CREATE_HOTKEY(record.first, record.second.title);
	}
/*
	CREATE_HOTKEY(Hotkey::STOP, "Stop playback");
	CREATE_HOTKEY(Hotkey::PLAY_QUEUED, "Play queued");
	CREATE_HOTKEY(Hotkey::REPLAY, "Replay");
	for(int i = 0; i < soundHotkeyCount; ++i) {
		CStringA hotkey;
		hotkey.Format(Hotkey::PLAY_PRESET_TEMPLATE, i);

		CStringA title;
		title.Format("Play sound #%d", i);

		CREATE_HOTKEY(Hotkey::REPLAY, "Replay");
	}*/

	END_CREATE_HOTKEYS;
}
