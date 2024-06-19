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
#include "../audioview/AudioView.h"
#include "../MouseDelayPlanner.h"
#include "../TsunamiWindow.h"
#include "../helper/Drawing.h"
#include "../helper/graph/SceneGraph.h"
#include "../../module/Module.h"
#include "../../module/port/Port.h"
#include "../../data/base.h"
#include "../../Session.h"

namespace tsunami {

const float MODULE_WIDTH = MODULE_GRID * 7;
const float MODULE_HEIGHT = MODULE_GRID * 2;
static const float PORT_SWARM_CENTER_OFFSET = 35;
static const float PORT_SWARM_RADIUS = 50;
static const float PORT_SWARM_DPHI = 0.4f;

string module_header(Module *m) {
	if (m->module_name.num > 0)
		return m->module_name;
	if (m->module_class.num > 0)
		return m->module_class;
	return Module::category_to_str(m->module_category);
}

SignalType module_in_type(Module *m) {
	for (auto p: m->port_in)
		return p->type;
	return SignalType::Group;
}

SignalType module_out_type(Module *m) {
	for (auto p: m->port_out)
		return p->type;
	return SignalType::Group;
}

color module_color(Module *m) {
	auto ti = module_in_type(m);
	auto to = module_out_type(m);
	if (m->module_category == ModuleCategory::STREAM)
		return theme.neon(0); // red
	if (m->module_category == ModuleCategory::SYNTHESIZER)
		return theme.neon(3); // green
	if (m->module_category == ModuleCategory::PLUMBING) {
		if (sa_contains({"AudioSucker", "MidiSucker", "BeatMidifier"}, m->module_class))
			return theme.neon(1); // orange
		return theme.text_soft3;
	}
	if (m->module_category == ModuleCategory::AUDIO_SOURCE)
		//return theme.neon(4); // blue
		//return theme.neon(3); // green
		return theme.neon(1); // orange
	if (m->module_category == ModuleCategory::AUDIO_VISUALIZER)
		//return theme.neon(5); // purple
		return theme.neon(2); // yellow
	if (ti == SignalType::Audio and to == SignalType::Audio)
		return theme.neon(2); // yellow

	if (ti == SignalType::Beats or to == SignalType::Beats)
		return theme.neon(2); // yellow;
	if (ti == SignalType::Midi and to == SignalType::Midi)
		return theme.neon(3);
	if (ti == SignalType::Midi and to == SignalType::Audio)
		return theme.neon(2);

	return theme.neon(2); // yellow

	//bg = theme.neon(m->type.hash() + 2);
}



static float module_port_in_x(Module *m, int index) {
	float phi = ((float)index - (float)(m->port_in.num-1) / 2) * PORT_SWARM_DPHI;
	return PORT_SWARM_CENTER_OFFSET - cos(phi) * PORT_SWARM_RADIUS;
}

static float module_port_in_y(Module *m, int index) {
	float phi = ((float)index - (float)(m->port_in.num-1) / 2) * PORT_SWARM_DPHI;
	return MODULE_HEIGHT/2 + sin(phi) * PORT_SWARM_RADIUS;
}

static float module_port_out_x(Module *m, int index) {
	float phi = ((float)index - (float)(m->port_out.num-1) / 2) * PORT_SWARM_DPHI;
	return MODULE_WIDTH - PORT_SWARM_CENTER_OFFSET + cos(phi) * PORT_SWARM_RADIUS;
}

static float module_port_out_y(Module *m, int index) {
	float phi = ((float)index - (float)(m->port_out.num-1) / 2) * PORT_SWARM_DPHI;
	return MODULE_HEIGHT/2 + sin(phi) * PORT_SWARM_RADIUS;
}


class MouseDelayModuleDnD : public scenegraph::MouseDelayAction {
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
		m0 = tab->graph()->m;
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
			m->module_x = (floor(m->module_x / MODULE_GRID) + 0.5f) * MODULE_GRID;
			m->module_y = (floor(m->module_y / MODULE_GRID) + 0.5f) * MODULE_GRID;
		}
		tab->update_module_positions();
	}
};




SignalEditorModule::SignalEditorModule(SignalEditorTab *t, Module *m) : scenegraph::NodeRel({m->module_x, m->module_y}, MODULE_WIDTH, MODULE_HEIGHT) {
	tab = t;
	module = m;
	set_perf_name("se:module");
	foreachi(auto p, m->port_in, i)
		in.add(new SignalEditorModulePort(tab, module, i, p->type, module_port_in_x(module, i), module_port_in_y(module, i), false));
	foreachi(auto p, m->port_out, i)
		out.add(new SignalEditorModulePort(tab, module, i, p->type, module_port_out_x(module, i), module_port_out_y(module, i), true));
	for (auto p: in + out)
		add_child(p);
}

void SignalEditorModule::on_draw(Painter *p) {
	string title = module_header(module);
	color bg = theme.text_soft1;
//	bg = theme.pitch_color(type.hash());
	bg = module_color(module);
//	bg = theme.neon((int)module->module_category);
	if (is_cur_hover())
		bg = theme.hoverify(bg);
	p->set_color(bg.with_alpha(0.3f));
	p->set_roundness(theme.CORNER_RADIUS);
	p->draw_rect(area);
	p->set_fill(false);
	p->set_color(bg);
	p->draw_rect(area);
	p->set_line_width(1);
	p->set_fill(true);
	p->set_roundness(0);
	p->set_font_size(theme.FONT_SIZE);
	if (tab->sel_modules.contains(module)) {
		p->set_color(theme.text);
		p->set_font("", theme.FONT_SIZE*1.1f, true, false);
	} else {
		p->set_color(theme.text_soft1);
	}
	draw_str_constrained(p, area.center() - vec2(0, p->font_size/2), area.width() - 12, title, TextAlign::CENTER);
	p->set_font("", theme.FONT_SIZE, false, false);

}

bool SignalEditorModule::on_left_button_down(const vec2 &m) {
	if (!tab->sel_modules.contains(module))
		tab->select_module(module, tab->session->win->get_key(hui::KEY_CONTROL));
	tab->graph()->mdp_prepare(new MouseDelayModuleDnD(tab));
	return true;
}

bool SignalEditorModule::on_right_button_down(const vec2 &m) {
	if (!tab->sel_modules.contains(module))
		tab->select_module(module, tab->session->win->get_key(hui::KEY_CONTROL));
	tab->popup_module();
	return true;
}

string SignalEditorModule::get_tip() const {
	return "module: " + module->module_class + "\ncategory: " + Module::category_to_str(module->module_category);
}

}
