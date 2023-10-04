/*
 * SignalEditorModule.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorModule.h"
#include "SignalEditorTab.h"
#include "SignalEditorPort.h"
#include "SignalEditorBackground.h"
#include "../../../module/Module.h"
#include "../../../module/port/Port.h"
#include "../../audioview/AudioView.h"
#include "../../MouseDelayPlanner.h"
#include "../../helper/Drawing.h"
#include "../../helper/graph/SceneGraph.h"

const float MODULE_WIDTH = 140;
const float MODULE_HEIGHT = 23;

string module_header(Module *m) {
	if (m->module_name.num > 0)
		return m->module_name;
	if (m->module_class.num > 0)
		return m->module_class;
	return m->category_to_str(m->module_category);
}



static float module_port_in_x(Module *m) {
	return - 5;
}

static float module_port_in_y(Module *m, int index) {
	return MODULE_HEIGHT/2 + (index - (float)(m->port_in.num-1)/2)*20;
}

static float module_port_out_x(Module *m) {
	return MODULE_WIDTH + 5;
}

static float module_port_out_y(Module *m, int index) {
	return MODULE_HEIGHT/2 + (index - (float)(m->port_out.num-1)/2)*20;
}


class MouseDelayModuleDnD : public MouseDelayAction {
public:
	base::set<Module*> sel;
	SignalEditorTab *tab;
	Array<float> px0, py0;
	vec2 m0;
	MouseDelayModuleDnD(SignalEditorTab *t) {
		tab = t;
		sel = tab->sel_modules;
		for (auto *m: sel) {
			px0.add(m->module_x);
			py0.add(m->module_y);
		}
		m0 = tab->graph->m;
	}
	void on_update(const vec2 &m) override {
		foreachi (auto mm, sel, i) {
			mm->module_x = px0[i] + m.x - m0.x;
			mm->module_y = py0[i] + m.y - m0.y;
		}
		tab->update_module_positions();
	}
	void on_finish(const vec2 &m) override {
		for (auto m: sel) {
			m->module_x = round(m->module_x / MODULE_GRID) * MODULE_GRID;
			m->module_y = round(m->module_y / MODULE_GRID) * MODULE_GRID;
		}
		tab->update_module_positions();
	}
};




SignalEditorModule::SignalEditorModule(SignalEditorTab *t, Module *m) : scenegraph::NodeRel({m->module_x, m->module_y}, MODULE_WIDTH, MODULE_HEIGHT) {
	tab = t;
	module = m;
	set_perf_name("se:module");
	foreachi(auto &pd, m->port_in, i)
		in.add(new SignalEditorModulePort(tab, module, i, pd.type, module_port_in_x(module), module_port_in_y(module, i), false));
	foreachi(auto p, m->port_out, i)
		out.add(new SignalEditorModulePort(tab, module, i, p->type, module_port_out_x(module), module_port_out_y(module, i), true));
	for (auto p: in + out)
		add_child(p);
}

void SignalEditorModule::on_draw(Painter *p) {
	color bg = theme.blob_bg;
	if (tab->sel_modules.contains(module))
		bg = theme.blob_bg_selected;
	if (is_cur_hover())
		bg = theme.hoverify(bg);
	p->set_color(bg);
	p->set_roundness(theme.CORNER_RADIUS);
	rect r = area;
	p->draw_rect(r);
	p->set_roundness(0);
	p->set_font_size(theme.FONT_SIZE);// * 1.2f);
	if (tab->sel_modules.contains(module)) {
		p->set_color(theme.text);
		p->set_font("", -1, true, false);
	} else {
		p->set_color(theme.text_soft1);
	}
	string type = module_header(module);
	draw_str_constrained(p, r.center() - vec2(0, p->font_size/2), r.width() - 12, type, TextAlign::CENTER);
	p->set_font("", theme.FONT_SIZE, false, false);

}

bool SignalEditorModule::on_left_button_down(const vec2 &m) {
	if (!tab->sel_modules.contains(module))
		tab->select_module(module, tab->win->get_key(hui::KEY_CONTROL));
	tab->graph->mdp_prepare(new MouseDelayModuleDnD(tab));
	return true;
}

bool SignalEditorModule::on_right_button_down(const vec2 &m) {
	if (!tab->sel_modules.contains(module))
		tab->select_module(module, tab->win->get_key(hui::KEY_CONTROL));
	tab->popup_module();
	return true;
}

string SignalEditorModule::get_tip() const {
	return "module...";
}
