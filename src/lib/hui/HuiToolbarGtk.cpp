/*
 * HuiToolbarGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiToolbar.h"
#include "Controls/HuiControl.h"

#ifdef HUI_API_GTK

HuiToolbar::HuiToolbar(HuiWindow *_win, bool vertical)
{
	win = _win;
	enabled = false;
	text_enabled = true;
	large_icons = true;

	widget = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(widget), true);
	if (vertical)
		gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
}

HuiToolbar::~HuiToolbar()
{
	Reset();
}


void HuiToolbar::Enable(bool _enabled)
{
	if (_enabled)
		gtk_widget_show(widget);
	else
		gtk_widget_hide(widget);
	enabled = _enabled;
}

void HuiToolbar::Configure(bool _text_enabled, bool _large_icons)
{
	gtk_toolbar_set_style(GTK_TOOLBAR(widget), _text_enabled ? GTK_TOOLBAR_BOTH : GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(widget), _large_icons ? GTK_ICON_SIZE_LARGE_TOOLBAR : GTK_ICON_SIZE_SMALL_TOOLBAR);
	text_enabled = _text_enabled;
	large_icons = _large_icons;
}

void HuiToolbar::add(HuiControl *c)
{
	c->panel = win;
	item.add(c);
	win->control.add(c);
	//gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(c->widget), true);
	gtk_widget_show(c->widget);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), GTK_TOOL_ITEM(c->widget), -1);
}

#endif

