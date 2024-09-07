/*
 * ControlRadioButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlRadioButton.h"
#include "../hui.h"

#include <gtk/gtk.h>

namespace hui {

void on_gtk_radio_button_toggle(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);
}

ControlRadioButton::ControlRadioButton(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_RADIOBUTTON, id)
{
	auto parts = split_title(title);
	string group_id = id.head(id.find(":"));
#if GTK_CHECK_VERSION(4,0,0)
	GtkWidget *group = nullptr;
#else
	GSList *group = nullptr;
#endif

	panel->apply_foreach("*", [&](Control *c) {
		if (c->type == CONTROL_RADIOBUTTON)
			if (c->id.find(":"))
				if (c->id.head(c->id.find(":")) == group_id) {
#if GTK_CHECK_VERSION(4,0,0)
					group = c->widget;
#else
					group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(c->widget));
#endif
				}
	});

#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_check_button_new_with_label(sys_str(parts[0]));
	if (group)
		gtk_check_button_set_group(GTK_CHECK_BUTTON(widget), GTK_CHECK_BUTTON(group));
#else
	widget = gtk_radio_button_new_with_label(group, sys_str(parts[0]));
#endif
	take_gtk_ownership();
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&on_gtk_radio_button_toggle), this);
	focusable = true;
	set_options(get_option_from_title(title));
}

void ControlRadioButton::__set_string(const string &str) {
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlRadioButton::__check(bool checked) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), checked);
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
#endif
}

string ControlRadioButton::get_string() {
	return gtk_button_get_label(GTK_BUTTON(widget));
}

bool ControlRadioButton::is_checked() {
#if GTK_CHECK_VERSION(4,0,0)
	return (bool)gtk_check_button_get_active(GTK_CHECK_BUTTON(widget));
#else
	return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
#endif
}

};
