/*
 * HuiToolItemMenuButtonGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "../Menu.h"
#include "ToolItemMenuButton.h"

#ifdef HUI_API_GTK

namespace hui
{

void *get_gtk_image(const string &image, bool large);

void OnGtkToolbarItemPress(GtkWidget *widget, gpointer data);

ToolItemMenuButton::ToolItemMenuButton(const string &title, Menu *menu, const string &image, const string &id) :
	Control(TOOL_ITEM_MENUBUTTON, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, true);
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_menu_tool_button_new(im, sys_str(title)));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(widget), menu->widget);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkToolbarItemPress), this);
}

}

#endif

