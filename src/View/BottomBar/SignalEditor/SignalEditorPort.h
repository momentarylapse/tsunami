/*
 * SignalEditorPort.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once


#include "../../Helper/Graph/Node.h"

class SignalEditorTab;
class Module;
enum class SignalType;


class SignalEditorModulePort : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	SignalType type;
	bool is_out;
	SignalEditorModulePort(SignalEditorTab *t, SignalType _type, float dx, float dy, bool out);
	void on_draw(Painter *p) override;
	bool on_left_button_down(float mx, float my) override;
	string get_tip() override;
};


