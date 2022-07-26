/*
 * BufferPainter.cpp
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#include "BufferPainter.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewTrack.h" // AudioViewMode...
#include "../../lib/math/vec2.h"
#include "../../lib/image/Painter.h"
#include "../../data/Range.h"
#include "../../data/audio/AudioBuffer.h"
#include "../../lib/math/complex.h"
#include "../../plugins/FastFourierTransform.h"
#include "../../Session.h"

BufferPainter::BufferPainter(AudioView *_view) {
	view = _view;
	area = rect(0,0,0,0);
	x0 = x1 = 0;
	mode = AudioViewMode::PEAKS;
}



static Array<vec2> tt;

inline void draw_line_buffer(Painter *c, double view_pos, double zoom, float hf, float x0, float x1, float y0, const Array<float> &buf, int offset) {
	int nl = 0;
	int i0 = max((double)x0 / zoom + view_pos - offset    , 0.0);
	int i1 = min((double)x1 / zoom + view_pos - offset + 2, (double)buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);

	for (int i=i0; i<i1; i++) {

		double p = ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 - buf[i] * hf;
		if (zoom > 5)
			c->draw_circle({(float)p, tt[nl].y}, 2);
		nl ++;
	}
	c->draw_lines(tt);
}

inline void draw_line_buffer_sel(Painter *c, double view_pos, double zoom, float hf, float x0, float x1, float y0, const Array<float> &buf, int offset) {
	int nl = 0;
	int i0 = max((double)x0 / zoom + view_pos - offset    , 0.0);
	int i1 = min((double)x1 / zoom + view_pos - offset + 1, (double)buf.num);
	if (i1 < i0)
		return;

	tt.resize(i1 - i0);
	c->set_line_width(3.0f);

	for (int i=i0; i<i1; i++) {

		double p = ((double)(i + offset) + 0.5 - view_pos) * zoom;
		tt[nl].x = (float)p;
		tt[nl].y = y0 - buf[i] * hf;
		if (zoom > 5)
			c->draw_circle({(float)p, tt[nl].y}, 4);
		nl ++;
	}
	tt.resize(nl);
	c->draw_lines(tt);
	c->set_line_width(1.0f);
}

inline void draw_peak_buffer(Painter *c, int di, double view_pos_rel, double zoom, float f, float hf, float x0, float x1, float y0, const string &buf, int offset) {
	if ((x1 - x0) / di < 1)
		return;
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize((int)((x1 - x0)/di + 10));
	for (int i=x0; i<x1+di; i+=di) {

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) and (ip < buf.num))
			if (((int)(p) < offset + buf.num*f) and (p >= offset)) {
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

inline void draw_peak_buffer_sel(Painter *c, int di, double view_pos_rel, double zoom, float f, float hf, float x0, float x1, float y0, const string &buf, int offset) {
	if ((x1 - x0) / di < 1)
		return;
	int nl = 0;
	double dpos = 1.0 / zoom;
	// pixel position
	// -> buffer position
	double p0 = view_pos_rel;
	tt.resize((int)((x1 - x0)/di + 10));
	for (int i=x0; i<x1+di; i+=di) {

		double p = p0 + dpos * (double)i + 0.5;
		int ip = (int)(p - offset)/f;
		if ((ip >= 0) and (ip < buf.num))
			if (((int)(p) < offset + buf.num*f) and (p >= offset)) {
				tt[nl].x = i;
				float dy = ((float)(buf[ip])/255.0f) * hf;
				tt[nl].y  = y0 - dy - 1;
				nl ++;
			}
	}
	if (nl == 0)
		return;
	tt.resize(nl * 2);
	for (int i=0; i<nl; i++) {
		tt[nl + i].x = tt[nl - i - 1].x;
		tt[nl + i].y = y0 *2 - tt[nl - i - 1].y + 1;
	}
	c->draw_polygon(tt);
}

void BufferPainter::draw_buffer(Painter *c, AudioBuffer &b, int offset) {
	if (mode == AudioViewMode::PEAKS)
		draw_peaks(c, b, offset);
	else if (mode == AudioViewMode::SPECTRUM)
		draw_spectrum(c, b, offset);
}

void BufferPainter::draw_peaks(Painter *c, AudioBuffer &b, int offset) {
	double view_pos_rel = view->cam.screen2sample(0);
	c->set_antialiasing(view->antialiasing);

	if (b.has_compressed()) {
		c->set_color(Red);
		c->set_font_size(8);
		c->draw_str(vec2((offset - view_pos_rel) * view->cam.pixels_per_sample, area.y1), "compressed:" + b.compressed->codec);
	}

	//float w = area.width();
	float h = area.height();
	float hf = h / (2 * b.channels);

	// zero heights of both channels
	float y0[16];
	for (int ci=0; ci<b.channels; ci++)
		y0[ci] = area.y1 + hf * (2*ci + 1);


	int di = view->detail_steps;
	//c->set_color(col1);

	//int l = min(view->prefered_buffer_layer - 1, b.peaks.num / 4);
	int pm = AudioBuffer::PEAK_MAGIC_LEVEL2 * b.channels;
	int l = view->prefered_buffer_layer * 2 * b.channels;
	if (l >= 0) {
		double bzf = view->buffer_zoom_factor;

		// no peaks yet? -> show dummy
		if (b.peaks.num <= l) {
			c->set_color(color::interpolate(col1, Red, 0.3f));
			float x0 = (offset - view_pos_rel) * view->cam.pixels_per_sample;
			c->draw_rect(rect(x0, x0 + b.length * view->cam.pixels_per_sample, area.y1, area.y1 + h));
			c->set_antialiasing(false);
			return;
		}

		// maximum
		double _bzf = bzf;
		int ll = l;
		//color cc = col;
		//cc.a *= 0.3f;
		c->set_color(col2);
		for (int ci=0; ci<b.channels; ci++)
			draw_peak_buffer(c, di, view_pos_rel, view->cam.pixels_per_sample, _bzf, hf, x0, x1, y0[ci], b.peaks[ll+ci], offset);


		// mean square
		c->set_color(col1);
		for (int ci=0; ci<b.channels; ci++)
			draw_peak_buffer(c, di, view_pos_rel, view->cam.pixels_per_sample, bzf, hf, x0, x1, y0[ci], b.peaks[l+b.channels+ci], offset);


		// invalid peaks...
		if (b.peaks.num >= pm) {
			int nn = min(b.length / b.PEAK_CHUNK_SIZE, b.peaks[pm].num);
			for (int i=0; i<nn; i++) {
				if (b._peaks_chunk_needs_update(i)) {
					c->set_color(color::interpolate(col1, Red, 0.3f));
					float xx0 = max((float)view->cam.sample2screen(offset + i*b.PEAK_CHUNK_SIZE), x0);
					float xx1 = min((float)view->cam.sample2screen(offset + (i+1)*b.PEAK_CHUNK_SIZE), x1);
					c->draw_rect(rect(xx0, xx1, area.y1, area.y1 + h));
				}
			}
		}
	} else {

		// directly show every sample
		for (int ci=0; ci<b.channels; ci++)
			draw_line_buffer(c, view_pos_rel, view->cam.pixels_per_sample, hf, x0, x1, y0[ci], b.c[ci], offset);
	}
	c->set_antialiasing(false);
}

float sum_abs(const Array<complex> &z) {
	float r = 0;
	for (auto &x: z)
		r += x.abs();
	return r;
}

float max_abs(const Array<complex> &z) {
	float r = 0;
	for (auto &x: z)
		r = max(r, x.abs());
	return r;
}

const int SPECTRUM_CHUNK = 512;
const int SPECTRUM_FFT_INPUT = SPECTRUM_CHUNK * 8;
const int SPECTRUM_N = 256;
const float MIN_FREQ = 60.0f;
const float MAX_FREQ = 3000.0f;

bool prepare_spectrum(AudioBuffer &b, float sample_rate) {
	if (b.spectrum.num > 0)
		return false;

	float ww = (float)SPECTRUM_FFT_INPUT / sample_rate;

	bytes spectrum;
	Array<complex> z;
	for (int i=0; i<b.length/SPECTRUM_CHUNK; i++) {
		FastFourierTransform::fft_r2c(b.c[0].sub_ref(i * SPECTRUM_CHUNK, i * SPECTRUM_CHUNK + SPECTRUM_FFT_INPUT), z);
		for (int k=0; k<SPECTRUM_N; k++) {
			float fmin = MIN_FREQ * exp( log(MAX_FREQ / MIN_FREQ) / (SPECTRUM_N - 1) * k);
			float fmax = MIN_FREQ * exp( log(MAX_FREQ / MIN_FREQ) / (SPECTRUM_N - 1) * (k + 1));
			int j0 = fmin * ww;
			int j1 = fmax * ww + 1;
			j0 = clamp(j0, 0, z.num);
			j1 = clamp(j1, 0, z.num);
			float f = sum_abs(z.sub_ref(j0, j1)) / (SPECTRUM_FFT_INPUT / 2) * pi * 3; // arbitrary... just "louder"
			//float f = max_abs(z.sub_ref(j0, j1)) / (SPECTRUM_FFT_INPUT / 2) * pi * 3; // arbitrary... just "louder"
			// / (SPECTRUM_FFT_INPUT / 2 / SPECTRUM_N);
			f = clamp(f, 0.0f, 1.0f);
			//f = 1-exp(-f);
			spectrum.add(254 * f);
		}
	}
	b.mtx.lock();
	b.spectrum.exchange(spectrum);
	b.mtx.unlock();
	return true;
}

Image _spec_image_;

struct SpectrumImage {
	AudioBuffer *buf;
	Image *image;
};
static Array<SpectrumImage> spectrum_images;

Image *get_spectrum_image(AudioBuffer &b) {
	for (auto &si: spectrum_images)
		if (si.buf == &b)
			return si.image;
	SpectrumImage s;
	s.buf = &b;
	s.image = new Image;
	spectrum_images.add(s);
	return s.image;
}

color col_heat_map(float f) {
	//f = sqrt(f);
	if (f < 0.333f)
		return color(f*3, 0, 0, f*3);
	if (f < 0.6666f)
		return color(1, f*3 - 1, 0, 2 - f*3);
	return color(1, 1, f*3 - 2, 0);
}


Image *render_spectrum_image(AudioBuffer &b, float sample_rate) {
	auto im = get_spectrum_image(b);
	if (prepare_spectrum(b, sample_rate)) {
		im->create(b.length/SPECTRUM_CHUNK, SPECTRUM_N, Black);

		for (int i=0; i<b.length/SPECTRUM_CHUNK; i++) {
			float x1, x2;
			for (int k=0; k<SPECTRUM_N; k++) {
				float f = (float)b.spectrum[i * SPECTRUM_N + k] / 254.0f;
				//f = 1-exp(-f*2);
				//f = 1/(exp((0.5f-f) * 3) + 1);
				//im->set_pixel(i, k, col_heat_map(f));//White.with_alpha(1-exp(-f)));
				im->set_pixel(i, SPECTRUM_N - 1 - k, col_heat_map(f));//White.with_alpha(1-exp(-f)));
			}
		}
	}
	return im;
}

void BufferPainter::draw_spectrum(Painter *c, AudioBuffer &b, int offset) {
	auto im = render_spectrum_image(b, this->view->session->sample_rate());

	if (false) {

	for (int i=0; i<b.length/SPECTRUM_CHUNK; i++) {
		float x1, x2;
		view->cam.range2screen(Range(offset + i * SPECTRUM_CHUNK, SPECTRUM_CHUNK), x1, x2);
		for (int k=0; k<SPECTRUM_N; k++) {
			float f = (float)b.spectrum[i * SPECTRUM_N + k] / 254.0f;
			c->set_color(White.with_alpha(1-exp(-f)));
			c->draw_rect(rect(x1, x2, area.y2 - area.height() * k / SPECTRUM_N, area.y2 - area.height() * (k+1) / SPECTRUM_N));
		}
	}

	} else {

	//float x = view->cam.sample2screen_f(offset);
	float x1, x2;
	view->cam.range2screen(Range(offset, b.length), x1, x2);

	float scale[] = {(x2 - x1) / im->width, 0.0f, 0.0f, area.height() / im->height};
	c->set_transform(scale, vec2(x1, area.y1));
	c->draw_image({0,0}, im);
	scale[0] = 1.0f;
	scale[3] = 1.0f;
	c->set_transform(scale, {0,0});

	}

}

void BufferPainter::draw_buffer_selection(Painter *c, AudioBuffer &b, int offset) {
	std::swap(col2, col1);
	draw_buffer(c, b, offset);
	std::swap(col2, col1);
	return;

	double view_pos_rel = view->cam.screen2sample(0);
	c->set_antialiasing(view->antialiasing);

	float h = area.height();
	float hf = h / (2 * b.channels);

	// zero heights of both channels
	float y0[2];
	for (int ci=0; ci<b.channels; ci++)
		y0[ci] = area.y1 + hf * (2*ci + 1);


	int di = view->detail_steps;
	//color cc = col;
	//cc.a = 0.7f;
	//c->set_color(cc);

	//int l = min(view->prefered_buffer_layer - 1, b.peaks.num / 4);
	int l = view->prefered_buffer_layer * 2 * b.channels;
	if (l >= 0) {
		double bzf = view->buffer_zoom_factor;


		// no peaks yet? -> show dummy
		if (b.peaks.num <= l) {
			c->set_color(color::interpolate(col1, Red, 0.3f));
			float x0 = (offset - view_pos_rel) * view->cam.pixels_per_sample;
			c->draw_rect(rect(x0, x0 + b.length * view->cam.pixels_per_sample, area.y1, area.y1 + h));
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
			draw_peak_buffer_sel(c, di, view_pos_rel, view->cam.pixels_per_sample, _bzf, hf, x0, x1, y0[ci], b.peaks[ll+ci], offset);
	} else {

		// directly show every sample
		for (int ci=0; ci<b.channels; ci++)
			draw_line_buffer_sel(c, view_pos_rel, view->cam.pixels_per_sample, hf, x0, x1, y0[ci], b.c[ci], offset);
	}
	c->set_antialiasing(false);
}

void BufferPainter::set_context(const rect &_area, AudioViewMode _mode) {
	area = _area;
	x0 = area.x1;
	x1 = area.x2;
	col1 = theme.text_soft1;
	col2 = theme.text_soft3;
	mode = _mode;
}

void BufferPainter::set_color(const color &fg, const color &bg) {
	col1 = fg;
	col2 = color::interpolate(fg, bg, 0.8f);
	col2sel = col2;//color::interpolate(fg, bg, 0.3f);
}

void BufferPainter::set_clip(const Range &r) {
	view->cam.range2screen_clip(r, area, x0, x1);
}

