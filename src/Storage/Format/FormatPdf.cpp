/*
 * FormatPdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "FormatPdf.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Data/Track.h"
#include "../../Data/TrackLayer.h"
#include "../../Data/TrackMarker.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Midi/Clef.h"
#include "../../Data/SongSelection.h"
#include "../../lib/xfile/pdf.h"
#include "../../lib/image/Painter.h"
#include "../../lib/math/rect.h"
#include "../../lib/math/vector.h"
#include "../Dialog/PdfConfigDialog.h"
#include "../../View/ViewPort.h"
#include "../../View/Painter/MidiPainter.h"
#include "../../View/ColorScheme.h"
#include "../../View/Helper/SymbolRenderer.h"
#include "../../View/HoverData.h"
#include <math.h>

static const color NOTE_COLOR = color(1, 0.3f, 0.3f, 0.3f);
static const color NOTE_COLOR_TAB = color(1, 0.8f, 0.8f, 0.8f);

Array<MidiKeyChange> get_key_changes(const TrackLayer *l);

FormatDescriptorPdf::FormatDescriptorPdf() :
	FormatDescriptor(_("Pdf sheet"), "pdf", Flag::MIDI | Flag::MULTITRACK | Flag::WRITE) {}


bool FormatPdf::get_parameters(StorageOperationData *od, bool save) {
	// optional defaults
	if (!od->parameters.has("horizontal-scale"))
		od->parameters.map_set("horizontal-scale", 1.0f);

	if (od->parameters.has("tracks"))
		return true;

	// mandatory defaults
	if (!od->parameters.has("tracks"))
		od->parameters.map_set("tracks", {});
	
	auto dlg = ownify(new PdfConfigDialog(od, od->win));
	dlg->_run();
	return dlg->ok;
}

ColorScheme create_pdf_color_scheme() {
	ColorScheme bright;
	bright.background = White;
	bright.text = Black;//color(1, 0.3f, 0.3f, 0.1f);
	bright.selection = color(1, 0.2f, 0.2f, 0.7f);
	bright.hover = White;
	bright.gamma = 1.0f;
	bright.name = "pdf";
	bright.auto_generate();
	return bright;
}

FormatPdf::LineData::LineData(Track *t, float _y0, float _y1) {
	y0 = _y0;
	y1 = _y1;
	track = t;
}

int FormatPdf::draw_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale) {
	int slack = song->sample_rate / 15;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	mp->set_context(rect(x0, x0+w, y0-25, y0+90), t->instrument, true, MidiMode::CLASSICAL);
	mp->set_line_weight(0.66f);
	mp->set_key_changes(get_key_changes(t->layers[0].get()));
	mp->set_quality(200, true);

	float ya = mp->clef_pos_to_screen(8);
	float yb = mp->clef_pos_to_screen(0);
	draw_beats(p, x0, w, ya, yb-ya, r);
	line_data.add(LineData(t, ya, yb));

	auto clef = t->instrument.get_clef();

	// clef lines
	p->set_color(colors->text_soft1);
	for (int i=0; i<10; i+=2) {
		float y = mp->clef_pos_to_screen(i);
		p->draw_line(vec2(x0, y), vec2(x0 + w, y));
	}
	//p->draw_str(x0, y0, clef.symbol);

	// midi
	auto midi = t->layers[0]->midi.get_notes(r_inside);
	mp->draw(p, midi);

	return y0 + 50;
}

int FormatPdf::draw_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale) {
	float string_dy = 13;

	int slack = song->sample_rate / 15;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	int n = t->instrument.string_pitch.num;

	mp->set_context(rect(x0, x0+w, y0, y0+string_dy*n), t->instrument, true, MidiMode::TAB);
	mp->set_line_weight(0.66f);
	mp->set_key_changes(get_key_changes(t->layers[0].get()));
	mp->set_quality(200, true);
	/*mp->rr *= 1.3f;
	mp->neck_width *= 0.7f;*/

	float sy0 = mp->string_to_screen(n - 1) - string_dy/2;
	float sy1 = mp->string_to_screen(0) + string_dy/2;
	line_data.add(LineData(t, sy0, sy1));

	draw_beats(p, x0, w, sy0, sy1 - sy0, r);

	// string lines
	p->set_color(colors->text_soft1);
	for (int i=0; i<t->instrument.string_pitch.num; i++) {
		float y = mp->string_to_screen(i);
		p->draw_line({x0, y}, {x0 + w, y});
	}

	// midi
	auto midi = t->layers[0]->midi.get_notes(r_inside);
	mp->draw(p, midi);

	return y0 + string_dy * n;
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

void FormatPdf::draw_beats(Painter *p, float x0, float w, float y, float h, const Range &r) {
	auto beats = song->bars.get_beats(Range(r.offset, r.length + 1), true, false);
	for (auto b: beats) {
		float x = cam->sample2screen(b.range.offset);
		if (b.level == 0) {
			p->set_color(colors->text_soft1);
			p->set_line_width(1);
		} else {
			p->set_color(colors->text_soft3);
			p->set_line_width(0.5f);
		}
		p->draw_line({x, y}, {x, y + h});
	}
	p->set_line_width(0.5f);
}

