/*
 * SceneGraph.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "SceneGraph.h"

#include "../HoverData.h"



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

bool SceneGraph::on_left_button_down() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover()) {
			if (c->on_left_button_down()) {
				mouse_owner = c;
				return true;
			}
		}
	return false;
}

bool SceneGraph::on_left_button_up() {
	if (mouse_owner) {
		bool r = mouse_owner->on_left_button_up();
		mouse_owner = nullptr;
		return r;
	} else {
		auto nodes = collect_children_down(this);
		for (auto *c: nodes)
			if (c->hover())
				if (c->on_left_button_up())
					return true;
	}
	return false;
}

bool SceneGraph::on_right_button_down() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_right_button_down())
				return true;
	return false;
}

bool SceneGraph::on_mouse_move() {
	if (mouse_owner) {
		return mouse_owner->on_mouse_move();
	} else {
		auto nodes = collect_children_down(this);
		for (auto *c: nodes)
			if (c->hover())
				if (c->on_mouse_move())
					return true;
	}
	return false;
}

ViewNode *SceneGraph::get_hover() {
	auto nodes = collect_children_down(this);

	for (auto *c: nodes)
		if (c->hover())
			return c;

	return nullptr;
}

string SceneGraph::get_tip() {
	auto nodes = collect_children_down(this);

	for (auto *c: nodes)
		if (c->hover())
			return c->get_tip();
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

