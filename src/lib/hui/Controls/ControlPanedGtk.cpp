/*
 * ControlPanedGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "ControlPaned.h"

#ifdef HUI_API_GTK

namespace hui
{

ControlPaned::ControlPaned(const string &title, const string &id) :
	Control(CONTROL_PANED, id)
{
	GetPartStrings(title);

	if (OptionString.find("vertical") >= 0)
		widget = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	else
		widget = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	set_options(OptionString);
}

void ControlPaned::add(Control *child, int x, int y) {
	if (x == 0 and y == 0)
		gtk_paned_add1(GTK_PANED(widget), child->get_frame());
	else
		gtk_paned_add2(GTK_PANED(widget), child->get_frame());
}

int ControlPaned::get_int() {
	return gtk_paned_get_position(GTK_PANED(widget));
}

void ControlPaned::__set_int(int i) {
	gtk_paned_set_position(GTK_PANED(widget), i);
}

}
;

#endif
