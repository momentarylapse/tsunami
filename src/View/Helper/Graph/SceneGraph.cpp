/*
 * SceneGraph.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "SceneGraph.h"

#include "../../HoverData.h"
#include "../../AudioView.h"
#include "../../ViewPort.h"
#include "../../MouseDelayPlanner.h"


namespace scenegraph {

SceneGraph::SceneGraph() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	mx = -1;
	my = -1;
	mdp = new MouseDelayPlanner(this);
	set_perf_name("graph");
}

void SceneGraph::set_callback_set_current(hui::Callback f) {
	cb_set_current = f;
}

void SceneGraph::set_callback_redraw(hui::Callback f) {
	cb_redraw = f;
}

bool SceneGraph::on_left_button_down(float mx, float my) {
	set_mouse(mx, my);
	hover = get_hover_data(mx, my);

	if (hui::GetEvent()->just_focused)
		if (!allow_handle_click_when_gaining_focus())
			return true;

	set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			if (c->on_left_button_down(mx, my))
				return true;
	return false;
}

bool SceneGraph::on_left_button_up(float mx, float my) {
	set_mouse(mx, my);
	mdp->finish(mx, my);
	hover = get_hover_data(mx, my);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			if (c->on_left_button_up(mx, my))
				return true;
	return false;
}

bool SceneGraph::on_left_double_click(float mx, float my) {
	set_mouse(mx, my);
	hover = get_hover_data(mx, my);
	set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			if (c->on_left_double_click(mx, my))
				return true;
	return false;
}

bool SceneGraph::on_right_button_down(float mx, float my) {
	set_mouse(mx, my);
	hover = get_hover_data(mx, my);
	set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			if (c->on_right_button_down(mx, my))
				return true;
	return false;
}

bool SceneGraph::on_mouse_move(float mx, float my) {
	set_mouse(mx, my);

	if (!mdp->update(mx, my)) {
		hover = get_hover_data(mx, my);

		auto nodes = collect_children_down();
		for (auto *c: nodes)
			if (c->hover(mx, my))
				if (c->on_mouse_move(mx, my))
					return true;
	}
	return false;
}

bool SceneGraph::on_mouse_wheel(float dx, float dy) {
	//set_mouse(mx, my);
	//hover = get_hover_data(mx, my);
	//set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			if (c->on_mouse_wheel(dx, dy))
				return true;
	return false;
}

bool SceneGraph::allow_handle_click_when_gaining_focus() {
	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			return c->allow_handle_click_when_gaining_focus();
	return false;
}

HoverData SceneGraph::get_hover_data(float mx, float my) {
	auto nodes = collect_children_down();

	for (auto *c: nodes)
		if (c->hover(mx, my))
			return c->get_hover_data(mx, my);

	return HoverData();
}

string SceneGraph::get_tip() {
	auto nodes = collect_children_down();

	for (auto *c: nodes)
		if (c->hover(mx, my))
			return c->get_tip();
	return "";
}

void SceneGraph::on_draw(Painter *p) {
	//auto xxx = p->clip();
	//p->set_clip(area);

	/*auto nodes = collect_children_up();
	for (auto *n: nodes) {
		//p->set_clip(area and n->area);
		n->on_draw(p);
	}
	p->set_clip(xxx);*/

	//if (mdp->acting())
	//	mdp->action->on_draw_post(p);
}

void SceneGraph::draw(Painter *p) {
	auto xxx = p->clip();
	p->set_clip(area);

	draw_recursive(p);
	p->set_clip(xxx);

	if (mdp->acting())
		mdp->action->on_draw_post(p);
}

void SceneGraph::set_mouse(float _mx, float _my) {
	mx = _mx;
	my = _my;
	//select_xor = win->get_key(hui::KEY_CONTROL);
}

void SceneGraph::set_current(const HoverData &h) {
	cur_selection = h;
	if (cb_set_current)
		cb_set_current();
}

void SceneGraph::mdp_prepare(MouseDelayAction *a) {
	mdp->prepare(a);
}

void SceneGraph::mdp_run(MouseDelayAction *a, float mx, float my) {
	mdp->prepare(a);
	mdp->start_acting(mx, my);
}

class MouseDelayActionWrapper : public MouseDelayAction {
public:
	hui::Callback callback;
	MouseDelayActionWrapper(hui::Callback c) { callback = c; }
	void on_update(float mx, float my) override { callback(); }
};

void SceneGraph::mdp_prepare(hui::Callback update) {
	mdp->prepare(new MouseDelayActionWrapper(update));
}

}
