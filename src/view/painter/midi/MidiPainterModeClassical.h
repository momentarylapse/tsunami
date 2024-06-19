/*
 * MidiPainterModeClassical.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODECLASSICAL_H_
#define SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODECLASSICAL_H_

#include "MidiPainterMode.h"

namespace tsunami {

class MidiKeyChange;
class MidiNote;
enum class NoteModifier;

class MidiPainterModeClassical : public MidiPainterMode {
public:
	MidiPainterModeClassical(MidiPainter *mp);

	void reset() override;
	void update() override;

	void draw_notes(Painter *c, const MidiNoteBuffer &midi) override;
	void draw_background(Painter *c, bool force = false) override;

	void draw_key_symbol(Painter *c, const MidiKeyChange &kc);
	void draw_note(Painter *c, const MidiNote *n, MidiNoteState state);


	float clef_pos_to_screen(int pos) const;
	int screen_to_clef_pos(float y) const;

	float pitch2y(int pitch) const;
	int y2pitch(float y, NoteModifier modifier) const;
	int y2clef(float y, NoteModifier &mod) const;

	float clef_dy = 0;
	float clef_y0 = 0;
	float clef_line_width = 0;


	Array<MidiKeyChange> key_changes;
	void set_key_changes(const Array<MidiKeyChange> &changes);
};

}

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODECLASSICAL_H_ */
