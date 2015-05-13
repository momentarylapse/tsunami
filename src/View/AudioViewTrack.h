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
class MidiData;
class TrackMarker;

class AudioViewTrack
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	static color getPitchColor(int pitch);

	void drawTrackBuffers(HuiPainter *c, const rect &r, double pos, const color &col);
	void drawBuffer(HuiPainter *c, const rect &r, BufferBox &b, double view_pos_rel, const color &col);
	void drawSampleFrame(HuiPainter *c, const rect &r, SampleRef *s, const color &col, int delay);
	void drawSample(HuiPainter *c, const rect &r, SampleRef *s);
	void drawMarker(HuiPainter *c, const rect &r, TrackMarker &marker);
	void drawMidi(HuiPainter *c, const rect &r, MidiData &midi, int shift);
	void drawMidiEditable(HuiPainter *c, const rect &r, MidiData &midi, color col);
	void drawTrack(HuiPainter *c, const rect &r, color col, int track_no);

	Track *track;
	rect area;
	rect area_last, area_target;
	AudioView *view;
	static const float MIN_GRID_DIST;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
