/*
 * MenuItemToggle.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEMTOGGLE_H_
#define MENUITEMTOGGLE_H_

#include "Control.h"

namespace hui
{

class MenuItemToggle : public Control
{
public:
	MenuItemToggle(const string &title, const string &id, Panel *panel);

	void __check(bool checked) override;
	bool is_checked() override;

#if GTK_CHECK_VERSION(4,0,0)
	GMenuItem *item;
#endif
	bool checked = false;
};

};

#endif /* MENUITEMTOGGLE_H_ */
