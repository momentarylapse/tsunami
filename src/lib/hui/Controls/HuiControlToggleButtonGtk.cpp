/*
 * HuiControlToggleButton.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControlToggleButton.h"

#ifdef HUI_API_GTK

void *get_gtk_image(const string &image, bool large); // -> hui_menu_gtk.cpp

void OnGtkToggleButtonToggle(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->notify("hui:change");	}

HuiControlToggleButton::HuiControlToggleButton(const string &title, const string &id) :
	HuiControl(HUI_KIND_TOGGLEBUTTON, id)
{
	GetPartStrings(id, title);
	widget = gtk_toggle_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&OnGtkToggleButtonToggle), this);
	//SetInt(id, 0);
	setOptions(OptionString);
}

string HuiControlToggleButton::getString()
{
	return "";
}

void HuiControlToggleButton::__setString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlToggleButton::setImage(const string& str)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, false);
	gtk_button_set_image(GTK_BUTTON(widget), im);
}

void HuiControlToggleButton::__check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool HuiControlToggleButton::isChecked()
{
	return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

#endif
