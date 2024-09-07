/*
 * HuiControlComboBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlComboBox.h"
#include "../Event.h"
#include "../language.h"

#include <gtk/gtk.h>

namespace hui
{

#if GTK_CHECK_VERSION(4,10,0)
static void on_gtk_drop_down_activate(GtkDropDown *self, void *p, Control *data) {
	data->notify(EventID::CHANGE);
}
#else
static void on_gtk_combo_box_change(GtkWidget *widget, Control *data) {
	data->notify(EventID::CHANGE);
}
#endif

ControlComboBox::ControlComboBox(const string &title, const string &id) :
	Control(CONTROL_COMBOBOX, id)
{
	auto parts = split_title(title);
	editable = option_has(get_option_from_title(title), "editable");
#if GTK_CHECK_VERSION(4,10,0)
	auto model = gtk_string_list_new(nullptr);
	widget = gtk_drop_down_new(G_LIST_MODEL(model), nullptr);
#else

	if (editable) {
		widget = gtk_combo_box_text_new_with_entry();
#if GTK_CHECK_VERSION(4,0,0)
		gtk_entry_set_activates_default(GTK_ENTRY(gtk_combo_box_get_child(GTK_COMBO_BOX(widget))), true);
#else
		gtk_entry_set_activates_default(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))), true);
#endif
	} else {
		widget = gtk_combo_box_text_new();
	}
#endif

	if ((parts.num > 1) or (parts[0] != ""))
		for (string &p: parts)
			__add_string(p);

	set_int(0);

#if GTK_CHECK_VERSION(4,10,0)
	take_gtk_ownership();
	g_signal_connect(G_OBJECT(widget), "notify::selected", G_CALLBACK(&on_gtk_drop_down_activate), this);
#else
	take_gtk_ownership();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&on_gtk_combo_box_change), this);
#endif
	focusable = true;
}

string ControlComboBox::get_string() {
#if GTK_CHECK_VERSION(4,10,0)
	auto o = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(widget));
	return gtk_string_object_get_string(GTK_STRING_OBJECT(o)); //g_value_get_string(G_VALUE(o));
#else
	char *c = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
	string s = c;
	g_free(c);
	return s;
#endif
}

void ControlComboBox::__set_string(const string &str) {
	if (editable) {
#if GTK_CHECK_VERSION(4,10,0)
#elif GTK_CHECK_VERSION(4,0,0)
		gtk_editable_set_text(GTK_EDITABLE(gtk_combo_box_get_child(GTK_COMBO_BOX(widget))), sys_str(str));
#else
		gtk_entry_set_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))), sys_str(str));
#endif
	} else {
		__add_string(str);
		//gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),sys_str(str));
	}
}

void ControlComboBox::__add_string(const string& str) {
#if GTK_CHECK_VERSION(4,10,0)
	auto m = gtk_drop_down_get_model(GTK_DROP_DOWN(widget));
	gtk_string_list_append(GTK_STRING_LIST(m), sys_str(str));
#else
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, sys_str(str));
#endif
}

void ControlComboBox::__set_int(int i) {
#if GTK_CHECK_VERSION(4,10,0)
	gtk_drop_down_set_selected(GTK_DROP_DOWN(widget), i);
#else
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), i);
#endif
}

int ControlComboBox::get_int() {
#if GTK_CHECK_VERSION(4,10,0)
	return gtk_drop_down_get_selected(GTK_DROP_DOWN(widget));
#else
	return gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
#endif
}

void ControlComboBox::__reset() {
#if GTK_CHECK_VERSION(4,10,0)
	auto m = gtk_drop_down_get_model(GTK_DROP_DOWN(widget));
	gtk_string_list_splice(GTK_STRING_LIST(m), 0, g_list_model_get_n_items(m), nullptr);
#else
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widget));
#endif
}

};
