/*
 * BufferPainter.cpp
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#include "BufferPainter.h"
#include "../AudioView.h"
#include "../../lib/math/complex.h"
#include "../../lib/image/Painter.h"
#include "../../Data/Range.h"
#include "../../Data/Audio/AudioBuffer.h"

BufferPainter::BufferPainter(AudioView *_view)
{
	view = _view;
	area = rect(0,0,0,0);
	x0 = x1 = 0;
}



static Array<complex> tt;

inline void draw_line_buffer(Painter *c, double view_pos, double zoom, float hf, float x0, float x1, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	int i0 = max((double)x0 / zoom + view_pos - offset    , 0.0);
	int i1 = min((double)x1 / zoom + view_pos - offset + 2, (double)buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);

	for (int i=i0; i<i1; i++){

		double p = x0 + ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 + buf[i] * hf;
		if (zoom > 5)
			c->draw_circle(p, tt[nl].y, 2);
		nl ++;
	}
	c->draw_lines(tt);
}

inline void draw_line_buffer_sel(Painter *c, double view_pos, double zoom, float hf, float x0, float x1, float y0, const Array<float> &buf, int offset)
{
	int nl = 0;
	int i0 = max((double)x0 / zoom + view_pos - offset    , 0.0);
	int i1 = min((double)x1 / zoom + view_pos - offset + 2, (double)buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);
	c->set_line_width(3.0f);

	for (int i=i0; i<i1; i++){

		double p = x0 + ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 + buf[i] * hf;
		if (zoom > 5)
			c->draw_circle(p, tt[nl].y, 4);
		nl ++;
	}
	tt.resize(nl);
	c->draw_lines(tt);
	c->set_line_width(1.0f);
}

inline void draw_peak_buffer(Painter *c, int di, double view_pos_rel, double zoom, float f, float hf, float x0, float x1, float y0, const string &buf, int offset)
{
	if ((x1 - x0) / di < 1)
		return;
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize((int)((x1 - x0)/di + 10));
	for (int i=x0; i<x1+di; i+=di){

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) and (ip < buf.num))
			if (((int)(p) < offset + buf.num*f) and (p >= offset)){
				tt[nl].x = i;
				float dy = ((float)(buf[ip])/255.0f) * hf;
				tt[nl].y  = y0 - dy;
				nl ++;
			}
	}
	if (nl == 0)
		return;
	tt.resize(nl * 2);
	for (int i=0; i<nl; i++){
		tt[nl + i].x = tt[nl - i - 1].x;
		tt[nl + i].y = y0 *2 - tt[nl - i - 1].y + 1;
	}
	c->draw_polygon(tt);
}

inline void draw_peak_buffer_sel(Painter *c, int di, double view_pos_rel, double zoom, float f, float hf, float x0, float x1, float y0, const string &buf, int offset)
{
	if ((x1 - x0) / di < 1)
		return;
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize((int)((x1 - x0)/di + 10));
	for (int i=x0; i<x1+di; i+=di){

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) and (ip < buf.num))
			if (((int)(p) < offset + buf.num*f) and (p >= offset)){
				tt[nl].x = i;
				float dy = ((float)(buf[ip])/255.0f) * hf;
				tt[nl].y  = y0 - dy - 1;
				nl ++;
			}
	}
	if (nl == 0)
		return;
	tt.resize(nl * 2);
	for (int i=0; i<nl; i++){
		tt[nl + i].x = tt[nl - i - 1].x;
		tt[nl + i].y = y0 *2 - tt[nl - i - 1].y + 1;
	}
	c->draw_polygon(tt);
}

void BufferPainter::draw_buffer(Painter *c, AudioBuffer &b, int offset)
{
	double view_pos_rel = view->cam.screen2sample(0);
	c->set_antialiasing(view->antialiasing);

	//float w = area.width();
	float h = area.height();
	float hf = h / (2 * b.channels);

	// zero heights of both channels
	float y0[2];
	for (int ci=0; ci<b.channels; ci++)
		y0[ci] = area.y1 + hf * (2*ci + 1);


	int di = view->detail_steps;
	c->set_color(col);

	//int l = min(view->prefered_buffer_layer - 1, b.peaks.num / 4);
	int l = view->prefered_buffer_layer * 4;
	if (l >= 0){
		double bzf = view->buffer_zoom_factor;

		// no peaks yet? -> show dummy
		if (b.peaks.num <= l){
			c->set_color(ColorInterpolate(col, Red, 0.3f));
			c->draw_rect((offset - view_pos_rel) * view->cam.scale, area.y1, b.length * view->cam.scale, h);
			c->set_antialiasing(false);
			return;
		}

		// maximum
		double _bzf = bzf;
		int ll = l;
		color cc = col;
		cc.a *= 0.3f;
		c->set_color(cc);
		for (int ci=0; ci<b.channels; ci++)
			draw_peak_buffer(c, di, view_pos_rel, view->cam.scale, _bzf, hf, x0, x1, y0[ci], b.peaks[ll+ci], offset);


		// mean square
		c->set_color(col);
		for (int ci=0; ci<b.channels; ci++)
			draw_peak_buffer(c, di, view_pos_rel, view->cam.scale, bzf, hf, x0, x1, y0[ci], b.peaks[l+2+ci], offset);


		// invalid peaks...
		if (b.peaks.num >= b.PEAK_MAGIC_LEVEL4){
			int nn = min(b.length / b.PEAK_CHUNK_SIZE, b.peaks[b.PEAK_MAGIC_LEVEL4].num);
			for (int i=0; i<nn; i++){
				if (b._peaks_chunk_needs_update(i)){
					c->set_color(ColorInterpolate(col, Red, 0.3f));
					float xx0 = max((float)view->cam.sample2screen(offset + i*b.PEAK_CHUNK_SIZE), x0);
					float xx1 = min((float)view->cam.sample2screen(offset + (i+1)*b.PEAK_CHUNK_SIZE), x1);
					c->draw_rect(xx0, area.y1, xx1 - xx0, h);
				}
			}
		}
	}else{

		// directly show every sample
		for (int ci=0; ci<b.channels; ci++)
			draw_line_buffer(c, view_pos_rel, view->cam.scale, hf, x0, x1, y0[ci], b.c[ci], offset);
	}
	c->set_antialiasing(false);
}

void BufferPainter::draw_buffer_selection(Painter *c, AudioBuffer &b, int offset)
{
	double view_pos_rel = view->cam.screen2sample(0);
	c->set_antialiasing(view->antialiasing);

	float h = area.height();
	float hf = h / (2 * b.channels);

	// zero heights of both channels
	float y0[2];
	for (int ci=0; ci<b.channels; ci++)
		y0[ci] = area.y1 + hf * (2*ci + 1);


	int di = view->detail_steps;
	color cc = col;
	cc.a = 0.7f;
	c->set_color(cc);

	//int l = min(view->prefered_buffer_layer - 1, b.peaks.num / 4);
	int l = view->prefered_buffer_layer * 4;
	if (l >= 0){
		double bzf = view->buffer_zoom_factor;


		// no peaks yet? -> show dummy
		if (b.peaks.num <= l){
			c->set_color(ColorInterpolate(col, Red, 0.3f));
			c->draw_rect((offset - view_pos_rel) * view->cam.scale, area.y1, b.length * view->cam.scale, h);
			c->set_antialiasing(false);
			return;
		}

		// maximum
		double _bzf = bzf;
		int ll = l;
		/*double _bzf = bzf;
		int ll = l;
		if (ll + 4 < b.peaks.num){
			ll += 4;
			_bzf *= 2;
		}*/
		for (int ci=0; ci<b.channels; ci++)
			draw_peak_buffer_sel(c, di, view_pos_rel, view->cam.scale, _bzf, hf, x0, x1, y0[ci], b.peaks[ll+ci], offset);
	}else{

		// directly show every sample
		for (int ci=0; ci<b.channels; ci++)
			draw_line_buffer_sel(c, view_pos_rel, view->cam.scale, hf, x0, x1, y0[ci], b.c[ci], offset);
	}
	c->set_antialiasing(false);
}

void BufferPainter::set_context(const rect &_area)
{
	area = _area;
	x0 = area.x1;
	x1 = area.x2;
	col = view->colors.text;
}

void BufferPainter::set_color(const color &_col)
{
	col = _col;
}

void BufferPainter::set_clip(const Range &r)
{
	view->cam.range2screen_clip(r, area, x0, x1);
}

