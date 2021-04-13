/*
 * SignalEditorModule.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#include "SignalEditorModule.h"
#include "SignalEditorTab.h"
#include "SignalEditorPort.h"
#include "../../../Module/Module.h"
#include "../../../Module/Port/Port.h"
#include "../../AudioView.h"
#include "../../MouseDelayPlanner.h"

#include "../../Helper/Graph/SceneGraph.h"

const float MODULE_WIDTH = 140;
const float MODULE_HEIGHT = 23;
const float MODULE_GRID = 23;
string module_header(Module *m);


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
	Set<Module*> sel;
	SignalEditorTab *tab;
	Array<float> px0, py0;
	float mx0, my0;
	MouseDelayModuleDnD(SignalEditorTab *t) {
		tab = t;
		sel = tab->sel_modules;
		for (auto *m: sel) {
			px0.add(m->module_x);
			py0.add(m->module_y);
		}
		mx0 = tab->graph->mx;
		my0 = tab->graph->my;
	}
	void on_update(float mx, float my) override {
		foreachi (auto m, sel, i) {
			m->module_x = px0[i] + mx - mx0;
			m->module_y = py0[i] + my - my0;
		}
		tab->update_module_positions();
	}
	void on_finish(float mx, float my) override {
		for (auto m: sel) {
			m->module_x = round(m->module_x / MODULE_GRID) * MODULE_GRID;
			m->module_y = round(m->module_y / MODULE_GRID) * MODULE_GRID;
		}
		tab->update_module_positions();
	}
};




SignalEditorModule::SignalEditorModule(SignalEditorTab *t, Module *m) : scenegraph::NodeRel(m->module_x, m->module_y, MODULE_WIDTH, MODULE_HEIGHT) {
	tab = t;
	module = m;
	foreachi(auto &pd, m->port_in, i)
		in.add(new SignalEditorModulePort(tab, module, i, pd.type, module_port_in_x(module), module_port_in_y(module, i), false));
	foreachi(auto p, m->port_out, i)
		out.add(new SignalEditorModulePort(tab, module, i, p->type, module_port_out_x(module), module_port_out_y(module, i), true));
	for (auto p: in + out)
		add_child(p);
}

void SignalEditorModule::on_draw(Painter *p) {
	auto view = tab->view;

	color bg = view->colors.blob_bg;
	if (tab->sel_modules.contains(module))
		bg = view->colors.blob_bg_selected;
	if (is_cur_hover())
		bg = view->colors.hoverify(bg);
	p->set_color(bg);
	p->set_roundness(view->CORNER_RADIUS);
	rect r = area;
	p->draw_rect(r);
	p->set_roundness(0);
	p->set_font_size(AudioView::FONT_SIZE);// * 1.2f);
	if (tab->sel_modules.contains(module)) {
		p->set_color(view->colors.text);
		p->set_font("", -1, true, false);
	} else {
		p->set_color(view->colors.text_soft1);
	}
	string type = module_header(module);
	AudioView::draw_str_constrained(p, r.mx(), r.my() - p->font_size/2, r.width() - 12, type, 0);
	p->set_font("", AudioView::FONT_SIZE, false, false);

}

bool SignalEditorModule::on_left_button_down(float mx, float my) {
	if (!tab->sel_modules.contains(module))
		tab->select_module(module, tab->win->get_key(hui::KEY_CONTROL));
	tab->graph->mdp_prepare(new MouseDelayModuleDnD(tab));
	return true;
}

bool SignalEditorModule::on_right_button_down(float mx, float my) {
	if (!tab->sel_modules.contains(module))
		tab->select_module(module, tab->win->get_key(hui::KEY_CONTROL));
	tab->popup_module();
	return true;
}

string SignalEditorModule::get_tip() {
	return "module...";
}
