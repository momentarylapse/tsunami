/*
 * SceneGraph.cpp
 *
 *  Created on: 08.06.2019
 *      Author: michi
 */

#include "SceneGraph.h"
#include "../../HoverData.h"
#include "../../MouseDelayPlanner.h"
#include "../../../lib/hui/hui.h"


namespace scenegraph {

SceneGraph::SceneGraph() {
	align.horizontal = AlignData::Mode::FILL;
	align.vertical = AlignData::Mode::FILL;
	m = {-1, -1};
	mdp = new MouseDelayPlanner(this);
	set_perf_name("graph");

	show_debug = hui::config.get_bool("scene-graph.debug", false);
}

void SceneGraph::set_callback_set_current(hui::Callback f) {
	cb_set_current = f;
}

void SceneGraph::set_callback_redraw(hui::Callback f) {
	cb_redraw = f;
}

void SceneGraph::update_hover() {
	hover = get_hover_data(m);
}

bool SceneGraph::on_left_button_down(const vec2 &m) {
	set_mouse(m);
	update_hover();

	if (hui::get_event()->just_focused)
		if (!allow_handle_click_when_gaining_focus())
			return true;

	set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			if (c->on_left_button_down(m))
				return true;
	return false;
}

bool SceneGraph::on_left_button_up(const vec2 &m) {
	set_mouse(m);
	mdp->finish(m);
	hover = get_hover_data(m);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			if (c->on_left_button_up(m))
				return true;
	return false;
}

bool SceneGraph::on_left_double_click(const vec2 &m) {
	set_mouse(m);
	update_hover();
	set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			if (c->on_left_double_click(m))
				return true;
	return false;
}

bool SceneGraph::on_right_button_down(const vec2 &m) {
	set_mouse(m);
	update_hover();
	set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			if (c->on_right_button_down(m))
				return true;
	return false;
}

bool SceneGraph::on_right_button_up(const vec2 &m) {
	set_mouse(m);
	update_hover();

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			if (c->on_right_button_up(m))
				return true;
	return false;
}

bool SceneGraph::on_mouse_move(const vec2 &m) {
	set_mouse(m);

	if (!mdp->update(m)) {
		update_hover();

		auto nodes = collect_children_down();
		for (auto *c: nodes)
			if (c->hover(m))
				if (c->on_mouse_move(m))
					return true;
	}
	return false;
}

bool SceneGraph::on_mouse_wheel(const vec2 &d) {
	//set_mouse(m);
	//hover = get_hover_data(m);
	//set_current(hover);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			if (c->on_mouse_wheel(d))
				return true;
	return false;
}

bool SceneGraph::on_key(int key) {
	auto nodes = collect_children_down();
	for (auto *c: nodes)
		//if (c->hover(m))
			if (c->on_key(key))
				return true;
	return false;
}

bool SceneGraph::allow_handle_click_when_gaining_focus() const {
	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(m))
			return c->allow_handle_click_when_gaining_focus();
	return false;
}

HoverData SceneGraph::get_hover_data(const vec2 &m) {
	auto nodes = collect_children_down();

	for (auto *c: nodes)
		if (c->hover(m))
			return c->get_hover_data(m);

	return HoverData();
}

string SceneGraph::get_tip() const {
	auto nodes = collect_children_down();

	for (auto *c: nodes)
		if (c->hover(m))
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

void SceneGraph::set_mouse(const vec2 &_m) {
	m = _m;
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

void SceneGraph::mdp_run(MouseDelayAction *a, const vec2 &m) {
	mdp->prepare(a);
	mdp->start_acting(m);
}

class MouseDelayActionWrapper : public MouseDelayAction {
public:
	hui::Callback callback;
	MouseDelayActionWrapper(hui::Callback c) { callback = c; }
	void on_update(const vec2 &m) override { callback(); }
};

void SceneGraph::mdp_prepare(hui::Callback update) {
	mdp->prepare(new MouseDelayActionWrapper(update));
}

void SceneGraph::integrate(hui::Panel *panel, const string &id, std::function<void(Painter *)> custom_draw, bool fill) {
	if (fill) {
		for (auto c: weak(children)) {
			c->align.horizontal = Node::AlignData::Mode::FILL;
			c->align.vertical = Node::AlignData::Mode::FILL;
		}
	}

	set_callback_redraw([=] {
		panel->redraw(id);
	});
	panel->event_xp(id, "hui:draw", [=](Painter* p) {
		if (custom_draw) {
			custom_draw(p);
		} else {
			update_geometry_recursive(p->area());
			draw(p);
		}
	});
	panel->event_x(id, "hui:left-button-down", [=] {
		on_left_button_down(hui::get_event()->m);
		request_redraw();
	});
	panel->event_x(id, "hui:left-button-up", [=] {
		on_left_button_up(hui::get_event()->m);
		request_redraw();
	});
	panel->event_x(id, "hui:left-double-click", [=] {
		on_left_double_click(hui::get_event()->m);
		request_redraw();
	});
	panel->event_x(id, "hui:right-button-down", [=] {
		on_right_button_down(hui::get_event()->m);
		request_redraw();
	});
	panel->event_x(id, "hui:right-button-up", [=] {
		on_right_button_up(hui::get_event()->m);
		request_redraw();
	});
	panel->event_x(id, "hui:mouse-wheel", [=] {
		on_mouse_wheel(hui::get_event()->scroll);
		request_redraw();
	});
	panel->event_x(id, "hui:mouse-move", [=] {
		on_mouse_move(hui::get_event()->m);
		request_redraw();
	});
	panel->event_x(id, "hui:key-down", [=] {
		on_key(hui::get_event()->key_code);
		request_redraw();
	});
}

SceneGraph *SceneGraph::create_integrated(hui::Panel *panel, const string &id, Node *node, const string &perf_name, std::function<void(Painter *)> custom_draw, bool fill) {
	auto graph = new SceneGraph();
	graph->set_perf_name(perf_name);
	graph->add_child(node);

	graph->integrate(panel, id, custom_draw, fill);
	return graph;
}

}
