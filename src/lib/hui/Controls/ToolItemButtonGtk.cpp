/*
 * HuiToolItemButtonGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "ToolItemButton.h"

#ifdef HUI_API_GTK

namespace hui
{

void *get_gtk_image(const string &image, GtkIconSize size); // -> hui_menu_gtk.cpp

void on_gtk_tool_button_click(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:click");	}

ToolItemButton::ToolItemButton(const string &title, const string &image, const string &id) :
	Control(TOOL_ITEM_BUTTON, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_tool_button_new(im, sys_str(title)));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);
}

void ToolItemButton::__set_option(const string &op, const string &value) {
	if (op == "important")
		gtk_tool_item_set_is_important(GTK_TOOL_ITEM(widget), true);
}

}

#endif
