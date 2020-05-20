/*
 * HuiControlComboBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlComboBox.h"

#ifdef HUI_API_GTK

namespace hui
{

void on_gtk_combo_box_change(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlComboBox::ControlComboBox(const string &title, const string &id) :
	Control(CONTROL_COMBOBOX, id)
{
	auto parts = split_title(title);
	editable = option_has(get_option_from_title(title), "editable");
	if (editable) {
		widget = gtk_combo_box_text_new_with_entry();
		gtk_entry_set_activates_default(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))), true);
	} else {
		widget = gtk_combo_box_text_new();
	}
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&on_gtk_combo_box_change), this);

	if ((parts.num > 1) or (parts[0] != ""))
		for (string &p: parts)
			__add_string(p);

	set_int(0);
}

string ControlComboBox::get_string() {
	char *c = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
	string s = c;
	g_free(c);
	return s;
}

void ControlComboBox::__set_string(const string &str) {
	if (editable) {
		gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))), sys_str(str));
	} else {
		__add_string(str);
		//gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),sys_str(str));
	}
}

void ControlComboBox::__add_string(const string& str) {
#if GTK_CHECK_VERSION(3,0,0)
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, sys_str(str));
#else
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), sys_str(str));
#endif
}

void ControlComboBox::__set_int(int i) {
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), i);
}

int ControlComboBox::get_int() {
	return gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

void ControlComboBox::__reset()
{
#if GTK_CHECK_VERSION(3,0,0)
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widget));
#else
	/*GtkTreeModel *m = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	gtk_tree_model_cl
	gtk_combo_box_remove_all(GTK_COMBO_BOX_TEXT(widget));*/
	msg_todo("ComboBox.Reset for gtk 2.24");
#endif
}

};

#endif
