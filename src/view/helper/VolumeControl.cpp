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


VolumeControl::VolumeControl(hui::Panel* _panel, const string& _id_slider, const string& _id_spin, const string& _id_unit, Callback _cb) {
	panel = _panel;
	id_slider = _id_slider;
	id_spin = _id_spin;
	id_unit = _id_unit;
	cb = _cb;

	_min = 0;
	_max = 1;
	_min_db = -120;
	_max_db = 0;
	value = 1;
	set_mode(Mode::PERCENT);

	panel->event(id_spin, [this] {
		value = get_spin();
		set_slider(value);
		cb(value);
	});
	panel->event(id_slider, [this] {
		value = get_slider();
		set_spin(value);
		cb(value);
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
	_min = min;
	_max = max;
	_min_db = amplitude2db(::max(_min, db2amplitude(-120)));
	_max_db = amplitude2db(_max);
	set_mode(mode);
}

float VolumeControl::db2slider(float db) {
	return (atan(db / TAN_SCALE) - atan(DB_MIN / TAN_SCALE)) / (atan(DB_MAX / TAN_SCALE) - atan(DB_MIN / TAN_SCALE));
}

float VolumeControl::slider2db(float val) {
	return tan(atan(DB_MIN / TAN_SCALE) + val * (atan(DB_MAX / TAN_SCALE)- atan(DB_MIN / TAN_SCALE))) * TAN_SCALE;
}

void VolumeControl::set_mode(Mode m) {
	mode = m;
	panel->reset(id_slider);
	if (mode == Mode::PERCENT) {
		panel->set_options(id_spin, format("range=%f:%f:%f", _min * 100, _max * 100, 0.1f));
		panel->hide_control(id_unit, false);
		panel->set_string(id_unit, "%");
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", 0.0f, 0));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", 0.25f, 50));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", 0.5f, 100));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", 0.75f, 150));
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%d</span>", 1.0f, 200));
	} else if (mode == Mode::DB) {
		panel->set_options(id_spin, format("range=%f:%f:%f", _min_db, _max_db, 0.1f));
		panel->hide_control(id_unit, false);
		panel->set_string(id_unit, "dB");
		panel->add_string(id_slider, format("%f\\<span size='x-small'>%+d</span>", db2slider(DB_MAX), (int)DB_MAX));
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
		panel->set_float(id_slider, f / 2);
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
		return panel->get_float(id_slider) * 2;
	return 0;
}



