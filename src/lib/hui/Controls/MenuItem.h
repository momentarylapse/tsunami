/*
 * MenuItem.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEM_H_
#define MENUITEM_H_

#include "Control.h"

namespace hui {

class MenuItem : public Control {
public:
	MenuItem(const string &title, const string &id, Panel *panel);

	void set_image(const string &image) override;

#if GTK_CHECK_VERSION(4,0,0)
	GMenuItem *item;
#endif
};

};

#endif /* MENUITEM_H_ */
