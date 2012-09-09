#include "hui.h"
#include "hui_internal.h"
#ifdef HUI_API_GTK


void AddToolbarItem(HuiToolbar *tb, const string &id, int type, CHuiMenu *menu);


void CHuiWindow::EnableToolbar(bool enabled)
{
	if (enabled)
		gtk_widget_show(cur_toolbar->widget);
	else
		gtk_widget_hide(cur_toolbar->widget);
	cur_toolbar->enabled = enabled;
}

void CHuiWindow::ToolbarConfigure(bool text_enabled, bool large_icons)
{
	gtk_toolbar_set_style(GTK_TOOLBAR(cur_toolbar->widget), text_enabled ? GTK_TOOLBAR_BOTH : GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(cur_toolbar->widget), large_icons ? GTK_ICON_SIZE_LARGE_TOOLBAR : GTK_ICON_SIZE_SMALL_TOOLBAR);
	cur_toolbar->text_enabled = text_enabled;
	cur_toolbar->large_icons = large_icons;
}

// -> hui_window_control_gtk.cpp
void NotifyWindowByWidget(CHuiWindow *win, GtkWidget *widget, const string &message = "", bool is_default = true);

void OnGtkToolbarItemPress(GtkWidget *widget, gpointer data)
{	NotifyWindowByWidget((CHuiWindow*)data, widget);	}

// add a default button
void CHuiWindow::ToolbarAddItem(const string &title, const string &tool_tip, const string &image, const string &id)
{
	GtkWidget *im = (GtkWidget*)get_gtk_image(image, true);
	gtk_widget_show(im);
	GtkToolItem *it=gtk_tool_button_new(im,sys_str(title));
	gtk_tool_item_set_tooltip_text(it, sys_str(tool_tip));
	gtk_tool_item_set_homogeneous(it,true);
	if ((image != "hui:redo") && (image != "hui:open") && (image != "hui:paste") && (image != "hui:media-stop"))
		gtk_tool_item_set_is_important(it, true);
	gtk_widget_show(GTK_WIDGET(it));
	gtk_toolbar_insert(GTK_TOOLBAR(cur_toolbar->widget),it,-1);
	g_signal_connect(G_OBJECT(it),"clicked",G_CALLBACK(&OnGtkToolbarItemPress),this);
	AddToolbarItem(cur_toolbar,id,HuiToolButton,NULL);
	cur_toolbar->item.back().widget = it;
}

// add a checkable button
void CHuiWindow::ToolbarAddItemCheckable(const string &title, const string &tool_tip, const string &image, const string &id)
{
	GtkWidget *im=(GtkWidget*)get_gtk_image(image, true);
	gtk_widget_show(im);
	GtkToolItem *it=gtk_toggle_tool_button_new();
	gtk_tool_item_set_tooltip_text(it, sys_str(tool_tip));
	gtk_tool_item_set_homogeneous(it,true);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(it),sys_str(title));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it),im);
	gtk_widget_show(GTK_WIDGET(it));
	gtk_toolbar_insert(GTK_TOOLBAR(cur_toolbar->widget),it,-1);
	g_signal_connect(G_OBJECT(it),"clicked",G_CALLBACK(&OnGtkToolbarItemPress),this);
	AddToolbarItem(cur_toolbar,id,HuiToolCheckable,NULL);
	cur_toolbar->item.back().widget = it;
}

void CHuiWindow::ToolbarAddItemMenu(const string &title, const string &tool_tip, const string &image, CHuiMenu *menu, const string &id)
{
	if (!menu)
		return;
	GtkWidget *im=(GtkWidget*)get_gtk_image(image,true);
	gtk_widget_show(im);
	GtkToolItem *it=gtk_menu_tool_button_new(im,sys_str(title));
	gtk_tool_item_set_tooltip_text(it, sys_str(tool_tip));
	gtk_tool_item_set_homogeneous(it,true);
	gtk_widget_show(GTK_WIDGET(it));
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it),menu->g_menu);
	gtk_toolbar_insert(GTK_TOOLBAR(cur_toolbar->widget),it,-1);
	g_signal_connect(G_OBJECT(it),"clicked",G_CALLBACK(&OnGtkToolbarItemPress),this);
	AddToolbarItem(cur_toolbar,id,HuiToolMenu,menu);
	cur_toolbar->item.back().widget = it;
}

void CHuiWindow::ToolbarAddSeparator()
{
	GtkToolItem *it=gtk_separator_tool_item_new();
	gtk_widget_show(GTK_WIDGET(it));
	gtk_toolbar_insert(GTK_TOOLBAR(cur_toolbar->widget),it,-1);
	AddToolbarItem(cur_toolbar,"",HuiToolSeparator,NULL);
	cur_toolbar->item.back().widget = it;
}

// remove all items from the toolbar
void CHuiWindow::ToolbarReset()
{
	for (int i=0;i<cur_toolbar->item.num;i++)
		gtk_widget_destroy(GTK_WIDGET(cur_toolbar->item[i].widget));
	cur_toolbar->item.clear();
}

void CHuiWindow::_ToolbarEnable_(const string &id, bool enabled)
{
	allow_signal_level ++;
	for (int t=0;t<4;t++)
		foreach(toolbar[t].item, it)
			if (id == it->id){
			    it->enabled = enabled;
				gtk_widget_set_sensitive(GTK_WIDGET(it->widget), enabled);
			}
	allow_signal_level --;
}

void CHuiWindow::_ToolbarCheck_(const string &id, bool checked)
{
	allow_signal_level++;
	for (int t=0;t<4;t++)
		foreach(toolbar[t].item, it)
			if (id == it->id)
				if (it->type == HuiToolCheckable)
					gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(it->widget), checked);
	allow_signal_level--;
}

bool CHuiWindow::_ToolbarIsChecked_(const string &id)
{
	for (int t=0;t<4;t++)
		foreach(toolbar[t].item, it)
			if (id == it->id)
				if (it->type == HuiToolCheckable)
					return gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(it->widget));
	return false;
}

#endif
