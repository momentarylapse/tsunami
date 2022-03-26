/*
 * MultiLinePainter.h
 *
 *  Created on: 21 Feb 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MULTILINEPAINTER_H_
#define SRC_VIEW_PAINTER_MULTILINEPAINTER_H_

#include "../../lib/base/base.h"

class Song;
class Track;
class MidiPainter;
class ColorScheme;
class ViewPort;
class Range;
class Painter;
class Any;
class SongSelection;
class HoverData;
class vec2;

class MultiLinePainter {
public:
	MultiLinePainter(Song *s, ColorScheme &c);
	virtual ~MultiLinePainter();

	void __init__(Song *s, ColorScheme &c);
	void __delete__();

	struct TrackData {
		Track *track;
		bool allow_classical, allow_tab;
	};
	Array<TrackData> track_data;
	void set_context(const Any &conf, float page_width, float avg_samples_per_line);
	void set(const Any &conf);


	MidiPainter *mp;
	ViewPort *cam;
	ColorScheme *colors;
	SongSelection *sel;
	HoverData *hover;

	float page_width = 1200;
	float border = 25;
	float line_space = 25;
	float track_space = 10;
	float line_height = 50;
	bool antialiasing = true;
	float string_dy = 10;
	bool allow_shadows = false;

	float w = 0;
	float avg_samples_per_line = 0;

	//int samples = od->song->range_with_time().end();

	Song *song;

	double pdf_bpm;
	Array<int> pdf_pattern;

	struct LineData {
		Track *track;
		float y0, y1;
	};
	Array<LineData> line_data;

	int good_samples(const Range &r0);
	float draw_track_classical(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	float draw_track_tab(Painter *p, float x0, float w, float y0, const Range &r, Track *t, float scale);
	float draw_line(Painter *p, float x0, float w, float y0, const Range &r, float scale);

	void draw_beats(Painter *p, float x0, float w, float y, float h, const Range &r);
	void draw_bar_markers(Painter *p, float x0, float w, float y, float h, const Range &r);

	int next_line_samples(int offset);
	float draw_next_line(Painter *p, int &offset, const vec2 &pos);
	float get_line_dy();
};

#endif /* SRC_VIEW_PAINTER_MULTILINEPAINTER_H_ */
