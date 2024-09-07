/*
 * HuiToolItemMenuButtonGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "../Menu.h"
#include "ToolItemMenuButton.h"
#include "../hui.h"

#include <gtk/gtk.h>

namespace hui
{

void *get_gtk_image(const string &image, IconSize size); // -> hui_menu_gtk.cpp

void on_gtk_tool_button_click(GtkWidget *widget, gpointer data);

ToolItemMenuButton::ToolItemMenuButton(const string &title, Menu *menu, const string &image, const string &id, Panel *_panel) :
	Control(TOOL_ITEM_MENUBUTTON, id)
{
	panel = _panel;
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, IconSize::TOOLBAR_LARGE);
#if GTK_CHECK_VERSION(4,0,0)
	frame = gtk_grid_new();


	widget = gtk_button_new();
	if (im)
		gtk_button_set_child(GTK_BUTTON(widget), im);
	else
		gtk_button_set_label(GTK_BUTTON(widget), sys_str(title));
	//gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	gtk_widget_set_can_focus(widget, false);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);


	gtk_grid_attach(GTK_GRID(frame), widget, 0, 0, 1, 1);

	auto mb = gtk_menu_button_new();
	gtk_grid_attach(GTK_GRID(frame), mb, 1, 0, 1, 1);
	gtk_widget_set_focusable(mb, false);

	if (menu) {
		menu->set_panel(panel);
		//msg_error("MenuButton.menu gtk4...");
		panel->_connect_menu_to_panel(menu);
		auto w = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu->gmenu));
		gtk_menu_button_set_popover(GTK_MENU_BUTTON(mb), w);
	}

#else
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_menu_tool_button_new(im, sys_str(title)));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(widget), menu->widget);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&on_gtk_tool_button_click), this);
#endif
	take_gtk_ownership();
}

void ToolItemMenuButton::__set_option(const string &op, const string &value) {
#if !GTK_CHECK_VERSION(4,0,0)
	if (op == "important")
		gtk_tool_item_set_is_important(GTK_TOOL_ITEM(widget), true);
#endif
}

}

