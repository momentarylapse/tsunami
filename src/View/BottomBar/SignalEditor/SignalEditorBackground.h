/*
 * SignalEditorBackground.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../Helper/Graph/Node.h"
#include "../../Helper/Graph/Scrollable.h"

class SignalEditorTab;


class SignalEditorBackground : public Scrollable<scenegraph::Node> {
public:
	SignalEditorTab *tab;
	SignalEditorBackground(SignalEditorTab *t);
	void on_draw(Painter *p) override;
	bool on_left_button_down(float mx, float my) override;
	bool on_right_button_down(float mx, float my) override;
};

