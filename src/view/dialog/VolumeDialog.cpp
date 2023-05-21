/*
 * VolumeDialog.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "VolumeDialog.h"
#include "../../lib/base/pointer.h"
#include "../../data/base.h"
#include <math.h>


bool VolumeDialog::aborted;
bool VolumeDialog::maximize;



VolumeDialog::VolumeDialog(hui::Window *_parent, float value0, float min, float max, Callback _cb)
: hui::Dialog("volume-dialog", _parent) {
	cb = _cb;
	result = value0;
	aborted = true;
	maximize = false;
	_min = min;
	_max = max;
	_min_db = amplitude2db(::max(_min, 0.001f));
	_max_db = amplitude2db(_max);

	set_mode(Mode::PERCENT);

	event("value", [this] {
		result = get_spin();
		set_slider(result);
	});
	event("slider", [this] {
		result = get_slider();
		set_spin(result);
	});
	event("unit", [this] {
		if (mode == Mode::PERCENT)
			set_mode(Mode::DB);
		else if (mode == Mode::DB)
			set_mode(Mode::PERCENT);
	});
	event("maximize", [this] {
		enable("value", !is_checked("maximize"));
		enable("slider", !is_checked("maximize"));
	});
	event("cancel", [this] {
		cb(-1);
		request_destroy();
	});
	event("ok", [this] {
		aborted = false;
		maximize = is_checked("maximize");
		cb(result);
		request_destroy();
	});
}

float VolumeDialog::db2slider(float db) {
	return (atan(db / TAN_SCALE) - atan(DB_MIN / TAN_SCALE)) / (atan(DB_MAX / TAN_SCALE) - atan(DB_MIN / TAN_SCALE));
}

float VolumeDialog::slider2db(float val) {
	return tan(atan(DB_MIN / TAN_SCALE) + val * (atan(DB_MAX / TAN_SCALE)- atan(DB_MIN / TAN_SCALE))) * TAN_SCALE;
}

void VolumeDialog::set_mode(Mode m) {
	mode = m;
	reset("slider");
	if (mode == Mode::PERCENT) {
		set_options("value", format("range=%f:%f:%f", _min * 100, _max * 100, 0.1f));
		hide_control("unit", false);
		hide_control("maximize", false);
		set_string("unit", "%");
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", 0.0f, 0));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", 0.25f, 50));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", 0.5f, 100));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", 0.75f, 150));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", 1.0f, 200));
	} else if (mode == Mode::DB) {
		set_options("value", format("range=%f:%f:%f", _min_db, _max_db, 0.1f));
		hide_control("unit", false);
		hide_control("maximize", false);
		set_string("unit", "dB");
		add_string("slider", format("%f\\<span size='x-small'>%+d</span>", db2slider(DB_MAX), (int)DB_MAX));
		add_string("slider", format("%f\\", db2slider(6), 6));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", db2slider(0), 0));
		add_string("slider", format("%f\\", db2slider(-6), -6));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", db2slider(-12), -12));
		add_string("slider", format("%f\\<span size='x-small'>%d</span>", db2slider(-24), -24));
		add_string("slider", format(u8"%f\\<span size='x-small'>-\u221e</span>", db2slider(DB_MIN))); // \u221e
	}
	set_spin(result);
	set_slider(result);
}

void VolumeDialog::set_spin(float f) {
	if (mode == Mode::PERCENT)
		set_float("value", f * 100);
	else if (mode == Mode::DB)
		set_float("value", amplitude2db(f));
}

void VolumeDialog::set_slider(float f) {
	if (mode == Mode::DB)
		set_float("slider", db2slider(amplitude2db(f)));
	else if (mode == Mode::PERCENT)
		set_float("slider", f / 2);
}

float VolumeDialog::get_spin() {
	if (mode == Mode::PERCENT)
		return get_float("value") / 100;
	if (mode == Mode::DB)
		return db2amplitude(get_float("value"));
	return 0;
}

float VolumeDialog::get_slider() {
	if (mode == Mode::DB)
		return db2amplitude(slider2db(get_float("slider")));
	if (mode == Mode::PERCENT)
		return get_float("slider") * 2;
	return 0;
}

void VolumeDialog::ask(hui::Window *parent, float value0, float min, float max, Callback cb) {
	hui::fly(new VolumeDialog(parent, value0, min, max, cb));
}

