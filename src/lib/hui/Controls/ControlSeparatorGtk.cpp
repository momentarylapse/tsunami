/*
 * ControlSeparator.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "ControlSeparator.h"

namespace hui
{

ControlSeparator::ControlSeparator(const string &text, const string &_id) :
	Control(CONTROL_SEPARATOR, _id)
{
	GetPartStrings(text);
	if (OptionString.find("vertical") >= 0)
		widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	else
		widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	set_options(OptionString);
}

};
