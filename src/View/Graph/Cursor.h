/*
 * Cursor.h
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_GRAPH_CURSOR_H_
#define SRC_VIEW_GRAPH_CURSOR_H_

#include "../Helper/Graph/Node.h"
#include "../../Data/Range.h"

class AudioView;
class color;

class Cursor : public scenegraph::NodeFree {
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

class SelectionMarker : public scenegraph::NodeFree {
public:
	SelectionMarker(AudioView *view);
	bool hover(float mx, float my) override { return false; }

	AudioView *view;
	void draw(Painter *p) override;
	void draw_bar_gap_selector(Painter* p, int bar_gap, const color &col);
};

#endif /* SRC_VIEW_GRAPH_CURSOR_H_ */
