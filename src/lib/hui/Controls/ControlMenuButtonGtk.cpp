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

void *get_gtk_image(const string &image, GtkIconSize size); // -> hui_menu_gtk.cpp

Menu *_create_res_menu_(const string &ns, Resource *res); // -> Resource.cpp

//void OnGtkMenuButtonPress(GtkWidget *widget, gpointer data)
//{	reinterpret_cast<Control*>(data)->notify("hui:click");	}


ControlMenuButton::ControlMenuButton(const string &title, const string &id) :
	Control(CONTROL_MENU_BUTTON, id)
{
	auto parts = split_title(title);
	widget = gtk_menu_button_new();
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(parts[0]));
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkMenuButtonPress), this);

	menu = NULL;

//	SetImageById(this, id);
	set_options(get_option_from_title(title));
}

string ControlMenuButton::get_string() {
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void ControlMenuButton::__set_string(const string &str) {
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlMenuButton::set_image(const string& str) {
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), im);
#if GTK_CHECK_VERSION(3,6,0)
	if (strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
		gtk_button_set_always_show_image(GTK_BUTTON(widget), true);
#endif
}

void ControlMenuButton::__set_option(const string &op, const string &value) {
	if (op == "flat") {
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
	} else if (op == "menu") {
		menu = CreateResourceMenu(value);
		if (menu) {
			menu->set_panel(panel);
			gtk_menu_button_set_popup(GTK_MENU_BUTTON(widget), menu->widget);
		}
	} else if (op == "menusource") {
		auto res = ParseResource(value);
		menu = _create_res_menu_("source", &res);
		if (menu) {
			menu->set_panel(panel);
			gtk_menu_button_set_popup(GTK_MENU_BUTTON(widget), menu->widget);
		}
	}
}

#endif

};

