/*
 * ControlSpinButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlSpinButton.h"
#include "../Event.h"
#include "../language.h"
#include <math.h>

#include <gtk/gtk.h>

namespace hui
{

void parse_range(const string &str, float &vmin, float &vmax, float &step); // -> slider

void on_gtk_spin_button_change(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);	}

ControlSpinButton::ControlSpinButton(const string &title, const string &id) :
	Control(CONTROL_SPINBUTTON, id)
{
	auto parts = split_title(title);
	float vmin = -1000000.0f;
	float vmax = 1000000.0f;
	float step = 1;
	widget = gtk_spin_button_new_with_range(vmin, vmax, step);
#if GTK_CHECK_VERSION(4,0,0)
#else
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
#endif
	take_gtk_ownership();
	set_options(get_option_from_title(title));

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), s2f(parts[0]));
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(&on_gtk_spin_button_change), this);
}

string ControlSpinButton::get_string() {
	return f2s((float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget)), gtk_spin_button_get_digits(GTK_SPIN_BUTTON(widget)));
	//return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void ControlSpinButton::__set_string(const string &str) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_editable_set_text(GTK_EDITABLE(widget), sys_str(str));
#else
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
#endif
}

void ControlSpinButton::__set_int(int i) {
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), i);
}

int ControlSpinButton::get_int() {
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

float ControlSpinButton::get_float() {
	return (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void ControlSpinButton::__set_float(float f) {
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), f);
}

int count_digits_left(float f) {
	if (f == 0)
		return 1;
	if (f < 0)
		f = abs(f);
	return max(1, (int)(log10(f) + 1.0));
}

int count_digits_right(float f) {
	if (f == 0)
		return 0;
	f = abs(f);
	return max(0, (int)(0.5-log10(f)));
}

void count_digits(float f, int &left, int &right) {
	left = count_digits_left(f);
	right = count_digits_right(f);
}

int count_digits_range(float vmin, float vmax, float step) {
	int l, r;
	count_digits(max(abs(vmin), abs(vmax)), l, r);
	[[maybe_unused]] int a = max(count_digits_left(vmax), count_digits_left(vmin));
	if (step > 0 and step < 1) {
		int l2, r2;
		count_digits(step, l2, r2);
		return max(l, l2) + max(r, r2);
	}
	return l + r;
}

void ControlSpinButton::__set_option(const string &op, const string &value) {
	if (op == "range") {
		float vmin, vmax, step;
		parse_range(value, vmin, vmax, step);
		if (step != 1.0f) {
			gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget), step, step * 10);
			gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), count_digits_right(step));
		}
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(widget), vmin, vmax);
#if GTK_CHECK_VERSION(4,0,0)
		gtk_editable_set_width_chars(GTK_EDITABLE(widget), count_digits_range(vmin, vmax, step));
#else
		gtk_entry_set_max_width_chars(GTK_ENTRY(widget), count_digits_range(vmin, vmax, step));
#endif
	} else if (op == "wrap") {
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), val_is_positive(value, true));
	} else if (op == "nowrap") {
		gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), false);
	}
}

};
