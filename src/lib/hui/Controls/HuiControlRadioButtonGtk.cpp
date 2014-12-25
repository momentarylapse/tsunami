/*
 * HuiControlRadioButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "HuiControlRadioButton.h"
#include "../hui.h"


#ifdef HUI_API_GTK

void OnGtkRadioButtonToggle(GtkWidget *widget, gpointer data)
{	((HuiControl*)data)->notify("hui:change");	}

HuiControlRadioButton::HuiControlRadioButton(const string &title, const string &id, HuiPanel *panel) :
	HuiControl(HuiKindRadioButton, id)
{
	GetPartStrings(id, title);
	string group_id = id.head(id.find(":"));
	GSList *group = NULL;
	foreach(HuiControl *c, panel->control)
		if (c->type == HuiKindRadioButton)
			if (c->id.find(":"))
				if (c->id.head(c->id.find(":")) == group_id)
					group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(c->widget));

	widget = gtk_radio_button_new_with_label(group, sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&OnGtkRadioButtonToggle), this);
	setOptions(OptionString);
}

void HuiControlRadioButton::__setString(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void HuiControlRadioButton::__check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

string HuiControlRadioButton::getString()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

bool HuiControlRadioButton::isChecked()
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

#endif
