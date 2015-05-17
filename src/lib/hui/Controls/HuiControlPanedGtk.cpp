/*
 * HuiControlPanedGtk.cpp
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#include "HuiControlPaned.h"

#ifdef HUI_API_GTK

HuiControlPaned::HuiControlPaned(const string &title, const string &id) :
	HuiControl(HUI_KIND_PANED, id)
{
}

void HuiControlPaned::add(HuiControl *child, int x, int y)
{
}

#endif
