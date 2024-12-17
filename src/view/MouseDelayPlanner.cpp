/*
 * MouseDelayPlanner.cpp
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#include "MouseDelayPlanner.h"
#include "../lib/hui/config.h"
#include "../lib/hui/Event.h"
#include "helper/graph/SceneGraph.h"
#include <cmath>

namespace scenegraph {

MouseDelayPlanner::MouseDelayPlanner(scenegraph::SceneGraph *sg) {
	scene_graph = sg;
	min_move_to_start = hui::config.get_int("View.MouseMinMoveToSelect", 5);
	hui::config.set_int("View.MouseMinMoveToSelect", min_move_to_start);
}

void MouseDelayPlanner::prepare(MouseDelayAction *a, const vec2 &m) {
	if (action)
		cancel();
	dist = 0;
	x0 = m.x;
	y0 = m.y;
	action = a;
	action->scene_graph = scene_graph;
}

bool MouseDelayPlanner::update(const vec2 &m) {
	if (acting()) {
		action->on_update(m);
		//view->force_redraw();
	} else if (has_focus()) {
		auto e = hui::get_event();
		dist += fabs(e->d.x) + fabs(e->d.y);
		if (dist > min_move_to_start)
			start_acting(m);
	}
	return has_focus();
}

bool MouseDelayPlanner::acting() {
	return _started_acting;
}

void MouseDelayPlanner::start_acting(const vec2 &m) {
	_started_acting = true;
	action->on_start(m);
}

bool MouseDelayPlanner::has_focus() {
	return dist >= 0;
}

void MouseDelayPlanner::finish(const vec2 &m) {
	if (acting()) {
		action->on_finish(m);
		action->on_clean_up();
		action = nullptr;
		_started_acting = false;
		//view->force_redraw();
	}
	dist = -1;
}

void MouseDelayPlanner::cancel() {
	if (acting()) {
		action->on_cancel();
		action->on_clean_up();
		action = nullptr;
		_started_acting = false;
		//view->force_redraw();
	}
	dist = -1;
}

void MouseDelayPlanner::draw_post(Painter *p) {
	if (acting())
		action->on_draw_post(p);
}

}

