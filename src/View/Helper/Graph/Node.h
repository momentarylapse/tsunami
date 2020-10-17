/*
 * Node.h
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#ifndef SRC_VIEW_HELPER_GRAPH_NODE_H_
#define SRC_VIEW_HELPER_GRAPH_NODE_H_

#include "../../../lib/base/base.h"
#include "../../../lib/base/pointer.h"
#include "../../../lib/math/math.h"
#include "../../../Stuff/Observable.h"

class Painter;
class HoverData;

namespace scenegraph {

class Node : public Sharable<Observable<VirtualBase>> {
public:
	Node();
	Node(float w, float h);

	virtual bool allow_handle_click_when_gaining_focus() { return true; }

	virtual bool on_left_button_down() { return false; }
	virtual bool on_left_button_up() { return false; }
	virtual bool on_left_double_click() { return false; }
	virtual bool on_right_button_down() { return false; }
	virtual bool on_mouse_move() { return false; }

	virtual bool hover(float mx, float my);
	virtual void on_draw(Painter *p) {}
	virtual HoverData get_hover_data(float mx, float my);
	void add_child(Node *child);
	void delete_child(Node *child);

	bool is_cur_hover();
	bool is_cur_hover_non_recursive();

	virtual string get_tip();

	virtual void update_geometry(const rect &target_area);
	virtual void update_geometry_recursive(const rect &target_area);

	Node *root();

	Node *parent;
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
	shared_array<Node> children;
	rect area;
	int z;
	bool hidden;
};

class NodeFree : public Node {
public:
	NodeFree();
};

class NodeRel : public Node {
public:
	NodeRel(float dx, float dy, float w, float h);
};


class HBox : public Node {
public:
	HBox();
	void update_geometry_recursive(const rect &target_area) override;
};

class VBox : public Node {
public:
	VBox();
	void update_geometry_recursive(const rect &target_area) override;
};

}

#endif /* SRC_VIEW_HELPER_GRAPH_NODE_H_ */
