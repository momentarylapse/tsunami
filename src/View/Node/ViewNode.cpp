/*
 * ViewNode.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "ViewNode.h"
#include "../AudioView.h"

ViewNode::ViewNode(AudioView *_view) : ViewNode(nullptr, 0, 0, 0, 0) {
	view = _view;
}

ViewNode::ViewNode(ViewNode *_parent, float dx, float dy, float w, float h) {
	view = nullptr;
	align.fit_h = align.fit_w = false;
	align.right = false;
	align.bottom = false;
	align.dx = dx;
	align.dy = dy;
	align.w = w;
	align.h = h;
	parent = _parent;
	area = rect::EMPTY;
	hidden = false;
	z = 0;
	if (parent) {
		view = parent->view;
		z = parent->z + 1;
	}
}

ViewNode::~ViewNode() {
	for (auto c: children)
		delete c;
}

bool ViewNode::hover(float mx, float my) {
	if (hidden)
		return false;
	return area.inside(mx, my);
}

bool ViewNode::view_hover(const HoverData &h) {
	for (auto *c: children)
		if (c->view_hover(h))
			return true;
	return h.node == this;
}

bool ViewNode::view_hover_non_recursive(const HoverData &h) {
	return h.node == this;
}

HoverData ViewNode::get_hover_data(float mx, float my) {
	HoverData h;
	h.node = this;
	return h;
}

string ViewNode::get_tip() {
	return "";
}

void ViewNode::update_area() {
	if (parent) {
		float w = align.w;
		if (align.fit_w)
			w = parent->area.width();
		float h = align.h;
		if (align.fit_h)
			h = parent->area.height();
		float x = parent->area.x1 + align.dx;
		if (align.right)
			x = parent->area.x2 - w - align.dx;
		float y = parent->area.y1 + align.dy;
		if (align.bottom)
			y = parent->area.y2 - h - align.dy;
		area = rect(x, x + w, y, y + h);
	}
}

