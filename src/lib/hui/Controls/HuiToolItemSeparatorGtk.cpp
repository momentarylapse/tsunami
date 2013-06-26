/*
 * HuiToolItemSeparator.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiToolItemSeparator.h"

HuiToolItemSeparator::HuiToolItemSeparator() :
	HuiControl(HuiKindToolSeparator, "")
{
	widget = GTK_WIDGET(gtk_separator_tool_item_new());
}

HuiToolItemSeparator::~HuiToolItemSeparator()
{
}

