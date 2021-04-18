/*
 * SignalEditorPort.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */


#include "SignalEditorPort.h"
#include "SignalEditorTab.h"
#include "../../../Data/base.h"
#include "../../../Module/SignalChain.h"
#include "../../MouseDelayPlanner.h"
#include "../../Helper/Graph/SceneGraph.h"
#include "../../AudioView.h"

const float R = 10;



class MouseDelayCableCreate : public MouseDelayAction {
public:
	SignalEditorTab *tab;
	SignalEditorModulePort *port;
	SignalEditorModulePort *target = nullptr;
	MouseDelayCableCreate(SignalEditorTab *t, SignalEditorModulePort *p) {
		tab = t;
		port = p;
	}
	void on_update(float mx, float my) override {
		tab->graph->hover = tab->graph->get_hover_data(mx, my);
		target = nullptr;
		if (tab->graph->hover.type == HoverData::Type::PORT_IN or tab->graph->hover.type == HoverData::Type::PORT_OUT) {
			auto pp = static_cast<SignalEditorModulePort*>(tab->graph->hover.node);
			if ((pp->module != port->module) and (pp->is_out != port->is_out) and (pp->type == port->type))
				target = pp;
		}
		if (!target)
			tab->graph->hover = HoverData();
	}
	void on_finish(float mx, float my) override {
		if (target) {
			if (port->is_out)
				tab->chain->connect(port->module, port->index, target->module, target->index);
			else
				tab->chain->connect(target->module, target->index, port->module, port->index);
		}
	}
	virtual void on_draw_post(Painter *p) {
		p->set_color(tab->view->colors.text);
		if (target) {
			p->set_line_width(5);
			p->draw_line(port->area.mx(), port->area.my(), target->area.mx(), target->area.my());
			p->set_line_width(1);
		} else {
			p->draw_line(port->area.mx(), port->area.my(), hui::GetEvent()->mx, hui::GetEvent()->my);
		}
	}
};




SignalEditorModulePort::SignalEditorModulePort(SignalEditorTab *t, Module *m, int _index, SignalType _type, float dx, float dy, bool out) :
		scenegraph::NodeRel(dx - R, dy - R, R*2, R*2) {

	set_perf_name("se:port");
	tab = t;
	module = m;
	index = _index;
	type = _type;
	is_out = out;
}

void SignalEditorModulePort::on_draw(Painter *p) {
	bool hovering = is_cur_hover();
	p->set_color(tab->signal_color(type, hovering));
	float r = hovering ? 6 : 4;
	p->draw_circle(area.mx(), area.my(), r);
}

bool SignalEditorModulePort::on_left_button_down(float mx, float my) {
	if (is_out) {
		tab->chain->disconnect_out(module, index);
	} else {
		tab->chain->disconnect_in(module, index);
	}
	tab->graph->mdp_prepare(new MouseDelayCableCreate(tab, this));
	return true;
}

string SignalEditorModulePort::get_tip() {
	if (is_out)
		return _("output: ") + signal_type_name(type);
	else
		return _("input: ") + signal_type_name(type);
}

HoverData SignalEditorModulePort::get_hover_data(float mx, float my) {
	auto h = HoverData();
	h.node = this;
	h.index = index;
	h.type = is_out ? HoverData::Type::PORT_OUT : HoverData::Type::PORT_IN;
	return h;
}


