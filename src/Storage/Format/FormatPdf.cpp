/*
 * FormatPdf.cpp
 *
 *  Created on: 22.04.2018
 *      Author: michi
 */

#include "FormatPdf.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../../Data/Midi/Clef.h"
#include "../../lib/xfile/pdf.h"
#include <math.h>

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

	auto midi = t->layers[0]->midi.getNotes(r_inside);
	int mm = 0;
	for (auto n: midi){
		mm = max(mm, n->range.offset);
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		float x2 = x0 + (n->range.end() - r.offset) * scale;
		int pos = n->clef_position;
		float y = clef_pos_to_pdf(y0, line_dy, pos);
		p->setColor(color(1, 0.9f, 0.9f, 0.9f));
		p->drawRect(rect(x1 + rr, x2, y-rr, y + rr));
	}

	// clef lines
	p->setColor(Gray);
	for (int i=0; i<10; i+=2){
		float y = clef_pos_to_pdf(y0, line_dy, i);
		p->drawLine(x0, y, x0 + w, y);
	}
	//p->drawStr(x0, y0, clef.symbol);

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
					p->setColor(color(1, 0.3f, 0.3f, 0.3f));
					p->drawStr(x, ya-15, format("%.1f bpm", bpm));
				}
			}
			p->setColor(color(1, 0.5f, 0.5f, 0.5f));
			p->setLineWidth(2);
		}else{
			p->setColor(color(1, 0.8f, 0.8f, 0.8f));
			p->setLineWidth(1);
		}
		p->drawLine(x, ya, x, yb);
	}
	p->setLineWidth(1);

	// midi
	float fs = 12;
	p->setFontSize(fs);
	auto midi2 = t->layers[0]->midi.getNotes(r_inside);
	for (auto n: midi2){
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		int pos = n->clef_position;
		float y = clef_pos_to_pdf(y0, line_dy, pos);
		p->setColor(color(1, 0.8f, 0.8f, 0.8f));
		p->drawCircle(x1 + rr, y, rr);
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

	auto midi = t->layers[0]->midi.getNotes(r_inside);
	int mm = 0;
	for (auto n: midi){
		mm = max(mm, n->range.offset);
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		float x2 = x0 + (n->range.end() - r.offset) * scale;
		float y = y0 + (t->instrument.string_pitch.num - n->stringno - 1) * string_dy;
		p->setColor(color(1, 0.9f, 0.9f, 0.9f));
		p->drawRect(rect(x1 + rr, x2, y, y + string_dy));
	}

	// string lines
	p->setColor(Gray);
	for (int i=0; i<t->instrument.string_pitch.num; i++)
		p->drawLine(x0, y0 + i*string_dy + string_dy/2, x0 + w, y0 + i*string_dy + string_dy/2);

	// beats
	auto beats = song->bars.get_beats(Range(r.offset, r.length + 1), true, false);
	for (auto b: beats){
		if (b.level == 0){
			p->setColor(color(1, 0.5f, 0.5f, 0.5f));
			p->setLineWidth(2);
		}else{
			p->setColor(color(1, 0.8f, 0.8f, 0.8f));
			p->setLineWidth(1);
		}
		float x = x0 + (b.range.offset - r.offset) * scale;
		p->drawLine(x, y0, x, y0 + string_dy * t->instrument.string_pitch.num);
	}
	p->setLineWidth(1);

	// midi
	float fs = 12;
	p->setFontSize(fs);
	auto midi2 = t->layers[0]->midi.getNotes(r_inside);
	for (auto n: midi2){
		float x1 = x0 + (n->range.offset - r.offset) * scale;
		float y = y0 + (t->instrument.string_pitch.num - n->stringno - 0.5f) * string_dy;
		p->setColor(color(1, 0.8f, 0.8f, 0.8f));
		p->drawCircle(x1 + rr, y, rr);
		p->setColor(Black);
		string hand = i2s(n->pitch - t->instrument.string_pitch[n->stringno]);
		p->drawStr(x1 + rr - hand.num * fs*0.35f, y-fs/2, hand);
	}

	return y0 + string_dy * t->instrument.string_pitch.num;
}

static int render_line(Painter *p, float x0, float w, float y0, const Range &r, Song *song, float scale)
{
	float track_space = 20;

	auto bars = song->bars.get_bars(r + 1000);
	p->setColor(SetColorHSB(1, 0, 0, 0.4f));
	p->setFontSize(12);
	if (bars.num > 0)
		p->drawStr(x0 + 5, y0 - 15, i2s(bars[0]->index_text + 1));

	for (Track* t : song->tracks){
		if (t->type != t->Type::MIDI)
			continue;

		y0 = render_track_classical(p, x0, w, y0, r, t, scale) + track_space;
		if (t->instrument.string_pitch.num > 0)
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

void FormatPdf::saveSong(StorageOperationData* od)
{
	auto parser = pdf::save(od->filename);
	auto p = parser->add_page(1200, 2000);

	float border = 50;

	float x0 = border;
	float w = p->width - 2*border;

	float avg_scale = 100.0f / od->song->sample_rate;
	float avg_samples_per_line = w / avg_scale;

	int samples = od->song->getRangeWithTime().end();
	//int num_lines =  / samples_per_line + 1;
	float y0 = 140;
	float line_space = 50;


	p->setFontSize(40);
	p->drawStr(200, 50, od->song->getTag("title"));
	p->setFontSize(15);
	p->setColor(SetColorHSB(1, 0, 0, 0.4f));
	p->drawStr(p->width - 300, 100, "by " + od->song->getTag("artist"));

	pdf_bpm = 0;

	int offset = 0;
	while (offset < samples){
		int line_samples = good_samples(od->song, Range(offset, avg_samples_per_line));
		float scale = w / line_samples;
		Range r = Range(offset, line_samples);


		y0 = render_line(p, x0, w, y0, r, od->song, scale) + line_space;

		offset += line_samples;
	}

	delete p;
	delete parser;
}
