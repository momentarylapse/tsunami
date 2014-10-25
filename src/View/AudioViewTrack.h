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

class AudioViewTrack
{
public:
	AudioViewTrack(AudioView *view, Track *track);
	virtual ~AudioViewTrack();

	static color GetPitchColor(int pitch);

	void DrawTrackBuffers(HuiPainter *c, const rect &r, double pos, const color &col);
	void DrawBuffer(HuiPainter *c, const rect &r, BufferBox &b, double view_pos_rel, const color &col);
	void DrawSampleFrame(HuiPainter *c, const rect &r, SampleRef *s, const color &col, int delay);
	void DrawSample(HuiPainter *c, const rect &r, SampleRef *s);
	void DrawMidi(HuiPainter *c, const rect &r, MidiData &midi, int shift);
	void DrawMidiEditable(HuiPainter *c, const rect &r, MidiData &midi, color col);
	void DrawTrack(HuiPainter *c, const rect &r, color col, int track_no);

	Track *track;
	rect area;
	rect area_last, area_target;
	AudioView *view;
	static const float MIN_GRID_DIST;
};

#endif /* SRC_VIEW_AUDIOVIEWTRACK_H_ */
