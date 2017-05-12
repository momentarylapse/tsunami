/*
 * HuiMenuItemSeparatorGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "MenuItemSeparator.h"

#ifdef HUI_API_GTK

namespace hui
{

MenuItemSeparator::MenuItemSeparator() :
	Control(MENU_ITEM_SEPARATOR, "")
{
	widget = gtk_separator_menu_item_new();
}

};

#endif

