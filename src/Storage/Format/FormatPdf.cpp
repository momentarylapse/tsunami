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
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Midi/Clef.h"
#include "../../lib/xfile/pdf.h"
#include "../../lib/image/Painter.h"
#include "../../lib/math/rect.h"
#include "../Dialog/PdfConfigDialog.h"
#include <math.h>

static const color NOTE_COLOR = color(1, 0.3f, 0.3f, 0.3f);
static const color NOTE_COLOR_TAB = color(1, 0.8f, 0.8f, 0.8f);

FormatDescriptorPdf::FormatDescriptorPdf() :
	FormatDescriptor(_("Pdf sheet"), "pdf", Flag::MIDI | Flag::MULTITRACK | Flag::WRITE)
{
}

static float clef_pos_to_pdf(float y0, float line_dy, int i)
{
	return y0 + (12-i) * line_dy / 2;
}

static double pdf_bpm;

static int render_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale)
{
	float line_dy = 16;
	float rr = line_dy/2;
	Song *song = t->song;

	int slack = t->song->sample_rate / 30;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	auto clef = t->instrument.get_clef();

	auto midi = t->layers[0]->midi.get_notes(r_inside);
	int mm = 0;
	for (auto n: midi){
		mm = max(mm, n->range.offset);
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		float x2 = x0 + (n->range.end() - r.offset) * scale;
		int pos = n->clef_position;
		float y = clef_pos_to_pdf(y0, line_dy, pos);
		p->set_color(color(1, 0.9f, 0.9f, 0.9f));
		p->draw_rect(rect(x1 + rr, x2, y-rr, y + rr));
	}

	// clef lines
	p->set_color(Gray);
	for (int i=0; i<10; i+=2){
		float y = clef_pos_to_pdf(y0, line_dy, i);
		p->draw_line(x0, y, x0 + w, y);
	}
	//p->draw_str(x0, y0, clef.symbol);

	// beats
	auto beats = song->bars.get_beats(Range(r.offset, r.length + 1), true, false);
	for (auto b: beats){
		float x = x0 + (b.range.offset - r.offset) * scale;
		float ya = clef_pos_to_pdf(y0, line_dy, 8);
		float yb = clef_pos_to_pdf(y0, line_dy, 0);
		if (b.level == 0){
			if (b.bar_no >= 0){
				double bpm = 60.0 * (double)song->sample_rate / (double)b.range.length;
				if (fabs(bpm - pdf_bpm) > 0.2){
					pdf_bpm = bpm;
					p->set_color(color(1, 0.3f, 0.3f, 0.3f));
					p->draw_str(x, ya-15, format("%.1f bpm", bpm));
				}
			}
			p->set_color(color(1, 0.5f, 0.5f, 0.5f));
			p->set_line_width(2);
		}else{
			p->set_color(color(1, 0.8f, 0.8f, 0.8f));
			p->set_line_width(1);
		}
		p->draw_line(x, ya, x, yb);
	}
	p->set_line_width(1);

	// midi
	float fs = 18;
	p->set_font_size(fs);
	auto midi2 = t->layers[0]->midi.get_notes(r_inside);
	for (auto n: midi2){
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		int pos = n->clef_position;
		float y = clef_pos_to_pdf(y0, line_dy, pos);
		p->set_color(NOTE_COLOR);
		p->draw_circle(x1 + rr, y, rr);
		if (n->modifier == NoteModifier::FLAT)
			p->draw_str(x1 - rr, y-fs/2, "b");
		else if (n->modifier == NoteModifier::SHARP)
			p->draw_str(x1 - rr, y-fs/2, "#");
	}

	return y0 + line_dy * 7;
}

