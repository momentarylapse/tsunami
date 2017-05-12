/*
 * ControlRevealerGtk.cpp
 *
 *  Created on: 17.05.2015
 *      Author: michi
 */

#include "ControlRevealer.h"

#ifdef HUI_API_GTK

namespace hui
{

void OnGtkExpanderExpand(GObject* object, GParamSpec *param_spec, gpointer user_data);

ControlRevealer::ControlRevealer(const string &title, const string &id) :
	Control(CONTROL_REVEALER, id)
{
	GetPartStrings(title);

#if GTK_CHECK_VERSION(3,10,0)
	widget = gtk_revealer_new();
#else
	widget = gtk_expander_new(sys_str("<b>Revealer...</b>"));
	gtk_expander_set_use_markup(GTK_EXPANDER(widget), true);
	g_signal_connect(widget, "notify::expanded", G_CALLBACK(OnGtkExpanderExpand), NULL);
	if (!gtk_expander_get_expanded(GTK_EXPANDER(widget)))
		gtk_widget_set_vexpand(widget, false);
#endif

	setOptions(OptionString);

#if GTK_CHECK_VERSION(3,10,0)
	if (OptionString.find("slide-up") >= 0)
		gtk_revealer_set_transition_type(GTK_REVEALER(widget), GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
	if (OptionString.find("slide-down") >= 0)
		gtk_revealer_set_transition_type(GTK_REVEALER(widget), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
	if (OptionString.find("slide-left") >= 0)
		gtk_revealer_set_transition_type(GTK_REVEALER(widget), GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
	if (OptionString.find("slide-right") >= 0)
		gtk_revealer_set_transition_type(GTK_REVEALER(widget), GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
	if (OptionString.find("cross-fade") >= 0)
		gtk_revealer_set_transition_type(GTK_REVEALER(widget), GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
#endif
}


void ControlRevealer::add(Control *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
	children.add(child);
	child->parent = this;
}

void ControlRevealer::reveal(bool reveal)
{
#if GTK_CHECK_VERSION(3,10,0)
	gtk_revealer_set_reveal_child(GTK_REVEALER(widget), reveal);
#else
	gtk_expander_set_expanded(GTK_EXPANDER(widget), reveal);
#endif
}

bool ControlRevealer::isRevealed()
{
#if GTK_CHECK_VERSION(3,10,0)
	return gtk_revealer_get_reveal_child(GTK_REVEALER(widget));
#else
	return (bool)gtk_expander_get_expanded(GTK_EXPANDER(widget));
#endif
}

};

#endif
