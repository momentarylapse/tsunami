/*
 * HuiControlButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlButton.h"

#ifdef HUI_API_GTK

void *get_gtk_image(const string &image, bool large); // -> hui_menu_gtk.cpp

void OnGtkButtonPress(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:click");	}

HuiControlButton::HuiControlButton(const string &title, const string &id) :
	HuiControl(HuiKindButton, id)
{
	GetPartStrings(id, title);
	widget = gtk_button_new_with_label(sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkButtonPress), this);

//	SetImageById(this, id);
	SetOptions(OptionString);
}

HuiControlButton::~HuiControlButton()
{
}

string HuiControlButton::GetString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

void HuiControlButton::__SetString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlButton::SetImage(const string& str)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(str, false);
	gtk_button_set_image(GTK_BUTTON(widget), im);
#if GTK_MAJOR_VERSION >= 3
	if (strlen(gtk_button_get_label(GTK_BUTTON(widget))) == 0)
		gtk_button_set_always_show_image(GTK_BUTTON(widget), true);
#endif
}

void HuiControlButton::__SetOption(const string &op, const string &value)
{
	if (op == "flat")
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
}

#endif
