/*
 * Dial.h
 *
 *  Created on: Apr 18, 2021
 *      Author: michi
 */

#pragma once

#include "graph/Node.h"

namespace tsunami {

class Dial : public scenegraph::Node {
public:
	Dial(const string &label, float vmin, float vmax);

	obs::xsource<float> out_value{this, "value"};

	float val_min, val_max;
	float value;
	float reference_value;
	string label;
	int digits;
	string unit;

	float value_to_rel(float v) const;
	vec2 rel_to_pos(float rel, float R);

	float get_value() const;
	void set_value(float v);

	void drag_update(const vec2 &m);

	void on_draw(Painter *p) override;
	void draw_arc(Painter *p, float v0, float v1, float R);

	bool on_left_button_down(const vec2 &m) override;
	bool on_mouse_wheel(const vec2 &d) override;

	string get_tip() const override;
};

}

