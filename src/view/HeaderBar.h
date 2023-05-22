/*
 * HeaderBar.h
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#ifndef SRC_VIEW_HEADERBAR_H_
#define SRC_VIEW_HEADERBAR_H_

#include "../lib/hui/hui.h"

class TsunamiWindow;

class HeaderBar {
public:
	HeaderBar(TsunamiWindow* win);

	TsunamiWindow* win;
};

#endif /* SRC_VIEW_HEADERBAR_H_ */
