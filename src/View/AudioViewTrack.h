/*
 * AudioViewTrack.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_AUDIOVIEWTRACK_H_
#define SRC_VIEW_AUDIOVIEWTRACK_H_

#include "../lib/math/math.h"

class Track;
class Painter;
class AudioView;
class BufferBox;
class SampleRef;
class MidiData;
class MidiNote;
class MidiEvent;
class TrackMarker;
class Clef;
class Scale;
class Range;

class AudioViewTrack
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	static color getPitchColor(int pitch);

	float clef_pos_to_screen(int pos);
	int screen_to_clef_pos(float y);
	float string_to_screen(int string);
	int screen_to_string(float y);

	float pitch2y_classical(int pitch);
	float pitch2y_linear(int pitch);
	int y2pitch_classical(float y, int modifier);
	int y2pitch_linear(float y);
	int y2clef_classical(float y, int &mod);
	int y2clef_linear(float y, int &mod);

	void setPitchMinMax(int pitch_min, int pitch_max);
	int pitch_min, pitch_max;

	enum MidiNoteState
	{
		STATE_DEFAULT = 0,
		STATE_HOVER = 1,
		STATE_REFERENCE = 2,
		STATE_SELECTED = 4,
	};

	color getBackgroundColor();
	void drawBlankBackground(Painter *c);
	void drawGridBars(Painter *c, const color &bg, bool show_time, int beat_partition);

	void drawTrackBuffers(Painter *c, double pos);
	void drawBuffer(Painter *c, BufferBox &b, double view_pos_rel, const color &col);
	void drawBufferSelection(Painter *c, BufferBox &b, double view_pos_rel, const color &col, const Range &r);
	void drawSampleFrame(Painter *c, SampleRef *s, const color &col, int delay);
	void drawSample(Painter *c, SampleRef *s);
	void drawMarker(Painter *c, const TrackMarker *marker, int index, bool hover);
	void drawMidiLinear(Painter *c, const MidiData &midi, bool as_reference, int shift);
	void drawMidiNoteLinear(Painter *c, const MidiNote &n, int shift, MidiNoteState state);
	void drawMidiTab(Painter *c, const MidiData &midi, bool as_reference, int shift);
	void drawMidiNoteTab(Painter *c, const MidiNote *n, int shift, MidiNoteState state);
	void drawMidiClefTab(Painter *c);
	void drawMidiClassical(Painter *c, const MidiData &midi, bool as_reference, int shift);
	void drawMidiClefClassical(Painter *c, const Clef &clef, const Scale &scale);
	void drawMidiNoteClassical(Painter *c, const MidiNote *n, int shift, MidiNoteState state, const Clef &clef);
	void drawHeader(Painter *c);
	void draw(Painter *c);

	static void draw_simple_note(Painter *c, float x1, float x2, float y, float r, float rx, const color &col, const color &col_shadow, bool force_circle);

	Track *track;
	rect area;
	rect area_last, area_target;
	Array<rect> marker_areas;
	Array<int> reference_tracks;
	AudioView *view;
	static const float MIN_GRID_DIST;

	int height_wish, height_min;

	float clef_dy;
	float clef_y0;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
