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

gboolean on_get_menu_click(GtkWidget *widget, gpointer data);

string get_gtk_action_name(const string &id, bool with_scope);

MenuItemToggle::MenuItemToggle(const string &title, const string &id) :
	Control(MENU_ITEM_TOGGLE, id)
{
#if GTK_CHECK_VERSION(4,0,0)
	item = g_menu_item_new(get_lang_sys(id, get_lang_sys(id, title), false), get_gtk_action_name(id, true).c_str());
#else
	widget = gtk_check_menu_item_new_with_label(get_lang_sys(id, title, false));
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(on_get_menu_click), this);
#endif
}

void MenuItemToggle::__check(bool checked) {
#if !GTK_CHECK_VERSION(4,0,0)
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), checked);
#endif
}

bool MenuItemToggle::is_checked() {
#if !GTK_CHECK_VERSION(4,0,0)
	return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
#endif
}

};

#endif
