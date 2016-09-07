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
	void drawTrackBackgroundDefault(Painter *c, AudioViewTrack *t);
	virtual void drawTrackData(Painter *c, AudioViewTrack *t);

	void drawMidiNote(Painter *c, const MidiNote &n, int state);
	void drawMidiEvent(Painter *c, const MidiEvent &e);
	void drawMidiEditable(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area);
	void drawMidiEditableDefault(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area);
	void drawMidiEditableScore(Painter *c, AudioViewTrack *t, const MidiData &midi, bool as_reference, Track *track, const rect &area);

	virtual Selection getHover();

	Array<MidiNote> getCreationNotes();
	int y2clef(int y, int &mod);
	int y2pitch(int y);
	float pitch2y(int p);
	void setPitchMin(int pitch);
	void setBeatPartition(int partition);

	Array<int> getCreationPitch();

	int pitch_min, pitch_max;
	int beat_partition;
	int midi_mode;
	int chord_type;
	int chord_inversion;
	int midi_interval;

	int modifier;

	enum
	{
		MIDI_MODE_SELECT,
		MIDI_MODE_NOTE,
		MIDI_MODE_INTERVAL,
		MIDI_MODE_CHORD
	};

	OutputStream *preview_stream;
	MidiRenderer *preview_renderer;

	rect scroll_bar;
	rect track_rect;
	float scroll_offset;

	bool deleting;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
