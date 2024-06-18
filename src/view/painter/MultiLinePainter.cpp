/*
 * MultiLinePainter.cpp
 *
 *  Created on: 21 Feb 2022
 *      Author: michi
 */

#include "MultiLinePainter.h"
#include "MidiPainter.h"
#include "../audioview/ViewPort.h"
#include "../ColorScheme.h"
#include "../HoverData.h"
#include "../../data/base.h"
#include "../../data/Song.h"
#include "../../data/Track.h"
#include "../../data/TrackLayer.h"
#include "../../data/TrackMarker.h"
#include "../../data/rhythm/Bar.h"
#include "../../data/rhythm/Beat.h"
#include "../../data/midi/Clef.h"
#include "../../data/SongSelection.h"
#include "../../lib/image/Painter.h"
#include "../../lib/math/rect.h"
#include "../../lib/math/vec2.h"
#include "../../lib/os/msg.h"
#include <math.h>


Array<MidiKeyChange> get_key_changes(const TrackLayer *l);
color hash_color(int h);

MultiLinePainter::MultiLinePainter(Song *s, const ColorScheme &c) :
	colors(c)
{
	song = s;


	cam = new ViewPort();
	cam->area = rect(border, page_width - border, 0, 2000);
	sel = new SongSelection;
	hover = new HoverData;
	mp = new MidiPainter(song, cam.get(), sel, hover, c);

	pdf_bpm = 0;
}

MultiLinePainter::~MultiLinePainter() {
}


void MultiLinePainter::__init__(Song *s, const ColorScheme &c) {
	new (this) MultiLinePainter(s, c);
}

void MultiLinePainter::__delete__() {
	this->MultiLinePainter::~MultiLinePainter();
}


float MultiLinePainter::draw_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale) {
	int slack = song->sample_rate / 15;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	y0 += padding_y_classical;

	mp->set_context(rect(x0, x0+w, y0, y0+line_height), t->instrument, true, MidiMode::CLASSICAL);
	mp->set_size_data(true, line_height / 75);
	mp->set_min_font_size(min_font_size);
	mp->set_key_changes(get_key_changes(t->layers[0].get()));
	mp->set_quality(200, antialiasing);
	mp->allow_shadows = allow_shadows;

	p->set_antialiasing(antialiasing);

	float ya = mp->clef_pos_to_screen(8);
	float yb = mp->clef_pos_to_screen(0);
	draw_beats(p, x0, w, ya, yb-ya, r);
	line_data.add({t, ya, yb});

	auto clef = t->instrument.get_clef();

	// clef lines
	p->set_color(colors.text_soft1);
	for (int i=0; i<10; i+=2) {
		float y = mp->clef_pos_to_screen(i);
		p->draw_line(vec2(x0, y), vec2(x0 + w, y));
	}
	//p->draw_str(x0, y0, clef.symbol);

	// midi
	for (auto l: weak(t->layers)) {
		auto midi = l->midi.get_notes(r_inside);
		mp->draw(p, midi);
	}

	y0 += padding_y_classical;

	return y0 + line_height;
}

float MultiLinePainter::draw_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale) {

	int slack = song->sample_rate / 15;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	int n = t->instrument.string_pitch.num;

	y0 += padding_y_tab;

	mp->set_context(rect(x0, x0+w, y0, y0+string_dy*n), t->instrument, true, MidiMode::TAB);
	mp->set_size_data(true, line_height / 75);//0.66f);
	mp->set_key_changes(get_key_changes(t->layers[0].get()));
	mp->set_quality(200, antialiasing);
	mp->allow_shadows = allow_shadows;

	p->set_antialiasing(antialiasing);

	float sy0 = mp->string_to_screen(n - 1) - string_dy/2;
	float sy1 = mp->string_to_screen(0) + string_dy/2;
	line_data.add({t, sy0, sy1});

	draw_beats(p, x0, w, sy0, sy1 - sy0, r);

	// string lines
	p->set_color(colors.text_soft1);
	p->set_line_width(line_height / 100);
	for (int i=0; i<t->instrument.string_pitch.num; i++) {
		float y = mp->string_to_screen(i);
		p->draw_line({x0, y}, {x0 + w, y});
	}

	// midi
	auto midi = t->layers[0]->midi.get_notes(r_inside);
	mp->draw(p, midi);

	y0 += padding_y_tab;

	return y0 + string_dy * n;
}

