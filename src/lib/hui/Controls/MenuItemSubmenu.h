/*
 * MenuItemSubmenu.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEMSUBMENU_H_
#define MENUITEMSUBMENU_H_

#include "MenuItem.h"

namespace hui {

class Menu;

class MenuItemSubmenu : public BasicMenuItem {
public:
	MenuItemSubmenu(const string &title, xfer<Menu> menu, const string &id);
	virtual ~MenuItemSubmenu();

	owned<Menu> sub_menu;
};

};

#endif /* MENUITEMSUBMENU_H_ */
