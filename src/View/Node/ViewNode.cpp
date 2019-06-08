/*
 * ViewNode.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "ViewNode.h"
#include "../AudioView.h"
#include "../Selection.h"

ViewNode::ViewNode(AudioView *_view) : ViewNode(nullptr, 0, 0, 0, 0) {
	view = _view;
}

ViewNode::ViewNode(ViewNode *_parent, float dx, float dy, float w, float h) {
	view = nullptr;
	node_offset_x = dx;
	node_offset_y = dy;
	node_width = w;
	node_height = h;
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

bool ViewNode::hover() {
	if (hidden)
		return false;
	return area.inside(view->mx, view->my);
}

Selection ViewNode::get_hover() {
	if (!hover())
		return Selection();
	Selection s;
	s.type = Selection::Type::SOME_NODE;
	s.node = this;
	return s;
}

string ViewNode::get_tip() {
	return "";
}

void ViewNode::update_area() {
	if (parent) {
		float x = parent->area.x1 + node_offset_x;
		float y = parent->area.y1 + node_offset_y;
		area = rect(x, x + node_width, y, y + node_height);
	}
}

