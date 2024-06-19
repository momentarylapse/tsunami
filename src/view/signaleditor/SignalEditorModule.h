/*
 * SignalEditorModule.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../helper/graph/Node.h"

namespace tsunami {

class SignalEditorTab;
class SignalEditorModulePort;
class Module;


class SignalEditorModule : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	Module *module;
	Array<SignalEditorModulePort*> in, out;
	SignalEditorModule(SignalEditorTab *t, Module *m);
	void on_draw(Painter *p) override;
	bool on_left_button_down(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;
	string get_tip() const override;
};

}
