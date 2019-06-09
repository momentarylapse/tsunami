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



void SceneGraph::MouseDelayPlanner::prepare(hui::Callback _a, hui::Callback _b, hui::Callback _c) {
	dist = 0;
	pos0 = view->cam.screen2sample(view->mx);
	x0 = view->mx;
	y0 = view->my;
	cb_start = _a;
	cb_update = _b;
	cb_end = _c;
}

void SceneGraph::MouseDelayPlanner::update() {
	if (active()) {
		cb_update();
	} else if (dist >= 0) {
		auto e = hui::GetEvent();
		dist += fabs(e->dx) + fabs(e->dy);
	}
}

bool SceneGraph::MouseDelayPlanner::active() {
	return dist > min_move_to_start;
}

void SceneGraph::MouseDelayPlanner::stop() {
	if (active())
		cb_end();
	dist = -1;
}

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


SceneGraph::SceneGraph(AudioView *view) : ViewNode(view) {
	mdp.view = view;
	mdp.min_move_to_start = hui::Config.get_int("View.MouseMinMoveToSelect", 5);
	hui::Config.set_int("View.MouseMinMoveToSelect", mdp.min_move_to_start);
}

bool SceneGraph::on_left_button_down() {
	hover_node = get_hover();
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_left_button_down())
				return true;
	return false;
}

bool SceneGraph::on_left_button_up() {
	mdp.stop();
	hover_node = get_hover();
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_left_button_up())
				return true;
	return false;
}

bool SceneGraph::on_right_button_down() {
	hover_node = get_hover();
	auto nodes = collect_children_down(this);
	for (auto *c: nodes)
		if (c->hover())
			if (c->on_right_button_down())
				return true;
	return false;
}

bool SceneGraph::on_mouse_move() {
	mdp.update();
	if (mdp.active()) {
		return true;
	} else {
		hover_node = get_hover();
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
	/*auto nodes = collect_children_down(this);

	for (auto *c: nodes)
		if (c->hover())
			return c->get_tip();*/
	if (hover_node)
		return hover_node->get_tip();
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

