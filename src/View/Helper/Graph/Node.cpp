/*
 * Node.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "Node.h"
#include "SceneGraph.h"

#include "../../../lib/file/msg.h"

namespace scenegraph {

Node::Node() : Node(0, 0) {
}

Node::Node(float w, float h) {
	//msg_write("new Node " + p2s(this));
	align.horizontal = AlignData::Mode::NONE;
	align.vertical = AlignData::Mode::NONE;
	align.dx = 0;
	align.dy = 0;
	align.w = w;
	align.h = h;
	align.dz = 1;
	area = rect(0, w, 0, h);
	parent = nullptr;
	area = rect::EMPTY;
	hidden = false;
	z = 0;
}

bool Node::hover(float mx, float my) {
	if (hidden)
		return false;
	return area.inside(mx, my);
}

void Node::add_child(Node* child) {
	children.add(child);
	child->parent = this;
}

void Node::delete_child(Node* child) {
	for (int i=0; i<children.num; i++)
		if (children[i] == child)
			children.erase(i);
}

SceneGraph *Node::graph() {
	Node *r = this;
	while (r->parent)
		r = r->parent;
	return dynamic_cast<SceneGraph*>(r);
}

bool Node::is_cur_hover() {
	for (auto *c: weak(children))
		if (c->is_cur_hover())
			return true;
	return is_cur_hover_non_recursive();
}

bool Node::is_cur_hover_non_recursive() {
	if (auto *sg = graph()) {
		return sg->hover.node == this;
	}
	return false;
}

HoverData Node::get_hover_data(float mx, float my) {
	HoverData h;
	h.node = this;
	return h;
}

string Node::get_tip() {
	return "";
}

void Node::update_geometry(const rect &target_area) {
	if (align.horizontal == AlignData::Mode::FILL) {
		area.x1 = target_area.x1;
		area.x2 = target_area.x2;
	} else if (align.horizontal == AlignData::Mode::LEFT) {
		area.x1 = target_area.x1 + align.dx;
		area.x2 = area.x1 + align.w;
	} else if (align.horizontal == AlignData::Mode::RIGHT) {
		area.x2 = target_area.x2 + align.dx;
		area.x1 = area.x2 - align.w;
	}

	if (align.vertical == AlignData::Mode::FILL) {
		area.y1 = target_area.y1;
		area.y2 = target_area.y2;
	} else if (align.vertical == AlignData::Mode::TOP) {
		area.y1 = target_area.y1 + align.dy;
		area.y2 = area.y1 + align.h;
	} else if (align.vertical == AlignData::Mode::BOTTOM) {
		area.y2 = target_area.y2 + align.dy;
		area.y1 = area.y2 - align.h;
	}
}

void Node::set_hidden(bool hide) {
	if (hide != hidden) {
		hidden = hide;
		request_redraw();
	}
}

void Node::update_geometry_recursive(const rect &target_area) {
	update_geometry(target_area);
	if (parent)
		z = parent->z + align.dz;

	for (auto *c: weak(children))
		c->update_geometry_recursive(area);
}


Array<Node*> Node::collect_children(bool include_hidden) {
	Array<Node*> nodes;
	for (auto *c: weak(children))
		if (!c->hidden or include_hidden) {
			nodes.add(c);
			nodes.append(c->collect_children(include_hidden));
		}
	return nodes;
}

Array<Node*> Node::collect_children_up() {
	auto nodes = collect_children(false);
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z > nodes[j]->z)
				nodes.swap(i, j);
	return nodes;
}

Array<Node*> Node::collect_children_down() {
	auto nodes = collect_children(false);
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z < nodes[j]->z)
				nodes.swap(i, j);
	return nodes;
}

void Node::request_redraw() {
	if (auto g = graph())
		if (g->cb_redraw)
			g->cb_redraw();
}






NodeFree::NodeFree() : Node(0, 0) {
}

NodeRel::NodeRel(float dx, float dy, float w, float h) : Node(w, h) {
	align.horizontal = AlignData::Mode::LEFT;
	align.dx = dx;
	align.vertical = AlignData::Mode::TOP;
	align.dy = dy;
}



HBox::HBox() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
}
void HBox::update_geometry_recursive(const rect &target_area) {
	update_geometry(target_area);
	if (parent)
		z = parent->z + align.dz;
	float w_avail = area.width();
	float w_min = 0;
	int n_expand = 0;
	for (auto *c: weak(children)) {
		if (c->hidden)
			continue;
		if (c->align.horizontal == AlignData::Mode::FILL)
			n_expand ++;
		else
			w_min += c->align.w;
	}
	float offset = 0;
	for (auto *c: weak(children)) {
		float w = c->align.w;
		if (c->align.horizontal == AlignData::Mode::FILL)
			w = (w_avail - w_min) / n_expand;
		if (c->hidden)
			w = 0;
		c->update_geometry_recursive(rect(area.x1 + offset, area.x1 + offset + w, area.y1, area.y2));
		offset += w;
	}
}

VBox::VBox() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
}

void VBox::update_geometry_recursive(const rect &target_area) {
	update_geometry(target_area);
	if (parent)
		z = parent->z + align.dz;
	float h_avail = area.height();
	float h_min = 0;
	int n_expand = 0;
	for (auto *c: weak(children)) {
		if (c->hidden)
			continue;
		if (c->align.vertical == AlignData::Mode::FILL)
			n_expand ++;
		else
			h_min += c->align.h;
	}
	float offset = 0;
	for (auto *c: weak(children)) {
		float h = c->align.h;
		if (c->align.vertical == AlignData::Mode::FILL)
			h = (h_avail - h_min) / n_expand;
		if (c->hidden)
			h = 0;
		c->update_geometry_recursive(rect(area.x1, area.x2, area.y1 + offset, area.y1 + offset + h));
		offset += h;
	}
}

}
