/*
 * HuiToolItemButton.cpp
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#include "HuiToolItemButton.h"

void *get_gtk_image(const string &image, bool large);

void OnGtkToolbarItemPress(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:click");	}

HuiToolItemButton::HuiToolItemButton(const string &title, const string &image, const string &id) :
	HuiControl(HuiKindToolButton, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, true);
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_tool_button_new(im, sys_str(get_lang(id, title))));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	if ((image != "hui:redo") && (image != "hui:open") && (image != "hui:paste") && (image != "hui:media-stop"))
		gtk_tool_item_set_is_important(GTK_TOOL_ITEM(widget), true);
	gtk_widget_show(widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkToolbarItemPress), this);

	/*gtk_toolbar_insert(GTK_TOOLBAR(cur_toolbar->widget),it,-1);
	AddToolbarItem(cur_toolbar,id,HuiToolButton,NULL);
	cur_toolbar->item.back().widget = it;*/
}

HuiToolItemButton::~HuiToolItemButton()
{
}

