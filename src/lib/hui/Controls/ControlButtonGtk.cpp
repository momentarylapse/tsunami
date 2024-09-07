/*
 * HuiControlButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlButton.h"
#include "../Window.h"
#include "../../os/msg.h"
#include <gtk/gtk.h>

#ifdef HUGE
#undef HUGE
#endif

namespace hui
{

#ifdef HUI_API_GTK

GtkWidget *get_gtk_image_x(const string &image, IconSize size, GtkWidget *w); // -> hui_menu_gtk.cpp

void on_gtk_button_press(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CLICK);
}

gboolean on_gtk_link_button_activate(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CLICK);
	return true;
}

ControlButton::ControlButton(const string &title, const string &id, Panel *panel) :
		Control(CONTROL_BUTTON, id)
{
	auto parts = split_title(title);
	this->panel = panel;
	if (option_has(get_option_from_title(title), "link")) {
		widget = gtk_link_button_new_with_label(parts[0].c_str(), parts[0].c_str());
		g_signal_connect(G_OBJECT(widget), "activate-link", G_CALLBACK(&on_gtk_link_button_activate), this);
		//set_options("padding=2");
	} else {
		widget = gtk_button_new_with_label(parts[0].c_str());
		g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_button_press), this);
	}
	take_gtk_ownership();

	image_size = IconSize::REGULAR;
	focusable = true;
	set_options(get_option_from_title(title));

//	SetImageById(this, id);
}

string ControlButton::get_string() {
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void ControlButton::__set_string(const string &str) {
	gtk_button_set_label(GTK_BUTTON(widget), str.c_str());
}

void ControlButton::set_image(const string& str) {
	GtkWidget *im = get_gtk_image_x(str, image_size, widget);
#if GTK_CHECK_VERSION(4,0,0)
	if (get_string() == "")
		gtk_button_set_child(GTK_BUTTON(widget), im);
#else
	gtk_button_set_image(GTK_BUTTON(widget), im);
#if GTK_CHECK_VERSION(3,6,0)
	if (strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
		gtk_button_set_always_show_image(GTK_BUTTON(widget), true);
#endif
#endif
}

void ControlButton::__set_option(const string &op, const string &value) {
	if (op == "flat") {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_button_set_has_frame(GTK_BUTTON(widget), false);
#else
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
#endif
	} else if (op == "default") {
#if GTK_CHECK_VERSION(4,0,0)
		if (panel->win) { // otherwise gtk will complain
			gtk_window_set_default_widget(GTK_WINDOW(panel->win->window), widget);
			gtk_widget_set_receives_default(GTK_WIDGET(widget), true);
		} else {
			msg_error("trying to set default without window..." + id);
		}
		add_css_class("suggested-action");
#else
		gtk_widget_set_can_default(widget, true);
		if (panel->win) // otherwise gtk will complain
			gtk_widget_grab_default(widget);
		add_css_class(GTK_STYLE_CLASS_SUGGESTED_ACTION);
#endif
	} else if (op == "danger") {
#if GTK_CHECK_VERSION(4,0,0)
		remove_css_class("suggested-action");
		add_css_class("destructive-action");
#else
		remove_css_class(GTK_STYLE_CLASS_SUGGESTED_ACTION);
		add_css_class(GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
#endif
	} else if (op == "circular") {
		add_css_class("circular");
	} else if (op == "big") {
		image_size = IconSize::LARGE;
	} else if (op == "huge") {
		image_size = IconSize::HUGE;
	}

};

#endif

}

