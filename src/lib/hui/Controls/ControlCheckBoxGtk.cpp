/*
 * HuiControlCheckBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlCheckBox.h"
#include "../Event.h"
#include "../internal.h"
#include "../language.h"
#include "../../os/config.h"

#include <gtk/gtk.h>

namespace hui {

void on_gtk_checkbox_clicked(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);
}

void on_gtk_switch_clicked(GtkWidget *widget, GParamSpec *pspec, gpointer data) {
	//msg_write(" - switch clicked " + p2s(pspec) + "  " + p2s(data));
	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);
}

bool on_gtk_switch_clicked2(GtkWidget *widget, gboolean state, gpointer data) {
	//msg_write(" - switch change " + p2s(data));
	//reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);
	return false;
}

ControlCheckBox::ControlCheckBox(const string &title, const string &id) :
	Control(CONTROL_CHECKBOX, id)
{
	auto parts = split_title(title);
	is_switch = option_has(get_option_from_title(title), "switch");
	if (hui::config.has("hui.prevent-checkbox-switch"))
		is_switch = false;
	if (is_switch) {
		widget = gtk_switch_new();
		gtk_widget_set_valign(widget, GTK_ALIGN_CENTER);
		g_signal_connect(G_OBJECT(widget), "notify::active", G_CALLBACK(&on_gtk_switch_clicked), this);
		//g_signal_connect(G_OBJECT(widget), "state-set", G_CALLBACK(&on_gtk_switch_clicked2), this);
	} else {
		widget = gtk_check_button_new_with_label(sys_str(parts[0]));
#if GTK_CHECK_VERSION(4,0,0)
		g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&on_gtk_checkbox_clicked), this);
#else
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_checkbox_clicked), this);
#endif
	}
	take_gtk_ownership();
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
	if (is_switch) {
		gtk_switch_set_active(GTK_SWITCH(widget), checked);
	} else {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_check_button_set_active(GTK_CHECK_BUTTON(widget), checked);
#else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
#endif
	}
}

bool ControlCheckBox::is_checked() {
	if (is_switch) {
		return gtk_switch_get_active(GTK_SWITCH(widget));
	} else {
#if GTK_CHECK_VERSION(4,0,0)
		return gtk_check_button_get_active(GTK_CHECK_BUTTON(widget));
#else
		return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
#endif
	}
}

};

