/*
 * DetuneSynthesizerDialog.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "DetuneSynthesizerDialog.h"
#include "../AudioView/AudioView.h"
#include "../../Data/Track.h"
#include "../../Module/Synth/Synthesizer.h"
#include "../../lib/math/vector.h"
#include "../../Tsunami.h"
#include <math.h>

const int RELATIVE_NUM_PITCHES = 6;

DetuneSynthesizerDialog::DetuneSynthesizerDialog(Synthesizer *s, Track *t, AudioView *v, hui::Window *parent) :
	hui::Dialog("detune_synthesizer_dialog", parent)
{
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

void DetuneSynthesizerDialog::on_draw(Painter *p) {
	p->set_line_width(0.8f);
	p->set_font_size(12);
	float w = p->width;
	float h = p->height;
	height = h;
	width = w;

	p->set_color(theme.background);
	p->draw_rect(p->area());

	if (hover >= 0) {
		if (is_checked("all_octaves")) {
			color c = color::interpolate(theme.background, theme.capture_marker, 0.1f);
			p->set_color(c);
			for (int i=(hover%12); i<MAX_PITCH; i+=12)
				p->draw_rect(rect(pitch2x(i), pitch2x(i+1), 0, h));
		}
		color c = color::interpolate(theme.background, theme.capture_marker, 0.2f);
		p->set_color(c);
		p->draw_rect(rect(pitch2x(hover), pitch2x(hover+1), 0, h));
	}

	// grid
	p->set_color(theme.text_soft3);
	for (int i=1; i<MAX_PITCH; i++) {
		float x = pitch2x(i);
		p->draw_line({x, 0}, {x, h});
	}
	if (mode_relative) {
		for (int i=-RELATIVE_NUM_PITCHES; i<=RELATIVE_NUM_PITCHES; i++) {
			float y = relpitch2y(i, 0);
			p->draw_line({0, y}, {w, y});
		}
	} else {
		for (int i=0; i<=MAX_PITCH; i++) {
			float y = pitch2y(i);
			p->draw_line({0, y}, {w, y});
		}
	}


	// reference
	p->set_color(theme.preview_marker);
	for (int i=0; i<MAX_PITCH; i++) {
		float y = pitch2y(i);
		if (mode_relative) {
			y = relpitch2y(i, i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->draw_line({x0, y}, {x1, y});
		p->set_line_width(2.0f);
	}

	// current tuning
	p->set_line_width(2.0f);
	p->set_color(theme.capture_marker);
	for (int i=0; i<MAX_PITCH; i++) {
		float y = pitch2y(freq_to_pitch(synth->tuning.freq[i]));
		if (mode_relative) {
			y = relpitch2y(freq_to_pitch(synth->tuning.freq[i]), i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->draw_line({x0, y}, {x1, y});
	}
	p->set_line_width(1.0f);


	if (hover >= 0) {
		p->set_color(theme.text);
		p->draw_str({20, 17}, pitch_name(hover));
		p->draw_str({70, 17}, format("%+.2f semi tones", freq_to_pitch(synth->tuning.freq[hover]) - hover));
	}
}

float DetuneSynthesizerDialog::pitch2x(float p) {
	return width * p / MAX_PITCH;
}

float DetuneSynthesizerDialog::pitch2y(float p) {
	return height * (1 - (p + 5) / (MAX_PITCH + 10));
}

float DetuneSynthesizerDialog::relpitch2y(float p, float p0) {
	return height/2 * (1 - (p - p0)/RELATIVE_NUM_PITCHES);
}

float DetuneSynthesizerDialog::x2pitch(float x) {
	return x / width * MAX_PITCH;
}

float DetuneSynthesizerDialog::y2pitch(float y) {
	return (1 - y / height) * (MAX_PITCH + 10) - 5;
}

float DetuneSynthesizerDialog::y2relpitch(float y, float p0) {
	return p0 + (1 - y*2/height) * RELATIVE_NUM_PITCHES;
}

void DetuneSynthesizerDialog::on_left_button_down() {
	if (hover >= 0) {
		auto tuning = track->synth->tuning;
		if (mode_relative) {
			tuning.freq[hover] = pitch_to_freq(y2relpitch(hui::get_event()->m.y, hover));
		} else {
			tuning.freq[hover] = pitch_to_freq(y2pitch(hui::get_event()->m.y));
		}
		track->detune_synthesizer(tuning.freq);
		redraw("detune_area");
	}
}

void DetuneSynthesizerDialog::on_left_button_up() {
}

void DetuneSynthesizerDialog::on_mouse_move() {
	hover = -1;
	auto e = hui::get_event();
	if ((e->m.x >= 0) and (e->m.x < width))
		hover = (e->m.x / width * MAX_PITCH);

	redraw("detune_area");
}

void DetuneSynthesizerDialog::on_mouse_wheel() {
	if (hover >= 0) {
		auto e = hui::get_event();
		float speed = mode_relative ? 0.01f : 0.1f;
		bool all_octaves = is_checked("all_octaves");
		auto tuning = track->synth->tuning;
		if (e->scroll.y != 0) {
			float dpitch = speed * e->scroll.y;
			if (all_octaves) {
				for (int i=(hover % 12); i<MAX_PITCH; i+=12)
					tuning.freq[i] *= pitch_to_freq(dpitch) / pitch_to_freq(0);
			} else {
				tuning.freq[hover] *= pitch_to_freq(dpitch) / pitch_to_freq(0);
			}
			track->detune_synthesizer(tuning.freq);
		}
		redraw("detune_area");
	}
}

void DetuneSynthesizerDialog::on_relative() {
	mode_relative = is_checked("relative");
	redraw("detune_area");
}

void DetuneSynthesizerDialog::on_close() {
	request_destroy();
}
