/*
 * Node.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "Node.h"
#include "SceneGraph.h"
#include "../../../stuff/PerformanceMonitor.h"
#include "../../../lib/image/Painter.h"
#include "../../../lib/image/color.h"

namespace scenegraph {

bool Node::show_debug = false;


void sort_nodes_up(Array<Node*> &nodes) {
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z > nodes[j]->z)
				nodes.swap(i, j);
}

void sort_nodes_down(Array<Node*> &nodes) {
	for (int i=0; i<nodes.num; i++)
		for (int j=i+1; j<nodes.num; j++)
			if (nodes[i]->z < nodes[j]->z)
				nodes.swap(i, j);
}


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
	clip = false;
	z = 0;
	perf_channel = PerformanceMonitor::create_channel("node", this);
}

Node::~Node() {
	PerformanceMonitor::delete_channel(perf_channel);
}

bool Node::hover(const vec2 &m) const {
	if (hidden)
		return false;
	return area.inside(m);
}

void Node::add_child(Node* child) {
	children.add(child);
	child->parent = this;
	PerformanceMonitor::set_parent(child->perf_channel, perf_channel);
}

void Node::delete_child(Node* child) {
	for (int i=0; i<children.num; i++)
		if (children[i] == child)
			children.erase(i);
}

SceneGraph *Node::graph() const {
	Node *r = const_cast<Node*>(this);
	while (r->parent)
		r = r->parent;
	return dynamic_cast<SceneGraph*>(r);
}

bool Node::is_cur_hover() const {
	for (auto *c: weak(children))
		if (c->is_cur_hover())
			return true;
	return is_cur_hover_non_recursive();
}

bool Node::is_cur_hover_non_recursive() const {
	if (auto *sg = graph()) {
		return sg->hover.node == this;
	}
	return false;
}

HoverData Node::get_hover_data(const vec2 &m) {
	HoverData h;
	h.node = this;
	return h;
}

string Node::get_tip() const {
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

void Node::draw_recursive(Painter *p) {
	if (hidden)
		return;
	PerformanceMonitor::start_busy(perf_channel);

	rect clip_before = p->clip();
	if (clip)
		p->set_clip(area);

	on_draw(p);

	if (show_debug) {
		p->set_font_size(8);
		p->set_color(is_cur_hover_non_recursive() ? Red : Green);
		p->draw_str({area.x1, area.y1}, PerformanceMonitor::get_name(perf_channel));
		p->set_fill(false);
		p->draw_rect(area);
		p->set_fill(true);
	}

	auto nodes = weak(children);
	sort_nodes_up(nodes);
	for (auto *c: nodes)
		if (!c->hidden)
			c->draw_recursive(p);

	if (clip)
		p->set_clip(clip_before);
	PerformanceMonitor::end_busy(perf_channel);
}

void Node::set_hidden(bool hide) {
	if (hide != hidden) {
		hidden = hide;
		request_redraw();
	}
}

void Node::set_perf_name(const string &name) {
	PerformanceMonitor::set_name(perf_channel, name);
}

void Node::update_geometry_recursive(const rect &target_area) {
	update_geometry(target_area);
	if (parent)
		z = parent->z + align.dz;

	for (auto *c: weak(children))
		c->update_geometry_recursive(area);
}


Array<Node*> Node::collect_children(bool include_hidden) const {
	Array<Node*> nodes;
	for (auto *c: weak(children))
		if (!c->hidden or include_hidden) {
			nodes.add(c);
			nodes.append(c->collect_children(include_hidden));
		}
	return nodes;
}

Array<Node*> Node::collect_children_up() const {
	auto nodes = collect_children(false);
	sort_nodes_up(nodes);
	return nodes;
}

Array<Node*> Node::collect_children_down() const {
	auto nodes = collect_children(false);
	sort_nodes_down(nodes);
	return nodes;
}

void Node::request_redraw() {
	if (auto g = graph())
		g->out_redraw.notify();
}






NodeFree::NodeFree() : Node(0, 0) {
}

NodeRel::NodeRel(const vec2 &d, float w, float h) : Node(w, h) {
	align.horizontal = AlignData::Mode::LEFT;
	align.dx = d.x;
	align.vertical = AlignData::Mode::TOP;
	align.dy = d.y;
}



HBox::HBox() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	set_perf_name("hbox");
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
	set_perf_name("vbox");
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
