//
// Created by michi on 3/24/26.
//

#include "Grid.h"
#include <lib/base/algo.h>

namespace layout {

int sum(const Array<int> &a) {
	int r = 0;
	for (int i=0; i<a.num; i++)
		r += a[i];
	return r;
}

float sum(const Array<float> &a) {
	float r = 0;
	for (int i=0; i<a.num; i++)
		r += a[i];
	return r;
}

Grid::Grid(Node& _node) : node(_node) {
}

void Grid::add_child(shared<Node> c, int x, int y) {
	if (vertical)
		std::swap(x, y);
	children.add({c.get(), x, y});
	nx = max(nx, x+1);
	ny = max(ny, y+1);
}

void Grid::remove_child(Node* c) {
	base::remove_if(children, [c] (const layout::GridChild& child) {
		return child.node == c;
	});
}

void Grid::get_min_sizes(Array<float> &w, Array<float> &h) const {
	w.resize(nx);
	h.resize(ny);
	for (auto &c: children) {
		if (!c.node->visible)
			continue;
		vec2 s = c.node->get_effective_min_size();
		w[c.x] = max(w[c.x], s.x);
		h[c.y] = max(h[c.y], s.y);
	}
}

vec2 Grid::get_content_min_size() const {
	Array<float> w, h;
	get_min_sizes(w, h);
	vec2 s;
	s.x = sum(w) + spacing * (float)(w.num - 1);
	s.y = sum(h) + spacing * (float)(h.num - 1);
	return s;
}

vec2 Grid::get_greed_factor() const {
	Array<float> xx, yy;
	get_greed_factors(xx, yy);
	vec2 f = {0, 0};
	if (node.size_mode_x == SizeMode::Expand)
		f.x = node.greed_factor.x;
	else if (node.size_mode_x == SizeMode::ForwardChild)
		f.x = sum(xx);
	if (node.size_mode_y == SizeMode::Expand)
		f.y = node.greed_factor.y;
	else if (node.size_mode_y == SizeMode::ForwardChild)
		f.y = sum(yy);
	return f;
}

void Grid::get_greed_factors(Array<float> &x, Array<float> &y) const {
	x.resize(nx);
	y.resize(ny);
	for (auto& c: children)
		if (c.node->visible) {
			vec2 f = c.node->get_greed_factor();
			x[c.x] = max(x[c.x], f.x);
			y[c.y] = max(y[c.y], f.y);
		}
}

void Grid::negotiate_content_area(const rect &available) {
	Array<float> w, h;
	get_min_sizes(w, h);
	vec2 total_min_size = get_content_min_size();
	float diff_x = max(available.width() - total_min_size.x, 0.0f); //  - spacing * (w.num + 1)
	float diff_y = max(available.height() - total_min_size.y, 0.0f); //  - spacing * (h.num + 1)

	Array<float> gx, gy;
	get_greed_factors(gx, gy);
	vec2 total_greed = get_greed_factor();

	float greed_to_x = (total_greed.x > 0) ? diff_x / total_greed.x : 0;
	float greed_to_y = (total_greed.y > 0) ? diff_y / total_greed.y : 0;

	for (int i=0; i<w.num; i++)
		w[i] += greed_to_x * gx[i];
	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];

	for (auto &c: children)
		if (c.node->visible) {
			float x0 = available.x1;
			float y0 = available.y1;
			for (int i=0; i<c.x; i++)
				x0 += w[i] + spacing;
			for (int i=0; i<c.y; i++)
				y0 += h[i] + spacing;
			c.node->negotiate_outer_area(rect(x0, x0 + w[c.x], y0, y0 + h[c.y]));
		}
}

Array<Node*> Grid::get_children(ChildFilter f) const {
	Array<Node*> r;
	for (auto& c: children)
		if (f == ChildFilter::All or c.node->visible)
			r.add(c.node.get());
	return r;
}

void Grid::set_option(const string& key, const string& value) {
	if (key == "spacing") {
		spacing = value._float();
	} else if (key == "vertical") {
		vertical = true;
	}
}
} // layout