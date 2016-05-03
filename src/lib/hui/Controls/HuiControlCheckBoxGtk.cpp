/*
 * HuiControlCheckBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlCheckBox.h"

#ifdef HUI_API_GTK

void OnGtkCheckboxChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->notify("hui:change");	}

HuiControlCheckBox::HuiControlCheckBox(const string &title, const string &id) :
	HuiControl(HUI_KIND_CHECKBOX, id)
{
	GetPartStrings(title);
	widget = gtk_check_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkCheckboxChange), this);
	setOptions(OptionString);
}

string HuiControlCheckBox::getString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void HuiControlCheckBox::__setString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlCheckBox::__check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

bool HuiControlCheckBox::isChecked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

#endif
