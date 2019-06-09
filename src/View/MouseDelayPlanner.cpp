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

void MouseDelayPlanner::prepare(MouseDelayAction *a) {
	if (action)
		cancel();
	dist = 0;
	pos0 = view->cam.screen2sample(view->mx);
	x0 = view->mx;
	y0 = view->my;
	action = a;
}

bool MouseDelayPlanner::update() {
	if (acting()) {
		action->on_update();
	} else if (has_focus()) {
		auto e = hui::GetEvent();
		dist += fabs(e->dx) + fabs(e->dy);
		if (acting())
			action->on_start();
	}
	return has_focus();
}

bool MouseDelayPlanner::acting() {
	return dist > min_move_to_start;
}

bool MouseDelayPlanner::has_focus() {
	return dist >= 0;
}

void MouseDelayPlanner::finish() {
	if (acting()) {
		action->on_finish();
		action->on_clean_up();
		delete action;
		action = nullptr;
	}
	dist = -1;
}

void MouseDelayPlanner::cancel() {
	if (acting()) {
		action->on_cancel();
		action->on_clean_up();
		delete action;
		action = nullptr;
	}
	dist = -1;
}

void MouseDelayPlanner::draw_post(Painter *p) {
	if (acting())
		action->on_draw_post(p);
}

