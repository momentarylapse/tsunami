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
	void get_greed_factors(Array<float> &x, Array<float> &y, SizeMode mamx, SizeMode mamy) const;

	vec2 get_content_min_size() const;
	void negotiate_content_area(const rect& available);

	Array<Node*> get_children(ChildFilter f) const;

	void set_option(const string& key, const string& value);
};

template<class X>
Array<Node*> weak_nodes(const Array<X>& children) {
	return weak(children).template sub_ref_as<Array<Node*>>(0);
}

vec2 hbox_get_content_min_size(const Array<Node*>& children, float spacing);
void hbox_negotiate_content_area(const Node* self, const rect& available, const Array<Node*>& children, float spacing);

vec2 vbox_get_content_min_size(const Array<Node*>& children, float spacing);
void vbox_negotiate_content_area(const Node* self, const rect& available, const Array<Node*>& children, float spacing);

}
