/*
 * HuiMenuItemSubmenuWin.cpp
 *
 *  Created on: 13.07.2013
 *      Author: michi
 */

#include "../Menu.h"
#include "MenuItemSubmenu.h"

namespace hui
{

#ifdef HUI_API_WIN

HuiMenuItemSubmenu::HuiMenuItemSubmenu(const string &title, HuiMenu *menu, const string &id) :
	HuiControl(HuiKindMenuItemSubmenu, id)
{
}

HuiMenuItemSubmenu::~HuiMenuItemSubmenu()
{
}

#endif

};