void FormatPdf::draw_bar_markers(Painter *p, float x0, float w, float y, float h, const Range &r) {
	auto bars = song->bars.get_bars(Range(r.offset, r.length - 50));
	for (auto b: bars){
		float x = cam->sample2screen(b->offset);
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
			p->set_color(colors->text_soft2);
			//p->draw_line(x + 10, y - 65, x - 20, y + 5);
			//p->draw_line(x + 10, y - 65, x + 20, y - 65);
			p->set_font_size(8);
			float dx = 10;
			if (b == bars[0])
				dx += 15;
			p->draw_str({x + dx, y-2.5f}, s);
		}

		// part?
		auto *m = get_bar_part(song, b->offset);
		if (m) {
			p->set_color(colors->text);
			p->draw_line({x - 10, y - 32.5f}, {x - 10, y - 17.5f});
			p->draw_line({x - 10, y - 32.5f}, {x + 10, y - 32.5f});
			p->draw_line({x - 10, y - 17.5f}, {x + 10, y - 17.5f});
			p->set_font_size(10);
			p->draw_str({x - 7.5f, y-30}, m->nice_text());
		}
	}
}

int FormatPdf::draw_line(Painter *p, float x0, float w, float y0, const Range &r, float scale) {
	float track_space = 10;
	p->set_line_width(0.5f);

	cam->pos = r.offset;
	cam->pixels_per_sample = (double)cam->area.width() / (double)r.length;


	auto bars = song->bars.get_bars(r + 1000);
	if (bars.num > 0) {
		p->set_color(colors->text_soft1);
		p->set_font_size(10);
		p->draw_str({x0 + 2.5f, y0 - 7.5f}, i2s(bars[0]->index_text + 1));
	}

	line_data.clear();

	draw_bar_markers(p, x0, w, y0, 50, r);

	foreachi (Track* t, weak(song->tracks), ti) {
		if (t->type != SignalType::MIDI)
			continue;

		Any at;
		for (Any &a: od->parameters["tracks"].as_array())
			if (a["index"]._int() == ti)
				at = a;
		bool allow_classical = at.has("classical");
		bool allow_tab = at.has("tab") and (t->instrument.string_pitch.num > 0);
		if (!allow_classical and !allow_tab)
			continue;

		if (allow_classical)
			y0 = draw_track_classical(p, x0, w, y0, r, t, scale) + track_space;
		if (allow_tab)
			y0 = draw_track_tab(p, x0, w, y0, r, t, scale) + track_space;
		y0 += track_space;
	}

	// line connector
	p->set_line_width(1.5f);
	p->set_color(colors->text_soft1);
	float sy0 = line_data[0].y0;
	float sy1 = line_data.back().y1;
	p->draw_line({x0 - 2.5f, sy0}, {x0 - 2.5f, sy1});
	p->draw_line({x0, sy0 - 5}, {x0 - 2.5f, sy0});
	p->draw_line({x0, sy1 + 5}, {x0 - 2.5f, sy1});
	p->draw_line({x0 + w + 2.5f, sy0}, {x0 + w + 2.5f, sy1});
	p->set_line_width(0.5f);

	return y0;
}

int FormatPdf::good_samples(const Range &r0) {
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

void FormatPdf::save_song(StorageOperationData* _od) {
	od = _od;
	song = od->song;

	float page_width = 1200;
	float page_height = 2000;
	// A4
	page_width = 595.276f;
	page_height = 841.89f;

	pdf::Parser parser;
	parser.set_page_size(page_width, page_height);

	float border = 25;

	cam = new ViewPort(nullptr);
	cam->area = rect(border, page_width - border, 0, page_height);
	SongSelection sel;
	HoverData hover;
	ColorScheme _colors = create_pdf_color_scheme();
	colors = &_colors;
	mp = new MidiPainter(song, cam, &sel, &hover, _colors);

	SymbolRenderer::enable(false);

	float x0 = border;
	float w = page_width - 2*border;

	float avg_scale = 65.0f / od->song->sample_rate * od->parameters["horizontal-scale"]._float();
	float avg_samples_per_line = w / avg_scale;

	int samples = od->song->range_with_time().end();
	//int num_lines =  / samples_per_line + 1;
	float y0 = 70;
	float line_space = 25;

	bool first_page = true;


	auto p = parser.add_page();

	if (first_page) {
		p->set_color(colors->text);
		p->set_font("Times", 26, false, false);
		//p->set_font("Helvetica", 25, false, false);
		p->draw_str(vec2(100, 25), od->song->get_tag("title"));
		if (od->song->get_tag("artist").num > 0) {
			p->set_font("Courier", 15, false, false);
			p->set_font_size(15);
			p->set_color(colors->text_soft2);
			p->draw_str(vec2(p->width - 150, 25), "by " + od->song->get_tag("artist"));
		}
		first_page = false;
	}
	p->set_font("Helvetica", 8, false, false);

	pdf_bpm = 0;

	int offset = 0;
	while (offset < samples) {
		int line_samples = good_samples(Range(offset, avg_samples_per_line));
		float scale = w / line_samples;
		Range r = Range(offset, line_samples);

		float y_prev = y0;
		y0 = draw_line(p, x0, w, y0, r, scale) + line_space;

		offset += line_samples;

		// new page?
		float dy = y0 - y_prev;
		if (y0 + dy > page_height and offset < samples) {
			p = parser.add_page();
			y0 = 50;
		}
	}

	parser.save(od->filename);
	delete cam;
	delete mp;
}
