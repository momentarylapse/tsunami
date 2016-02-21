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
class HuiPainter;
class AudioView;
class BufferBox;
class SampleRef;
class MidiNoteData;
class MidiNote;
class MidiEvent;
class TrackMarker;

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


	void drawTrackBuffers(HuiPainter *c, double pos);
	void drawBuffer(HuiPainter *c, BufferBox &b, double view_pos_rel, const color &col);
	void drawSampleFrame(HuiPainter *c, SampleRef *s, const color &col, int delay);
	void drawSample(HuiPainter *c, SampleRef *s);
	void drawMarker(HuiPainter *c, const TrackMarker &marker, int index, bool hover);
	void drawMidi(HuiPainter *c, const MidiNoteData &midi, int shift);
	void drawMidiDefault(HuiPainter *c, const MidiNoteData &midi, int shift);
	void drawMidiTab(HuiPainter *c, const MidiNoteData &midi, int shift);
	void drawMidiScore(HuiPainter *c, const MidiNoteData &midi, int shift);
	void drawMidiScoreClef(HuiPainter *c, int clef);
	void drawMidiNoteScore(HuiPainter *c, const MidiNote &n, int shift, MidiNoteState state, int clef);
	void drawHeader(HuiPainter *c);
	void draw(HuiPainter *c);

	Track *track;
	rect area;
	rect area_last, area_target;
	Array<rect> marker_areas;
	int reference_track;
	AudioView *view;
	static const float MIN_GRID_DIST;

	int height_wish, height_min;

	float clef_dy;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
