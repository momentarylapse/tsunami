/*
 * ControlSeparator.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "ControlSeparator.h"

namespace hui
{

ControlSeparator::ControlSeparator(const string &title, const string &_id) :
	Control(CONTROL_SEPARATOR, _id)
{
	if (option_has(get_option_from_title(title), "vertical"))
		widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	else
		widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	set_options(get_option_from_title(title));
}

};
