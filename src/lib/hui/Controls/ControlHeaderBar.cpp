/*
 * ControlHeaderBar.cpp
 *
 *  Created on: 11 Feb 2022
 *      Author: michi
 */

#include "ControlHeaderBar.h"
#include "../common.h"
#include "../../os/msg.h"
#include <gtk/gtk.h>


namespace hui {

ControlHeaderBar::ControlHeaderBar(const string &title, const string &id, Panel *_panel) :
	Control(CONTROL_HEADER_BAR, id)
{
	panel = _panel;
	widget = gtk_header_bar_new();

#if GTK_CHECK_VERSION(4,0,0)
	gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(widget), true);
#else
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widget), true);
	gtk_widget_show(widget);
#endif
	take_gtk_ownership();
}

void ControlHeaderBar::add_child(shared<Control> child, int x, int y) {
	GtkWidget *child_widget = child->get_frame();

	if (y == 1)
		gtk_header_bar_pack_end(GTK_HEADER_BAR(widget), child_widget);
	else
		gtk_header_bar_pack_start(GTK_HEADER_BAR(widget), child_widget);
	control_link(this, child);
}

void ControlHeaderBar::remove_child(Control *child) {
	msg_write("Header.remove");
	//gtk_grid_remove(GTK_GRID(widget), child->widget);
	control_unlink(this, child);
}

void ControlHeaderBar::__set_option(const string &op, const string &value) {
	if (op == "closebutton") {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(widget), val_is_positive(value, true));
#else
		gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widget), val_is_positive(value, true));
#endif
	}
}

};
