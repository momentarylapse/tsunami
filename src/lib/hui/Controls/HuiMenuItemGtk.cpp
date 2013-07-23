/*
 * HuiMenuItemGtk.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiMenuItem.h"
#include "../hui.h"
#include "../hui_internal.h"

#ifdef HUI_API_GTK

void *get_gtk_image(const string &image, bool large);

void try_add_accel(GtkWidget *item, const string &id);

gboolean OnGtkMenuClick(GtkWidget *widget, gpointer data)
{
	((HuiControl*)data)->Notify("hui:click");
	return FALSE;
}

HuiMenuItem::HuiMenuItem(const string &title, const string &id) :
	HuiControl(HuiKindMenuItem, id)
{
	widget = gtk_image_menu_item_new_with_label(get_lang_sys(id, get_lang_sys(id, title), false));
	/*GtkWidget *im = (GtkWidget*)get_gtk_image(image, false);
	if (im)
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(i->widget), im);*/
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(OnGtkMenuClick), this);

	try_add_accel(widget, id);
}

HuiMenuItem::~HuiMenuItem()
{
}

void HuiMenuItem::SetImage(const string &image)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, false);
	if (im)
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(widget), im);
}

#endif
