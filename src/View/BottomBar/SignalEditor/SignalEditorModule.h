/*
 * SignalEditorModule.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../Helper/Graph/Node.h"

class SignalEditorTab;
class Module;


class SignalEditorModule : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	Module *module;
	SignalEditorModule(SignalEditorTab *t, Module *m);
	void on_draw(Painter *p) override;
	bool on_left_button_down(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
	string get_tip() override;
};
