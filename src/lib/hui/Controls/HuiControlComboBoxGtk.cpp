/*
 * HuiControlComboBox.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlComboBox.h"

#ifdef HUI_API_GTK

void OnGtkComboboxChange(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->Notify("hui:change");	}

HuiControlComboBox::HuiControlComboBox(const string &title, const string &id) :
	HuiControl(HuiKindComboBox, id)
{
	GetPartStrings(id, title);
	widget = gtk_combo_box_text_new();
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(&OnGtkComboboxChange), this);
	if ((PartString.num > 1) || (PartString[0] != ""))
		for (int i=0;i<PartString.num;i++)
			__SetString(PartString[i]);
	SetInt(0);
	SetOptions(OptionString);
}

HuiControlComboBox::~HuiControlComboBox()
{
}

string HuiControlComboBox::GetString()
{
	return "TODO";
}

void HuiControlComboBox::__SetString(const string &str)
{
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),sys_str(str));
}

void HuiControlComboBox::__AddString(const string& str)
{
#if GTK_MAJOR_VERSION >= 3
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, sys_str(str));
#else
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), sys_str(str));
#endif
}

void HuiControlComboBox::__SetInt(int i)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), i);
}

int HuiControlComboBox::GetInt()
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

void HuiControlComboBox::__Reset()
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
