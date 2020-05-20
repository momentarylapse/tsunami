/*
 * ControlSlider.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlSlider.h"

#ifdef HUI_API_GTK

namespace hui
{

void on_gtk_slider_change(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlSlider::ControlSlider(const string &title, const string &id) :
	Control(CONTROL_SLIDER, id)
{
	vertical = false;
	if (option_has(get_option_from_title(title), "vertical"))
		vertical = true;
	if (vertical) {
#if GTK_CHECK_VERSION(3,0,0)
		widget = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 1.0, 0.0001);
#else
		widget = gtk_vscale_new_with_range(0.0, 1.0, 0.0001);
#endif
		gtk_range_set_inverted(GTK_RANGE(widget), true);
	} else {
#if GTK_CHECK_VERSION(3,0,0)
		widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.0001);
#else
		widget = gtk_vscale_new_with_range(0.0, 1.0, 0.0001);
#endif
	}
	gtk_scale_set_draw_value(GTK_SCALE(widget), false);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(&on_gtk_slider_change), this);
	set_options(get_option_from_title(title));
}

float ControlSlider::get_float() {
	return (float)gtk_range_get_value(GTK_RANGE(widget));
}

void ControlSlider::__set_float(float f) {
	gtk_range_set_value(GTK_RANGE(widget), f);
}

void ControlSlider::__add_string(const string &s) {
	auto p = s.explode("\\");//HuiComboBoxSeparator);
	if (p.num != 2)
		return;
	gtk_scale_add_mark(GTK_SCALE(widget), p[0]._float(), vertical ? GTK_POS_LEFT : GTK_POS_TOP, ("<small>" + p[1] + "</small>").c_str());
}

void parse_range(const string &str, float &vmin, float &vmax, float &step) {
	vmin = -100000000000.0f;
	vmax = 100000000000.0f;
	step = 1;
	auto v = str.replace("\\", ":").explode(":");
	if (v.num >= 1) {
		if (v[0].num > 0)
			vmin = v[0]._float();
	}
	if (v.num >= 2) {
		if (v[1].num > 0)
			vmax = v[1]._float();
	}
	if (v.num >= 3) {
		if (v[2].num > 0) {
			step = v[2]._float();
		}
	}
}

void ControlSlider::__set_option(const string &op, const string &value) {
	if (op == "range") {
		float vmin, vmax, step;
		parse_range(value, vmin, vmax, step);
		gtk_range_set_range(GTK_RANGE(widget), vmin, vmax);
		gtk_range_set_increments(GTK_RANGE(widget), step, step * 10);
	} else if (op == "origin") {
		gtk_scale_set_has_origin(GTK_SCALE(widget), val_is_positive(value, true));
	} else if (op == "showvalue") {
		gtk_scale_set_draw_value(GTK_SCALE(widget), val_is_positive(value, true));
	} else if (op == "mark") {
		__add_string(value);
	} else if (op == "clearmarks") {
		gtk_scale_clear_marks(GTK_SCALE(widget));
	}
}

};

#endif
