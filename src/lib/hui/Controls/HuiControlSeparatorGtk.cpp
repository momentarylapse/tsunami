/*
 * HuiControlSeparator.cpp
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#include "HuiControlSeparator.h"

HuiControlSeparator::HuiControlSeparator(const string &text, const string &_id) :
	HuiControl(HuiKindSeparator, _id)
{
	GetPartStrings(id, text);
	if (OptionString.find("vertical"))
		widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	else
		widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	SetOptions(OptionString);
}

HuiControlSeparator::~HuiControlSeparator()
{
}

