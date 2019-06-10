/*
 * SceneGraph.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "SceneGraph.h"

#include "../HoverData.h"
#include "../AudioView.h"
#include "../ViewPort.h"




Array<ViewNode*> collect_children(ViewNode *n, bool include_hidden) {
	Array<ViewNode*> nodes;
	for (auto *c: n->children)
		if (!c->hidden or include_hidden) {
			nodes.add(c);
			nodes.append(collect_children(c, include_hidden));
		}
	return nodes;
}

Array<ViewNode*> collect_children_up(ViewNode *n) {
	auto nodes = collect_children(n, false);
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z > nodes[j]->z)
				nodes.swap(i, j);
	return nodes;
}

Array<ViewNode*> collect_children_down(ViewNode *n) {
	auto nodes = collect_children(n, false);
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
		if (c->hover())
			if (c->on_left_button_down())
				return true;
	return false;
}

bool SceneGraph::on_left_button_up() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_left_button_up())
				return true;
	return false;
}

bool SceneGraph::on_left_double_click() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_left_double_click())
				return true;
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
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_mouse_move())
				return true;
	return false;
}

bool SceneGraph::allow_handle_click_when_gaining_focus() {
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			return c->allow_handle_click_when_gaining_focus();
	return false;
}

HoverData SceneGraph::get_hover_data() {
	auto nodes = collect_children_down(this);

	for (auto *c: nodes)
		if (c->hover())
			return c->get_hover_data();

	return HoverData();
}

string SceneGraph::get_tip() {
	auto nodes = collect_children_down(this);

	for (auto *c: nodes)
		if (c->hover())
			return c->get_tip();
	return "";
}

void SceneGraph::update_area() {
	auto nodes = collect_children(this, true);
	for (auto *n: nodes)
		n->update_area();
}

void SceneGraph::draw(Painter *p) {
	update_area();

	auto nodes = collect_children_up(this);
	for (auto *n: nodes)
		n->draw(p);
}

