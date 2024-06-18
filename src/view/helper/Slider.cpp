/*
 * Slider.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Slider.h"
#include <cmath>

Slider::Slider(hui::Panel *_panel, const string & _id_slider, const string & _id_edit) {
	panel = _panel;
	id_slider = _id_slider;
	id_edit = _id_edit;
	value_min = 0;
	value_max = 1;
	value_min_slider = 0;
	value_max_slider = 1;
	factor = 1;
	value = 0;
	mode = Mode::Linear;

	if (panel) {
		event_handler_id[0] = panel->event(id_slider, [this] { on_slide(); });
		event_handler_id[1] = panel->event(id_edit, [this] { on_edit(); });
	}
}



Slider::~Slider() {
	if (panel) {
		panel->remove_event_handler(event_handler_id[0]);
		panel->remove_event_handler(event_handler_id[1]);
	}
}

void Slider::__init_ext__(hui::Panel *_panel, const string &_id_slider, const string &_id_edit, Callable<void()> *_func) {
	new(this) Slider(_panel, _id_slider, _id_edit);
	out_value >> create_data_sink<float>([_func] (float f) { (*_func)(); });
}

void Slider::__delete__() {
	this->Slider::~Slider();
}

void Slider::set_scale(float _factor) {
	factor = _factor;
	set(value);
}

void Slider::set_range(float _v_min, float _v_max, float _step) {
	value_min = _v_min;
	value_max = _v_max;
	value_step = _step;
	value_min_slider = _v_min;
	value_max_slider = _v_max;
	panel->set_options(id_edit, format("range=%f:%f:%f", value_min*factor, value_max*factor, value_step*factor));
	set(value);
}

void Slider::set_slider_range(float _v_min, float _v_max) {
	value_min_slider = _v_min;
	value_max_slider = _v_max;
	set(value);
}


void Slider::set(float v) {
	value = v;
	set_slide(v);
	panel->set_float(id_edit, value * factor);
}


void Slider::set_slide(float v) {
	if (mode == Mode::Exponential)
		panel->set_float(id_slider, (log(value) - log(value_min_slider)) / (log(value_max_slider) - log(value_min_slider)));
	else if (mode == Mode::Square)
		panel->set_float(id_slider, (sqrt(value) - sqrt(value_min_slider)) / (sqrt(value_max_slider) - sqrt(value_min_slider)));
	else
		panel->set_float(id_slider, (value - value_min_slider) / (value_max_slider - value_min_slider));
}



float Slider::get() {
	return value;
}

void Slider::set_mode(Mode m) {
	mode = m;
	set(value);
}

void Slider::enable(bool enabled) {
	panel->enable(id_slider, enabled);
	panel->enable(id_edit, enabled);
}


void Slider::on_slide() {
	if (mode == Mode::Exponential)
		value = value_min_slider + exp(panel->get_float(id_slider)) * (value_max_slider - value_min_slider);
	else if (mode == Mode::Square)
		value = value_min_slider + sqr(panel->get_float(id_slider)) * (value_max_slider - value_min_slider);
	else
		value = value_min_slider + panel->get_float(id_slider) * (value_max_slider - value_min_slider);
	panel->set_float(id_edit, value * factor);
	out_value(value);
}

void Slider::on_edit() {
	value = panel->get_float(id_edit) / factor;
	set_slide(value);
	out_value(value);
}

