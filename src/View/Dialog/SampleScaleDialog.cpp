/*
 * SampleScaleDialog.cpp
 *
 *  Created on: 22.04.2017
 *      Author: michi
 */

#include "SampleScaleDialog.h"
#include "../../Data/Song.h"
#include "../../Data/BufferInterpolator.h"

SampleScaleDialog::SampleScaleDialog(HuiWindow *root, Sample *s):
	HuiDialog("", 100, 100, root, false)
{
	fromResource("sample_scale_dialog");
	sample = s;

	setInt("samples_orig", sample->buf.length);
	setInt("rate_orig", sample->owner->sample_rate);
	setInt("rate_inv_new", sample->owner->sample_rate);

	new_size = sample->buf.length;
	update();

	event("factor", this, &SampleScaleDialog::onFactor);
	event("samples_new", this, &SampleScaleDialog::onSamples);
	event("rate_new", this, &SampleScaleDialog::onSampleRate);
	event("rate_inv_orig", this, &SampleScaleDialog::onSampleRateInv);
	event("ok", this, &SampleScaleDialog::onOk);
	event("cancel", this, &SampleScaleDialog::onClose);
	event("hui:close", this, &SampleScaleDialog::onClose);
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
		setInt("samples_new", new_size);
	if (mode != 1)
		setInt("rate_new", new_rate);
	if (mode != 2)
		setInt("rate_inv_orig", orig_rate_inv);
	if (mode != 3)
		setFloat("factor", factor * 100.0f);
}

void SampleScaleDialog::onSamples()
{
	new_size = getInt("");
	update(0);
}

void SampleScaleDialog::onSampleRate()
{
	new_size = (double)sample->buf.length * (double)getInt("") / (double)sample->owner->sample_rate;
	update(1);
}

void SampleScaleDialog::onSampleRateInv()
{
	new_size = (double)sample->buf.length / (double)getInt("") * (double)sample->owner->sample_rate;
	update(2);
}

void SampleScaleDialog::onFactor()
{
	new_size = sample->buf.length * getFloat("") / 100.0f;
	update(3);
}

void SampleScaleDialog::onOk()
{
	BufferInterpolator::Method method = BufferInterpolator::METHOD_LINEAR;
	if (getInt("method") == 1)
		method = BufferInterpolator::METHOD_CUBIC;
	else if (getInt("method") == 2)
		method = BufferInterpolator::METHOD_SINC;
	else if (getInt("method") == 3)
		method = BufferInterpolator::METHOD_FOURIER;

	BufferBox out;
	BufferInterpolator::interpolate(sample->buf, out, new_size, method);
	sample->buf = out;
	sample->owner->notify();
	destroy();
}

void SampleScaleDialog::onClose()
{
	destroy();
}

