/*
 * DetuneSynthesizerDialog.cpp
 *
 *  Created on: 25.12.2015
 *      Author: michi
 */

#include "TemperamentDialog.h"
#include "../audioview/AudioView.h"
#include "../../data/Track.h"
#include "../../module/synthesizer/Synthesizer.h"
#include "../../Tsunami.h"
#include <math.h>

namespace tsunami {

const int RELATIVE_NUM_PITCHES = 2;

TemperamentDialog::TemperamentDialog(Track *t, AudioView *v, hui::Window *parent) :
	hui::Dialog("detune_synthesizer_dialog", parent)
{
	track = t;
	temperament = track->synth->temperament;
	view = v;
	width = 1;
	height = 1;

	hover = -1;
	mode_relative = true;
	all_octaves = true;

	add_string("preset", "  <custom>");
	add_string("preset", "12 tone equal temperment");
	add_string("preset", "1/4 comma meantone");
	add_string("preset", "1/3 comma meantone");
	add_string("preset", "Pythagorean");
	add_string("preset", "5 limit diatonic major");

	for (int i=0; i<12; i++)
		add_string("reference-pitch", pitch_name(MiddleC + i));

	TemperamentType type;
	int ref_pitch;
	float ref_freq;

	if (temperament.guess_parameters(type, ref_pitch, ref_freq)) {
		set_int("preset", (int)type);
		set_int("reference-pitch", pitch_to_rel(ref_pitch));
		set_float("reference-freq", ref_freq);
	} else {
		set_int("preset", 0);
		set_int("reference-pitch", 0);
		set_float("reference-freq", temperament.freq[MiddleC]);
	}

	check("all_octaves", all_octaves);
	check("relative", mode_relative);

	event("cancel", [this] { on_close(); });
	event("hui:close", [this] { on_close(); });
	event("ok", [this] { on_ok(); });
	event("relative", [this] { on_relative(); });
	event("all_octaves", [this] { on_all_octaves(); });
	event("preset", [this] { on_preset(); });
	event("reference-pitch", [this] { on_reference_pitch(); });
	event("reference-freq", [this] { on_reference_freq(); });
}

void TemperamentDialog::on_draw(Painter *p) {
	p->set_line_width(0.8f);
	p->set_font_size(12);
	float w = p->width;
	float h = p->height;
	height = h;
	width = w;

	int N = all_octaves ? 12 : MaxPitch;

	p->set_color(theme.background);
	p->draw_rect(p->area());

	if (hover >= 0) {
		if (all_octaves) {
			color c = color::interpolate(theme.background, theme.capture_marker, 0.1f);
			p->set_color(c);
			for (int i=(hover%12); i<MaxPitch; i+=12)
				p->draw_rect(rect(pitch2x(i), pitch2x(i+1), 0, h));
		}
		color c = color::interpolate(theme.background, theme.capture_marker, 0.2f);
		p->set_color(c);
		p->draw_rect(rect(pitch2x(hover), pitch2x(hover+1), 0, h));
	}

	// grid
	p->set_color(theme.text_soft3);
	for (int i=1; i<N; i++) {
		float x = pitch2x(i);
		p->draw_line({x, 0}, {x, h});
	}
	if (mode_relative) {
		for (int i=-RELATIVE_NUM_PITCHES; i<=RELATIVE_NUM_PITCHES; i++) {
			float y = relpitch2y(i, 0);
			p->draw_line({0, y}, {w, y});
		}
	} else {
		for (int i=0; i<=N; i++) {
			float y = pitch2y(i);
			p->draw_line({0, y}, {w, y});
		}
	}


	// reference
	p->set_color(theme.preview_marker);
	for (int i=0; i<N; i++) {
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
	for (int i=0; i<N; i++) {
		float y = pitch2y(freq_to_pitch(temperament.freq[i]));
		if (mode_relative) {
			y = relpitch2y(freq_to_pitch(temperament.freq[i]), i);
		}
		float x0 = pitch2x(i);
		float x1 = pitch2x(i + 1);
		p->draw_line({x0, y}, {x1, y});
	}
	p->set_line_width(1.0f);


	if (hover >= 0) {
		p->set_color(theme.text);
		if (all_octaves)
			p->draw_str({20, 17}, rel_pitch_name(pitch_to_rel(hover)));
		else
			p->draw_str({20, 17}, pitch_name(hover));
		p->draw_str({70, 17}, format("%+.2f semi tones", freq_to_pitch(temperament.freq[hover]) - hover));
	}
}

float TemperamentDialog::pitch2x(float p) {
	if (all_octaves)
		return width * p / 12;
	return width * p / MaxPitch;
}

float TemperamentDialog::pitch2y(float p) {
	return height * (1 - (p + 5) / (MaxPitch + 10));
}

float TemperamentDialog::relpitch2y(float p, float p0) {
	return height/2 * (1 - (p - p0)/RELATIVE_NUM_PITCHES);
}

float TemperamentDialog::x2pitch(float x) {
	if (all_octaves)
		return x / width * 12;
	return x / width * MaxPitch;
}

float TemperamentDialog::y2pitch(float y) {
	return (1 - y / height) * (MaxPitch + 10) - 5;
}

float TemperamentDialog::y2relpitch(float y, float p0) {
	return p0 + (1 - y*2/height) * RELATIVE_NUM_PITCHES;
}

void TemperamentDialog::on_left_button_down(const vec2& m) {
	if (hover >= 0) {
		if (mode_relative) {
			temperament.freq[hover] = pitch_to_freq(y2relpitch(m.y, hover));
		} else {
			temperament.freq[hover] = pitch_to_freq(y2pitch(m.y));
		}
		set_int("preset", 0);
		redraw("detune_area");
	}
}

void TemperamentDialog::on_left_button_up(const vec2& m) {
}

void TemperamentDialog::on_mouse_move(const vec2& m) {
	hover = -1;
	if ((m.x >= 0) and (m.x < width)) {
		if (all_octaves)
			hover = (m.x / width * 12);
		else
			hover = (m.x / width * MaxPitch);
	}

	redraw("detune_area");
}

void TemperamentDialog::on_mouse_wheel(const vec2& scroll) {
	if (hover >= 0) {
		float speed = mode_relative ? 0.01f : 0.1f;
		if (scroll.y != 0) {
			float dpitch = speed * scroll.y;
			if (all_octaves) {
				for (int i=(hover % 12); i<MaxPitch; i+=12)
					temperament.freq[i] *= pitch_to_freq(dpitch) / pitch_to_freq(0);
			} else {
				temperament.freq[hover] *= pitch_to_freq(dpitch) / pitch_to_freq(0);
			}
			set_int("preset", 0);
		}
		redraw("detune_area");
	}
}

void TemperamentDialog::on_relative() {
	mode_relative = is_checked("relative");
	redraw("detune_area");
}

void TemperamentDialog::on_all_octaves() {
	all_octaves = is_checked("all_octaves");
	if (all_octaves) {
		if (!temperament.has_equal_octaves()) {
			for (int i=12; i<MaxPitch; i++)
				temperament.freq[i] = temperament.freq[i % 12] * pow(2.0f, float(i/12));
			track->detune_synthesizer(temperament);
		}
	}
	redraw("detune_area");
}

void TemperamentDialog::apply_preset() {
	int n = get_int("preset");
	if (n < 0)
		return;
	int pitch_ref = MiddleC + get_int("reference-pitch");
	float freq_ref = get_float("reference-freq");

	temperament = Temperament::create((TemperamentType)n, pitch_ref, freq_ref);

	/*all_octaves = true;
	mode_relative = true;
	check("all_octaves", all_octaves);
	check("relative", mode_relative);*/

	redraw("detune_area");
}

void TemperamentDialog::on_reference_pitch() {
	int pitch_ref = MiddleC + get_int("reference-pitch");
	set_float("reference-freq", pitch_to_freq(pitch_ref));
	apply_preset();
}

void TemperamentDialog::on_reference_freq() {
	apply_preset();
}

void TemperamentDialog::on_preset() {
	apply_preset();
}

void TemperamentDialog::on_close() {
	request_destroy();
}

void TemperamentDialog::on_ok() {
	track->detune_synthesizer(temperament);
	request_destroy();
}

}
