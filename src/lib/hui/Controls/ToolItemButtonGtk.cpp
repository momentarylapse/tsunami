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

void *get_gtk_image(const string &image, bool large);

void OnGtkToolbarItemPress(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:click");	}

ToolItemButton::ToolItemButton(const string &title, const string &image, const string &id) :
	Control(TOOL_ITEM_BUTTON, id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, true);
	gtk_widget_show(im);
	widget = GTK_WIDGET(gtk_tool_button_new(im, sys_str(title)));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(widget), true);
	if ((image != "hui:redo") and (image != "hui:open") and (image != "hui:paste") and (image != "hui:media-stop"))
		gtk_tool_item_set_is_important(GTK_TOOL_ITEM(widget), true);
	//gtk_widget_set_tooltip_text(widget, sys_str(get_lang(id, title)));
	gtk_widget_show(widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(&OnGtkToolbarItemPress), this);

	/*gtk_toolbar_insert(GTK_TOOLBAR(cur_toolbar->widget),it,-1);
	AddToolbarItem(cur_toolbar,id,HuiToolButton,NULL);
	cur_toolbar->item.back().widget = it;*/
}

}

#endif
