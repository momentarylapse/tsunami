/*
 * HuiControlSlider.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlSlider.h"

#ifdef HUI_API_GTK

void OnGtkSliderChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->notify("hui:change");	}

HuiControlSlider::HuiControlSlider(const string &title, const string &id, bool _vertical) :
	HuiControl(HUI_KIND_SLIDER, id)
{
	vertical = _vertical;
	GetPartStrings(id, title);
	if (OptionString.find("vertical") >= 0)
		vertical = true;
	if (vertical){
#if GTK_MAJOR_VERSION >= 3
		widget = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 1.0, 0.0001);
#else
		widget = gtk_vscale_new_with_range(0.0, 1.0, 0.0001);
#endif
		gtk_range_set_inverted(GTK_RANGE(widget), true);
	}else{
#if GTK_MAJOR_VERSION >= 3
		widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.0001);
#else
		widget = gtk_vscale_new_with_range(0.0, 1.0, 0.0001);
#endif
	}
	gtk_scale_set_draw_value(GTK_SCALE(widget), false);
	g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(&OnGtkSliderChange), this);
	setOptions(OptionString);
}

float HuiControlSlider::getFloat()
{
	return (float)gtk_range_get_value(GTK_RANGE(widget));
}

void HuiControlSlider::__setFloat(float f)
{
	gtk_range_set_value(GTK_RANGE(widget), f);
}

void HuiControlSlider::__addString(const string &s)
{
	Array<string> p = s.explode("\\");//HuiComboBoxSeparator);
	if (p.num != 2)
		return;
	gtk_scale_add_mark(GTK_SCALE(widget), p[0]._float(), vertical ? GTK_POS_LEFT : GTK_POS_TOP, ("<small>" + p[1] + "</small>").c_str());
}

void HuiControlSlider::__setOption(const string &op, const string &value)
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
				gtk_range_set_increments(GTK_RANGE(widget), step, step * 10);
			}
		}
		gtk_range_set_range(GTK_RANGE(widget), vmin, vmax);
	}else if (op == "origin")
		gtk_scale_set_has_origin(GTK_SCALE(widget), value._bool());
	else if (op == "show-value")
		gtk_scale_set_draw_value(GTK_SCALE(widget), value._bool());
	else if (op == "mark")
		__addString(value);
	else if (op == "clear-marks")
		gtk_scale_clear_marks(GTK_SCALE(widget));
}

#endif
