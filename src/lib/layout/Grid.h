#pragma once

#include "Node.h"

namespace layout {

struct GridChild {
	shared<Node> node;
	int x, y;
};

struct Grid {
	explicit Grid(Node& node);

	Node& node;
	Array<GridChild> children;
	int nx = 0, ny = 0;
	float spacing = 0;
	bool vertical = false;

	void add_child(shared<Node> child, int x, int y);
	void remove_child(Node* child);

	void get_min_sizes(Array<float> &w, Array<float> &h) const;
	void get_greed_factors(Array<float> &x, Array<float> &y) const;

	vec2 get_greed_factor() const;
	vec2 get_content_min_size() const;
	void negotiate_content_area(const rect& available);

	Array<Node*> get_children(ChildFilter f) const;

	void set_option(const string& key, const string& value);
};
}
