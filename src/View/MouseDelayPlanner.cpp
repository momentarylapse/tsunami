/*
 * MouseDelayPlanner.cpp
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#include "MouseDelayPlanner.h"
#include "../lib/hui/hui.h"
#include "AudioView.h"
#include <cmath>

MouseDelayPlanner::MouseDelayPlanner(AudioView *_view) {
	view = _view;
	min_move_to_start = hui::Config.get_int("View.MouseMinMoveToSelect", 5);
	hui::Config.set_int("View.MouseMinMoveToSelect", min_move_to_start);
}

void MouseDelayPlanner::prepare(hui::Callback _a, hui::Callback _b, hui::Callback _c) {
	dist = 0;
	pos0 = view->cam.screen2sample(view->mx);
	x0 = view->mx;
	y0 = view->my;
	cb_start = _a;
	cb_update = _b;
	cb_end = _c;
}

bool MouseDelayPlanner::update() {
	if (acting()) {
		cb_update();
	} else if (has_focus()) {
		auto e = hui::GetEvent();
		dist += fabs(e->dx) + fabs(e->dy);
		if (acting())
			cb_start();
	}
	return has_focus();
}

bool MouseDelayPlanner::acting() {
	return dist > min_move_to_start;
}

bool MouseDelayPlanner::has_focus() {
	return dist >= 0;
}

void MouseDelayPlanner::stop() {
	if (acting())
		cb_end();
	dist = -1;
}

