/*
 * HuiControlComboBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlComboBox.h"

#ifdef HUI_API_GTK

void OnGtkComboboxChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->notify("hui:change");	}

HuiControlComboBox::HuiControlComboBox(const string &title, const string &id) :
	HuiControl(HuiKindComboBox, id)
{
	GetPartStrings(id, title);
	if (OptionString.find("editable") >= 0){
		widget = gtk_combo_box_text_new_with_entry();
		gtk_entry_set_activates_default(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(widget))), true);
	}else{
		widget = gtk_combo_box_text_new();
	}
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkComboboxChange), this);
	if ((PartString.num > 1) || (PartString[0] != ""))
		for (int i=0;i<PartString.num;i++)
			__setString(PartString[i]);
	setInt(0);
	setOptions(OptionString);
}

string HuiControlComboBox::getString()
{
	char *c = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
	string s = c;
	g_free(c);
	return s;
}

void HuiControlComboBox::__setString(const string &str)
{
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),sys_str(str));
}

void HuiControlComboBox::__addString(const string& str)
{
#if GTK_MAJOR_VERSION >= 3
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, sys_str(str));
#else
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), sys_str(str));
#endif
}

void HuiControlComboBox::__setInt(int i)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), i);
}

int HuiControlComboBox::getInt()
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

void HuiControlComboBox::__reset()
{
#if GTK_MAJOR_VERSION >= 3
	gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widget));
#else
	/*GtkTreeModel *m = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	gtk_tree_model_cl
	gtk_combo_box_remove_all(GTK_COMBO_BOX_TEXT(widget));*/
	msg_todo("ComboBox.Reset for gtk 2.24");
#endif
}

#endif
