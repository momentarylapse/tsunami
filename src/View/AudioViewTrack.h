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
class MidiEvent;
class MidiNote;
class TrackMarker;

class AudioViewTrack
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	static color getPitchColor(int pitch);

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
	void drawMidiEditable(HuiPainter *c, const MidiNoteData &midi, bool as_reference);
	void drawMidiNote(HuiPainter *c, const MidiNote &n, MidiNoteState state);
	void drawMidiEvent(HuiPainter *c, const MidiEvent &e);
	void draw(HuiPainter *c, int track_no);

	Track *track;
	rect area;
	rect area_last, area_target;
	Array<rect> marker_areas;
	int reference_track;
	AudioView *view;
	static const float MIN_GRID_DIST;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
