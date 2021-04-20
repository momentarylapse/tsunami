/*
 * Dial.cpp
 *
 *  Created on: Apr 18, 2021
 *      Author: michi
 */

#include "Dial.h"
#include "../../lib/image/Painter.h"
#include "../AudioView.h"


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

void Dial::drag_update(float mx, float my) {
}

float Dial::value_to_rel(float v) const {
	return (v - val_min) / (val_max - val_min);
}

void Dial::draw_arc(Painter *p, float v0, float v1, float R) {
	Array<complex> z;
	int n = 20;
	for (int i=0; i<=n; i++)
		//z.add(rel_to_pos((float)i / (float)n, R));
		z.add(rel_to_pos(value_to_rel(v0 + ((float)i / (float)n) * (v1 - v0)), R));
	p->draw_lines(z);
}

complex Dial::rel_to_pos(float rel, float R) {
	float phi = -dphi + dphi * 2 * rel;
	return complex(area.mx() + sin(phi) * R, area.my() - cos(phi) * R);
}

void Dial::on_draw(Painter *p) {
	p->set_color(Black);
	//p->draw_rect(area);

	// label
	p->set_color(AudioView::colors.text_soft1);
	float w = p->get_str_width(label);
	p->draw_str(area.mx() - w/2, area.y2 - 10, label);


	// value
	p->set_color(AudioView::colors.text);
	p->set_font_size(AudioView::FONT_SIZE * 0.8f);
	auto vv = f2s(value, digits);
	if (unit.num > 0)
		vv += " " + unit;
	w = p->get_str_width(vv);
	p->draw_str(area.mx() - w/2, area.my() - p->font_size/2, vv);
	p->set_font_size(AudioView::FONT_SIZE);



	Array<complex> z;
	p->set_line_width(3);
	p->set_color(AudioView::colors.text_soft3);
	float r = min(area.width()/2, area.height()/2) - 3;
	draw_arc(p, val_min, val_max, r);

	{
		auto q1 = rel_to_pos(0, r);
		auto q2 = rel_to_pos(0, r+4);
		p->draw_line(q1.x, q1.y, q2.x, q2.y);
	}
	{
		auto q1 = rel_to_pos(1, r);
		auto q2 = rel_to_pos(1, r+4);
		p->draw_line(q1.x, q1.y, q2.x, q2.y);
	}
	p->set_line_width(4);
	p->set_color(AudioView::colors.text);
	{
		auto q1 = rel_to_pos((value - val_min) / (val_max - val_min), r);
		auto q2 = rel_to_pos((value - val_min) / (val_max - val_min), r-7);
		p->draw_line(q1.x, q1.y, q2.x, q2.y);
	}
	draw_arc(p, reference_value, value, r);
	p->set_line_width(1);
}

bool Dial::on_left_button_down(float mx, float my) {
	return true;
}

bool Dial::on_mouse_wheel(float dx, float dy) {
	// TODO check shift key for high precision
	set_value(clamp(value + dy * (val_max - val_min) * 0.02f, val_min, val_max));
	if (cb_update)
		cb_update(value);
	return true;
}

void Dial::set_callback(std::function<void(float)> callback) {
	cb_update = callback;
}

