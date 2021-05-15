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

	show_debug = hui::Config.get_bool("scene-graph.debug", false);
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

bool SceneGraph::on_right_button_up(float mx, float my) {
	set_mouse(mx, my);
	hover = get_hover_data(mx, my);

	auto nodes = collect_children_down();
	for (auto *c: nodes)
		if (c->hover(mx, my))
			if (c->on_right_button_up(mx, my))
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

bool SceneGraph::on_key(int key) {
	auto nodes = collect_children_down();
	for (auto *c: nodes)
		//if (c->hover(mx, my))
			if (c->on_key(key))
				return true;
	return false;
}

bool SceneGraph::allow_handle_click_when_gaining_focus() const {
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

string SceneGraph::get_tip() const {
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
		on_left_button_down(hui::GetEvent()->mx, hui::GetEvent()->my);
		request_redraw();
	});
	panel->event_x(id, "hui:left-button-up", [=] {
		on_left_button_up(hui::GetEvent()->mx, hui::GetEvent()->my);
		request_redraw();
	});
	panel->event_x(id, "hui:left-double-click", [=] {
		on_left_double_click(hui::GetEvent()->mx, hui::GetEvent()->my);
		request_redraw();
	});
	panel->event_x(id, "hui:right-button-down", [=] {
		on_right_button_down(hui::GetEvent()->mx, hui::GetEvent()->my);
		request_redraw();
	});
	panel->event_x(id, "hui:right-button-up", [=] {
		on_right_button_up(hui::GetEvent()->mx, hui::GetEvent()->my);
		request_redraw();
	});
	panel->event_x(id, "hui:mouse-wheel", [=] {
		on_mouse_wheel(hui::GetEvent()->scroll_x, hui::GetEvent()->scroll_y);
		request_redraw();
	});
	panel->event_x(id, "hui:mouse-move", [=] {
		on_mouse_move(hui::GetEvent()->mx, hui::GetEvent()->my);
		request_redraw();
	});
	panel->event_x(id, "hui:key-down", [=] {
		on_key(hui::GetEvent()->key_code);
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
