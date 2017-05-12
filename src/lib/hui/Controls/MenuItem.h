/*
 * MenuItem.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEM_H_
#define MENUITEM_H_

#include "Control.h"

namespace hui
{

class MenuItem : public Control
{
public:
	MenuItem(const string &title, const string &id);

	virtual void setImage(const string &image);
};

};

#endif /* MENUITEM_H_ */
