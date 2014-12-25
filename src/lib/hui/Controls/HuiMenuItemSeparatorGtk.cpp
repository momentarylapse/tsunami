/*
 * HuiMenuItemSeparatorGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItemSeparator.h"

#ifdef HUI_API_GTK

HuiMenuItemSeparator::HuiMenuItemSeparator() :
	HuiControl(HuiKindMenuItemSeparator, "")
{
	widget = gtk_separator_menu_item_new();
}

#endif

