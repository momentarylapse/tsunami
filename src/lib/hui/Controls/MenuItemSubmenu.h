/*
 * MenuItemSubmenu.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEMSUBMENU_H_
#define MENUITEMSUBMENU_H_

#include "Control.h"

namespace hui
{

class Menu;

class MenuItemSubmenu : public Control
{
public:
	MenuItemSubmenu(const string &title, Menu *menu, const string &id);
	virtual ~MenuItemSubmenu();

	Menu *sub_menu;
};

};

#endif /* MENUITEMSUBMENU_H_ */
