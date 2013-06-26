/*
 * HuiMenuItemSeparator.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItemSeparator.h"

HuiMenuItemSeparator::HuiMenuItemSeparator() :
	HuiControl(HuiKindMenuItemSeparator, "")
{
	widget = gtk_separator_menu_item_new();
}

HuiMenuItemSeparator::~HuiMenuItemSeparator()
{
}

