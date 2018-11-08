/*
 * HuiControlGroup.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlGroup.h"
#include "../Window.h"

#ifdef HUI_API_GTK

namespace hui
{

ControlGroup::ControlGroup(const string &title, const string &id) :
	Control(CONTROL_GROUP, id)
{
	GetPartStrings(title);
	widget = gtk_frame_new(sys_str(PartString[0]));
	gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_NONE);
	GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(widget));
	gtk_label_set_markup(GTK_LABEL(label), sys_str("<b>" + PartString[0] + "</b>"));
	set_options(OptionString);
}

void ControlGroup::add(Control *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	int ind = child->indent;
	if (ind < 0)
		ind = 20;
#if GTK_CHECK_VERSION(3,12,0)
	gtk_widget_set_margin_start(child_widget, ind);
#else
	gtk_widget_set_margin_left(child_widget, ind);
#endif
	gtk_widget_set_margin_top(child_widget, 2);
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
	children.add(child);
	child->parent = this;
}

};

#endif
