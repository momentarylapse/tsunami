/*
 * ToolTipOverlay.h
 *
 *  Created on: 01.05.2024
 *      Author: michi
 */

#pragma once

#include "Node.h"

class Painter;

namespace tsunami {

class ToolTipOverlay : public scenegraph::Node {
public:
	ToolTipOverlay();

	void on_draw(Painter *c) override;
	bool has_hover(const vec2 &m) const override;

	mutable vec2 m;
};

}
