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
class HoverData;

class ViewNode : public Observable<VirtualBase> {
public:
	ViewNode(AudioView *view);
	ViewNode(ViewNode *parent, float dx, float dy, float w, float h);
	virtual ~ViewNode();

	virtual bool allow_handle_click_when_gaining_focus() { return true; }

	virtual bool on_left_button_down() { return false; }
	virtual bool on_left_button_up() { return false; }
	virtual bool on_left_double_click() { return false; }
	virtual bool on_right_button_down() { return false; }
	virtual bool on_mouse_move() { return false; }

	virtual bool hover();
	virtual void draw(Painter *p) {}
	bool view_hover();
	bool view_hover_non_recursive();
	virtual HoverData get_hover_data();

	virtual string get_tip();

	virtual void update_area();

	AudioView *view;
	ViewNode *parent;
	struct AlignData {
		bool align_right;
		bool align_bottom;
		float dx, dy;
		float w, h;
	} align;
	Array<ViewNode*> children;
	rect area;
	int z;
	bool hidden;
};

#endif /* SRC_VIEW_NODE_VIEWNODE_H_ */
