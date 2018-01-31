/*
 * ControlColorButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlColorButton.h"

#ifdef HUI_API_GTK

namespace hui
{

void OnGtkButtonPress(GtkWidget *widget, gpointer data);


void OnGtkColorButtonChange(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlColorButton::ControlColorButton(const string &title, const string &id) :
	Control(CONTROL_COLORBUTTON, id)
{
	GetPartStrings(title);
	widget = gtk_color_button_new();
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
	g_signal_connect(G_OBJECT(widget), "color-set", G_CALLBACK(&OnGtkColorButtonChange), this);
	setOptions(OptionString);
}

int col_f_to_i16(float f)
{	return (int)(f * 65535.0f);	}

float col_i16_to_f(int i)
{	return (float)i / 65535.0f;	}

void ControlColorButton::__setColor(const color& c)
{
	GdkRGBA gcol;
	gcol.red = c.r;
	gcol.green = c.g;
	gcol.blue = c.b;
	gcol.alpha = c.a;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widget), &gcol);
}

color ControlColorButton::getColor()
{
	color col;
	GdkRGBA gcol;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &gcol);
	col.r = gcol.red;
	col.g = gcol.green;
	col.b = gcol.blue;
	col.a = gcol.alpha;
	return col;
}

void ControlColorButton::__setOption(const string &op, const string &value)
{
	if (op == "alpha")
		gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), true);
}

};

#endif
