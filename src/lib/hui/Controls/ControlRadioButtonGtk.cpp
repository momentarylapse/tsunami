/*
 * ControlRadioButton.cpp
 *
 *  Created on: 17.06.2013
 *      Author: michi
 */

#include "ControlRadioButton.h"
#include "../hui.h"


#ifdef HUI_API_GTK

namespace hui
{

void OnGtkRadioButtonToggle(GtkWidget *widget, gpointer data)
{	reinterpret_cast<Control*>(data)->notify("hui:change");	}

ControlRadioButton::ControlRadioButton(const string &title, const string &id, Panel *panel) :
	Control(CONTROL_RADIOBUTTON, id)
{
	GetPartStrings(title);
	string group_id = id.head(id.find(":"));
	GSList *group = nullptr;

	panel->apply_foreach("*", [&](Control *c){
		if (c->type == CONTROL_RADIOBUTTON)
			if (c->id.find(":"))
				if (c->id.head(c->id.find(":")) == group_id){
					group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(c->widget));
				}
	});


	widget = gtk_radio_button_new_with_label(group, sys_str(PartString[0]));
	g_signal_connect(G_OBJECT(widget), "toggled", G_CALLBACK(&OnGtkRadioButtonToggle), this);
	set_options(OptionString);
}

void ControlRadioButton::__set_string(const string &str)
{
	gtk_button_set_label(GTK_BUTTON(widget), sys_str(str));
}

void ControlRadioButton::__check(bool checked)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), checked);
}

string ControlRadioButton::get_string()
{
	return gtk_button_get_label(GTK_BUTTON(widget));
}

bool ControlRadioButton::is_checked()
{
	return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

};

#endif
