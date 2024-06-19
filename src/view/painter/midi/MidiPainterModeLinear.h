/*
 * MidiPainterModeLinear.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODELINEAR_H_
#define SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODELINEAR_H_

#include "MidiPainterMode.h"

namespace tsunami {

class MidiNote;
enum class NoteModifier;

class MidiPainterModeLinear : public MidiPainterMode {
public:
	MidiPainterModeLinear(MidiPainter *mp);

    void reset() override;
    void update() override;

	void draw_notes(Painter *c, const MidiNoteBuffer &midi) override;
	void draw_background(Painter *c, bool force = false) override;

    void draw_note(Painter *c, const MidiNote &n, MidiNoteState state);

    void draw_pitch_grid(Painter *c);


    float pitch2y(float pitch) const;
    int y2pitch(float y) const;
    int y2clef(float y, NoteModifier &mod) const;


	static const int PITCH_MIN_DEFAULT = 25;
	static const int PITCH_MAX_DEFAULT = 105;
	void set_linear_range(float pitch_min, float pitch_max);

	float pitch_min = 0, pitch_max = 0;
    float clef_line_width = 0;
};

}

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODELINEAR_H_ */
