/*
 * SignalEditorBackground.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../helper/graph/Node.h"
#include "../../helper/graph/Scrollable.h"

class SignalEditorTab;

extern const float MODULE_GRID;

class SignalEditorBackground : public Scrollable<scenegraph::Node> {
public:
	SignalEditorTab *tab;
	explicit SignalEditorBackground(SignalEditorTab *t);
	void on_draw(Painter *p) override;
	bool on_left_button_down(const vec2 &m) override;
	bool on_right_button_down(const vec2 &m) override;
};

