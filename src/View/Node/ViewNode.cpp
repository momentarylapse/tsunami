/*
 * ViewNode.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "ViewNode.h"
#include "SceneGraph.h"

ViewNode::ViewNode() : ViewNode(0, 0) {
}

ViewNode::ViewNode(float w, float h) {
	msg_write("new ViewNode " + p2s(this));
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

bool ViewNode::hover(float mx, float my) {
	if (hidden)
		return false;
	return area.inside(mx, my);
}

void ViewNode::add_child(ViewNode* child) {
	children.add(child);
	child->parent = this;
}

void ViewNode::delete_child(ViewNode* child) {
	for (int i=0; i<children.num; i++)
		if (children[i] == child)
			children.erase(i);
}

ViewNode *ViewNode::root() {
	ViewNode *r = this;
	while (r->parent)
		r = r->parent;
	return r;
}

bool ViewNode::is_cur_hover() {
	for (auto *c: weak(children))
		if (c->is_cur_hover())
			return true;
	return is_cur_hover_non_recursive();
}

bool ViewNode::is_cur_hover_non_recursive() {
	if (auto *sg = dynamic_cast<SceneGraph*>(root())) {
		return sg->hover.node == this;
	}
	return false;
}

HoverData ViewNode::get_hover_data(float mx, float my) {
	HoverData h;
	h.node = this;
	return h;
}

string ViewNode::get_tip() {
	return "";
}

void ViewNode::update_geometry(const rect &target_area) {
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

void ViewNode::update_geometry_recursive(const rect &target_area) {
	update_geometry(target_area);
	if (parent)
		z = parent->z + align.dz;

	for (auto *c: weak(children))
		c->update_geometry_recursive(area);
}

ViewNodeFree::ViewNodeFree() : ViewNode(0, 0) {
}

ViewNodeRel::ViewNodeRel(float dx, float dy, float w, float h) : ViewNode(w, h) {
	align.horizontal = AlignData::Mode::LEFT;
	align.dx = dx;
	align.vertical = AlignData::Mode::TOP;
	align.dy = dy;
}



NodeHBox::NodeHBox() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
}
void NodeHBox::update_geometry_recursive(const rect &target_area) {
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

NodeVBox::NodeVBox() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
}

void NodeVBox::update_geometry_recursive(const rect &target_area) {
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

