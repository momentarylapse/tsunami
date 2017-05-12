/*
 * MenuItemSubmenuGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "../Menu.h"
#include "MenuItemSubmenu.h"

#ifdef HUI_API_GTK

namespace hui
{

MenuItemSubmenu::MenuItemSubmenu(const string &title, Menu *menu, const string &id) :
	Control(MENU_ITEM_SUBMENU, id)
{
	widget = gtk_menu_item_new_with_label(get_lang_sys(id, title));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), menu->widget);

	sub_menu = menu;
}

MenuItemSubmenu::~MenuItemSubmenu()
{
	delete(sub_menu);
}

};

#endif

