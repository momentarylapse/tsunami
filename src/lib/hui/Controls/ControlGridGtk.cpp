/*
 * ControlGrid.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlGrid.h"
#include "../hui.h"

#ifdef HUI_API_GTK

namespace hui {

const int FRAME_MARGIN = 8;


void control_link(Control *parent, Control *child);
void control_unlink(Control *parent, Control *child);

void DBDEL_X(const string &m);

/*void on_gtk_destroy(GtkWidget *w, void *p) {
	auto c = (Control*)p;
	msg_write(format("DES  %s   c=%s  w=%s", c->id, p2s(c), p2s(w)));
}*/

ControlGrid::ControlGrid(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_GRID, id)
{
	vertical = false;
	button_bar = false;
	action_bar = false;
	widget = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(widget), panel->spacing);
	gtk_grid_set_column_spacing(GTK_GRID(widget), panel->spacing);
	set_options(get_option_from_title(title));

	//g_signal_connect(G_OBJECT(widget), "destroy", G_CALLBACK(&on_gtk_destroy), this);
}

void ControlGrid::add(Control *child, int x, int y) {
	if (vertical) {
		int t = x;
		x = y;
		y = t;
	}
	GtkWidget *child_widget = child->get_frame();
	gtk_grid_attach(GTK_GRID(widget), child_widget, x, y, 1, 1);
	control_link(this, child);


	DBDEL_X(format("Grid.add  %s  cw=%s", child->id, p2s(child->widget)));

	if (button_bar) {
		int width, height;
		gtk_widget_get_size_request(child_widget, &width, &height);
		if (width < 0)
			gtk_widget_set_size_request(child_widget, 100, height);
		//gtk_widget_set_halign(child_widget, GTK_ALIGN_END);
		child->set_options("padding=6");
	} else if (action_bar) {
		child->set_options("min-width=25,min-height=25,padding=3");
	} else {
		if (((child->type == CONTROL_GROUP) or (child->type == CONTROL_EXPANDER)) and (y > 0)) {
			gtk_widget_set_margin_top(child->widget, FRAME_MARGIN);
		}

	}
}

void ControlGrid::remove_child(Control *child) {
	DBDEL_X(format("Grid.remove  %s  cw=%s", child->id, p2s(child->widget)));
	auto child_widget = child->get_frame();
#if GTK_CHECK_VERSION(4,0,0)
	gtk_grid_remove(GTK_GRID(widget), child_widget);
#else
	gtk_container_remove(GTK_CONTAINER(widget), child_widget);
#endif
	control_unlink(this, child);
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
