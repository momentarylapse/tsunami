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

	virtual void drawTrackBackground(Painter *c, AudioViewTrack *t);
	void drawTrackPitchGrid(Painter *c, AudioViewTrack *t);
	virtual void drawTrackData(Painter *c, AudioViewTrack *t);
	virtual void drawPost(Painter *c);

	virtual Selection getHover();
	virtual void startSelection();

	virtual int which_midi_mode(Track *t);

	MidiData getCreationNotes(Selection *sel, int pos0);
	void setBeatPartition(int partition);

	Array<int> getCreationPitch(int base_pitch);
	Range getMidiEditRange();
	void startMidiPreview(const Array<int> &pitch, float ttl);

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

	AudioViewTrack *cur_track;

	rect scroll_bar;
	float scroll_offset;

	bool moving;
	int string_no;
	int octave;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
