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

void *get_gtk_image(const string &image, bool large); // -> hui_menu_gtk.cpp

void OnGtkToggleButtonToggle(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlToggleButton::ControlToggleButton(const string &title, const string &id) :
	Control(CONTROL_TOGGLEBUTTON, id)
{
	GetPartStrings(title);
	widget = gtk_toggle_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&OnGtkToggleButtonToggle), this);
	setOptions(OptionString);
}

string ControlToggleButton::getString()
{
	return "";
}

void ControlToggleButton::__setString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlToggleButton::setImage(const string& str)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, false);
	gtk_button_set_image(GTK_BUTTON(widget), im);
}

void ControlToggleButton::__check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool ControlToggleButton::isChecked()
{
	return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void ControlToggleButton::__setOption(const string &op, const string &value)
{
	if (op == "flat")
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
}

};

#endif
