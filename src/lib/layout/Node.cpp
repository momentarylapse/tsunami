#include "Node.h"
#include <lib/base/algo.h>
#include <lib/os/msg.h>

namespace layout {



Node::Node(const string &_id) {
	id = _id;
	min_width_user = -1;
	min_height_user = -1;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Expand;
	area = rect::EMPTY;
	padding = rect::EMPTY;
	margin = rect::EMPTY;
}

Node::~Node() = default;

/*Array<Control*> Node::get_children_recursive(bool include_me, ChildFilter f) const {
	Array<Control*> r;
	if (include_me)
		r.add(const_cast<Control*>(this));
	for (auto c: get_children(f))
		r.append(c->get_children_recursive(true, f));
	return r;
}*/



vec2 Node::get_content_min_size() const {
	return {0,0};
}

vec2 Node::get_greed_factor() const {
	vec2 f = greed_factor;
	if (size_mode_x != SizeMode::Expand)
		f.x = 0;
	if (size_mode_y != SizeMode::Expand)
		f.y = 0;
	return f;
}

vec2 Node::get_effective_min_size() const {
	vec2 s = get_content_min_size() + padding.p00() + padding.p11();
	if (min_width_user >= 0)
		s.x = min_width_user;
	if (min_height_user >= 0)
		s.y = min_height_user;
	return s + margin.p00() + margin.p11();
}

void Node::negotiate_content_area(const rect &available) {
}

void Node::negotiate_outer_area(const rect& _available) {
	const rect available = {_available.p00() + margin.p00(), _available.p11() - margin.p11()};
	area = available;

	if (size_mode_x != SizeMode::Expand or size_mode_y != SizeMode::Expand) {
		const auto min_size = get_effective_min_size() - margin.p00() - margin.p11();
		if (size_mode_x == SizeMode::Shrink) {
			area.x1 = available.x1 + align.x * (available.width() - min_size.x);
			area.x2 = area.x1 + min_size.x;
		}
		if (size_mode_y == SizeMode::Shrink) {
			area.y1 = available.y1 + align.y * (available.height() - min_size.y);
			area.y2 = area.y1 + min_size.y;
		}
	}

	negotiate_content_area(content_area());
}

rect Node::content_area() const {
	return {area.p00() + padding.p00(), area.p11() - padding.p11()};
}

rect Node::outer_area() const {
	return {area.p00() - margin.p00(), area.p11() + margin.p11()};
}

Array<Node*> Node::get_children_recursive(bool include_me, ChildFilter f) const {
	Array<Node*> r;
	if (include_me)
		r.add(const_cast<Node*>(this));
	for (auto c: _get_children(f))
		r.append(c->get_children_recursive(true, f));
	return r;
}

void Node::set_option(const string& key, const string& value) {
	if (key == "expand") {
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
	} else if (key == "expandx") {
		size_mode_x = SizeMode::Expand;
		if (value._bool())
			size_mode_x = SizeMode::Fill;
	} else if (key == "expandy") {
		size_mode_y = SizeMode::Expand;
		if (value._bool())
			size_mode_y = SizeMode::Fill;
	} else if (key == "noexpandx" or key == "fillx") {
		size_mode_x = SizeMode::Fill;
	} else if (key == "noexpandy" or key == "filly") {
		size_mode_y = SizeMode::Fill;
	} else if (key == "fill") {
		size_mode_x = SizeMode::Fill;
		size_mode_y = SizeMode::Fill;
	} else if (key == "shrink") {
		size_mode_x = SizeMode::Shrink;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "shrinkx") {
		size_mode_x = SizeMode::Shrink;
	} else if (key == "shrinky") {
		size_mode_y = SizeMode::Shrink;
	} else if (key == "width") {
		min_width_user = value._float();
		size_mode_x = SizeMode::Shrink;
	} else if (key == "height") {
		min_height_user = value._float();
		size_mode_y = SizeMode::Shrink;
	} else if (key == "greedfactorx") {
		greed_factor.x = value._float();
		size_mode_x = SizeMode::Expand;
	} else if (key == "greedfactory") {
		greed_factor.y = value._float();
		size_mode_y = SizeMode::Expand;
	} else if (key == "top") {
		align.y = 0;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "centery" or key == "centerv") {
		align.y = 0.5f;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "bottom") {
		align.y = 1;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "left") {
		align.x = 0;
		size_mode_x = SizeMode::Shrink;
	} else if (key == "centerx" or key == "centerh") {
		align.x = 0.5f;
		size_mode_x = SizeMode::Shrink;
	} else if (key == "right") {
		align.x = 1;
		size_mode_x = SizeMode::Shrink;
	} else if (key == "center") {
		align = {0.5f, 0.5f};
		size_mode_x = SizeMode::Shrink;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "padding") {
		float f = value._float();
		padding = {f, f, f, f};
	} else if (key == "paddingx" or key == "paddingh") {
		float f = value._float();
		padding.x1 = padding.x2 = f;
	} else if (key == "paddingy" or key == "paddingv") {
		float f = value._float();
		padding.y1 = padding.y2 = f;
	} else if (key == "paddingtop") {
		padding.y1 = value._float();
	} else if (key == "paddingbottom") {
		padding.y2 = value._float();
	} else if (key == "paddingleft") {
		padding.x1 = value._float();
	} else if (key == "paddingright") {
		padding.x2 = value._float();
	} else if (key == "margin") {
		float f = value._float();
		margin = rect(f, f, f, f);
	} else if (key == "hmargin" or key == "marginh" or key == "marginx") {
		margin.x1 = margin.x2 = value._float();
	} else if (key == "vmargin" or key == "marginv" or key == "marginy") {
		margin.y1 = margin.y2 = value._float();
	} else if (key == "margintop") {
		margin.y1 = value._float();
	} else if (key == "marginbottom") {
		margin.y2 = value._float();
	} else if (key == "marginleft") {
		margin.x1 = value._float();
	} else if (key == "marginright") {
		margin.x2 = value._float();
	} else if (key == "hidden") {
		visible = false;
	} else if (key == "visible") {
		visible = value._bool() or value == "";
	}
}


}
