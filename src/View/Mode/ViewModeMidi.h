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

class MidiNoteBuffer;
class MidiNote;
class MidiEvent;
class OutputStream;
class Synthesizer;
class MidiPreviewSource;

class ViewModeMidi : public ViewModeDefault
{
public:
	ViewModeMidi(AudioView *view);
	virtual ~ViewModeMidi();

	void onLeftButtonDown() override;
	void onLeftButtonUp() override;
	void onMouseMove() override;
	void onKeyDown(int k) override;
	void updateTrackHeights() override;
	void onCurTrackChange() override;

	void drawLayerBackground(Painter *c, AudioViewLayer *l) override;
	void drawLayerPitchGrid(Painter *c, AudioViewLayer *l);
	void drawLayerData(Painter *c, AudioViewLayer *l) override;
	void drawTrackData(Painter *c, AudioViewTrack *t) override;
	void drawPost(Painter *c) override;

	virtual Selection getHover() override;
	virtual void startSelection() override;

	virtual int which_midi_mode(Track *t) override;

	MidiNoteBuffer getCreationNotes(Selection *sel, int pos0);
	void setBeatPartition(int partition);

	Array<int> getCreationPitch(int base_pitch);
	Range getMidiEditRange();
	void startMidiPreview(const Array<int> &pitch, float ttl);
	void onEndOfStream();
	void kill_preview();

	int beat_partition;
	int chord_type;
	int chord_inversion;
	int midi_interval;

	int modifier;

	void setMode(int mode);
	int mode_wanted;

	void setCreationMode(int mode);
	int creation_mode;
	enum CreationMode
	{
		SELECT,
		NOTE,
		INTERVAL,
		CHORD
	};

	Synthesizer *preview_synth;
	OutputStream *preview_stream;
	MidiPreviewSource *preview_source;

	AudioViewLayer *cur_layer;

	rect scroll_bar;
	float scroll_offset;

	bool moving;
	int string_no;
	int octave;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
