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
	node_align_right = false;
	node_align_bottom = false;
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

string ViewNode::get_tip() {
	return "";
}

void ViewNode::update_area() {
	if (parent) {
		float x = parent->area.x1 + node_offset_x;
		if (node_align_right)
			x = parent->area.x2 - node_width - node_offset_x;
		float y = parent->area.y1 + node_offset_y;
		if (node_align_bottom)
			y = parent->area.y2 - node_height - node_offset_y;
		area = rect(x, x + node_width, y, y + node_height);
	}
}

