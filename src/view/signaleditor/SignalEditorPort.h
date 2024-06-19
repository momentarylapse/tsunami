/*
 * SignalEditorPort.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once


#include "../helper/graph/Node.h"

namespace tsunami {

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
	bool on_left_button_down(const vec2 &m) override;
	string get_tip() const override;
	HoverData get_hover_data(const vec2 &m) override;
};

}
