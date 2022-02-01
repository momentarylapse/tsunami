/*
 * HuiMenuItemGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "../hui.h"
#include "../internal.h"
#include "MenuItem.h"

#ifdef HUI_API_GTK

namespace hui
{

// gtk3
gboolean on_get_menu_click(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CLICK);
	return FALSE;
}

string get_gtk_action_name(const string &id, Panel *scope);

MenuItem::MenuItem(const string &title, const string &id, Panel *_panel) :
	Control(MENU_ITEM_BUTTON, id)
{
	panel = _panel;
#if GTK_CHECK_VERSION(4,0,0)
	item = g_menu_item_new(get_lang_sys(id, get_lang_sys(id, title), false), get_gtk_action_name(id, panel).c_str());
#else
	widget = gtk_menu_item_new_with_label(get_lang_sys(id, get_lang_sys(id, title), false));
	/*GtkWidget *im = (GtkWidget*)get_gtk_image(image, GTK_ICON_SIZE_MENU);
	if (im)
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(i->widget), im);*/
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(on_get_menu_click), this);
#endif
}

void MenuItem::set_image(const string &image) {
	/*GtkWidget *im = (GtkWidget*)get_gtk_image(image, GTK_ICON_SIZE_MENU);
	if (im)
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(widget), im);*/
}

};

#endif
