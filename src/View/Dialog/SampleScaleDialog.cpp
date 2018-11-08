/*
 * SampleScaleDialog.cpp
 *
 *  Created on: 22.04.2017
 *      Author: michi
 */

#include "SampleScaleDialog.h"
#include "../../Data/Song.h"
#include "../../Data/Sample.h"
#include "../../Data/Audio/BufferInterpolator.h"

SampleScaleDialog::SampleScaleDialog(hui::Window *root, Sample *s):
	hui::Dialog("", 100, 100, root, false)
{
	from_resource("sample_scale_dialog");
	sample = s;

	set_int("samples_orig", sample->buf.length);
	set_int("rate_orig", sample->owner->sample_rate);
	set_int("rate_inv_new", sample->owner->sample_rate);

	new_size = sample->buf.length;
	update();

	event("factor", std::bind(&SampleScaleDialog::on_factor, this));
	event("samples_new", std::bind(&SampleScaleDialog::on_samples, this));
	event("rate_new", std::bind(&SampleScaleDialog::on_sample_rate, this));
	event("rate_inv_orig", std::bind(&SampleScaleDialog::on_sample_rate_inv, this));
	event("ok", std::bind(&SampleScaleDialog::on_ok, this));
	event("cancel", std::bind(&SampleScaleDialog::on_close, this));
	event("hui:close", std::bind(&SampleScaleDialog::on_close, this));
}

SampleScaleDialog::~SampleScaleDialog()
{
}

void SampleScaleDialog::update(int mode)
{
	float factor = (float)new_size / (float)sample->buf.length;
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

void SampleScaleDialog::on_samples()
{
	new_size = get_int("");
	update(0);
}

void SampleScaleDialog::on_sample_rate()
{
	new_size = (double)sample->buf.length * (double)get_int("") / (double)sample->owner->sample_rate;
	update(1);
}

void SampleScaleDialog::on_sample_rate_inv()
{
	new_size = (double)sample->buf.length / (double)get_int("") * (double)sample->owner->sample_rate;
	update(2);
}

void SampleScaleDialog::on_factor()
{
	new_size = sample->buf.length * get_float("") / 100.0f;
	update(3);
}

void SampleScaleDialog::on_ok()
{
	BufferInterpolator::Method method = BufferInterpolator::METHOD_LINEAR;
	if (get_int("method") == 1)
		method = BufferInterpolator::METHOD_CUBIC;
	else if (get_int("method") == 2)
		method = BufferInterpolator::METHOD_SINC;
	else if (get_int("method") == 3)
		method = BufferInterpolator::METHOD_FOURIER;

	sample->owner->scale_sample(sample, new_size, method);
	destroy();
}

void SampleScaleDialog::on_close()
{
	destroy();
}

