/*
 * MidiPainterMode.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODE_H_
#define SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODE_H_

#include "../../../lib/base/base.h"

class ViewPort;
class Song;
class ColorScheme;
class SongSelection;
class HoverData;
class Painter;
class MidiNoteBuffer;
class MidiPainter;
class MidiNote;
class color;



enum class MidiNoteState {
	DEFAULT = 0,
	HOVER = 1,
	SELECTED = 2,
	REFERENCE = 4,
};
inline bool operator&(MidiNoteState a, MidiNoteState b) {
	return (int)a & (int)b;
}
inline MidiNoteState operator|(MidiNoteState a, MidiNoteState b) {
	return (MidiNoteState)((int)a | (int)b);
}

void get_col(color &col, color &col_shadow, const MidiNote *n, MidiNoteState state, bool playable, const ColorScheme &colors);
MidiNoteState note_state(MidiNote *n, bool as_reference, SongSelection *sel, HoverData *hover);
void draw_shadow(Painter *c, float x1, float x2, float y, float rx, float rr, const color &col);
void draw_shadow2(Painter *c, float x1, float x2, float y, float dx, float clef_line_width, const color &col);


class MidiPainterMode {
public:
	MidiPainterMode(MidiPainter *mp);

	virtual void draw_notes(Painter *c, const MidiNoteBuffer &midi) {}//= 0;
	virtual void draw_background(Painter *c, bool force = false) {}//= 0;

	virtual void reset() {};
	virtual void update() {};


    MidiPainter *mp;
	ViewPort *cam;
	Song *song;
	SongSelection *sel;
	HoverData *hover;
	const ColorScheme &local_theme;
	bool direct_size_mode = false;
	float rr = 0;
};

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODE_H_ */
