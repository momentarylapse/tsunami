/*
 * SignalEditorBackground.h
 *
 *  Created on: Apr 13, 2021
 *      Author: michi
 */

#pragma once

#include "../../Helper/Graph/Node.h"

class SignalEditorTab;


class SignalEditorBackground : public scenegraph::Node {
public:
	SignalEditorTab *tab;
	SignalEditorBackground(SignalEditorTab *t);
	void on_draw(Painter *p) override;
};

