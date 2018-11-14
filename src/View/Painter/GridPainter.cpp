/*
 * GridPainter.cpp
 *
 *  Created on: 12.11.2018
 *      Author: michi
 */

#include "GridPainter.h"

#include "../AudioView.h"
#include "../AudioViewTrack.h"
#include "../ViewPort.h"
#include "../../Data/Song.h"
#include "../../Data/Rhythm/Bar.h"


color col_inter(const color a, const color &b, float t);
string i2s_small(int i);


GridPainter::GridPainter(AudioView *_view)
{
	view = _view;
	cam = &view->cam;
	song = view->song;
}


void GridPainter::draw_time(Painter *c)
{
	double dl = AudioViewTrack::MIN_GRID_DIST / cam->scale; // >= 10 pixel
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

	for (int n=nx0; n<nx1; n++){
		double sample = n * dl;
		if (view->sel.range.is_inside(sample))
			c->set_color(((n % 10) == 0) ? c2s : c1s);
		else
			c->set_color(((n % 10) == 0) ? c2 : c1);
		int xx = cam->sample2screen(sample);
		c->draw_line(xx, area.y1, xx, area.y2);
	}
}

void GridPainter::draw_time_numbers(Painter *c)
{
	double dl = AudioViewTrack::MIN_GRID_DIST / cam->scale; // >= 10 pixel
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

	c->set_color(AudioView::colors.grid);
	for (int n=nx0; n<nx1; n++){
		if ((cam->sample2screen(dl) - cam->sample2screen(0)) > 25){
			if (n % 5 == 0)
				c->draw_str(cam->sample2screen(n * dl) + 2, area.y1, song->get_time_str_fuzzy((double)n * dl, dt * 5));
		}else{
			if ((n % 10) == 0)
				c->draw_str(cam->sample2screen(n * dl) + 2, area.y1, song->get_time_str_fuzzy((double)n * dl, dt * 10));
		}
	}
}


void GridPainter::draw_bars(Painter *c, int beat_partition)
{
	if (song->bars.num == 0)
		return;
	int prev_num_beats = 0;
	float prev_bpm = 0;
	int s0 = cam->screen2sample(area.x1 - 1);
	int s1 = cam->screen2sample(area.x2);
	//c->SetLineWidth(2.0f);
	Array<float> dash = {5,4}, no_dash;


	//color c1 = ColorInterpolate(bg, colors.grid, exp_s_mod);
	//color c2 = colors.grid;

	//Array<Beat> beats = t->bar.GetBeats(Range(s0, s1 - s0));
	Array<Bar*> bars = song->bars.get_bars(RangeTo(s0, s1));
	for (Bar *b: bars){
		if (b->is_pause())
			continue;
		int xx = cam->sample2screen(b->range().offset);

		float dx_bar = cam->dsample2screen(b->range().length);
		float dx_beat = dx_bar / b->num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b->index_text % 5) == 0)
			f1 = 1;
		float f2 = min(1.0f, dx_beat / 25.0f);

		if (f1 >= 0.1f){
			if (view->sel.range.is_inside(b->range().offset))
				c->set_color(col_inter(colors.bg_sel, colors.fg_sel, f1));
			else
				c->set_color(col_inter(colors.bg, colors.fg, f1));
//			c->setLineDash(no_dash, area.y1);
			c->draw_line(xx, area.y1, xx, area.y2);
		}

		if (f2 >= 0.1f){
			color c1 = col_inter(colors.bg, colors.fg, f2*0.5f);
			color c1s = col_inter(colors.bg_sel, colors.fg_sel, f2*0.5f);
			float beat_length = (float)b->range().length / (float)b->num_beats;
//			c->setLineDash(dash, area.y1);
			for (int i=0; i<b->num_beats; i++){
				float beat_offset = b->range().offset + (float)i * beat_length;
				color c2 = col_inter(colors.bg, c1, 0.6f);
				color c2s = col_inter(colors.bg_sel, c1s, 0.6f);
				//c->setColor(c2);
				for (int j=1; j<beat_partition; j++){
					double sample = beat_offset + beat_length * j / beat_partition;
					int x = cam->sample2screen(sample);
					c->set_color(view->sel.range.is_inside(sample) ? c2s : c2);
					c->draw_line(x, area.y1, x, area.y2);
				}
				if (i == 0)
					continue;
				c->set_color(view->sel.range.is_inside(beat_offset) ? c1s : c1);
				int x = cam->sample2screen(beat_offset);
				c->draw_line(x, area.y1, x, area.y2);
			}
		}
	}
}

void GridPainter::draw_bar_numbers(Painter *c)
{
	if (song->bars.num == 0)
		return;
	int prev_num_beats = 0;
	float prev_bpm = 0;
	int s0 = cam->screen2sample(area.x1 - 1);
	int s1 = cam->screen2sample(area.x2);
	Array<Bar*> bars = song->bars.get_bars(RangeTo(s0, s1));

	c->set_font("", AudioView::FONT_SIZE, true, false);
	for (Bar *b: bars){
		if (b->is_pause())
			continue;
		int xx = cam->sample2screen(b->range().offset);

		float dx_bar = cam->dsample2screen(b->range().length);
		float dx_beat = dx_bar / b->num_beats;
		float f1 = min(1.0f, dx_bar / 40.0f);
		if ((b->index_text % 5) == 0)
			f1 = 1;
		if (f1 > 0.9f){
			c->set_color(AudioView::colors.text_soft1);
			c->draw_str(xx + 4, area.y1, i2s(b->index_text + 1));
		}
		float bpm = b->bpm(song->sample_rate);
		string s;
		if (prev_num_beats != b->num_beats)
			s = i2s(b->num_beats) + "/" + i2s_small(4);
		if (fabs(prev_bpm - bpm) > 0.5f)
			s += format(" \u2669=%.0f", bpm);
		if (s.num > 0){
			c->set_color(AudioView::colors.text_soft1);
			c->draw_str(max(xx + 4, 20), area.y2 - 16, s);
		}
		prev_num_beats = b->num_beats;
		prev_bpm = bpm;
	}
	c->set_font("", AudioView::FONT_SIZE, false, false);
	//c->setLineDash(no_dash, 0);
	c->set_line_width(AudioView::LINE_WIDTH);
}

void GridPainter::draw_whatever(Painter *c, int beat_partition)
{
	if (song->bars.num > 0)
		draw_bars(c, beat_partition);
	else
		draw_time(c);
}

void GridPainter::set_context(const rect& _area, const GridColors& c)
{
	area = _area;
	colors = c;
}
