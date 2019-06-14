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

class AudioView;

class Cursor : public ViewNode {
public:
	Cursor(AudioView *view, bool end);

	void draw(Painter *p) override;
	bool hover(float mx, float my) override;
	string get_tip() override;

	int pos();

	AudioView *view;
	Range drag_range;
	int is_end;

	bool on_left_button_down() override;
};

class SelectionMarker : public ViewNode {
public:
	SelectionMarker(AudioView *view);
	bool hover(float mx, float my) override { return false; }

	AudioView *view;
	void draw(Painter *p) override;
};

#endif /* SRC_VIEW_NODE_CURSOR_H_ */
