/*
 * HuiControlCheckBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlCheckBox.h"

#ifdef HUI_API_GTK

namespace hui
{

void OnGtkCheckboxChange(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlCheckBox::ControlCheckBox(const string &title, const string &id) :
	Control(CONTROL_CHECKBOX, id)
{
	GetPartStrings(title);
	widget = gtk_check_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkCheckboxChange), this);
	set_options(OptionString);
}

string ControlCheckBox::get_string()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void ControlCheckBox::__set_string(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlCheckBox::__check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool ControlCheckBox::is_checked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

};

#endif
