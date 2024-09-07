/*
 * ControlMenuButton.cpp
 *
 *  Created on: 07.08.2018
 *      Author: michi
 */

#include "../hui.h"
#include "ControlMenuButton.h"
#include "../Menu.h"
#include "../Resource.h"
#include "../../os/msg.h"

#include <gtk/gtk.h>

namespace hui
{

void *get_gtk_image(const string &image, IconSize size); // -> hui_menu_gtk.cpp

xfer<Menu> _create_res_menu_(const string &ns, Resource *res, Panel *p); // -> Resource.cpp

//void OnGtkMenuButtonPress(GtkWidget *widget, gpointer data)
//{	reinterpret_cast<Control*>(data)->notify("hui:click");	}


ControlMenuButton::ControlMenuButton(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_MENU_BUTTON, id)
{
	this->panel = panel; // for the menu...
	auto parts = split_title(title);
	widget = gtk_menu_button_new();
#if GTK_CHECK_VERSION(4,0,0)
	if (parts[0].num > 0)
		gtk_menu_button_set_label(GTK_MENU_BUTTON(widget), sys_str(parts[0]));
#else
	if (parts[0].num > 0)
		gtk_button_set_label(GTK_BUTTON(widget), sys_str(parts[0]));
#endif
	take_gtk_ownership();
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkMenuButtonPress), this);

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
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, IconSize::REGULAR);
#if GTK_CHECK_VERSION(4,0,0)
	gtk_menu_button_set_child(GTK_MENU_BUTTON(widget), im);
#else
	gtk_button_set_image(GTK_BUTTON(widget), im);
#if GTK_CHECK_VERSION(3,6,0)
	if (!gtk_button_get_label(GTK_BUTTON(widget)) || strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
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
		set_menu(create_resource_menu(value, panel));
	} else if (op == "menusource") {
		auto res = parse_resource(value, panel);
		set_menu(_create_res_menu_("source", &res, panel));
	} else if (op == "arrow") {
#if GTK_CHECK_VERSION(4,0,0)
		gtk_menu_button_set_always_show_arrow(GTK_MENU_BUTTON(widget), value._bool());
#endif
	}
}

void ControlMenuButton::set_menu(Menu *m) {
	menu = m;
	if (menu) {
		menu->set_panel(panel);
#if GTK_CHECK_VERSION(4,0,0)
		//msg_error("MenuButton.menu gtk4...");
		panel->_connect_menu_to_panel(menu);
		auto w = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu->gmenu));
		gtk_menu_button_set_popover(GTK_MENU_BUTTON(widget), w);
#else
		gtk_menu_button_set_popup(GTK_MENU_BUTTON(widget), menu->widget);
#endif
	}
}

};

