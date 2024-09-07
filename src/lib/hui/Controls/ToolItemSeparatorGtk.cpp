/*
 * HuiToolItemSeparatorGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "ToolItemSeparator.h"

#include <gtk/gtk.h>

namespace hui
{

ToolItemSeparator::ToolItemSeparator() :
	Control(TOOL_ITEM_SEPARATOR, "-separator-")
{
#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
#else
	widget = GTK_WIDGET(gtk_separator_tool_item_new());

	// prevent errors with some themes
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(widget), false);
#endif
	take_gtk_ownership();
}

}


