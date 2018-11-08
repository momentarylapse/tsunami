/*
 * HuiMenuItemToggleGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "MenuItemToggle.h"

#ifdef HUI_API_GTK

namespace hui
{

gboolean OnGtkMenuClick(GtkWidget *widget, gpointer data);

MenuItemToggle::MenuItemToggle(const string &title, const string &id) :
	Control(MENU_ITEM_TOGGLE, id)
{
	widget = gtk_check_menu_item_new_with_label(get_lang_sys(id, title, false));
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(OnGtkMenuClick), this);
}

void MenuItemToggle::__check(bool checked)
{
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), checked);
}

bool MenuItemToggle::is_checked()
{
	return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
}

};

#endif
