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
	Control(TOOL_ITEM_SEPARATOR, "")
{
	widget = GTK_WIDGET(gtk_separator_tool_item_new());
}

}

#endif

