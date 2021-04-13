/*
 * SignalEditorPort.cpp
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */


#include "SignalEditorPort.h"
#include "SignalEditorTab.h"
#include "../../../Data/base.h"

const float R = 10;

SignalEditorModulePort::SignalEditorModulePort(SignalEditorTab *t, SignalType _type, float dx, float dy, bool out) :
		scenegraph::NodeRel(dx - R, dy - R, R*2, R*2) {

	tab = t;
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
	return true;
}

string SignalEditorModulePort::get_tip() {
	if (is_out)
		return _("output: ") + signal_type_name(type);
	else
		return _("input: ") + signal_type_name(type);
}


