/*
 * HuiControlEdit.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlEdit.h"

#ifdef HUI_API_GTK

void set_list_cell(GtkListStore *store, GtkTreeIter &iter, int column, const string &str);

void OnGtkEditChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlEdit::HuiControlEdit(const string &title, const string &id) :
	HuiControl(HuiKindEdit, id)
{
	GetPartStrings(id, title);
	widget = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(widget), 3);
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(PartString[0]));
	gtk_entry_set_activates_default(GTK_ENTRY(widget), true);
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkEditChange), this);
	SetOptions(OptionString);
}

HuiControlEdit::~HuiControlEdit() {
	// TODO Auto-generated destructor stub
}

void HuiControlEdit::__SetString(const string &str)
{
	gtk_entry_set_text(GTK_ENTRY(widget), sys_str(str));
}

string HuiControlEdit::GetString()
{
	return de_sys_str(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void HuiControlEdit::CompletionAdd(const string &text)
{
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

void HuiControlEdit::CompletionClear()
{
	gtk_entry_set_completion(GTK_ENTRY(widget), NULL);
}

#endif
