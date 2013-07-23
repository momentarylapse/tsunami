/*
 * HuiToolItemMenuButtonGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiToolItemMenuButton.h"
#include "../HuiMenu.h"

#ifdef HUI_API_GTK

void *get_gtk_image(const string &image, bool large);

void OnGtkToolbarItemPress(GtkWidget *widget, gpointer data);

HuiToolItemMenuButton::HuiToolItemMenuButton(const string &title, HuiMenu *menu, const string &image, const string &id) :
	HuiControl(HuiKindToolMenuButton, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, true);
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_menu_tool_button_new(im, sys_str(get_lang(id, title))));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(widget), menu->widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkToolbarItemPress), this);
}

HuiToolItemMenuButton::~HuiToolItemMenuButton()
{
}

#endif

