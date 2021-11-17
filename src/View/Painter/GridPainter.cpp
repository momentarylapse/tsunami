/*
 * GridPainter.cpp
 *
 *  Created on: 12.11.2018
 *      Author: michi
 */

#include "GridPainter.h"

#include "../AudioView.h"
#include "../ColorScheme.h"
#include "../ViewPort.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"
#include "../../Data/Rhythm/Beat.h"
#include "../Graph/AudioViewTrack.h"
#include "../Helper/Drawing.h"


color col_inter(const color a, const color &b, float t);


GridPainter::GridPainter(Song *_song, ViewPort *_cam, SongSelection *_sel, HoverData *_hover, ColorScheme &_scheme) :
	 local_theme(_scheme) {
	sel = _sel;
	if (!sel)
		sel = new SongSelection;
	hover = _hover;
	if (!hover)
		hover = new HoverData;
	cam = _cam;
	song = _song;
}

void GridPainter::__init__(Song *_song, ViewPort *_cam, SongSelection *_sel, HoverData *_hover, ColorScheme &_scheme) {
	new(this) GridPainter(_song, _cam, _sel, _hover, _scheme);
}



void GridPainter::draw_empty_background(Painter *c) {
	if (sel->range().length > 0) {
		c->set_color(colors.bg);
		c->draw_rect(area);

		float x1, x2;
		cam->range2screen_clip(sel->range(), area, x1, x2);
		c->set_color(colors.bg_sel);
		c->draw_rect(rect(x1, x2, area.y1, area.y2));

	} else {
		c->set_color(colors.bg);
		c->draw_rect(area);
	}
}

void GridPainter::draw_time(Painter *c) {
	double dl = AudioViewTrack::MIN_GRID_DIST / cam->pixels_per_sample; // >= 10 pixel
	double dt = dl / song->sample_rate;
	double ldt = log10(dt);
	double factor = 1;
	if (ldt > 1.5)
		factor = 1.0/0.6/0.60000001;
	else if (ldt > 0)
		factor = 1.0/0.600000001;
	ldt += log10(factor);
	double exp_s = ceil(ldt);
	double exp_s_mod = exp_s - ldt;
	dt = pow(10, exp_s) / factor;
	dl = dt * song->sample_rate;
//	double dw = dl * a->view_zoom;
	int nx0 = ceil(cam->screen2sample(area.x1) / dl);
	int nx1 = ceil(cam->screen2sample(area.x2) / dl);
	color c1 = col_inter(colors.bg, colors.fg, exp_s_mod * 0.8f);
	color c2 = col_inter(colors.bg, colors.fg, 0.8f);
	color c1s = col_inter(colors.bg_sel, colors.fg, exp_s_mod * 0.8f);
	color c2s = col_inter(colors.bg_sel, colors.fg_sel, 0.8f);

	for (int n=nx0; n<nx1; n++) {
		double sample = n * dl;
		if (sel->range().is_inside(sample))
			c->set_color(((n % 10) == 0) ? c2s : c1s);
		else
			c->set_color(((n % 10) == 0) ? c2 : c1);
		float xx = cam->sample2screen(sample);
		c->draw_line({xx, area.y1}, {xx, area.y2});
	}
}

void GridPainter::draw_time_numbers(Painter *c) {
	c->set_font("", local_theme.FONT_SIZE, false, false);
	double dl = AudioViewTrack::MIN_GRID_DIST / cam->pixels_per_sample; // >= 10 pixel
	double dt = dl / song->sample_rate;
	double ldt = log10(dt);
	double factor = 1;
	if (ldt > 1.5)
		factor = 1.0/0.6/0.60000001;
	else if (ldt > 0)
		factor = 1.0/0.600000001;
	ldt += log10(factor);
	double exp_s = ceil(ldt);
//	double exp_s_mod = exp_s - ldt;
	dt = pow(10, exp_s) / factor;
	dl = dt * song->sample_rate;
//	double dw = dl * a->view_zoom;
	int nx0 = ceil(cam->screen2sample(area.x1) / dl);
	int nx1 = ceil(cam->screen2sample(area.x2) / dl);

	c->set_color(local_theme.text_soft3);//grid);
	for (int n=nx0; n<nx1; n++) {
		if ((cam->sample2screen(dl) - cam->sample2screen(0)) > 25) {
			if (n % 5 == 0)
				c->draw_str(vec2(cam->sample2screen(n * dl) + 2, area.y1 + 4), song->get_time_str_fuzzy((double)n * dl, dt * 5));
		} else {
			if ((n % 10) == 0)
				c->draw_str(vec2(cam->sample2screen(n * dl) + 2, area.y1 + 4), song->get_time_str_fuzzy((double)n * dl, dt * 10));
		}
	}
}


