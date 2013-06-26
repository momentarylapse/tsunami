/*
 * HuiControlMultilineEdit.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControlMultilineEdit.h"

void OnGtkMultilineEditChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlMultilineEdit::HuiControlMultilineEdit(const string &title, const string &id) :
	HuiControl(HuiKindMultilineEdit, id)
{
	GetPartStrings(id, title);
	GtkTextBuffer *tb = gtk_text_buffer_new(NULL);
	widget = gtk_text_view_new_with_buffer(tb);
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(scroll);
	gtk_container_add(GTK_CONTAINER(scroll), widget);

	// frame
	frame = scroll;
//	if (border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), scroll);
//	}
	g_signal_connect(G_OBJECT(tb), "changed", G_CALLBACK(&OnGtkMultilineEditChange), this);
}

HuiControlMultilineEdit::~HuiControlMultilineEdit()
{
	// TODO Auto-generated destructor stub
}

void HuiControlMultilineEdit::__SetString(const string &str)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	const char *str2 = sys_str(str);
	gtk_text_buffer_set_text(tb, str2, strlen(str2));
}

string HuiControlMultilineEdit::GetString()
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter is, ie;
	gtk_text_buffer_get_iter_at_offset(tb, &is, 0);
	gtk_text_buffer_get_iter_at_offset(tb, &ie, -1);
	return de_sys_str(gtk_text_buffer_get_text(tb, &is, &ie, false));
}

void HuiControlMultilineEdit::__AddString(const string& str)
{
}
