/*
 * HuiControlEdit.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlEdit.h"

#ifdef HUI_API_GTK

namespace hui
{

void set_list_cell(GtkListStore *store, GtkTreeIter &iter, int column, const string &str);

void on_gtk_edit_changed(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlEdit::ControlEdit(const string &title, const string &id) :
	Control(CONTROL_EDIT, id)
{
	auto parts = split_title(title);
	widget = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(widget), 3);
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(parts[0]));
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&on_gtk_edit_changed), this);
}

void ControlEdit::__set_string(const string &str) {
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
}

string ControlEdit::get_string() {
	return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void ControlEdit::__reset() {
	__set_string("");
}

void ControlEdit::completion_add(const string &text) {
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
}

void ControlEdit::completion_clear() {
	gtk_entry_set_completion(GTK_ENTRY(widget), nullptr);
}

void ControlEdit::__set_option(const string &op, const string &value) {
	if (op == "clearplaceholder")
		gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "");
	else if (op == "clearcompletion")
		gtk_entry_set_completion(GTK_ENTRY(widget), nullptr);
	else if (op == "placeholder")
		gtk_entry_set_placeholder_text(GTK_ENTRY(widget), value.c_str());
	else if (op == "completion")
		completion_add(value);
}

};

#endif