void MultiLinePainter::draw_track_markers(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale) {
	float y = y0;
	const float d = line_height / 20;
	for (auto l: weak(t->layers))
		for (auto m: weak(l->markers))
			if (r.is_inside(m->range.start())) {
				float x = cam->sample2screen(m->range.start());
				p->set_color(colors.text_soft1);
				p->draw_line({x - d*4, y - d*13}, {x - d*4, y - d*7});
				p->draw_line({x - d*4, y - d*13}, {x + d*4, y - d*13});
				p->draw_line({x - d*4, y - d*7},  {x + d*4, y - d*7});
				p->set_font_size(line_height / 5);
				p->draw_str({x - d*3, y-d*12}, m->nice_text());
			}
}

TrackMarker* get_bar_part(Song *s, int offset) {
	auto *t = s->time_track();
	if (!t)
		return nullptr;
	for (TrackMarker *m: weak(t->layers[0]->markers))
		if (abs(m->range.offset - offset) < 1000)
			return m;
	return nullptr;
}

void MultiLinePainter::draw_beats(Painter *p, float x0, float w, float y, float h, const Range &r) {
	auto beats = song->bars.get_beats(Range(r.offset, r.length + 1), true, false);
	for (auto b: beats) {
		float x = cam->sample2screen(b.range.offset);
		if (b.level == 0) {
			p->set_color(colors.text_soft1);
			p->set_line_width(line_height / 50);
		} else {
			p->set_color(colors.text_soft3);
			p->set_line_width(line_height / 100);
		}
		p->draw_line({x, y}, {x, y + h});
	}
	p->set_line_width(line_height / 100);
}

void MultiLinePainter::draw_bar_markers(Painter *p, float x0, float w, float y, float h, const Range &r) {
	auto bars = song->bars.get_bars(Range(r.offset, r.length - 50));
	p->set_antialiasing(antialiasing);
	const float d = line_height / 20;
	for (auto b: bars){
		float x1, x2;
		cam->range2screen(b->range(), x1, x2);
		double bpm = b->bpm(song->sample_rate);
		string s;
		if (b->beats != pdf_pattern) {
			pdf_pattern = b->beats;
			s += b->format_beats(false) + "   ";
		}
		if (fabs(bpm - pdf_bpm) > 0.2) {
			pdf_bpm = bpm;
			s += format("%.0f bpm ", bpm);
		}

		if (s != "") {
			p->set_color(colors.text_soft2);
			//p->draw_line(x + 10, y - 65, x - 20, y + 5);
			//p->draw_line(x + 10, y - 65, x + 20, y - 65);
			p->set_font_size(line_height / 8.5f);
			float dx = d*4;
			if (b == bars[0])
				dx += d*6;
			p->draw_str({x1 + dx, y-d}, s);
		}

		// part?
		if (auto *m = get_bar_part(song, b->offset)) {
			p->set_color(colors.text);
			float x = x1 * 0.75f + x2 * 0.25f;
			p->set_font_size(line_height / 3);
			p->draw_str({x - d*3, y-d*6}, m->nice_text());
		}
	}
}

void MultiLinePainter::set_context(const Any &conf, float _page_width, float _avg_samples_per_line) {
	page_width = _page_width;
	avg_samples_per_line = _avg_samples_per_line;
	update_scales();

	track_data.clear();

	foreachi (Track* t, weak(song->tracks), ti) {
		if (t->type != SignalType::Midi)
			continue;

		Any at;
		for (Any &a: conf.as_array())
			if (a["index"]._int() == ti)
				at = a;
		bool allow_classical = at.has("classical");
		bool allow_tab = at.has("tab") and (t->instrument.string_pitch.num > 0);
		if (!allow_classical and !allow_tab)
			continue;
		track_data.add({t, allow_classical, allow_tab});
	}
}

void MultiLinePainter::set(const Any &conf) {
	if (conf.has("border"))
		border = conf["border"]._float();
	if (conf.has("line-height"))
		line_height = conf["line-height"]._float();
	if (conf.has("line-space"))
		line_space = conf["line-space"]._float();
	if (conf.has("track-space"))
		track_space = conf["track-space"]._float();
	padding_y_classical = line_height * 0.25f;
	padding_y_tab = line_height * 0.15f;
	if (conf.has("padding-y-classical"))
		padding_y_classical = conf["padding-y-classical"]._float();
	if (conf.has("padding-y-tab"))
		padding_y_tab = conf["padding-y-tab"]._float();
	if (conf.has("antialiasing"))
		antialiasing = conf["antialiasing"]._bool();
	if (conf.has("allow-shadows"))
		allow_shadows = conf["allow-shadows"]._bool();
	if (conf.has("min-font-size"))
		min_font_size = conf["min-font-size"]._float();
	if (conf.has("part-colors"))
		allow_part_colors = conf["part-colors"]._bool();
	update_scales();
}

