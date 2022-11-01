/*
 * MidiPainterModeLinear.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODELINEAR_H_
#define SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODELINEAR_H_

#include "MidiPainterMode.h"

class MidiNote;
enum class NoteModifier;

class MidiPainterModeLinear : public MidiPainterMode {
public:
	MidiPainterModeLinear(MidiPainter *mp, Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors);

	void draw_notes(Painter *c, const MidiNoteBuffer &midi) override;
	void draw_background(Painter *c, bool force = false) override;

    void draw_note(Painter *c, const MidiNote &n, MidiNoteState state);

    void draw_pitch_grid(Painter *c);


    float pitch2y(float pitch) const;
    int y2pitch(float y) const;
    int y2clef(float y, NoteModifier &mod) const;


};

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODELINEAR_H_ */
