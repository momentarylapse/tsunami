/*
 * HuiControlSpinButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlSpinButton.h"

#ifdef HUI_API_GTK

void OnGtkEditChange(GtkWidget *widget, gpointer data);

HuiControlSpinButton::HuiControlSpinButton(const string &title, const string &id) :
	HuiControl(HuiKindSpinButton, id)
{
	GetPartStrings(id, title);
	float vmin = -100000000000.0f;
	float vmax = 100000000000.0f;
	float step = 1;
	if (PartString.num >= 2){
		if (PartString[1].num > 0)
			vmin = s2f(PartString[1]);
	}if (PartString.num >= 3){
		if (PartString[2].num > 0)
			vmax = s2f(PartString[2]);
	}if (PartString.num >= 4){
		if (PartString[3].num > 0)
			step = s2f(PartString[3]);
	}
	widget = gtk_spin_button_new_with_range(vmin, vmax, step);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), s2f(PartString[0]));
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkEditChange), this);
	SetOptions(OptionString);
}

HuiControlSpinButton::~HuiControlSpinButton() {
	// TODO Auto-generated destructor stub
}

string HuiControlSpinButton::GetString()
{
	return f2s(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)), gtk_spin_button_get_digits(GTK_SPIN_BUTTON(widget)));
	//return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void HuiControlSpinButton::__SetString(const string &str)
{
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
}

void HuiControlSpinButton::__SetInt(int i)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), i);
}

int HuiControlSpinButton::GetInt()
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

float HuiControlSpinButton::GetFloat()
{
	return gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void HuiControlSpinButton::__SetFloat(float f)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), f);
}

#endif
