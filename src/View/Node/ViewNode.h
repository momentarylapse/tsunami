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
class HoverData;

class ViewNode : public Observable<VirtualBase> {
public:
	ViewNode();
	ViewNode(float w, float h);
	virtual ~ViewNode();

	virtual bool allow_handle_click_when_gaining_focus() { return true; }

	virtual bool on_left_button_down() { return false; }
	virtual bool on_left_button_up() { return false; }
	virtual bool on_left_double_click() { return false; }
	virtual bool on_right_button_down() { return false; }
	virtual bool on_mouse_move() { return false; }

	virtual bool hover(float mx, float my);
	virtual void draw(Painter *p) {}
	virtual HoverData get_hover_data(float mx, float my);
	void add_child(ViewNode *child);
	void delete_child(ViewNode *child);

	bool is_cur_hover();
	bool is_cur_hover_non_recursive();

	virtual string get_tip();

	virtual void update_area();

	ViewNode *root();

	ViewNode *parent;
	struct AlignData {
		enum class Mode{
			NONE,
			LEFT,
			RIGHT,
			TOP,
			BOTTOM,
			FILL
		};
		Mode horizontal, vertical;
		float dx, dy, dz;
		float w, h;
	} align;
	Array<ViewNode*> children;
	rect area;
	int z;
	bool hidden;
};

class ViewNodeFree : public ViewNode {
public:
	ViewNodeFree();
};

class ViewNodeRel : public ViewNode {
public:
	ViewNodeRel(float dx, float dy, float w, float h);
};

#endif /* SRC_VIEW_NODE_VIEWNODE_H_ */
