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
	vec2 s = {0, 0};
	for (auto c: _get_children(ChildFilter::OnlyActive))
		s = vec2::max(s, c->effective_min_size());
	return s;
}

vec2 Node::effective_min_size() const {
	vec2 s = get_content_min_size() + padding.p00() + padding.p11();
	if (min_width_user >= 0)
		s.x = min_width_user;
	if (min_height_user >= 0)
		s.y = min_height_user;
	return s + margin.p00() + margin.p11();
}

SizeMode Node::most_aggressive_child_size_mode_x() const {
	auto m = SizeMode::Shrink;
	for (auto c: _get_children(ChildFilter::OnlyActive))
		m = max(m, c->effective_size_mode_x());
	return m;
}

SizeMode Node::most_aggressive_child_size_mode_y() const {
	auto m = SizeMode::Shrink;
	for (auto c: _get_children(ChildFilter::OnlyActive))
		m = max(m, c->effective_size_mode_y());
	return m;
}

SizeMode Node::effective_size_mode_x() const {
	if (size_mode_x == SizeMode::ForwardChild)
		return most_aggressive_child_size_mode_x();
	return size_mode_x;
}

SizeMode Node::effective_size_mode_y() const {
	if (size_mode_y == SizeMode::ForwardChild)
		return most_aggressive_child_size_mode_y();
	return size_mode_y;
}

void Node::negotiate_content_area(const rect &available) {
	for (auto c: _get_children(ChildFilter::OnlyActive))
		const_cast<Node*>(c)->negotiate_outer_area(content_area());
}

void Node::negotiate_outer_area(const rect& _available) {
	const rect available = {_available.p00() + margin.p00(), _available.p11() - margin.p11()};
	area = available;

	auto mx = effective_size_mode_x();
	auto my = effective_size_mode_y();
	if (mx == SizeMode::Shrink or my == SizeMode::Shrink) {
		const auto min_size = effective_min_size() - margin.p00() - margin.p11();
		if (mx == SizeMode::Shrink) {
			area.x1 = available.x1 + align.x * (available.width() - min_size.x);
			area.x2 = area.x1 + min_size.x;
		}
		if (my == SizeMode::Shrink) {
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
	} else if (key == "expandx" or key == "expand.x") {
		size_mode_x = SizeMode::Expand;
		if (value._bool())
			size_mode_x = SizeMode::Fill;
	} else if (key == "expandy" or key == "expand.y") {
		size_mode_y = SizeMode::Expand;
		if (value._bool())
			size_mode_y = SizeMode::Fill;
	} else if (key == "noexpandx" or key == "fillx" or key == "fill.x") {
		size_mode_x = SizeMode::Fill;
	} else if (key == "noexpandy" or key == "filly" or key == "fill.y") {
		size_mode_y = SizeMode::Fill;
	} else if (key == "fill") {
		size_mode_x = SizeMode::Fill;
		size_mode_y = SizeMode::Fill;
	} else if (key == "shrink") {
		size_mode_x = SizeMode::Shrink;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "shrinkx" or key == "shrink.x") {
		size_mode_x = SizeMode::Shrink;
	} else if (key == "shrinky" or key == "shrink.y") {
		size_mode_y = SizeMode::Shrink;
	} else if (key == "forwardchild") {
		size_mode_x = SizeMode::ForwardChild;
		size_mode_y = SizeMode::ForwardChild;
	} else if (key == "forwardchildx") {
		size_mode_x = SizeMode::ForwardChild;
	} else if (key == "forwardchildy") {
		size_mode_y = SizeMode::ForwardChild;
	} else if (key == "width") {
		min_width_user = value._float();
		size_mode_x = SizeMode::Shrink;
	} else if (key == "height") {
		min_height_user = value._float();
		size_mode_y = SizeMode::Shrink;
	} else if (key == "greedfactorx" or key == "greedx" or key == "greed.x") {
		greed_factor.x = value._float();
	} else if (key == "greedfactory" or key == "greedy" or key == "greed.y") {
		greed_factor.y = value._float();
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
	} else if (key == "paddingx" or key == "paddingh" or key == "padding.x") {
		float f = value._float();
		padding.x1 = padding.x2 = f;
	} else if (key == "paddingy" or key == "paddingv" or key == "padding.y") {
		float f = value._float();
		padding.y1 = padding.y2 = f;
	} else if (key == "paddingtop" or key == "padding.top") {
		padding.y1 = value._float();
	} else if (key == "paddingbottom" or key == "padding.bottom") {
		padding.y2 = value._float();
	} else if (key == "paddingleft" or key == "padding.left") {
		padding.x1 = value._float();
	} else if (key == "paddingright" or key == "padding.right") {
		padding.x2 = value._float();
	} else if (key == "margin") {
		float f = value._float();
		margin = rect(f, f, f, f);
	} else if (key == "hmargin" or key == "marginh" or key == "marginx" or key == "margin.x") {
		margin.x1 = margin.x2 = value._float();
	} else if (key == "vmargin" or key == "marginv" or key == "marginy" or key == "margin.y") {
		margin.y1 = margin.y2 = value._float();
	} else if (key == "margintop" or key == "margin.top") {
		margin.y1 = value._float();
	} else if (key == "marginbottom" or key == "margin.bottom") {
		margin.y2 = value._float();
	} else if (key == "marginleft" or key == "margin.left") {
		margin.x1 = value._float();
	} else if (key == "marginright" or key == "margin.right") {
		margin.x2 = value._float();
	} else if (key == "hidden") {
		visible = false;
	} else if (key == "visible") {
		visible = value._bool() or value == "";
	}
}

Node* get_hover(Node* top, const vec2& p) {
	Array<const Node*> seeds = {top};
	int cur_seed = 0;

	// we might need multiple seeds, if we encounter Overlays!

	const Node* best = nullptr;
	while (cur_seed < seeds.num) {
		auto c = seeds[cur_seed ++];
		while (c) {
			if (c->area.inside(p) and !c->ignore_hover and c->visible)
				best = c;
			const Node* next = nullptr;
			for (auto cc: c->_get_children(ChildFilter::OnlyActive))
				if (cc->area.inside(p) and cc->visible) {
					if (next)
						seeds.add(cc);
					else
						next = cc;
				}
			c = next;
		}
	}
	return const_cast<Node*>(best);
}

}
