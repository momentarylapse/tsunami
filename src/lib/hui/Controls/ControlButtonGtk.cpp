/*
 * HuiControlButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlButton.h"
#include "../hui.h"

namespace hui
{

#ifdef HUI_API_GTK

void *get_gtk_image(const string &image, GtkIconSize size); // -> hui_menu_gtk.cpp

void on_gtk_button_press(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify("hui:click");
}

gboolean on_gtk_link_button_activate(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify("hui:click");
	return true;
}

ControlButton::ControlButton(const string &title, const string &id, Panel *panel) :
		Control(CONTROL_BUTTON, id)
{
	auto parts = split_title(title);
	this->panel = panel;
	if (option_has(get_option_from_title(title), "link")) {
		widget = gtk_link_button_new_with_label(sys_str(parts[0]), sys_str(parts[0]));
		g_signal_connect(G_OBJECT(widget), "activate-link", G_CALLBACK(&on_gtk_link_button_activate), this);
		set_options("padding=4");
	} else {
		widget = gtk_button_new_with_label(sys_str(parts[0]));
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_button_press), this);
	}

//	SetImageById(this, id);
}

string ControlButton::get_string() {
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void ControlButton::__set_string(const string &str) {
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlButton::set_image(const string& str) {
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), im);
#if GTK_CHECK_VERSION(3,6,0)
	if (strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
		gtk_button_set_always_show_image(GTK_BUTTON(widget), true);
#endif
}

void ControlButton::__set_option(const string &op, const string &value) {
	if (op == "flat") {
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
	} else if (op == "default") {
		gtk_widget_set_can_default(widget, true);
		if (panel->win) // otherwise gtk will complain
			gtk_widget_grab_default(widget);
		auto sc = gtk_widget_get_style_context(widget);
		gtk_style_context_add_class(sc, GTK_STYLE_CLASS_SUGGESTED_ACTION);
	} else if (op == "danger") {
		auto sc = gtk_widget_get_style_context(widget);
		gtk_style_context_remove_class(sc, GTK_STYLE_CLASS_SUGGESTED_ACTION);
		gtk_style_context_add_class(sc, GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
	}
}

#endif

};
