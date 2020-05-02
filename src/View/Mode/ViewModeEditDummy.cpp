/*
 * ViewModeEditDummy.cpp
 *
 *  Created on: May 2, 2020
 *      Author: michi
 */

#include "ViewModeEditDummy.h"
#include "../SideBar/SideBar.h"

ViewModeEditDummy::ViewModeEditDummy(AudioView *view) : ViewModeDefault(view) {
}

void ViewModeEditDummy::on_start() {
	set_side_bar(SideBar::DUMMY_EDITOR_CONSOLE);
}

