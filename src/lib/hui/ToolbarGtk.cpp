/*
 * HuiToolbarGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "Controls/Control.h"
#include "Toolbar.h"
#include "Window.h"

#include <gtk/gtk.h>

namespace hui
{

void DBDEL_START(const string &type, const string &id, void *p);
void DBDEL_X(const string &);
void DBDEL_DONE();

#ifdef HUI_API_GTK

Toolbar::Toolbar(Window *_win, bool vertical) {
	win = _win;
	enabled = false;
	text_enabled = true;
	large_icons = true;

#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_box_new(vertical ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_add_css_class(widget, "toolbar");
#else
	widget = gtk_toolbar_new();
	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(widget), true);
	if (vertical)
		gtk_orientable_set_orientation(GTK_ORIENTABLE(widget), GTK_ORIENTATION_VERTICAL);
	//configure(true, true);
	//configure(false, true);
#endif
}

Toolbar::~Toolbar() {
	DBDEL_START("toolbar", id, this);
	reset();
	DBDEL_DONE();
}


void Toolbar::enable(bool _enabled) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_widget_set_visible(widget, _enabled);
#else
	if (_enabled)
		gtk_widget_show(widget);
	else
		gtk_widget_hide(widget);
#endif
	enabled = _enabled;
}

void Toolbar::configure(bool _text_enabled, bool _large_icons) {
#if !GTK_CHECK_VERSION(4,0,0)
	gtk_toolbar_set_style(GTK_TOOLBAR(widget), _text_enabled ? GTK_TOOLBAR_BOTH : GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(widget), _large_icons ? GTK_ICON_SIZE_LARGE_TOOLBAR : GTK_ICON_SIZE_SMALL_TOOLBAR);
	text_enabled = _text_enabled;
	large_icons = _large_icons;
#endif
}

void Toolbar::_add(shared<Control> c) {
	c->panel = win;
	items.add(c);
	//gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(c->widget), true);

#if GTK_CHECK_VERSION(4,0,0)
	DBDEL_X("TB add1");
	gtk_box_append(GTK_BOX(widget), c->get_frame());
	DBDEL_X("TB add2");
#else
	gtk_widget_show(c->widget);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), GTK_TOOL_ITEM(c->widget), -1);
#endif
}

void Toolbar::set_options(const string &options) {
#if !GTK_CHECK_VERSION(4,0,0)
	auto r = parse_options(options);
	for (auto x: r) {
		auto op = x.first;
		auto val = x.second;

		if (op == "style") {
			if (val == "text")
				gtk_toolbar_set_style(GTK_TOOLBAR(widget), GTK_TOOLBAR_TEXT);
			else if (val == "both")
				gtk_toolbar_set_style(GTK_TOOLBAR(widget), GTK_TOOLBAR_BOTH);
			else if (val == "icons")
				gtk_toolbar_set_style(GTK_TOOLBAR(widget), GTK_TOOLBAR_ICONS);
		} else if (op == "size") {
			if (val == "large")
				gtk_toolbar_set_icon_size(GTK_TOOLBAR(widget), GTK_ICON_SIZE_LARGE_TOOLBAR);
			else if (val == "small")
				gtk_toolbar_set_icon_size(GTK_TOOLBAR(widget), GTK_ICON_SIZE_SMALL_TOOLBAR);
		}
	}
#endif
}

#endif

};
