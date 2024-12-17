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
#include "../../../lib/math/rect.h"
#include "../../../lib/math/vec2.h"
#include "../../../lib/pattern/Observable.h"

class Painter;
namespace tsunami {
class HoverData;
}

namespace scenegraph {

class SceneGraph;

class Node : public Sharable<obs::Node<VirtualBase>> {
public:
	Node();
	Node(float w, float h);
	~Node() override;

	virtual bool allow_handle_click_when_gaining_focus() const { return true; }

	// return: block upwards propagation
	virtual bool on_left_button_down(const vec2 &m);
	virtual bool on_left_button_up(const vec2 &m);
	virtual bool on_left_double_click(const vec2 &m);
	virtual bool on_right_button_down(const vec2 &m);
	virtual bool on_right_button_up(const vec2 &m);
	virtual bool on_mouse_move(const vec2 &m);
	virtual bool on_mouse_wheel(const vec2 &d);
	virtual bool on_gesture(const string &id, const vec2 &m, const vec2 &param);
	virtual bool on_key_down(int key);
	virtual bool on_key_up(int key);

	virtual bool has_hover(const vec2 &m) const;

	virtual void draw_recursive(Painter *p);
	virtual void on_draw(Painter *p) {}
	virtual tsunami::HoverData get_hover_data(const vec2 &m);
	void add_child(Node *child);
	void delete_child(Node *child);

	// this or a decendent has hover?
	bool is_cur_hover() const;
	// top-most hover?
	bool is_cur_hover_non_recursive() const;

	virtual string get_tip() const;

	virtual void update_geometry(const rect &target_area);
	virtual void update_geometry_recursive(const rect &target_area);

	SceneGraph *graph() const;
	vec2 cursor() const;

	Node *parent;
	struct AlignData {
		enum class Mode{
			None,
			Left,
			Right,
			Top,
			Bottom,
			Fill,
			Absolute
		};
		Mode horizontal, vertical;
		float dx, dy, dz;
		float w, h;
	} align;
	shared_array<Node> children;
	rect area;
	int z;
	bool hidden;
	bool clip;
	void set_hidden(bool hide);

	int perf_channel;
	void set_perf_name(const string &name);

	Array<Node*> collect_children(bool include_hidden) const;
	Array<Node*> collect_children_up() const;
	Array<Node*> collect_children_down() const;
	void request_redraw();

	std::function<vec2(const vec2&)> f_local_to_parent;
	std::function<vec2(const vec2&)> f_parent_to_local;
	vec2 local_to_pixel(const vec2& p) const;
	vec2 pixel_to_local(const vec2& p) const;

	static bool show_debug;
};

class NodeFree : public Node {
public:
	NodeFree();
};

class NodeRel : public Node {
public:
	NodeRel(const vec2 &d, float w, float h);
};


class HBox : public Node {
public:
	HBox();
	void update_geometry_recursive(const rect &target_area) override;
	bool has_hover(const vec2 &m) const override { return false; }
};

class VBox : public Node {
public:
	VBox();
	void update_geometry_recursive(const rect &target_area) override;
	bool has_hover(const vec2 &m) const override { return false; }
};

}

#endif /* SRC_VIEW_HELPER_GRAPH_NODE_H_ */
