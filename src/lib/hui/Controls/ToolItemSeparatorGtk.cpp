/*
 * HuiToolItemSeparatorGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "ToolItemSeparator.h"

#ifdef HUI_API_GTK

namespace hui
{

ToolItemSeparator::ToolItemSeparator() :
	Control(TOOL_ITEM_SEPARATOR, "-separator-")
{
	widget = GTK_WIDGET(gtk_separator_tool_item_new());

	// prevent errors with some themes
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(widget), false);
}

}

#endif

