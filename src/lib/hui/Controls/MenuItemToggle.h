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
	MenuItemToggle(const string &title, const string &id);

	virtual void __check(bool checked);
	virtual bool isChecked();
};

};

#endif /* MENUITEMTOGGLE_H_ */