static int render_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale)
{
	float string_dy = 16;
	float rr = string_dy/2;
	Song *song = t->song;

	int slack = t->song->sample_rate / 30;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	auto midi = t->layers[0]->midi.get_notes(r_inside);
	int mm = 0;
	for (auto n: midi){
		mm = max(mm, n->range.offset);
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		float x2 = x0 + (n->range.end() - r.offset) * scale;
		float y = y0 + (t->instrument.string_pitch.num - n->stringno - 1) * string_dy;
		p->set_color(color(1, 0.9f, 0.9f, 0.9f));
		p->draw_rect(rect(x1 + rr, x2, y, y + string_dy));
	}

	// string lines
	p->set_color(Gray);
	for (int i=0; i<t->instrument.string_pitch.num; i++)
		p->draw_line(x0, y0 + i*string_dy + string_dy/2, x0 + w, y0 + i*string_dy + string_dy/2);

	// beats
	auto beats = song->bars.get_beats(Range(r.offset, r.length + 1), true, false);
	for (auto b: beats){
		if (b.level == 0){
			p->set_color(color(1, 0.5f, 0.5f, 0.5f));
			p->set_line_width(2);
		}else{
			p->set_color(color(1, 0.8f, 0.8f, 0.8f));
			p->set_line_width(1);
		}
		float x = x0 + (b.range.offset - r.offset) * scale;
		p->draw_line(x, y0, x, y0 + string_dy * t->instrument.string_pitch.num);
	}
	p->set_line_width(1);

	// midi
	float fs = 12;
	p->set_font_size(fs);
	auto midi2 = t->layers[0]->midi.get_notes(r_inside);
	for (auto n: midi2){
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		float y = y0 + (t->instrument.string_pitch.num - n->stringno - 0.5f) * string_dy;
		p->set_color(NOTE_COLOR_TAB);
		p->draw_circle(x1 + rr, y, rr);
		p->set_color(Black);
		string hand = i2s(n->pitch - t->instrument.string_pitch[n->stringno]);
		p->draw_str(x1 + rr - hand.num * fs*0.35f, y-fs/2, hand);
	}

	return y0 + string_dy * t->instrument.string_pitch.num;
}

static int render_line(Painter *p, float x0, float w, float y0, const Range &r, Song *song, float scale, PdfConfigData *data)
{
	float track_space = 20;

	auto bars = song->bars.get_bars(r + 1000);
	p->set_color(SetColorHSB(1, 0, 0, 0.4f));
	p->set_font_size(12);
	if (bars.num > 0)
		p->draw_str(x0 + 5, y0 - 15, i2s(bars[0]->index_text + 1));

	foreachi (Track* t, song->tracks, ti){
		if (t->type != SignalType::MIDI)
			continue;

		bool allow_classical = (data->track_mode[ti] & 1);
		bool allow_tab = (data->track_mode[ti] & 2);
		if (allow_classical)
			y0 = render_track_classical(p, x0, w, y0, r, t, scale) + track_space;
		if (t->instrument.string_pitch.num > 0 and allow_tab)
			y0 = render_track_tab(p, x0, w, y0, r, t, scale) + track_space;
		y0 += track_space;
	}

	return y0;
}

static int good_samples(Song *song, const Range &r0)
{
	auto bars = song->bars.get_bars(Range(r0.offset, r0.length * 2));
	int best_pos = -1;
	for (auto b: bars){
		if (b->range().offset <= r0.offset)
			continue;
		if (abs(b->range().offset - r0.end()) < abs(best_pos - r0.end())){
			best_pos = b->range().offset;
		}
	}

	if (best_pos < 0)
		return r0.length;
	return best_pos - r0.offset;
}

void FormatPdf::save_song(StorageOperationData* od)
{
	PdfConfigData data;
	auto *dlg = new PdfConfigDialog(&data, od->song, od->win);
	dlg->run();
	delete dlg;

	float page_width = 1200;
	float page_height = 2000;

	auto parser = pdf::save(od->filename);

	float border = 50;

	float x0 = border;
	float w = page_width - 2*border;

	float avg_scale = 100.0f / od->song->sample_rate * data.horizontal_scale;
	float avg_samples_per_line = w / avg_scale;

	int samples = od->song->range_with_time().end();
	//int num_lines =  / samples_per_line + 1;
	float y0 = 140;
	float line_space = 50;

	bool first_page = true;


	auto p = parser->add_page(page_width, page_height);

	if (first_page){
		p->set_font_size(40);
		p->draw_str(200, 50, od->song->get_tag("title"));
		p->set_font_size(15);
		p->set_color(SetColorHSB(1, 0, 0, 0.4f));
		p->draw_str(p->width - 300, 100, "by " + od->song->get_tag("artist"));
		first_page = false;
	}

	pdf_bpm = 0;

	int offset = 0;
	while (offset < samples){
		int line_samples = good_samples(od->song, Range(offset, avg_samples_per_line));
		float scale = w / line_samples;
		Range r = Range(offset, line_samples);

		float y_prev = y0;
		y0 = render_line(p, x0, w, y0, r, od->song, scale, &data) + line_space;

		offset += line_samples;

		// new page?
		float dy = y0 - y_prev;
		if (y0 + dy > page_height and offset < samples){
			delete p;
			p = parser->add_page(page_width, page_height);
			y0 = 100;
		}
	}

	delete p;
	delete parser;
}
