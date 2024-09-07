/*
 * ControlPanedGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "ControlPaned.h"
#include "../../os/msg.h"
#include <gtk/gtk.h>


namespace hui {

ControlPaned::ControlPaned(const string &title, const string &id) :
	Control(CONTROL_PANED, id)
{
	if (option_has(get_option_from_title(title), "vertical"))
		widget = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	else
		widget = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	take_gtk_ownership();
	set_options(get_option_from_title(title));
}

void ControlPaned::add_child(shared<Control> child, int x, int y) {
#if GTK_CHECK_VERSION(4,0,0)
	if (x == 0 and y == 0)
		gtk_paned_set_start_child(GTK_PANED(widget), child->get_frame());
	else
		gtk_paned_set_end_child(GTK_PANED(widget), child->get_frame());
#else
	if (x == 0 and y == 0)
		gtk_paned_add1(GTK_PANED(widget), child->get_frame());
	else
		gtk_paned_add2(GTK_PANED(widget), child->get_frame());
#endif
	control_link(this, child);
}

void ControlPaned::remove_child(Control *child) {
	msg_write("Paned.remove");
#if GTK_CHECK_VERSION(4,0,0)
	if (gtk_paned_get_start_child(GTK_PANED(widget)) == child->get_frame())
		gtk_paned_set_start_child(GTK_PANED(widget), nullptr);
	else if (gtk_paned_get_end_child(GTK_PANED(widget)) == child->get_frame())
		gtk_paned_set_end_child(GTK_PANED(widget), nullptr);
#else
	//gtk_container_add(GTK_CONTAINER(widget), child_widget);
#endif
}

int ControlPaned::get_int() {
	return gtk_paned_get_position(GTK_PANED(widget));
}

void ControlPaned::__set_int(int i) {
	gtk_paned_set_position(GTK_PANED(widget), i);
}

};
