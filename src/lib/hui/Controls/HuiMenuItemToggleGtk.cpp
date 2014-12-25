/*
 * HuiMenuItemToggleGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItemToggle.h"

#ifdef HUI_API_GTK

void try_add_accel(GtkWidget *item, const string &id);
gboolean OnGtkMenuClick(GtkWidget *widget, gpointer data);

HuiMenuItemToggle::HuiMenuItemToggle(const string &title, const string &id) :
	HuiControl(HuiKindMenuItemToggle, id)
{
	widget = gtk_check_menu_item_new_with_label(get_lang_sys(id, title, false));
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(OnGtkMenuClick), this);

	try_add_accel(widget, id);
}

void HuiMenuItemToggle::__check(bool checked)
{
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), checked);
}

bool HuiMenuItemToggle::isChecked()
{
	return gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
}

#endif
