/*
 * SignalEditorCable.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../Helper/Graph/Node.h"

class SignalEditorTab;
class Module;
struct Cable;
enum class SignalType;


class SignalEditorCable : public scenegraph::NodeRel {
public:
	SignalEditorTab *tab;
	Module *source, *target;
	int source_port, target_port;
	SignalType type;
	SignalEditorCable(SignalEditorTab *t, const Cable &c);
	void on_draw(Painter *p) override;
	//bool on_left_button_down(float mx, float my) override;
	//bool on_right_button_down(float mx, float my) override;
	//string get_tip() override;
	bool hover(float mx, float my) const override;
};

