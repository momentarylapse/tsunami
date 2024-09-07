/*
 * HuiControlGroup.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlGroup.h"
#include "../language.h"
#include "../Window.h"
#include "../../os/msg.h"
#include <gtk/gtk.h>

namespace hui {

const int FRAME_INDENT = 0; //20;
const int FRAME_MARGIN_BOTTOM = 12;

ControlGroup::ControlGroup(const string &title, const string &id) :
	Control(CONTROL_GROUP, id)
{
	auto parts = split_title(title);
	widget = gtk_frame_new(sys_str(parts[0]));

#if GTK_CHECK_VERSION(4,0,0)
	add_css_class("hui-no-border");
#else
	gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_NONE);
#endif
	gtk_widget_set_margin_bottom(widget, FRAME_MARGIN_BOTTOM);
	take_gtk_ownership();

	GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(widget));
	gtk_label_set_markup(GTK_LABEL(label), sys_str("<b>" + parts[0] + "</b>"));
}

void ControlGroup::add_child(shared<Control> child, int x, int y) {
	GtkWidget *child_widget = child->get_frame();
	int ind = child->indent;
	if (ind < 0)
		ind = FRAME_INDENT;
#if GTK_CHECK_VERSION(3,12,0)
	gtk_widget_set_margin_start(child_widget, ind);
#else
	gtk_widget_set_margin_left(child_widget, ind);
#endif
	gtk_widget_set_margin_top(child_widget, 2);
#if GTK_CHECK_VERSION(4,0,0)
	gtk_frame_set_child(GTK_FRAME(widget), child_widget);
#else
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
#endif
	control_link(this, child);
}

void ControlGroup::remove_child(Control *child) {
	msg_write("Group.remove");
#if GTK_CHECK_VERSION(4,0,0)
	gtk_frame_set_child(GTK_FRAME(widget), nullptr);
#else
	GtkWidget *child_widget = child->get_frame();
	gtk_container_remove(GTK_CONTAINER(widget), child_widget);
#endif
	control_unlink(this, child);
}

};
