/*
 * DetuneSynthesizerDialog.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "DetuneSynthesizerDialog.h"
#include "../../Tsunami.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../AudioView.h"
#include <math.h>

DetuneSynthesizerDialog::DetuneSynthesizerDialog(Synthesizer *s, Track *t, AudioView *v, HuiWindow *parent) :
	HuiDialog("", 500, 550, parent, false)
{
	fromResource("detune_synthesizer_dialog");

	track = t;
	synth = s;
	view = v;
	width = 1;
	height = 1;

	hover = -1;

	check("all_octaves", true);

	event("close", this, &DetuneSynthesizerDialog::onClose);
	event("hui:close", this, &DetuneSynthesizerDialog::onClose);
}

DetuneSynthesizerDialog::~DetuneSynthesizerDialog()
{
}

void DetuneSynthesizerDialog::onDraw(Painter *p)
{
	p->setLineWidth(0.8f);
	p->setFontSize(12);
	float w = p->width;
	float h = p->height;
	height = h;
	width = w;

	p->setColor(view->colors.background);
	p->drawRect(0, 0, w, h);

	if (hover >= 0){
		if (isChecked("all_octaves")){
			color c = ColorInterpolate(view->colors.background, view->colors.capture_marker, 0.1f);
			p->setColor(c);
			for (int i=(hover%12); i<MAX_PITCH; i+=12)
				p->drawRect(pitch2x(i), 0, pitch2x(1), h);
		}
		color c = ColorInterpolate(view->colors.background, view->colors.capture_marker, 0.2f);
		p->setColor(c);
		p->drawRect(pitch2x(hover), 0, pitch2x(1), h);
	}

	p->setColor(view->colors.text_soft3);
	for (int i=1; i<MAX_PITCH; i++){
		float x = pitch2x(i);
		p->drawLine(x, 0, x, h);
	}
	for (int i=0; i<=MAX_PITCH; i++){
		float y = freq2y(pitch_to_freq(i));
		p->drawLine(0, y, w, y);
	}

	for (int i=0; i<MAX_PITCH; i++){
		float y0 = freq2y(pitch_to_freq(i));
		float y1 = freq2y(synth->tuning.freq[i]);
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->setColor(view->colors.preview_marker);
		p->drawLine(x0, y0, x1, y0);
		p->setColor(view->colors.capture_marker);
		p->drawLine(x0, y1, x1, y1);
	}


	if (hover >= 0){
		p->setColor(view->colors.text);
		p->drawStr(20, 20, pitch_name(hover));
		p->drawStr(70, 20, format("%+.2f", freq_to_pitch(synth->tuning.freq[hover]) - hover));
	}
}

float DetuneSynthesizerDialog::pitch2x(float p)
{
	return width * p / MAX_PITCH;
}

float DetuneSynthesizerDialog::freq2y(float f)
{
	return height - height * (log(f) - log(pitch_to_freq(-5))) / (log(pitch_to_freq(MAX_PITCH + 5)) - log(pitch_to_freq(-5)));
}

void DetuneSynthesizerDialog::onLeftButtonDown()
{
}

void DetuneSynthesizerDialog::onLeftButtonUp()
{
}

void DetuneSynthesizerDialog::onMouseMove()
{
	hover = -1;
	if ((HuiGetEvent()->mx >= 0) and (HuiGetEvent()->mx < width))
		hover = (HuiGetEvent()->mx / width * MAX_PITCH);

	redraw("detune_area");
}

void DetuneSynthesizerDialog::onMouseWheel()
{
	if (hover >= 0){
		if (HuiGetEvent()->scroll_y != 0)
			track->detuneSynthesizer(hover, 0.1f * HuiGetEvent()->scroll_y, isChecked("all_octaves"));
		redraw("detune_area");
	}
}

void DetuneSynthesizerDialog::onClose()
{
	destroy();
}
