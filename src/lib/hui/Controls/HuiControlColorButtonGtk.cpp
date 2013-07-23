/*
 * HuiControlColorButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlColorButton.h"

#ifdef HUI_API_GTK

void OnGtkButtonPress(GtkWidget *widget, gpointer data);

HuiControlColorButton::HuiControlColorButton(const string &title, const string &id) :
	HuiControl(HuiKindColorButton, id)
{
	GetPartStrings(id, title);
	widget = gtk_color_button_new();
	if (OptionString.find("alpha") >= 0)
		gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(widget), true);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);
}

HuiControlColorButton::~HuiControlColorButton() {
	// TODO Auto-generated destructor stub
}

int col_f_to_i16(float f)
{	return (int)(f * 65535.0f);	}

float col_i16_to_f(int i)
{	return (float)i / 65535.0f;	}

void HuiControlColorButton::__SetColor(const color& c)
{
	GdkColor gcol;
	gcol.red = col_f_to_i16(c.r);
	gcol.green = col_f_to_i16(c.g);
	gcol.blue = col_f_to_i16(c.b);
	gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), &gcol);
	if (gtk_color_button_get_use_alpha(GTK_COLOR_BUTTON(widget)))
		gtk_color_button_set_alpha(GTK_COLOR_BUTTON(widget), col_f_to_i16(c.a));
}

color HuiControlColorButton::GetColor()
{
	color col;
	GdkColor gcol;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &gcol);
	col.r = col_i16_to_f(gcol.red);
	col.g = col_i16_to_f(gcol.green);
	col.b = col_i16_to_f(gcol.blue);
	if (gtk_color_button_get_use_alpha(GTK_COLOR_BUTTON(widget)))
		col.a = col_i16_to_f(gtk_color_button_get_alpha(GTK_COLOR_BUTTON(widget)));
	return col;
}

#endif
