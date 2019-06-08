/*
 * Cursor.h
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_CURSOR_H_
#define SRC_VIEW_NODE_CURSOR_H_

#include "ViewNode.h"

class Cursor : public ViewNode {
public:
	Cursor(AudioView *view);

	void draw(Painter *p) override;
	bool hover() override;
	string get_tip() override;
};

#endif /* SRC_VIEW_NODE_CURSOR_H_ */
