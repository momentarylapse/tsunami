/*
 * ViewModeMidi.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEMIDI_H_
#define SRC_VIEW_MODE_VIEWMODEMIDI_H_

#include "ViewModeDefault.h"
#include "../../lib/math/math.h"

class MidiData;
class MidiNote;
class MidiEvent;
class OutputStream;
class MidiRenderer;
class Synthesizer;

class ViewModeMidi : public ViewModeDefault
{
public:
	ViewModeMidi(AudioView *view);
	virtual ~ViewModeMidi();

	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onMouseMove();
	virtual void onKeyDown(int k);
	virtual void updateTrackHeights();
	virtual void onCurTrackChange();

	virtual void drawGridBars(Painter *c, const rect &r, const color &bg, bool show_time);
	virtual void drawTrackBackground(Painter *c, AudioViewTrack *t);
	void drawTrackPitchGrid(Painter *c, AudioViewTrack *t);
	virtual void drawTrackData(Painter *c, AudioViewTrack *t);

	void drawMidiNote(Painter *c, const MidiNote &n, int state);
	void drawMidiEditable(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area);
	void drawMidiEditableLinear(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area);
	void drawMidiEditableClassical(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area);

	virtual Selection getHover();

	virtual int which_midi_mode(Track *t);

	Array<MidiNote> getCreationNotes();
	int y2clef(int y, int &mod);
	int y2pitch(int y);
	float pitch2y(int p);
	void setPitchMin(int pitch);
	void setBeatPartition(int partition);

	Array<int> getCreationPitch();

	int pitch_min, pitch_max;
	int beat_partition;
	int chord_type;
	int chord_inversion;
	int midi_interval;

	int modifier;

	void setMode(int mode);
	int mode_wanted;
	void setCreationMode(int mode);
	int creation_mode;
	enum
	{
		CREATION_MODE_SELECT,
		CREATION_MODE_NOTE,
		CREATION_MODE_INTERVAL,
		CREATION_MODE_CHORD
	};

	Synthesizer *preview_synth;
	OutputStream *preview_stream;
	MidiRenderer *preview_renderer;

	rect scroll_bar;
	rect track_rect;
	float scroll_offset;

	bool deleting;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
