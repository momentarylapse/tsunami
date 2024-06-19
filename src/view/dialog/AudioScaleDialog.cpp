/*
 * AudioScaleDialog.cpp
 *
 *  Created on: 2 Dec 2023
 *      Author: michi
 */

#include "AudioScaleDialog.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../processing/audio/BufferInterpolator.h"
#include <math.h>

namespace tsunami {

AudioScaleDialog::AudioScaleDialog(hui::Window *parent, int _original_size):
	hui::Dialog("audio-scale-dialog", parent)
{
	original_size = _original_size;
	data.pitch_scale = 1.0f;
	data.new_size = original_size;
	data.method = BufferInterpolator::Method::LINEAR;

	set_int("samples-original", original_size);
	check("pitch-mode:natural", true);
	enable("pitch-shift", false);
	enable("pitch-shift-semitones", false);

	update();

	event("factor", [this] { on_factor(); });
	event("samples-new", [this] { on_samples(); });
	event("pitch-mode:constant", [this] { update(); });
	event("pitch-mode:natural", [this] { update(); });
	event("pitch-mode:arbitrary", [this] { update(); });
	event("pitch-shift", [this] { on_pitch_shift(); });
	event("pitch-shift-semitones", [this] { on_pitch_shift_semitones(); });
	event("ok", [this] { on_ok(); });
	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
}

void AudioScaleDialog::update() {
	float factor = (float)data.new_size / (float)original_size;
	set_int("samples-new", data.new_size);
	set_float("factor", factor * 100.0f);

	auto set_pitch = [this] (float scale) {
		data.pitch_scale = scale;
		set_float("pitch-shift", scale * 100.0f);
		set_float("pitch-shift-semitones", log2(scale) * 12.0f);
	};

	if (is_checked("pitch-mode:constant")) {
		set_pitch(1.0f);
		enable("pitch-shift", false);
		enable("pitch-shift-semitones", false);
	} else if (is_checked("pitch-mode:natural")) {
		set_pitch(1.0f / factor);
		enable("pitch-shift", false);
		enable("pitch-shift-semitones", false);
	} else {
		enable("pitch-shift", true);
		enable("pitch-shift-semitones", true);
	}
}

void AudioScaleDialog::on_samples() {
	data.new_size = get_int("");
	update();
}

void AudioScaleDialog::on_factor() {
	data.new_size = (float)original_size * get_float("") / 100.0f;
	update();
}

void AudioScaleDialog::on_pitch_shift() {
	data.pitch_scale = get_float("") / 100.0f;
	set_float("pitch-shift-semitones", log2(data.pitch_scale) * 12.0f);
	update();
}

void AudioScaleDialog::on_pitch_shift_semitones() {
	data.pitch_scale = pow(2.0f, get_float("") / 12.0f);
	set_float("pitch-shift", data.pitch_scale * 100.0f);
	update();
}

void AudioScaleDialog::on_ok() {
	data.method = BufferInterpolator::Method::LINEAR;
	if (get_int("method") == 1)
		data.method = BufferInterpolator::Method::CUBIC;
	else if (get_int("method") == 2)
		data.method = BufferInterpolator::Method::SINC;
	else if (get_int("method") == 3)
		data.method = BufferInterpolator::Method::FOURIER;
	_promise(data);
	request_destroy();
}

void AudioScaleDialog::on_close() {
	_promise.fail();
	request_destroy();
}

base::future<AudioScaleDialog::Data> AudioScaleDialog::ask(hui::Window *parent, int original_size) {
	auto dlg = new AudioScaleDialog(parent, original_size);
	hui::fly(dlg);
	return dlg->_promise.get_future();
}

}

