/*
 * HuiControlGroup.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlGroup.h"

#ifdef HUI_API_GTK

HuiControlGroup::HuiControlGroup(const string &title, const string &id) :
	HuiControl(HuiKindGroup, id)
{
	GetPartStrings(id, title);
	widget = gtk_frame_new(sys_str(PartString[0]));
	gtk_frame_set_shadow_type(GTK_FRAME(widget), GTK_SHADOW_NONE);
	GtkWidget *label = gtk_frame_get_label_widget(GTK_FRAME(widget));
	gtk_label_set_markup(GTK_LABEL(label), sys_str("<b>" + PartString[0] + "</b>"));
	SetOptions(OptionString);
}

HuiControlGroup::~HuiControlGroup()
{
}


void HuiControlGroup::add(HuiControl *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	gtk_widget_set_margin_left(child_widget, 20);
	gtk_widget_set_margin_top(child_widget, 2);
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
	children.add(child);
	child->parent = this;
}

#endif
