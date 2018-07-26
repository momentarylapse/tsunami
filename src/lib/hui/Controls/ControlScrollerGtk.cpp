/*
 * ControlScrollerGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "ControlScroller.h"

#ifdef HUI_API_GTK

namespace hui
{

ControlScroller::ControlScroller(const string &title, const string &id) :
	Control(CONTROL_SCROLLER, id)
{
	widget = gtk_scrolled_window_new(nullptr, nullptr);
	viewport = nullptr;
	//viewport = gtk_viewport_new(NULL, NULL);
	//gtk_container_add(GTK_CONTAINER(widget), viewport);
	//gtk_widget_show(viewport);
	setOptions(OptionString);
}

void ControlScroller::add(Control *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	//gtk_container_add(GTK_CONTAINER(viewport), child_widget);
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
	children.add(child);
	child->parent = this;
}

};

#endif
