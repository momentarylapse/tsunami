#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>

namespace layout {

enum class ChildFilter {
	All,
	OnlyActive
};

enum class Orientation {
	HORIZONTAL,
	VERTICAL,
};

enum class SizeMode {
	Shrink, // as small as possible = get_content_min_size()
	Fill,   // use up available space, >= get_content_min_size()
	Expand, // as large as possible (using greed-factor)
	ForwardChild
};

class Node : public Sharable<VirtualBase> {
public:
	explicit Node(const string& id);
	~Node() override;

	// full registration!
//	virtual void add_child(shared<Node> c, int x, int y) {}
//	virtual void remove_child(Node* c) {}

	virtual Array<const Node*> _get_children(ChildFilter f) const { return {}; }
	Array<Node*> get_children_recursive(bool include_me, ChildFilter f) const;

	virtual void on_left_button_down(const vec2& m) {}
	virtual void on_left_button_up(const vec2& m) {}
	virtual void on_left_double_click(const vec2& m) {}
	virtual void on_middle_button_down(const vec2& m) {}
	virtual void on_middle_button_up(const vec2& m) {}
	virtual void on_right_button_down(const vec2& m) {}
	virtual void on_right_button_up(const vec2& m) {}
	virtual void on_mouse_move(const vec2& m, const vec2& d) {}
	virtual void on_mouse_enter(const vec2& m) {}
	virtual void on_mouse_leave(const vec2& m) {}
	virtual void on_mouse_wheel(const vec2& d) {}
	/*virtual void on_key_down(int key) {}
	virtual void on_key_up(int key) {}
	virtual void on_key_char(int character) {}*/

	virtual void set_option(const string& key, const string& value);

	string id;
	rect area; // "drawable": content + padding
	rect content_area() const;
	rect outer_area() const;

	float min_width_user, min_height_user;
	SizeMode size_mode_x, size_mode_y;
	vec2 align = vec2(0.5f, 0.5f); // (0,0)=top-left (1,1)=bottom-right
	vec2 greed_factor = vec2(1, 1); // if expanding... (only compared against direct siblings!)
	bool visible = true;
	bool ignore_hover = false;
	rect padding; // space "inside", around children/content
	rect margin; // space "outside"

	virtual vec2 get_content_min_size() const; // excluding padding/margin
	vec2 effective_min_size() const; // including padding/margin
	SizeMode effective_size_mode_x() const;
	SizeMode effective_size_mode_y() const;
	SizeMode most_aggressive_child_size_mode_x() const;
	SizeMode most_aggressive_child_size_mode_y() const;

	virtual void negotiate_content_area(const rect& available); // excluding padding/margin
	void negotiate_outer_area(const rect& available); // including padding/margin
};

Node* get_hover(Node* top, const vec2& p);

template<class T>
T* get_hover_as(Node* top, const vec2& p) {
	return reinterpret_cast<T*>(get_hover(top, p));
}

}
