/*
 * Dial.h
 *
 *  Created on: Apr 18, 2021
 *      Author: michi
 */

#pragma once

#include "Graph/Node.h"


class Dial : public scenegraph::Node {
public:
	Dial(const string &label, float vmin, float vmax);

	float val_min, val_max;
	float value;
	float reference_value;
	string label;
	int digits;
	string unit;

	float value_to_rel(float v) const;
	complex rel_to_pos(float rel, float R);

	float get_value() const;
	void set_value(float v);

	void drag_update(float mx, float my);

	void on_draw(Painter *p) override;
	void draw_arc(Painter *p, float v0, float v1, float R);

	bool on_left_button_down(float mx, float my) override;
	bool on_mouse_wheel(float dx, float dy) override;

	string get_tip() const override;

	std::function<void(float)> cb_update;
	void set_callback(std::function<void(float)> callback);
};

