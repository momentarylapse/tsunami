/*
 * ViewModeEditDummy.cpp
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#include "ViewModeEditDummy.h"
#include "../sidebar/SideBar.h"

namespace tsunami {

ViewModeEditDummy::ViewModeEditDummy(AudioView *view) : ViewModeDefault(view) {
	mode_name = "dummy";
}

void ViewModeEditDummy::on_start() {
	set_side_bar(SideBar::DUMMY_EDITOR_CONSOLE);
}

string ViewModeEditDummy::get_tip() {
	return "track ALT+(↑,↓)";
}

}

