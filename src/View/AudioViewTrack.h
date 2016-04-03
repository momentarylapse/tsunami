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

class AudioViewTrack
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	static color getPitchColor(int pitch);

	float clef_pos_to_screen(int pos);
	int screen_to_clef_pos(float y);

	enum MidiNoteState
	{
		STATE_DEFAULT,
		STATE_HOVER,
		STATE_REFERENCE
	};


	void drawTrackBuffers(Painter *c, double pos);
	void drawBuffer(Painter *c, BufferBox &b, double view_pos_rel, const color &col);
	void drawSampleFrame(Painter *c, SampleRef *s, const color &col, int delay);
	void drawSample(Painter *c, SampleRef *s);
	void drawMarker(Painter *c, const TrackMarker &marker, int index, bool hover);
	void drawMidi(Painter *c, const MidiData &midi, int shift);
	void drawMidiDefault(Painter *c, const MidiData &midi, int shift);
	void drawMidiTab(Painter *c, const MidiData &midi, int shift);
	void drawMidiScore(Painter *c, const MidiData &midi, int shift);
	void drawMidiScoreClef(Painter *c, const Clef &clef, const Scale &scale);
	void drawMidiNoteScore(Painter *c, const MidiNote &n, int shift, MidiNoteState state, const Clef &clef);
	void drawHeader(Painter *c);
	void draw(Painter *c);

	Track *track;
	rect area;
	rect area_last, area_target;
	Array<rect> marker_areas;
	Array<int> reference_tracks;
	AudioView *view;
	static const float MIN_GRID_DIST;

	int height_wish, height_min;

	float clef_dy;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
