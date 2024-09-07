/*
 * HuiToolItemButtonGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "ToolItemButton.h"
#include "../Event.h"
#include "../language.h"

#include <gtk/gtk.h>

namespace hui
{

void *get_gtk_image(const string &image, IconSize size); // -> hui_menu_gtk.cpp

void on_gtk_tool_button_click(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify(EventID::CLICK);	}

ToolItemButton::ToolItemButton(const string &title, const string &image, const string &id) :
	Control(TOOL_ITEM_BUTTON, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, IconSize::TOOLBAR_LARGE);
#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_button_new();
	if (im)
		gtk_button_set_child(GTK_BUTTON(widget), im);
	else
		gtk_button_set_label(GTK_BUTTON(widget), sys_str(title));
	gtk_button_set_has_frame(GTK_BUTTON(widget), false);
	//gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	gtk_widget_set_can_focus(widget, false);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);
#else
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_tool_button_new(im, sys_str(title)));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);
#endif
	take_gtk_ownership();
}

void ToolItemButton::__set_option(const string &op, const string &value) {
#if !GTK_CHECK_VERSION(4,0,0)
	if (op == "important")
		gtk_tool_item_set_is_important(GTK_TOOL_ITEM(widget), true);
#endif
}

}

