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
#include "../../Data/SongSelection.h"
#include "../../lib/xfile/pdf.h"
#include "../../lib/image/Painter.h"
#include "../../lib/math/rect.h"
#include "../Dialog/PdfConfigDialog.h"
#include "../../View/ViewPort.h"
#include "../../View/Painter/MidiPainter.h"
#include "../../View/Selection.h"
#include "../../View/ColorScheme.h"
#include "../../View/Helper/SymbolRenderer.h"
#include <math.h>

static const color NOTE_COLOR = color(1, 0.3f, 0.3f, 0.3f);
static const color NOTE_COLOR_TAB = color(1, 0.8f, 0.8f, 0.8f);

FormatDescriptorPdf::FormatDescriptorPdf() :
	FormatDescriptor(_("Pdf sheet"), "pdf", Flag::MIDI | Flag::MULTITRACK | Flag::WRITE)
{
}

ColorScheme create_pdf_color_scheme()
{
	ColorSchemeBasic bright;
	bright.background = White;
	bright.text = Black;//color(1, 0.3f, 0.3f, 0.1f);
	bright.selection = color(1, 0.2f, 0.2f, 0.7f);
	bright.hover = White;
	bright.gamma = 1.0f;
	bright.name = "pdf";
	return bright.create(true);
}

FormatPdf::LineData::LineData(Track *t, float _y0, float _y1)
{
	y0 = _y0;
	y1 = _y1;
	track = t;
}

int FormatPdf::draw_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale)
{
	int slack = song->sample_rate / 30;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	mp->set_context(rect(x0, x0+w, y0-50, y0+180), t->instrument, Scale(Scale::Type::MAJOR, 0), true, MidiMode::CLASSICAL);

	float ya = mp->clef_pos_to_screen(8);
	float yb = mp->clef_pos_to_screen(0);
	draw_beats(p, x0, w, ya, yb-ya, r);
	line_data.add(LineData(t, ya, yb));

	auto clef = t->instrument.get_clef();

	// clef lines
	p->set_color(colors->text_soft1);
	for (int i=0; i<10; i+=2){
		float y = mp->clef_pos_to_screen(i);
		p->draw_line(x0, y, x0 + w, y);
	}
	//p->draw_str(x0, y0, clef.symbol);

	// midi
	auto midi = t->layers[0]->midi.get_notes(r_inside);
	mp->draw(p, midi);

	return y0 + 100;
}

int FormatPdf::draw_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale)
{
	float string_dy = 26;

	int slack = song->sample_rate / 30;
	Range r_inside = Range(r.offset + slack, r.length - slack * 2);

	int n = t->instrument.string_pitch.num;

	mp->set_context(rect(x0, x0+w, y0, y0+string_dy*n), t->instrument, Scale(Scale::Type::MAJOR, 0), true, MidiMode::TAB);

	float sy0 = mp->string_to_screen(n - 1) - string_dy/2;
	float sy1 = mp->string_to_screen(0) + string_dy/2;
	line_data.add(LineData(t, sy0, sy1));

	draw_beats(p, x0, w, sy0, sy1 - sy0, r);

	// string lines
	p->set_color(colors->text_soft1);
	for (int i=0; i<t->instrument.string_pitch.num; i++){
		float y = mp->string_to_screen(i);
		p->draw_line(x0, y, x0 + w, y);
	}

	// midi
	auto midi = t->layers[0]->midi.get_notes(r_inside);
	mp->draw(p, midi);

	return y0 + string_dy * n;
}

void FormatPdf::draw_beats(Painter *p, float x0, float w, float y, float h, const Range &r)
{
	auto beats = song->bars.get_beats(Range(r.offset, r.length + 1), true, false);
	for (auto b: beats){
		float x = cam->sample2screen(b.range.offset);
		if (b.level == 0){
			if (b.bar_no >= 0){
				double bpm = 60.0 * (double)song->sample_rate / (double)b.range.length;
				if (fabs(bpm - pdf_bpm) > 0.2){
					pdf_bpm = bpm;
					p->set_color(colors->text_soft2);
					p->set_font_size(15);
					p->draw_str(x + 15, y-20, format("%.0f bpm", bpm));
				}
			}
			p->set_color(colors->text_soft1);
			p->set_line_width(2);
		}else{
			p->set_color(colors->text_soft3);
			p->set_line_width(1);
		}
		p->draw_line(x, y, x, y + h);
	}
	p->set_line_width(1);
}

