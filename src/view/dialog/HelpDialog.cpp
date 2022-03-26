/*
 * HelpDialog.cpp
 *
 *  Created on: 31.05.2019
 *      Author: michi
 */

#include "HelpDialog.h"

HelpDialog::HelpDialog(hui::Window *_parent) :
	hui::Dialog("help-dialog", _parent)
{
	if (hui::config.get_bool("FirstStart", true))
		hide_control("first-time-message", false);
	event("close", [=]{ request_destroy(); });
}

