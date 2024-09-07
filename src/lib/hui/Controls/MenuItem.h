/*
 * MenuItem.h
 *
 *  Created on: 26.06.2013
 *      Author: michi
 */

#ifndef MENUITEM_H_
#define MENUITEM_H_

#include "Control.h"

typedef struct _GMenuItem GMenuItem;

namespace hui {

class BasicMenuItem : public Control {
public:
	BasicMenuItem(int type, const string &id);
	virtual ~BasicMenuItem();

	void take_gtk_ownership();
//#if GTK_CHECK_VERSION(4,0,0)
	GMenuItem *item = nullptr;
//#endif
};

class MenuItem : public BasicMenuItem {
public:
	MenuItem(const string &title, const string &id, Panel *panel);

	void set_image(const string &image) override;
};

};

#endif /* MENUITEM_H_ */
