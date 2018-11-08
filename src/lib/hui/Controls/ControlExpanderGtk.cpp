/*
 * ControlExpanderGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "ControlExpander.h"
#include "../Window.h"

#ifdef HUI_API_GTK

namespace hui
{

void OnGtkExpanderExpand(GObject* object, GParamSpec *param_spec, gpointer user_data)
{
	if (gtk_expander_get_expanded(GTK_EXPANDER(object))){
		gtk_widget_set_vexpand_set(GTK_WIDGET(object), false);
	}else{
		gtk_widget_set_vexpand(GTK_WIDGET(object), false);
	}
}

ControlExpander::ControlExpander(const string &title, const string &id) :
	Control(CONTROL_EXPANDER, id)
{
	GetPartStrings(title);
	widget = gtk_expander_new(sys_str("<b>" + PartString[0] + "</b>"));
	gtk_expander_set_use_markup(GTK_EXPANDER(widget), true);
	g_signal_connect(widget, "notify::expanded", G_CALLBACK(OnGtkExpanderExpand), nullptr);
	if (!gtk_expander_get_expanded(GTK_EXPANDER(widget)))
		gtk_widget_set_vexpand(widget, false);
	set_options(OptionString);
}

void ControlExpander::expand(int row, bool expand)
{
	gtk_expander_set_expanded(GTK_EXPANDER(widget), expand);
}

void ControlExpander::expand_all(bool expand)
{
	gtk_expander_set_expanded(GTK_EXPANDER(widget), expand);
}

bool ControlExpander::is_expanded(int row)
{
	return (bool)gtk_expander_get_expanded(GTK_EXPANDER(widget));
}

void ControlExpander::add(Control *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	//gtk_widget_set_vexpand(child_widget, true);
	//gtk_widget_set_hexpand(child_widget, true);
	int ind = child->indent;
	if (ind < 0)
		ind = 20;
#if GTK_CHECK_VERSION(3,12,0)
	gtk_widget_set_margin_start(child_widget, ind);
#else
	gtk_widget_set_margin_left(child_widget, ind);
#endif
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
	children.add(child);
	child->parent = this;
}

};

#endif
