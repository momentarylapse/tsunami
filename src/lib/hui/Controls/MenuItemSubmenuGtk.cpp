/*
 * MenuItemSubmenuGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "../Menu.h"
#include "MenuItemSubmenu.h"
#include "../language.h"

#include <gtk/gtk.h>

namespace hui
{

MenuItemSubmenu::MenuItemSubmenu(const string &title, xfer<Menu> menu, const string &id) :
	BasicMenuItem(MENU_ITEM_SUBMENU, id)
{
#if GTK_CHECK_VERSION(4,0,0)
	item = g_menu_item_new(get_lang_sys(id, title), nullptr);
	g_menu_item_set_submenu(item, G_MENU_MODEL(menu->gmenu));
#else
	widget = gtk_menu_item_new_with_label(get_lang_sys(id, title));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), menu->widget);
#endif
	sub_menu = menu;
	take_gtk_ownership();
}

MenuItemSubmenu::~MenuItemSubmenu() {
#if !GTK_CHECK_VERSION(4,0,0)
	//delete(sub_menu);
#endif
}

};
