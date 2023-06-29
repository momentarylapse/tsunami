/*
 * VolumeControl.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "VolumeControl.h"
#include "../../data/base.h"
#include "../../lib/hui/hui.h"
#include <math.h>


VolumeControl::VolumeControl(hui::Panel* _panel, const string& _id_slider, const string& _id_spin, const string& _id_unit) {
	panel = _panel;
	id_slider = _id_slider;
	id_spin = _id_spin;
	id_unit = _id_unit;

	min_value = 0;
	max_value = 1;
	min_slider_lin = 0;
	max_slider_lin = 1;
	min_slider_db = -120;
	max_slider_db = 0;
	value = 1;
	set_mode(Mode::DB);

	panel->event(id_spin, [this] {
		value = get_spin();
		set_slider(value);
		out_volume(value);
	});
	panel->event(id_slider, [this] {
		value = get_slider();
		set_spin(value);
		out_volume(value);
	});
	panel->event(id_unit, [this] {
		if (mode == Mode::PERCENT)
			set_mode(Mode::DB);
		else if (mode == Mode::DB)
			set_mode(Mode::PERCENT);
	});
}

void VolumeControl::set(float f) {
	value = f;
	set_spin(value);
	set_slider(value);
}

float VolumeControl::get() const {
	return value;
}

void VolumeControl::set_range(float min, float max) {
	min_value = min;
	max_value = max;
	min_slider_lin = ::max(min_value, 0.0f);
	max_slider_lin = ::min(max_value, 2.0f);
	min_slider_db = amplitude2db(::max(min_value, db2amplitude(-120)));
	max_slider_db = amplitude2db(::min(max_value, db2amplitude(DB_MAX)));
	set_mode(mode);
}

void VolumeControl::enable(bool enabled) {
	panel->enable(id_spin, enabled);
	panel->enable(id_slider, enabled);
	panel->enable(id_unit, enabled);
}

float VolumeControl::db2slider(float db) const {
	return (atan(db / TAN_SCALE) - atan(min_slider_db / TAN_SCALE)) / (atan(max_slider_db / TAN_SCALE) - atan(min_slider_db / TAN_SCALE));
}

float VolumeControl::slider2db(float val) const {
	return tan(atan(min_slider_db / TAN_SCALE) + val * (atan(max_slider_db / TAN_SCALE)- atan(min_slider_db / TAN_SCALE))) * TAN_SCALE;
}

float VolumeControl::amp2slider(float amp) const {
	return (amp - min_slider_lin) / (max_slider_lin - min_slider_lin);
}

float VolumeControl::slider2amp(float val) const {
	return min_slider_lin + val * (max_slider_lin - min_slider_lin);
}

void VolumeControl::set_mode(Mode m) {
	mode = m;
	panel->reset(id_slider);
	if (mode == Mode::PERCENT) {
		panel->set_options(id_spin, format("range=%f:%f:%f", min_value * 100, max_value * 100, 0.1f));
		panel->hide_control(id_unit, false);
		panel->set_string(id_unit, "%");
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", amp2slider(0.0f), 0));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", amp2slider(0.5f), 50));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", amp2slider(1.0f), 100));
		if (1.5f <= max_slider_lin)
			panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", amp2slider(1.5f), 150));
		if (2.0f <= max_slider_lin)
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", amp2slider(2.0f), 200));
	} else if (mode == Mode::DB) {
		panel->set_options(id_spin, format("range=%f:%f:%f", min_slider_db, amplitude2db(max_value), 0.1f));
		panel->hide_control(id_unit, false);
		panel->set_string(id_unit, "dB");
		if (DB_MAX <= max_slider_db)
			panel->add_string(id_slider, format("%f\\<span size='x-small'>%+d</span>", db2slider(DB_MAX), (int)DB_MAX));
		if (6 <= max_slider_db)
			panel->add_string(id_slider, format("%f\\", db2slider(6), 6));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", db2slider(0), 0));
		panel->add_string(id_slider, format("%f\\", db2slider(-6), -6));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", db2slider(-12), -12));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", db2slider(-24), -24));
		panel->add_string(id_slider, format(u8"%f\\<span size='x-small'>-\u221e</span>", db2slider(DB_MIN))); // \u221e
	}
	set_spin(value);
	set_slider(value);
}

void VolumeControl::set_spin(float f) {
	if (mode == Mode::PERCENT)
		panel->set_float(id_spin, f * 100);
	else if (mode == Mode::DB)
		panel->set_float(id_spin, amplitude2db(f));
}

void VolumeControl::set_slider(float f) {
	if (mode == Mode::DB)
		panel->set_float(id_slider, db2slider(amplitude2db(f)));
	else if (mode == Mode::PERCENT)
		panel->set_float(id_slider, amp2slider(f));
}

float VolumeControl::get_spin() const {
	if (mode == Mode::PERCENT)
		return panel->get_float(id_spin) / 100;
	if (mode == Mode::DB)
		return db2amplitude(panel->get_float(id_spin));
	return 0;
}

float VolumeControl::get_slider() const {
	if (mode == Mode::DB)
		return db2amplitude(slider2db(panel->get_float(id_slider)));
	if (mode == Mode::PERCENT)
		return slider2amp(panel->get_float(id_slider));
	return 0;
}



