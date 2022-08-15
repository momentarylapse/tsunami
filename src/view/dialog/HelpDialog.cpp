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
	auto select_page = [this] (int index) {
		set_int("tab", index);
		check("show-page-0", index == 0);
		check("show-page-1", index == 1);
		check("show-page-2", index == 2);
		check("show-page-3", index == 3);
	};
	select_page(0);

	if (hui::config.get_bool("FirstStart", true))
		hide_control("first-time-message", false);

	event("close", [this] { request_destroy(); });
	event("show-page-0", [select_page] { select_page(0); });
	event("show-page-1", [select_page] { select_page(1); });
	event("show-page-2", [select_page] { select_page(2); });
	event("show-page-3", [select_page] { select_page(3); });
}

