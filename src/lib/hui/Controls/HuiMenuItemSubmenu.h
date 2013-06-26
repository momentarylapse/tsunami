/*
 * HuiMenuItemSubmenu.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef HUIMENUITEMSUBMENU_H_
#define HUIMENUITEMSUBMENU_H_

#include "HuiControl.h"

class HuiMenu;

class HuiMenuItemSubmenu : public HuiControl
{
public:
	HuiMenuItemSubmenu(const string &title, HuiMenu *menu, const string &id);
	virtual ~HuiMenuItemSubmenu();

	HuiMenu *sub_menu;
};

#endif /* HUIMENUITEMSUBMENU_H_ */
