/*
 * HuiControlSeparator.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "HuiControlSeparator.h"

HuiControlSeparator::HuiControlSeparator(const string &text, const string &_id) :
	HuiControl(HUI_KIND_SEPARATOR, _id)
{
	GetPartStrings(text);
	if (OptionString.find("vertical") >= 0)
		widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	else
		widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	setOptions(OptionString);
}

