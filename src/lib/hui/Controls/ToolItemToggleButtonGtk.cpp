/*
 * HuiToolItemToggleButtonGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "ToolItemToggleButton.h"
#include "../language.h"

#include <gtk/gtk.h>

namespace hui
{

void *get_gtk_image(const string &image, IconSize size); // -> hui_menu_gtk.cpp

void on_gtk_tool_button_click(GtkWidget *widget, gpointer data);
void on_gtk_toggle_button_toggle(GtkWidget *widget, gpointer data);

ToolItemToggleButton::ToolItemToggleButton(const string &title, const string &image, const string &id) :
	Control(TOOL_ITEM_TOGGLEBUTTON, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, IconSize::TOOLBAR_LARGE);
#if GTK_CHECK_VERSION(4,0,0)
	widget = gtk_toggle_button_new();
	if (im)
		gtk_button_set_child(GTK_BUTTON(widget), im);
	else
		gtk_button_set_label(GTK_BUTTON(widget), sys_str(title));
	//gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	gtk_widget_set_can_focus(widget, false);
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&on_gtk_toggle_button_toggle), this);
	//g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);
#else
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_toggle_tool_button_new());
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(widget), sys_str(title));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(widget), im);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);
#endif
	take_gtk_ownership();
}

void ToolItemToggleButton::__check(bool checked) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
#else
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(widget), checked);
#endif
}

bool ToolItemToggleButton::is_checked() {
#if GTK_CHECK_VERSION(4,0,0)
	return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
#else
	return gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget));
#endif
}

void ToolItemToggleButton::__set_option(const string &op, const string &value) {
#if !GTK_CHECK_VERSION(4,0,0)
	if (op == "important")
		gtk_tool_item_set_is_important(GTK_TOOL_ITEM(widget), true);
#endif
}

}
