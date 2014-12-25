/*
 * HuiControlTabControl.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlTabControl.h"
#include "../hui.h"

#ifdef HUI_API_GTK

void OnGtkTabControlSwitch(GtkWidget *widget, GtkWidget *page, guint page_num, gpointer data)
{
	HuiControlTabControl *c = (HuiControlTabControl*)data;
	c->cur_page = page_num;
	c->notify("hui:change");
}


HuiControlTabControl::HuiControlTabControl(const string &title, const string &id, HuiPanel *panel) :
	HuiControl(HuiKindTabControl, id)
{
	GetPartStrings(id, title);
	widget = gtk_notebook_new();
	this->panel = panel; // for addPage()

	for (int i=0;i<PartString.num;i++)
		addPage(PartString[i]);

	cur_page = 0;
	g_signal_connect(G_OBJECT(widget), "switch-page", G_CALLBACK(&OnGtkTabControlSwitch), this);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(widget), true);
	if (OptionString.find("nobar") >= 0)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(widget), false);
	setOptions(OptionString);
}

string HuiControlTabControl::getString()
{
	return "";
}

void HuiControlTabControl::__setString(const string& str)
{
}

void HuiControlTabControl::__setInt(int i)
{
	gtk_notebook_set_current_page(GTK_NOTEBOOK(widget), i);
	cur_page = i;
}

int HuiControlTabControl::getInt()
{
	return cur_page;
}

void HuiControlTabControl::__addString(const string &str)
{
	addPage(str);
}

void HuiControlTabControl::addPage(const string &str)
{
	GtkWidget *inside;
	if (panel->win->is_resizable){
#if GTK_MAJOR_VERSION >= 3
		inside = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
		inside = gtk_hbox_new(true, 0);
#endif
		gtk_box_set_homogeneous(GTK_BOX(inside), true);
	}else
		inside = gtk_fixed_new();
	gtk_widget_show(inside);
	GtkWidget *label = gtk_label_new(sys_str(str));
	gtk_notebook_append_page(GTK_NOTEBOOK(widget), inside, label);
}

void HuiControlTabControl::add(HuiControl *child, int x, int y)
{
	GtkWidget *child_widget = child->widget;
	if (child->frame)
		child_widget = child->frame;
	GtkWidget *target_widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(widget), x);
	gtk_container_add(GTK_CONTAINER(target_widget), child_widget);
	if (panel)
		gtk_container_set_border_width(GTK_CONTAINER(target_widget), panel->border_width);
	children.add(child);
	child->parent = this;
}

void HuiControlTabControl::__setOption(const string &op, const string &value)
{
	if (op == "nobar")
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(widget), false);
	else if (op == "bar")
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(widget), value._bool());
}

#endif
