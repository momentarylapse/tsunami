/*
 * MidiPainterModeClassical.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODECLASSICAL_H_
#define SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODECLASSICAL_H_

#include "MidiPainterMode.h"

class MidiKeyChange;
class MidiNote;

class MidiPainterModeClassical : public MidiPainterMode {
public:
	MidiPainterModeClassical(MidiPainter *mp, Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors);

	void draw_notes(Painter *c, const MidiNoteBuffer &midi) override;
	void draw_background(Painter *c, bool force = false) override;

	void draw_key_symbol(Painter *c, const MidiKeyChange &kc);
	void draw_note(Painter *c, const MidiNote *n, MidiNoteState state);


	float clef_pos_to_screen(int pos) const;
	int screen_to_clef_pos(float y) const;
};

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODECLASSICAL_H_ */
