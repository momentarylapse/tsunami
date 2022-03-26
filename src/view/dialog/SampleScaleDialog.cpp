/*
 * SampleScaleDialog.cpp
 *
 *  Created on: 22.04.2017
 *      Author: michi
 */

#include "SampleScaleDialog.h"
#include "../../data/Song.h"
#include "../../data/Sample.h"
#include "../../data/audio/BufferInterpolator.h"

SampleScaleDialog::SampleScaleDialog(hui::Window *parent, Sample *s):
	hui::Dialog("sample_scale_dialog", parent)
{
	sample = s;

	set_int("samples_orig", sample->buf->length);
	set_int("rate_orig", sample->owner->sample_rate);
	set_int("rate_inv_new", sample->owner->sample_rate);

	new_size = sample->buf->length;
	update();

	event("factor", [=]{ on_factor(); });
	event("samples_new", [=]{ on_samples(); });
	event("rate_new", [=]{ on_sample_rate(); });
	event("rate_inv_orig", [=]{ on_sample_rate_inv(); });
	event("ok", [=]{ on_ok(); });
	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
}

void SampleScaleDialog::update(int mode) {
	float factor = (float)new_size / (float)sample->buf->length;
	float new_rate = (float)sample->owner->sample_rate * factor;
	float orig_rate_inv = (float)sample->owner->sample_rate / factor;
	if (mode != 0)
		set_int("samples_new", new_size);
	if (mode != 1)
		set_int("rate_new", new_rate);
	if (mode != 2)
		set_int("rate_inv_orig", orig_rate_inv);
	if (mode != 3)
		set_float("factor", factor * 100.0f);
}

void SampleScaleDialog::on_samples() {
	new_size = get_int("");
	update(0);
}

void SampleScaleDialog::on_sample_rate() {
	new_size = (double)sample->buf->length * (double)get_int("") / (double)sample->owner->sample_rate;
	update(1);
}

void SampleScaleDialog::on_sample_rate_inv() {
	new_size = (double)sample->buf->length / (double)get_int("") * (double)sample->owner->sample_rate;
	update(2);
}

void SampleScaleDialog::on_factor() {
	new_size = sample->buf->length * get_float("") / 100.0f;
	update(3);
}

void SampleScaleDialog::on_ok() {
	auto method = BufferInterpolator::Method::LINEAR;
	if (get_int("method") == 1)
		method = BufferInterpolator::Method::CUBIC;
	else if (get_int("method") == 2)
		method = BufferInterpolator::Method::SINC;
	else if (get_int("method") == 3)
		method = BufferInterpolator::Method::FOURIER;

	auto *buf = new AudioBuffer;
	buf->resize(new_size);
	BufferInterpolator::interpolate(*sample->buf, *buf, method);

	sample->owner->sample_replace_buffer(sample, buf);
	request_destroy();
}

void SampleScaleDialog::on_close() {
	request_destroy();
}

