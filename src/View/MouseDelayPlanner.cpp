/*
 * MouseDelayPlanner.cpp
 *
 *  Created on: 09.06.2019
 *      Author: michi
 */

#include "MouseDelayPlanner.h"
#include "../lib/hui/hui.h"
#include "Helper/Graph/SceneGraph.h"
#include <cmath>

MouseDelayPlanner::MouseDelayPlanner(scenegraph::SceneGraph *sg) {
	scene_graph = sg;
	min_move_to_start = hui::Config.get_int("View.MouseMinMoveToSelect", 5);
	hui::Config.set_int("View.MouseMinMoveToSelect", min_move_to_start);
}

void MouseDelayPlanner::prepare(MouseDelayAction *a) {
	if (action)
		cancel();
	dist = 0;
	x0 = scene_graph->mx;
	y0 = scene_graph->my;
	action = a;
	action->scene_graph = scene_graph;
}

bool MouseDelayPlanner::update(float mx, float my) {
	if (acting()) {
		action->on_update(mx, my);
		//view->force_redraw();
	} else if (has_focus()) {
		auto e = hui::GetEvent();
		dist += fabs(e->dx) + fabs(e->dy);
		if (dist > min_move_to_start)
			start_acting(mx, my);
	}
	return has_focus();
}

bool MouseDelayPlanner::acting() {
	return _started_acting;
}

void MouseDelayPlanner::start_acting(float mx, float my) {
	_started_acting = true;
	action->on_start(mx, my);
}

bool MouseDelayPlanner::has_focus() {
	return dist >= 0;
}

void MouseDelayPlanner::finish(float mx, float my) {
	if (acting()) {
		action->on_finish(mx, my);
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



