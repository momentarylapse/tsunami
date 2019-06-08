/*
 * SceneGraph.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "SceneGraph.h"
#include "../Selection.h"



Array<ViewNode*> collect_children(ViewNode *n) {
	Array<ViewNode*> nodes;
	for (auto *c: n->children)
		if (!c->hidden) {
			nodes.add(c);
			nodes.append(collect_children(c));
		}
	return nodes;
}

Array<ViewNode*> collect_children_up(ViewNode *n) {
	auto nodes = collect_children(n);
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z > nodes[j]->z)
				nodes.swap(i, j);
	return nodes;
}

Array<ViewNode*> collect_children_down(ViewNode *n) {
	auto nodes = collect_children(n);
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z < nodes[j]->z)
				nodes.swap(i, j);
	return nodes;
}


SceneGraph::SceneGraph(AudioView *view) : ViewNode(view) {}

void SceneGraph::on_left_button_down() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover()) {
			c->on_left_button_down();
			if (!mouse_owner)
				mouse_owner = c;
		}
}

void SceneGraph::on_left_button_up() {
	if (mouse_owner) {
		mouse_owner->on_left_button_up();
	} else {
		auto nodes = collect_children_down(this);
		for (auto *c: nodes)
			if (c->hover())
				c->on_left_button_up();
	}
	mouse_owner = nullptr;
}

void SceneGraph::on_right_button_down() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			c->on_right_button_down();
}

void SceneGraph::on_mouse_move() {
	if (mouse_owner) {
		mouse_owner->on_mouse_move();
	} else {
		auto nodes = collect_children_down(this);
		for (auto *c: nodes)
			if (c->hover())
				c->on_mouse_move();
	}
}

Selection SceneGraph::get_hover() {
	auto nodes = collect_children_down(this);

	for (auto *c: nodes)
		if (c->hover())
			return c->get_hover();

	return Selection();
}

string SceneGraph::get_tip() {
	auto s = get_hover();
	if (s.node)
		return s.node->get_tip();
	return "";
}

void SceneGraph::draw(Painter *p) {
	if (hidden)
		return;

	update_area();
	/*draw(p);

	// recursion
	for (auto *c: children)
		c->draw(p);*/
	auto nodes = collect_children_up(this);
	for (auto *n: nodes) {
		n->update_area();
		n->draw(p);
	}
}

