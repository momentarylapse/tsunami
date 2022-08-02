/*
 * MenuItemToggle.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEMTOGGLE_H_
#define MENUITEMTOGGLE_H_

#include "MenuItem.h"

namespace hui {

class MenuItemToggle : public BasicMenuItem {
public:
	MenuItemToggle(const string &title, const string &id, Panel *panel);

	void __check(bool checked) override;
	bool is_checked() override;

	bool checked = false;
};

};

#endif /* MENUITEMTOGGLE_H_ */
