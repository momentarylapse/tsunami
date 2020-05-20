/*
 * ControlGrid.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlGrid.h"
#include "../hui.h"

#ifdef HUI_API_GTK

namespace hui
{

ControlGrid::ControlGrid(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_GRID, id)
{
	vertical = false;
	button_bar = false;
	action_bar = false;
	widget = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(widget), panel->border_width);
	gtk_grid_set_column_spacing(GTK_GRID(widget), panel->border_width);
	set_options(get_option_from_title(title));
}

void ControlGrid::add(Control *child, int x, int y) {
	if (vertical) {
		int t = x;
		x = y;
		y = t;
	}
	GtkWidget *child_widget = child->get_frame();
	bool hb = false;
	if (button_bar) {
		if (panel and panel->win and panel->win->headerbar)
			hb = true;
	}

	if (hb) {
		if (y == 1)
			gtk_header_bar_pack_end(GTK_HEADER_BAR(panel->win->headerbar), child_widget);
		else
			gtk_header_bar_pack_start(GTK_HEADER_BAR(panel->win->headerbar), child_widget);
		hide(true);
	} else {
		gtk_grid_attach(GTK_GRID(widget), child_widget, x, y, 1, 1);
	}
	child->parent = this;
	children.add(child);

	if (hb)
		return;

	if (button_bar) {
		int width, height;
		gtk_widget_get_size_request(child_widget, &width, &height);
		if (width < 0)
			gtk_widget_set_size_request(child_widget, 100, height);
		//gtk_widget_set_halign(child_widget, GTK_ALIGN_END);
		child->set_options("padding=6");
	} else if (action_bar) {
		child->set_options("min-width=25,min-height=25,padding=3");
	}
}

void ControlGrid::__set_option(const string &op, const string &value) {
	if (op == "buttonbar") {
		button_bar = true;
		gtk_widget_set_margin_top(widget, 7);
		gtk_widget_set_margin_bottom(widget, 4);
		gtk_widget_set_halign(widget, GTK_ALIGN_END);
	} else if (op == "actionbar") {
		action_bar = true;
	} else if (op == "vertical") {
		vertical = true;
	} else if (op == "homogenousx") {
		gtk_grid_set_column_homogeneous(GTK_GRID(widget), true);
	} else if (op == "homogenousy") {
		gtk_grid_set_row_homogeneous(GTK_GRID(widget), true);
	}
}

};

#endif
