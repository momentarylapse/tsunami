/*
 * HuiControlCheckBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlCheckBox.h"

#ifdef HUI_API_GTK

namespace hui {

void on_gtk_checkbox_clicked(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

void on_gtk_switch_clicked(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

void on_gtk_switch_clicked2(GtkWidget *widget, gboolean state, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlCheckBox::ControlCheckBox(const string &title, const string &id) :
	Control(CONTROL_CHECKBOX, id)
{
	auto parts = split_title(title);
	is_switch = option_has(get_option_from_title(title), "switch");
	if (is_switch) {
		widget = gtk_switch_new();
		//g_signal_connect(G_OBJECT(widget), "notify::active", G_CALLBACK(&on_gtk_switch_clicked), this);
		g_signal_connect(G_OBJECT(widget), "state-set", G_CALLBACK(&on_gtk_switch_clicked2), this);
	} else {
		widget = gtk_check_button_new_with_label(sys_str(parts[0]));
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_checkbox_clicked), this);
	}
}

string ControlCheckBox::get_string() {
	if (is_switch)
		return "";
	else
		return gtk_button_get_label(GTK_BUTTON(widget));
}

void ControlCheckBox::__set_string(const string &str) {
	if (!is_switch)
		gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlCheckBox::__check(bool checked) {
	if (is_switch)
		gtk_switch_set_active(GTK_SWITCH(widget), checked);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool ControlCheckBox::is_checked() {
	if (is_switch)
		return gtk_switch_get_active(GTK_SWITCH(widget));
	else
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

};

#endif
