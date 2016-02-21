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

class MidiNoteData;
class MidiNote;
class MidiEvent;
class AudioStream;
class SynthesizerRenderer;

class ViewModeMidi : public ViewModeDefault
{
public:
	ViewModeMidi(AudioView *view);
	virtual ~ViewModeMidi();

	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();
	virtual void onMouseMove();
	virtual void updateTrackHeights();
	virtual void onCurTrackChange();

	virtual void drawGridBars(HuiPainter *c, const rect &r, const color &bg, bool show_time);
	virtual void drawTrackBackground(HuiPainter *c, AudioViewTrack *t);
	void drawTrackBackgroundDefault(HuiPainter *c, AudioViewTrack *t);
	virtual void drawTrackData(HuiPainter *c, AudioViewTrack *t);

	void drawMidiNote(HuiPainter *c, const MidiNote &n, int state);
	void drawMidiEvent(HuiPainter *c, const MidiEvent &e);
	void drawMidiEditable(HuiPainter *c, AudioViewTrack *t, const MidiNoteData &midi, bool as_reference, Track *track, const rect &area);
	void drawMidiEditableDefault(HuiPainter *c, AudioViewTrack *t, const MidiNoteData &midi, bool as_reference, Track *track, const rect &area);
	void drawMidiEditableScore(HuiPainter *c, AudioViewTrack *t, const MidiNoteData &midi, bool as_reference, Track *track, const rect &area);

	virtual Selection getHover();

	MidiNoteData getCreationNotes();
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

	enum
	{
		MIDI_MODE_SELECT,
		MIDI_MODE_NOTE,
		MIDI_MODE_INTERVAL,
		MIDI_MODE_CHORD
	};

	AudioStream *preview_stream;
	SynthesizerRenderer *preview_renderer;

	rect scroll_bar;
	rect track_rect;
	float scroll_offset;

	bool deleting;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
