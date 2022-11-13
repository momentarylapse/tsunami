/*
 * ControlColorButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlColorButton.h"
#include "../Event.h"
#include "../../image/color.h"

#ifdef HUI_API_GTK

namespace hui
{
	// perform gamma corrections?
	// (gtk uses sRGB internally)
	bool color_button_linear = false;


void OnGtkColorButtonChange(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);	}

ControlColorButton::ControlColorButton(const string &title, const string &id) :
	Control(CONTROL_COLORBUTTON, id)
{
	auto parts = split_title(title);
	widget = gtk_color_button_new();
	take_gtk_ownership();
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
	g_signal_connect(G_OBJECT(widget), "color-set", G_CALLBACK(&OnGtkColorButtonChange), this);

	_last_set = Black;
}

color color_gtk_to_user(const color &c) {
	if (color_button_linear)
		return c.srgb_to_lin();
	return c;
}

color color_user_to_gtk(const color &c) {
	if (color_button_linear)
		return c.lin_to_srgb();
	return c;
}

GdkRGBA color_to_gdk(const color &c) {
	GdkRGBA gcol;
	gcol.red = c.r;
	gcol.green = c.g;
	gcol.blue = c.b;
	gcol.alpha = c.a;
	return gcol;
}

color color_from_gdk(const GdkRGBA &gcol) {
	color col;
	col.r = (float)gcol.red;
	col.g = (float)gcol.green;
	col.b = (float)gcol.blue;
	col.a = (float)gcol.alpha;
	return col;
}

void ControlColorButton::__set_color(const color& c) {
	_last_set = c;
	GdkRGBA gcol = color_to_gdk(color_user_to_gtk(c));
	_last_set_gdk = gcol;
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widget), &gcol);
}

color ControlColorButton::get_color() {
	GdkRGBA gcol;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &gcol);

	// make sure, we get EXACTLY the same value, when nothing changed!
	if (memcmp(&gcol, &_last_set_gdk, sizeof(GdkRGBA)) == 0)
		return _last_set;
	return color_gtk_to_user(color_from_gdk(gcol));
}

void ControlColorButton::__set_option(const string &op, const string &value) {
	if (op == "alpha")
		gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), true);
}

};

#endif
