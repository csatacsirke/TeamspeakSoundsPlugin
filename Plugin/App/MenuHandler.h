#pragma once



class MenuHandler {
	struct MenuItem{
		CStringA title;
		std::function<void()> callback;
	};

private:
	//std::map<CStringA, MenuItem> hotkeys;
	std::vector<MenuItem> menuItems;
public:
	void Add(CStringA title, std::function<void()> callback);
	void OnMenuItemEvent(PluginMenuType type, int menuItemID, uint64 selectedItemID);
	void Configure(struct PluginMenuItem*** hotkey);
};



