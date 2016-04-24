#pragma once



class HotkeyHandler {
	struct HotkeyRecord {
		CStringA title;
		std::function<void()> callback;
	};

private:
	std::map<CStringA, HotkeyRecord> hotkeys;

public:
	void Add(CStringA key, CStringA title, std::function<void()> callback);
	void OnHotkeyEvent(CStringA key);
	void Configure(struct PluginHotkey*** hotkey);
};
