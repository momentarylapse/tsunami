/*
 * HuiControlEdit.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlEdit.h"
#include "../language.h"
#include "../Event.h"

#include <gtk/gtk.h>

#ifdef HUI_API_GTK

namespace hui
{

#if !GTK_CHECK_VERSION(4,0,0)
void set_list_cell(GtkListStore *store, GtkTreeIter &iter, int column, const string &str);
#endif

void on_gtk_edit_changed(GtkWidget *widget, gpointer data) {
	reinterpret_cast<Control*>(data)->notify(EventID::CHANGE);
}

ControlEdit::ControlEdit(const string &title, const string &id) :
	Control(CONTROL_EDIT, id)
{
	auto parts = split_title(title);
	widget = gtk_entry_new();
#if GTK_CHECK_VERSION(4,0,0)
	gtk_editable_set_text(GTK_EDITABLE(widget), sys_str(parts[0]));
#else
	gtk_entry_set_width_chars(GTK_ENTRY(widget), 3);
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(parts[0]));
#endif
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
	take_gtk_ownership();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&on_gtk_edit_changed), this);
	focusable = true;
}

void ControlEdit::__set_string(const string &str) {
#if GTK_CHECK_VERSION(4,0,0)
	gtk_editable_set_text(GTK_EDITABLE(widget), sys_str(str));
#else
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
#endif
}

string ControlEdit::get_string() {
#if GTK_CHECK_VERSION(4,0,0)
	return de_sys_str(gtk_editable_get_text(GTK_EDITABLE(widget)));
#else
	return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
#endif
}

void ControlEdit::__reset() {
	__set_string("");
}

void ControlEdit::completion_add(const string &text) {
#if !GTK_CHECK_VERSION(4,0,0)
	GtkEntryCompletion *comp = gtk_entry_get_completion(GTK_ENTRY(widget));
	if (!comp){
		comp = gtk_entry_completion_new();
		//gtk_entry_completion_set_minimum_key_length(comp, 2);
		gtk_entry_completion_set_text_column(comp, 0);
		gtk_entry_set_completion(GTK_ENTRY(widget), comp);
	}
	GtkTreeModel *m = gtk_entry_completion_get_model(comp);
	if (!m){
		m = (GtkTreeModel*)gtk_list_store_new(1, G_TYPE_STRING);
		gtk_entry_completion_set_model(comp, m);
	}
	GtkTreeIter iter;
	gtk_list_store_append(GTK_LIST_STORE(m), &iter);
	set_list_cell(GTK_LIST_STORE(m), iter, 0, text);
#endif
}

void ControlEdit::completion_clear() {
#if !GTK_CHECK_VERSION(4,10,0)
	gtk_entry_set_completion(GTK_ENTRY(widget), nullptr);
#endif
}

void ControlEdit::__set_option(const string &op, const string &value) {
	if (op == "clearplaceholder")
		gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "");
	else if (op == "clearcompletion")
		completion_clear();
	else if (op == "placeholder")
		gtk_entry_set_placeholder_text(GTK_ENTRY(widget), value.c_str());
	else if (op == "completion")
		completion_add(value);
}

};

#endif
