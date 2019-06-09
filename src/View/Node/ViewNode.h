/*
 * ViewNode.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_NODE_VIEWNODE_H_
#define SRC_VIEW_NODE_VIEWNODE_H_

#include "../../lib/base/base.h"
#include "../../lib/math/math.h"
#include "../../Stuff/Observable.h"

class Painter;
class AudioView;

class ViewNode : public Observable<VirtualBase> {
public:
	ViewNode(AudioView *view);
	ViewNode(ViewNode *parent, float dx, float dy, float w, float h);
	virtual ~ViewNode();

	virtual bool on_left_button_down() { return false; }
	virtual bool on_left_button_up() { return false; }
	virtual bool on_right_button_down() { return false; }
	virtual bool on_mouse_move() { return false; }

	virtual bool hover();
	virtual void draw(Painter *p) {}

	virtual string get_tip();

	virtual void update_area();

	AudioView *view;
	ViewNode *parent;
	bool node_align_right;
	bool node_align_bottom;
	float node_offset_x, node_offset_y;
	float node_width, node_height;
	Array<ViewNode*> children;
	rect area;
	int z;
	bool hidden;
};

#endif /* SRC_VIEW_NODE_VIEWNODE_H_ */
