/*
 * HeaderBar.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_HEADERBAR_H_
#define SRC_VIEW_HEADERBAR_H_

#include "../lib/hui/hui.h"

namespace tsunami {

class TsunamiWindow;

class HeaderBar {
public:
	HeaderBar(TsunamiWindow* win);

	void update();

	TsunamiWindow* win;

	owned<hui::Menu> menu_load;
};

}

#endif /* SRC_VIEW_HEADERBAR_H_ */
