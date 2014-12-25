/*
 * HuiControlMultilineEdit.cpp
 *
 *  Created on: 18.06.2013
 *      Author: michi
 */

#include "HuiControlMultilineEdit.h"

#ifdef HUI_API_GTK

gboolean OnGtkAreaKeyDown(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
gboolean OnGtkAreaKeyUp(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

void OnGtkMultilineEditChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->notify("hui:change");	}

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
	if (OptionString.find("noframe") < 0){ //(border_width > 0){
		frame = gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(frame), scroll);
	}
	gtk_widget_set_hexpand(widget, true);
	gtk_widget_set_vexpand(widget, true);
	g_signal_connect(G_OBJECT(tb), "changed", G_CALLBACK(&OnGtkMultilineEditChange), this);
	handle_keys = (OptionString.find("handlekeys") >= 0);
	if (handle_keys){
		int mask;
		g_object_get(G_OBJECT(widget), "events", &mask, NULL);
		mask |= GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
		g_object_set(G_OBJECT(widget), "events", mask, NULL);
		g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(&OnGtkAreaKeyDown), this);
		g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(&OnGtkAreaKeyUp), this);
	}
	setOptions(OptionString);
}

void HuiControlMultilineEdit::__setString(const string &str)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	const char *str2 = sys_str(str);
	gtk_text_buffer_set_text(tb, str2, strlen(str2));
}

string HuiControlMultilineEdit::getString()
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	GtkTextIter is, ie;
	gtk_text_buffer_get_iter_at_offset(tb, &is, 0);
	gtk_text_buffer_get_iter_at_offset(tb, &ie, -1);
	return de_sys_str(gtk_text_buffer_get_text(tb, &is, &ie, false));
}

void HuiControlMultilineEdit::__addString(const string& str)
{
}

void HuiControlMultilineEdit::setTabSize(int tab_size)
{
	PangoLayout *layout = gtk_widget_create_pango_layout(widget, "W");
	int width, height;
	pango_layout_get_pixel_size(layout, &width, &height);
	PangoTabArray *ta = pango_tab_array_new(1, true);
	pango_tab_array_set_tab(ta, 0, PANGO_TAB_LEFT, width * tab_size);
	gtk_text_view_set_tabs(GTK_TEXT_VIEW(widget), ta);
}

#endif