void GridPainter::draw_bars(Painter *c, int beat_partition) {
	if (song->bars.num == 0)
		return;
	int s0 = cam->screen2sample(area.x1 - 1);
	int s1 = cam->screen2sample(area.x2);

	auto bars = song->bars.get_bars(RangeTo(s0, s1));
	for (Bar *b: bars) {
		if (b->is_pause())
			continue;
		float xx = cam->sample2screen(b->range().offset);

		float dx_bar = cam->dsample2screen(b->range().length);
		float dx_beat = dx_bar / b->beats.num;
		float f1 = dx_bar / 50.0f;
		if ((b->index_text % 5) == 0)
			f1 *= 5;
		if ((b->index_text % 25) == 0)
			f1 *= 5;
		f1 = min(1.0f, f1);
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f) {
			if (sel->range().is_inside(b->range().offset))
				c->set_color(col_inter(colors.bg_sel, colors.fg_sel, f1));
			else
				c->set_color(col_inter(colors.bg, colors.fg, f1));
			c->draw_line({xx, area.y1}, {xx, area.y2});


			// end of last bar
			if (b == bars.back()) {
				if (sel->range().is_inside(b->range().end()))
					c->set_color(col_inter(colors.bg_sel, colors.fg_sel, f1));
				else
					c->set_color(col_inter(colors.bg, colors.fg, f1));
				float x = cam->sample2screen(b->range().end());
				c->draw_line({x, area.y1}, {x, area.y2});
			}
		}

		if (f2 >= 0.1f) {
			color c1 = col_inter(colors.bg, colors.fg, f2*0.5f);
			color c1s = col_inter(colors.bg_sel, colors.fg_sel, f2*0.5f);
			color c2 = col_inter(colors.bg, c1, 0.6f);
			color c2s = col_inter(colors.bg_sel, c1s, 0.6f);

			auto beats = b->get_beats(b->offset, beat_partition > 0, beat_partition);
			for (Beat &bb: beats) {
				if (bb.level == 0)
					continue;
				float x = cam->sample2screen(bb.range.offset);
				if (bb.level > 1)
					c->set_color(sel->range().is_inside(bb.range.offset) ? c2s : c2);
				else
					c->set_color(sel->range().is_inside(bb.range.offset) ? c1s : c1);
				c->draw_line({x, area.y1}, {x, area.y2});
			}
		}
	}
}

void GridPainter::draw_bar_numbers(Painter *c) {
	if (song->bars.num == 0)
		return;
	BarPattern prev;
	float prev_bpm = 0;
	int s0 = cam->screen2sample(area.x1 - 1);
	int s1 = cam->screen2sample(area.x2);
	auto bars = song->bars.get_bars(RangeTo(s0, s1));

	c->set_font("", local_theme.FONT_SIZE, true, false);
	float change_block_until = -1;
	bool block_reported = false;
	for (Bar *b: bars) {
		float xx = cam->sample2screen(b->range().offset);
		float dx_bar = cam->dsample2screen(b->range().length);

		color halo_col = color(0,0,0,0);
		if (sel->has(b))
			halo_col = local_theme.blob_bg_selected;
		if (hover->bar == b) {
			if (halo_col.a == 0)
				halo_col = local_theme.blob_bg;
			halo_col = local_theme.hoverify(halo_col);
		}

		string label = " ";
		if (!b->is_pause())
			label = i2s(b->index_text + 1);

		if (halo_col.a > 0) {
			draw_boxed_str(c, {xx + 4, area.y1+5}, label, local_theme.text, halo_col);
		} else {
			float f1 = min(1.0f, dx_bar / 60.0f);
			if ((b->index_text % 5) == 0)
				f1 *= 5;
			if ((b->index_text % 25) == 0)
				f1 *= 5;
			if (f1 > 0.9f){
				c->set_color(local_theme.text_soft1);
				c->draw_str({xx + 4, area.y1+5}, label);
			}
		}

		if (b->is_pause())
			continue;

		// info label (betas, bpm)
		float bpm = b->bpm(song->sample_rate);
		string s;
		if ((prev.beats != b->beats) or (prev.divisor != b->divisor)) {
			s = b->format_beats();
			prev = *b;
		}
		if (fabs(prev_bpm - bpm) > 1.5f) {
			s += format(u8" \u2669=%.0f", bpm);
			prev_bpm = bpm;
		}
		if (s.num > 0) {
			c->set_font("", local_theme.FONT_SIZE_SMALL, false, false);
			c->set_color(local_theme.text_soft1);
			float xxx = max(xx + 4, 20.0f);
			if (xxx > change_block_until) {
				c->draw_str({xxx, area.y2 - 15}, s);
				change_block_until = xxx + c->get_str_width(s);
				block_reported = false;
			} else if (!block_reported) {
				c->draw_str({change_block_until, area.y2 - 15}, "...!");
				change_block_until += c->get_str_width("...!");
				block_reported = true;
				
			}
			c->set_font("", local_theme.FONT_SIZE, true, false);
		}
	}
	c->set_font("", local_theme.FONT_SIZE, false, false);
	//c->setLineDash(no_dash, 0);
	c->set_line_width(local_theme.LINE_WIDTH);
}

void GridPainter::draw_whatever(Painter *c, int beat_partition) {
	if (song->bars.num > 0)
		draw_bars(c, beat_partition);
	else
		draw_time(c);
}
