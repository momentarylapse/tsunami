/*
 * Cursor.h
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_CURSOR_H_
#define SRC_VIEW_NODE_CURSOR_H_

#include "ViewNode.h"
#include "../../Data/Range.h"

class Cursor : public ViewNode {
public:
	Cursor(AudioView *view);

	void draw(Painter *p) override;
	bool hover() override;
	string get_tip() override;

	bool hover_start();
	bool hover_end();

	Range drag_range;

	bool on_left_button_down() override;
};

#endif /* SRC_VIEW_NODE_CURSOR_H_ */
