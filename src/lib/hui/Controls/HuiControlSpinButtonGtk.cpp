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
	HuiControl(HUI_KIND_SPINBUTTON, id)
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
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(&OnGtkEditChange), this);
	setOptions(OptionString);
}

string HuiControlSpinButton::getString()
{
	return f2s((float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)), gtk_spin_button_get_digits(GTK_SPIN_BUTTON(widget)));
	//return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void HuiControlSpinButton::__setString(const string &str)
{
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
}

void HuiControlSpinButton::__setInt(int i)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), i);
}

int HuiControlSpinButton::getInt()
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

float HuiControlSpinButton::getFloat()
{
	return (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void HuiControlSpinButton::__setFloat(float f)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), f);
}

void HuiControlSpinButton::__setOption(const string &op, const string &value)
{
	if (op == "range"){
		float vmin = -100000000000.0f;
		float vmax = 100000000000.0f;
		float step = 1;
		Array<string> v = value.explode("\\");
		if (v.num >= 1){
			if (v[0].num > 0)
				vmin = v[0]._float();
		}
		if (v.num >= 2){
			if (v[1].num > 0)
				vmax = v[1]._float();
		}
		if (v.num >= 3){
			if (v[2].num > 0){
				step = v[2]._float();
				gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget), step, step * 10);
			}
		}
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), vmin, vmax);
	}else if (op == "wrap"){
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), true);
	}else if (op == "nowrap"){
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), false);
	}
}

#endif
