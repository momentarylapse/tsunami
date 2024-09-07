/*
 * ControlScrollerGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "ControlScroller.h"
#include "../../os/msg.h"

#include <gtk/gtk.h>

namespace hui {

ControlScroller::ControlScroller(const string &title, const string &id) :
	Control(CONTROL_SCROLLER, id)
{
#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_scrolled_window_new();
#else
	widget = gtk_scrolled_window_new(nullptr, nullptr);
#endif
	viewport = nullptr;
	//viewport = gtk_viewport_new(NULL, NULL);
	//gtk_container_add(GTK_CONTAINER(widget), viewport);
	//gtk_widget_show(viewport);
#if GTK_CHECK_VERSION(3,22,0)
	gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(widget), true);
	gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(widget), true);
#endif
	take_gtk_ownership();
	set_options(get_option_from_title(title));
}

void ControlScroller::add_child(shared<Control> child, int x, int y) {
	GtkWidget *child_widget = child->get_frame();
	//gtk_container_add(GTK_CONTAINER(viewport), child_widget);
#if GTK_CHECK_VERSION(4,0,0)
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(widget), child_widget);
#else
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
#endif
	control_link(this, child);
}

void ControlScroller::remove_child(Control *child) {
	msg_write("Scroller.remove");
#if GTK_CHECK_VERSION(4,0,0)
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(widget), nullptr);
#else
	GtkWidget *child_widget = child->get_frame();
	gtk_container_remove(GTK_CONTAINER(widget), child_widget);
#endif
	control_unlink(this, child);
}


void ControlScroller::__set_option(const string& op, const string& value) {
	GtkPolicyType h_policy, v_policy;
	gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(widget), &h_policy, &v_policy);
	if (op == "scrollx") {
		if (value == "no")
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), GTK_POLICY_NEVER, v_policy);
		if (value == "yes")
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), GTK_POLICY_AUTOMATIC, v_policy);
	} else if (op == "scrolly") {
		if (value == "no")
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), h_policy, GTK_POLICY_NEVER);
		if (value == "yes")
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget), h_policy, GTK_POLICY_AUTOMATIC);
	}
}

};
