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
class MidiPreview;
enum class NoteModifier;
enum class ChordType;

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

	Selection getHover() override;
	void startSelection() override;

	MidiMode which_midi_mode(Track *t) override;

	MidiNoteBuffer getCreationNotes(Selection *sel, int pos0);
	void setBeatPartition(int partition);

	Array<int> getCreationPitch(int base_pitch);
	Range getMidiEditRange();
	void startMidiPreview(const Array<int> &pitch, float ttl);

	int beat_partition;
	ChordType chord_type;
	int chord_inversion;
	int midi_interval;

	NoteModifier modifier;

	void setMode(MidiMode mode);
	MidiMode mode_wanted;

	enum class CreationMode
	{
		SELECT,
		NOTE,
		INTERVAL,
		CHORD
	};
	void setCreationMode(CreationMode mode);
	CreationMode creation_mode;

	MidiPreview *preview;

	AudioViewLayer *cur_layer;

	rect scroll_bar;
	float scroll_offset;

	bool moving;
	Array<int> pre_moving_offsets;
	int mouse_pre_moving_pos;
	int string_no;
	int octave;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