void MultiLinePainter::update_scales() {
	string_dy = line_height / 50.0f * 13;
	w = page_width - 2 * border;
	cam->area = rect(border, page_width - border, 0, 2000);
}

float MultiLinePainter::draw_line(Painter *p, float x0, float w, float y0, const Range &r, float scale) {
	p->set_line_width(line_height / 100);

	cam->pos = r.offset;
	cam->pixels_per_sample = (double)cam->area.width() / (double)r.length;

	if (allow_part_colors) {
		float y1 = y0 + get_line_dy() - track_space*2 - line_space;
		for (auto part: song->get_parts()) {
			if (!part->range.overlaps(r))
				continue;
			auto rr = part->range and r;
			p->set_color(color::interpolate(hash_color(part->text.hash()), theme.background, 0.90f));
			float x0, x1;
			cam->range2screen(rr, x0, x1);
			p->draw_rect(rect(x0,x1, y0, y1));
		}
	}

	const float d = line_height / 20;

	// first bar number
	auto bars = song->bars.get_bars(r + 1000);
	if (bars.num > 0) {
		p->set_color(colors.text_soft1);
		p->set_font_size(line_height / 5);
		p->draw_str({x0 + d, y0 - d*3}, i2s(bars[0]->index_text + 1));
	}

	line_data.clear();

	draw_bar_markers(p, x0, w, y0, line_height, r);

	// notes
	for (auto &tt: track_data) {
		if (tt.allow_classical)
			y0 = draw_track_classical(p, x0, w, y0, r, tt.track, scale) + track_space;
		if (tt.allow_tab)
			y0 = draw_track_tab(p, x0, w, y0, r, tt.track, scale) + track_space;
		if (tt.allow_classical or tt.allow_tab) {
			draw_track_markers(p, x0, w, y0, r, tt.track, scale);
			y0 += track_space;
		}
	}

	if (line_data.num == 0)
		return y0;

	// line connector
	p->set_line_width(line_height / 33);
	p->set_color(colors.text_soft1);
	float sy0 = line_data[0].y0;
	float sy1 = line_data.back().y1;
	p->draw_lines({{x0, sy0 - d*3}, {x0 - d*1.5f, sy0-d}, {x0 - d*1.5f, sy1+d}, {x0, sy1 + d*3}});
	p->draw_line({x0 + w + d*1.5f, sy0}, {x0 + w + d*1.5f, sy1});
	p->set_line_width(line_height / 100);

	return y0;
}

float MultiLinePainter::draw_next_line(Painter *p, int &offset, const vec2 &pos) {
	int line_samples = next_line_samples(offset);
	float scale = w / line_samples;
	Range r = Range(offset, line_samples);

	float y0 = draw_line(p, pos.x + border, w, pos.y, r, scale);
	y0 += line_space;

	offset += line_samples;
	return y0;
}

int MultiLinePainter::next_line_samples(int offset) {
	return good_samples(Range(offset, avg_samples_per_line));
}

float MultiLinePainter::get_line_dy() {
	float h = 0;
	for (auto &tt: track_data) {
		if (tt.allow_classical)
			h += 2 * padding_y_classical + line_height + track_space;
		if (tt.allow_tab)
			h += 2 * padding_y_tab + tt.track->instrument.string_pitch.num * string_dy + track_space;
		// markers
		h += track_space;
	}
	return h + line_space;
}

int MultiLinePainter::good_samples(const Range &r0) {
	auto bars = song->bars.get_bars(Range(r0.offset, r0.length * 2));
	int best_pos = -1;
	float best_penalty = 100000;
	for (auto b: bars) {
		if (b->range().offset <= r0.offset)
			continue;
		// close to rough guess?
		float penalty = abs(b->range().offset - r0.end());

		// part start/end?
		if (get_bar_part(song, b->range().offset))
			penalty -= song->sample_rate * 8;

		if (penalty < best_penalty) {
			best_pos = b->range().offset;
			best_penalty = penalty;
		}
	}

	if (best_pos < 0)
		return r0.length;
	return best_pos - r0.offset;
}


