/*
 * ControlSpinButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlSpinButton.h"
#include <math.h>

#ifdef HUI_API_GTK

namespace hui
{

void OnGtkSpinButtonChange(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlSpinButton::ControlSpinButton(const string &title, const string &id) :
	Control(CONTROL_SPINBUTTON, id)
{
	GetPartStrings(title);
	float vmin = -100000000000.0f;
	float vmax = 100000000000.0f;
	float step = 1;
	widget = gtk_spin_button_new_with_range(vmin, vmax, step);
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
	set_options(OptionString);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), s2f(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(&OnGtkSpinButtonChange), this);
}

string ControlSpinButton::get_string()
{
	return f2s((float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)), gtk_spin_button_get_digits(GTK_SPIN_BUTTON(widget)));
	//return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void ControlSpinButton::__set_string(const string &str)
{
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
}

void ControlSpinButton::__set_int(int i)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), i);
}

int ControlSpinButton::get_int()
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

float ControlSpinButton::get_float()
{
	return (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void ControlSpinButton::__set_float(float f)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), f);
}

int count_digits(float f)
{
	if (f <= 0)
		return 0;
	return max(0, (int)(0.5-log10(f)));
}

void ControlSpinButton::__set_option(const string &op, const string &value)
{
	if (op == "range"){
		float vmin = -100000000000.0f;
		float vmax = 100000000000.0f;
		float step = 1;
		Array<string> v = value.replace("\\", ":").explode(":");
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
				gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), count_digits(step));
			}
		}
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), vmin, vmax);
	}else if (op == "wrap"){
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), true);
	}else if (op == "nowrap"){
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), false);
	}
}

};

#endif
