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
}

void ControlPaned::add(Control *child, int x, int y)
{
}

};

#endif
