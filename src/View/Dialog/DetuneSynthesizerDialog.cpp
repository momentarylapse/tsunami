/*
 * DetuneSynthesizerDialog.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "DetuneSynthesizerDialog.h"
#include "../../Tsunami.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../Data/Track.h"
#include "../AudioView.h"
#include <math.h>

const int RELATIVE_NUM_PITCHES = 6;

DetuneSynthesizerDialog::DetuneSynthesizerDialog(Synthesizer *s, Track *t, AudioView *v, hui::Window *parent) :
	hui::Dialog("", 500, 550, parent, false)
{
	fromResource("detune_synthesizer_dialog");

	track = t;
	synth = s;
	view = v;
	width = 1;
	height = 1;

	hover = -1;
	mode_relative = true;

	check("all_octaves", true);
	check("relative", mode_relative);

	event("close", std::bind(&DetuneSynthesizerDialog::onClose, this));
	event("hui:close", std::bind(&DetuneSynthesizerDialog::onClose, this));
	event("relative", std::bind(&DetuneSynthesizerDialog::onRelative, this));
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

	// grid
	p->setColor(view->colors.text_soft3);
	for (int i=1; i<MAX_PITCH; i++){
		float x = pitch2x(i);
		p->drawLine(x, 0, x, h);
	}
	if (mode_relative){
		for (int i=-RELATIVE_NUM_PITCHES; i<=RELATIVE_NUM_PITCHES; i++){
			float y = relpitch2y(i, 0);
			p->drawLine(0, y, w, y);
		}
	}else{
		for (int i=0; i<=MAX_PITCH; i++){
			float y = pitch2y(i);
			p->drawLine(0, y, w, y);
		}
	}


	// reference
	p->setColor(view->colors.preview_marker);
	for (int i=0; i<MAX_PITCH; i++){
		float y = pitch2y(i);
		if (mode_relative){
			y = relpitch2y(i, i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->drawLine(x0, y, x1, y);
		p->setLineWidth(2.0f);
	}

	// current tuning
	p->setLineWidth(2.0f);
	p->setColor(view->colors.capture_marker);
	for (int i=0; i<MAX_PITCH; i++){
		float y = pitch2y(freq_to_pitch(synth->tuning.freq[i]));
		if (mode_relative){
			y = relpitch2y(freq_to_pitch(synth->tuning.freq[i]), i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->drawLine(x0, y, x1, y);
	}
	p->setLineWidth(1.0f);


	if (hover >= 0){
		p->setColor(view->colors.text);
		p->drawStr(20, 20, pitch_name(hover));
		p->drawStr(70, 20, format("%+.2f semi tones", freq_to_pitch(synth->tuning.freq[hover]) - hover));
	}
}

float DetuneSynthesizerDialog::pitch2x(float p)
{
	return width * p / MAX_PITCH;
}

float DetuneSynthesizerDialog::pitch2y(float p)
{
	return height - height * (p + 5) / (MAX_PITCH + 10);
}

float DetuneSynthesizerDialog::relpitch2y(float p, float p0)
{
	return height/2 - height/RELATIVE_NUM_PITCHES/2 * (p - p0);
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
	auto e = hui::GetEvent();
	if ((e->mx >= 0) and (e->mx < width))
		hover = (e->mx / width * MAX_PITCH);

	redraw("detune_area");
}

void DetuneSynthesizerDialog::onMouseWheel()
{
	if (hover >= 0){
		auto e = hui::GetEvent();
		float speed = mode_relative ? 0.01f : 0.1f;
		if (e->scroll_y != 0)
			track->detuneSynthesizer(hover, speed * e->scroll_y, isChecked("all_octaves"));
		redraw("detune_area");
	}
}

void DetuneSynthesizerDialog::onRelative()
{
	mode_relative = isChecked("relative");
	redraw("detune_area");
}

void DetuneSynthesizerDialog::onClose()
{
	destroy();
}
