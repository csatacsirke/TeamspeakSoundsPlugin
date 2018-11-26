#include "stdafx.h"
#include "MenuHandler.h"




/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
	struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
	menuItem->type = type;
	menuItem->id = id;
	_strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
	_strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
	return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *pluginMenuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*pluginMenuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*pluginMenuItems)[n++] = NULL; assert(n == sz);



void MenuHandler::Add(CStringA title, std::function<void()> callback) {
	const int menuItemId = (int)menuItems.size();
	MenuItem menuItem = { title, callback };
	menuItems.push_back(menuItem);
}

void MenuHandler::OnMenuItemEvent(PluginMenuType type, int menuItemID, uint64 selectedItemID) {

	if (menuItemID >= menuItems.size()) {
		assert(0);
		return;
	}

	menuItems[menuItemID].callback();

}

void MenuHandler::Configure(PluginMenuItem *** pluginMenuItems) {
	//PLUGIN_MENU_TYPE_GLOBAL, menuItemId, title, 0

	BEGIN_CREATE_MENUS(menuItems.size());  /* IMPORTANT: Number of menu items must be correct! */

	int currentId = 0;
	for (auto& menuItem : menuItems) {
		int id = currentId++;
		CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, id, menuItem.title, "");
		//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_2, "Enqueue sound from file...", "2.png");
		//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_3, "Audio Processor Dialog", "3.png");
		//CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_4, "Open debug console", "4.png");
	}
	
	
	END_CREATE_MENUS;  /* Includes an assert checking if the number of menu items matched */
}
