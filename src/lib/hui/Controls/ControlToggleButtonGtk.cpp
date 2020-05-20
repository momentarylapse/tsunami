/*
 * ControlToggleButton.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "ControlToggleButton.h"

#ifdef HUI_API_GTK

namespace hui
{

void *get_gtk_image(const string &image, GtkIconSize size); // -> hui_menu_gtk.cpp

void on_gtk_toggle_button_toggle(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlToggleButton::ControlToggleButton(const string &title, const string &id) :
	Control(CONTROL_TOGGLEBUTTON, id)
{
	auto parts = split_title(title);
	widget = gtk_toggle_button_new_with_label(sys_str(parts[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&on_gtk_toggle_button_toggle), this);
	set_options(get_option_from_title(title));
}

string ControlToggleButton::get_string() {
	return "";
}

void ControlToggleButton::__set_string(const string &str) {
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlToggleButton::set_image(const string& str) {
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), im);
	#if GTK_CHECK_VERSION(3,6,0)
		if (strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
			gtk_button_set_always_show_image(GTK_BUTTON(widget), true);
	#endif
}

void ControlToggleButton::__check(bool checked) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool ControlToggleButton::is_checked() {
	return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void ControlToggleButton::__set_option(const string &op, const string &value) {
	if (op == "flat")
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
}

};

#endif
