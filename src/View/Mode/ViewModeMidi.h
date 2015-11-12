/*
 * ViewModeMidi.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEMIDI_H_
#define SRC_VIEW_MODE_VIEWMODEMIDI_H_

#include "ViewModeDefault.h"

class MidiNoteData;
class MidiNote;
class MidiEvent;

class ViewModeMidi : public ViewModeDefault
{
public:
	ViewModeMidi(AudioView *view, ViewMode *parent);
	virtual ~ViewModeMidi();

	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();

	virtual void drawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time);
	virtual void drawTrackBackground(HuiPainter *c, AudioViewTrack *t);
	virtual void drawTrackData(HuiPainter *c, AudioViewTrack *t);

	enum MidiNoteState
	{
		STATE_DEFAULT,
		STATE_HOVER,
		STATE_REFERENCE
	};

	void drawMidiNote(HuiPainter *c, const MidiNote &n, MidiNoteState state);
	void drawMidiEvent(HuiPainter *c, const MidiEvent &e);
	void drawMidiEditable(HuiPainter *c, const MidiNoteData &midi, bool as_reference, Track *track, const rect &area);

	virtual Selection getHover();

	MidiNoteData getCreationNotes();
	int y2pitch(int y);
	float pitch2y(int p);

	int pitch_min, pitch_max;
	int beat_partition;
	int midi_mode;
	int midi_scale;
	int chord_type;
	int chord_inversion;
	bool is_sharp(int pitch);

	enum
	{
		MIDI_MODE_SELECT,
		MIDI_MODE_NOTE,
		MIDI_MODE_CHORD
	};
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
