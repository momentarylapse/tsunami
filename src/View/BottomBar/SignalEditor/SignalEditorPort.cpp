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
	MouseDelayCableCreate(SignalEditorTab *t, SignalEditorModulePort *p) {
		tab = t;
		port = p;
	}
	void on_update(float mx, float my) override {
	}
	void on_finish(float mx, float my) override {
	}
	virtual void on_draw_post(Painter *p) {
		p->set_color(tab->view->colors.text);
		/*if (hover.target_module) {
			p->set_line_width(5);
			Module *t = hover.target_module;
			if (hover.type == hover.TYPE_PORT_IN)
				p->draw_line(sel.dx, sel.dy, module_port_out_x(t), module_port_out_y(t, hover.target_port));
			else
				p->draw_line(sel.dx, sel.dy, module_port_in_x(t), module_port_in_y(t, hover.target_port));
			p->set_line_width(1);
		} else*/ {
			p->draw_line(port->area.mx(), port->area.my(), hui::GetEvent()->mx, hui::GetEvent()->my);
		}

	}
};




SignalEditorModulePort::SignalEditorModulePort(SignalEditorTab *t, Module *m, int _index, SignalType _type, float dx, float dy, bool out) :
		scenegraph::NodeRel(dx - R, dy - R, R*2, R*2) {

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


