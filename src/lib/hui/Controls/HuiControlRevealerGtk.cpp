/*
 * HuiControlRevealerGtk.cpp
 *
 *  Created on: 17.05.2015
 *      Author: michi
 */

#include "HuiControlRevealer.h"

#ifdef HUI_API_GTK

HuiControlRevealer::HuiControlRevealer(const string &title, const string &id) :
	HuiControl(HUI_KIND_REVEALER, id)
{
	GetPartStrings(id, title);
	widget = gtk_revealer_new();
	setOptions(OptionString);

	// GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP
	//gtk_revealer_set_transition_type(GTK_REVEALER(widget), GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
}


void HuiControlRevealer::add(HuiControl *child, int x, int y)
{
	GtkWidget *child_widget = child->get_frame();
	gtk_container_add(GTK_CONTAINER(widget), child_widget);
	children.add(child);
	child->parent = this;
}

void HuiControlRevealer::reveal(bool reveal)
{
	gtk_revealer_set_reveal_child(GTK_REVEALER(widget), reveal);
}

bool HuiControlRevealer::isRevealed()
{
	return gtk_revealer_get_reveal_child(GTK_REVEALER(widget));
}

#endif
