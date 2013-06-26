/*
 * HuiControlTabControl.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlTabControl.h"
#include "../hui.h"

void OnGtkTabControlSwitch(GtkWidget *widget, GtkWidget *page, guint page_num, gpointer data)
{
	HuiControlTabControl *c = (HuiControlTabControl*)data;
	c->cur_page = page_num;
	c->Notify("hui:change");
}


HuiControlTabControl::HuiControlTabControl(const string &title, const string &id, HuiWindow *win) :
	HuiControl(HuiKindTabControl, id)
{
	GetPartStrings(id, title);
	widget = gtk_notebook_new();

	for (int i=0;i<PartString.num;i++){
		GtkWidget *inside;
		if (win->is_resizable){
			inside = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_box_set_homogeneous(GTK_BOX(inside), true);
		}else
			inside = gtk_fixed_new();
		gtk_widget_show(inside);
		GtkWidget *label = gtk_label_new(sys_str(PartString[i]));
		gtk_notebook_append_page(GTK_NOTEBOOK(widget), inside, label);
    }
	cur_page = 0;
	g_signal_connect(G_OBJECT(widget), "switch-page", G_CALLBACK(&OnGtkTabControlSwitch), this);
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(widget), true);
	if (OptionString.find("nobar") >= 0)
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(widget), false);
}

HuiControlTabControl::~HuiControlTabControl() {
	// TODO Auto-generated destructor stub
}

string HuiControlTabControl::GetString()
{
}

void HuiControlTabControl::__SetString(const string& str)
{
}

void HuiControlTabControl::__SetInt(int i)
{
	gtk_notebook_set_current_page(GTK_NOTEBOOK(widget), i);
	cur_page = i;
}

int HuiControlTabControl::GetInt()
{
	return cur_page;
}
