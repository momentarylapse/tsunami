/*
 * HuiMenuItemSubmenuGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItemSubmenu.h"
#include "../HuiMenu.h"

#ifdef HUI_API_GTK

HuiMenuItemSubmenu::HuiMenuItemSubmenu(const string &title, HuiMenu *menu, const string &id) :
	HuiControl(HuiKindMenuItemSubmenu, id)
{
	widget = gtk_menu_item_new_with_label(get_lang_sys(id, title));
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), menu->widget);

	sub_menu = menu;
}

HuiMenuItemSubmenu::~HuiMenuItemSubmenu()
{
	delete(sub_menu);
}

#endif

