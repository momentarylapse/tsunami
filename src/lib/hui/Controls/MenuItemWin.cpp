/*
 * HuiMenuItemWin.cpp
 *
 *  Created on: 13.07.2013
 *      Author: michi
 */

#include "../hui.h"
#include "../internal.h"
#include "MenuItem.h"

namespace hui
{

#ifdef HUI_API_WIN

HuiMenuItem::HuiMenuItem(const string &title, const string &id) :
	HuiControl(HuiKindMenuItem, id)
{
}

HuiMenuItem::~HuiMenuItem()
{
}

void HuiMenuItem::SetImage(const string &image)
{
}

#endif

}
