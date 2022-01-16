/*
 * ControlMenuButton.cpp
 *
 *  Created on: 07.08.2018
 *      Author: michi
 */

#include "ControlMenuButton.h"
#include "../Menu.h"
#include "../Resource.h"


namespace hui
{

#ifdef HUI_API_GTK

void *get_gtk_image(const string &image, IconSize size); // -> hui_menu_gtk.cpp

Menu *_create_res_menu_(const string &ns, Resource *res); // -> Resource.cpp

//void OnGtkMenuButtonPress(GtkWidget *widget, gpointer data)
//{	reinterpret_cast<Control*>(data)->notify("hui:click");	}


ControlMenuButton::ControlMenuButton(const string &title, const string &id) :
	Control(CONTROL_MENU_BUTTON, id)
{
	auto parts = split_title(title);
	widget = gtk_menu_button_new();
#if GTK_CHECK_VERSION(4,0,0)
	gtk_menu_button_set_label(GTK_MENU_BUTTON(widget), sys_str(parts[0]));
#else
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(parts[0]));
#endif
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkMenuButtonPress), this);

	menu = NULL;

//	SetImageById(this, id);
	set_options(get_option_from_title(title));
}

string ControlMenuButton::get_string() {
#if GTK_CHECK_VERSION(4,0,0)
	return gtk_menu_button_get_label(GTK_MENU_BUTTON(widget));
#else
	return gtk_button_get_label(GTK_BUTTON(widget));
#endif
}

void ControlMenuButton::__set_string(const string &str) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_menu_button_set_label(GTK_MENU_BUTTON(widget), sys_str(str));
#else
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
#endif
}

void ControlMenuButton::set_image(const string& str) {
#if GTK_CHECK_VERSION(4,0,0)
	msg_error("TODO: MenuButton.set_image() gtk4");
#else
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, IconSize::REGULAR);
	gtk_button_set_image(GTK_BUTTON(widget), im);
#if GTK_CHECK_VERSION(3,6,0)
	if (strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
		gtk_button_set_always_show_image(GTK_BUTTON(widget), true);
#endif
#endif
}

void ControlMenuButton::__set_option(const string &op, const string &value) {
	if (op == "flat") {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_menu_button_set_has_frame(GTK_MENU_BUTTON(widget), false);
#else
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
#endif
	} else if (op == "menu") {
#if GTK_CHECK_VERSION(4,0,0)
		msg_error("MenuButton.menu gtk4...");
#else
		menu = CreateResourceMenu(value);
		if (menu) {
			menu->set_panel(panel);
			gtk_menu_button_set_popup(GTK_MENU_BUTTON(widget), menu->widget);
		}
#endif
	} else if (op == "menusource") {
#if GTK_CHECK_VERSION(4,0,0)
		msg_error("MenuButton.menusource gtk4...");
#else
		auto res = ParseResource(value);
		menu = _create_res_menu_("source", &res);
		if (menu) {
			menu->set_panel(panel);
			gtk_menu_button_set_popup(GTK_MENU_BUTTON(widget), menu->widget);
		}
#endif
	}
}

#endif

};

