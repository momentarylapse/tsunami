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
	from_resource("detune_synthesizer_dialog");

	track = t;
	synth = s;
	view = v;
	width = 1;
	height = 1;

	hover = -1;
	mode_relative = true;

	check("all_octaves", true);
	check("relative", mode_relative);

	event("close", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
	event("relative", [=]{ on_relative(); });
}

DetuneSynthesizerDialog::~DetuneSynthesizerDialog()
{
}

void DetuneSynthesizerDialog::on_draw(Painter *p)
{
	p->set_line_width(0.8f);
	p->set_font_size(12);
	float w = p->width;
	float h = p->height;
	height = h;
	width = w;

	p->set_color(view->colors.background);
	p->draw_rect(0, 0, w, h);

	if (hover >= 0){
		if (is_checked("all_octaves")){
			color c = ColorInterpolate(view->colors.background, view->colors.capture_marker, 0.1f);
			p->set_color(c);
			for (int i=(hover%12); i<MAX_PITCH; i+=12)
				p->draw_rect(pitch2x(i), 0, pitch2x(1), h);
		}
		color c = ColorInterpolate(view->colors.background, view->colors.capture_marker, 0.2f);
		p->set_color(c);
		p->draw_rect(pitch2x(hover), 0, pitch2x(1), h);
	}

	// grid
	p->set_color(view->colors.text_soft3);
	for (int i=1; i<MAX_PITCH; i++){
		float x = pitch2x(i);
		p->draw_line(x, 0, x, h);
	}
	if (mode_relative){
		for (int i=-RELATIVE_NUM_PITCHES; i<=RELATIVE_NUM_PITCHES; i++){
			float y = relpitch2y(i, 0);
			p->draw_line(0, y, w, y);
		}
	}else{
		for (int i=0; i<=MAX_PITCH; i++){
			float y = pitch2y(i);
			p->draw_line(0, y, w, y);
		}
	}


	// reference
	p->set_color(view->colors.preview_marker);
	for (int i=0; i<MAX_PITCH; i++){
		float y = pitch2y(i);
		if (mode_relative){
			y = relpitch2y(i, i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->draw_line(x0, y, x1, y);
		p->set_line_width(2.0f);
	}

	// current tuning
	p->set_line_width(2.0f);
	p->set_color(view->colors.capture_marker);
	for (int i=0; i<MAX_PITCH; i++){
		float y = pitch2y(freq_to_pitch(synth->tuning.freq[i]));
		if (mode_relative){
			y = relpitch2y(freq_to_pitch(synth->tuning.freq[i]), i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->draw_line(x0, y, x1, y);
	}
	p->set_line_width(1.0f);


	if (hover >= 0){
		p->set_color(view->colors.text);
		p->draw_str(20, 17, pitch_name(hover));
		p->draw_str(70, 17, format("%+.2f semi tones", freq_to_pitch(synth->tuning.freq[hover]) - hover));
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

void DetuneSynthesizerDialog::on_left_button_down()
{
}

void DetuneSynthesizerDialog::on_left_button_up()
{
}

void DetuneSynthesizerDialog::on_mouse_move()
{
	hover = -1;
	auto e = hui::GetEvent();
	if ((e->mx >= 0) and (e->mx < width))
		hover = (e->mx / width * MAX_PITCH);

	redraw("detune_area");
}

void DetuneSynthesizerDialog::on_mouse_wheel()
{
	if (hover >= 0){
		auto e = hui::GetEvent();
		float speed = mode_relative ? 0.01f : 0.1f;
		if (e->scroll_y != 0)
			track->detune_synthesizer(hover, speed * e->scroll_y, is_checked("all_octaves"));
		redraw("detune_area");
	}
}

void DetuneSynthesizerDialog::on_relative()
{
	mode_relative = is_checked("relative");
	redraw("detune_area");
}

void DetuneSynthesizerDialog::on_close()
{
	destroy();
}
