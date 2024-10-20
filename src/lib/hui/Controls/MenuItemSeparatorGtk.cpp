/*
 * HuiMenuItemSeparatorGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "MenuItemSeparator.h"

#include <gtk/gtk.h>

namespace hui
{

MenuItemSeparator::MenuItemSeparator() :
	BasicMenuItem(MENU_ITEM_SEPARATOR, "")
{
#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
	widget = gtk_separator_menu_item_new();
#endif
	take_gtk_ownership();
}

};

