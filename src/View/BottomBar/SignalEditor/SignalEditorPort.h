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
	Module *module;
	SignalType type;
	bool is_out;
	int index;
	SignalEditorModulePort(SignalEditorTab *t, Module *m, int _index, SignalType _type, float dx, float dy, bool out);
	void on_draw(Painter *p) override;
	bool on_left_button_down(float mx, float my) override;
	string get_tip() const override;
	HoverData get_hover_data(float mx, float my) override;
};