int FormatPdf::draw_line(Painter *p, float x0, float w, float y0, const Range &r, float scale, PdfConfigData *data)
{
	float track_space = 20;

	cam->pos = r.offset;
	cam->scale = (double)cam->area.width() / (double)r.length;


	auto bars = song->bars.get_bars(r + 1000);
	if (bars.num > 0){
		p->set_color(colors->text_soft1);
		p->set_font_size(20);
		p->draw_str(x0 + 5, y0 - 15, i2s(bars[0]->index_text + 1));
	}

	line_data.clear();

	foreachi (Track* t, song->tracks, ti){
		if (t->type != SignalType::MIDI)
			continue;

		bool allow_classical = (data->track_mode[ti] & 1);
		bool allow_tab = (data->track_mode[ti] & 2) and (t->instrument.string_pitch.num > 0);
		if (!allow_classical and !allow_tab)
			continue;

		if (allow_classical)
			y0 = draw_track_classical(p, x0, w, y0, r, t, scale) + track_space;
		if (allow_tab)
			y0 = draw_track_tab(p, x0, w, y0, r, t, scale) + track_space;
		y0 += track_space;
	}

	// line connector
	p->set_line_width(3);
	p->set_color(colors->text_soft1);
	float sy0 = line_data[0].y0;
	float sy1 = line_data.back().y1;
	p->draw_line(x0 - 5, sy0, x0 - 5, sy1);
	p->draw_line(x0, sy0 - 10, x0 - 5, sy0);
	p->draw_line(x0, sy1 + 10, x0 - 5, sy1);
	p->draw_line(x0 + w + 5, sy0, x0 + w + 5, sy1);
	p->set_line_width(1);

	return y0;
}

int FormatPdf::good_samples(const Range &r0)
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
	song = od->song;
	auto *dlg = new PdfConfigDialog(&data, od->song, od->win);
	dlg->run();
	delete dlg;

	float page_width = 1200;
	float page_height = 2000;

	auto parser = pdf::save(od->filename);

	float border = 50;

	rect area = rect(border, page_width - border, 0, page_height);
	cam = new ViewPort(song, area);
	SongSelection sel;
	Selection hover;
	ColorScheme _colors = create_pdf_color_scheme();
	colors = &_colors;
	mp = new MidiPainter(song, cam, &sel, &hover, _colors);

	SymbolRenderer::enable(false);

	float x0 = border;
	float w = page_width - 2*border;

	float avg_scale = 130.0f / od->song->sample_rate * data.horizontal_scale;
	float avg_samples_per_line = w / avg_scale;

	int samples = od->song->range_with_time().end();
	//int num_lines =  / samples_per_line + 1;
	float y0 = 140;
	float line_space = 50;

	bool first_page = true;


	auto p = parser->add_page(page_width, page_height);

	if (first_page){
		p->set_color(colors->text);
		p->set_font("Times", 50, false, false);
		//p->set_font("Helvetica", 50, false, false);
		p->draw_str(200, 50, od->song->get_tag("title"));
		if (od->song->get_tag("artist").num > 0){
			p->set_font("Courier", 15, false, false);
			p->set_font_size(15);
			p->set_color(colors->text_soft2);
			p->draw_str(p->width - 300, 50, "by " + od->song->get_tag("artist"));
		}
		first_page = false;
	}
	p->set_font("Helvetica", 15, false, false);

	pdf_bpm = 0;

	int offset = 0;
	while (offset < samples){
		int line_samples = good_samples(Range(offset, avg_samples_per_line));
		float scale = w / line_samples;
		Range r = Range(offset, line_samples);

		float y_prev = y0;
		y0 = draw_line(p, x0, w, y0, r, scale, &data) + line_space;

		offset += line_samples;

		// new page?
		float dy = y0 - y_prev;
		if (y0 + dy > page_height and offset < samples){
			delete p;
			p = parser->add_page(page_width, page_height);
			y0 = 100;
		}
	}

	delete cam;
	delete mp;
	delete p;
	delete parser;
}
