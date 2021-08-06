/*
 * Dial.cpp
 *
 *  Created on: Apr 18, 2021
 *      Author: michi
 */

#include "Dial.h"
#include "../../lib/image/Painter.h"
#include "../../lib/math/complex.h"
#include "../AudioView.h"
#include "Graph/SceneGraph.h"
#include "../../lib/math/vector.h"


const float dphi = 2.0f;

Dial::Dial(const string &_label, float vmin, float vmax) : Node(60, 60) {
	set_perf_name("dial");
	label = _label;
	val_min = vmin;
	val_max = vmax;
	value = val_min;
	reference_value = val_min;
	digits = 1;
}

float Dial::get_value() const {
	return value;
}

void Dial::set_value(float v) {
	value = v;
	request_redraw();
}

void Dial::drag_update(const vec2 &m) {
}

float Dial::value_to_rel(float v) const {
	return (v - val_min) / (val_max - val_min);
}

void Dial::draw_arc(Painter *p, float v0, float v1, float R) {
	Array<vec2> z;
	int n = 20;
	for (int i=0; i<=n; i++)
		//z.add(rel_to_pos((float)i / (float)n, R));
		z.add(rel_to_pos(value_to_rel(v0 + ((float)i / (float)n) * (v1 - v0)), R));
	p->draw_lines(z);
}

vec2 Dial::rel_to_pos(float rel, float R) {
	float phi = -dphi + dphi * 2 * rel;
	return vec2(area.mx() + sin(phi) * R, area.my() - cos(phi) * R);
}

void Dial::on_draw(Painter *p) {
	p->set_antialiasing(true);

	// label
	p->set_color(theme.text_soft1);
	float w = p->get_str_width(label);
	p->draw_str({area.mx() - w/2, area.y2 - 10}, label);


	// value
	p->set_color(theme.text);
	p->set_font_size(theme.FONT_SIZE * 0.8f);
	auto vv = f2s(value, digits);
	if (unit.num > 0)
		vv += " " + unit;
	w = p->get_str_width(vv);
	p->draw_str({area.mx() - w/2, area.my() - p->font_size/2}, vv);
	p->set_font_size(theme.FONT_SIZE);



	Array<vec2> z;
	p->set_option("line-cap", "square");
	p->set_line_width(3);
	p->set_color(theme.text_soft3);
	float r = min(area.width()/2, area.height()/2) - 3;
	draw_arc(p, val_min, val_max, r);

	{
		auto q1 = rel_to_pos(0, r);
		auto q2 = rel_to_pos(0, r+4);
		p->draw_line(q1, q2);
	}
	{
		auto q1 = rel_to_pos(1, r);
		auto q2 = rel_to_pos(1, r+4);
		p->draw_line(q1, q2);
	}
	p->set_line_width(4);
	p->set_color(theme.text);
	{
		auto q1 = rel_to_pos((value - val_min) / (val_max - val_min), r);
		auto q2 = rel_to_pos((value - val_min) / (val_max - val_min), r-7);
		p->draw_line(q1, q2);
	}
	draw_arc(p, reference_value, value, r);
	p->set_option("line-cap", "butt");
	p->set_line_width(1);
}

bool Dial::on_left_button_down(const vec2 &m) {
	if (auto g = graph()) {
		g->mdp_prepare([=] {
			//drag_update(g->mx, g->my);
			auto e = hui::GetEvent();
			if (e->key_code & hui::KEY_SHIFT)
				set_value(clamp(value - e->d.y * (val_max - val_min) * 0.0002f, val_min, val_max));
			else
				set_value(clamp(value - e->d.y * (val_max - val_min) * 0.002f, val_min, val_max));
			if (cb_update)
				cb_update(value);
		});
	}
	return true;
}

bool Dial::on_mouse_wheel(const vec2 &d) {
	auto e = hui::GetEvent();
	if (e->key_code & hui::KEY_SHIFT)
		set_value(clamp(value + d.y * (val_max - val_min) * 0.02f, val_min, val_max));
	else
		set_value(clamp(value + d.y * (val_max - val_min) * 0.02f, val_min, val_max));
	if (cb_update)
		cb_update(value);
	return true;
}

string Dial::get_tip() const {
	//return label;
	return "";
}

void Dial::set_callback(std::function<void(float)> callback) {
	cb_update = callback;
}

