/*
 * MidiPainterModeTab.h
 *
 *  Created on: 01.11.2022
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODETAB_H_
#define SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODETAB_H_

#include "MidiPainterMode.h"

class MidiNote;

class MidiPainterModeTab : public MidiPainterMode {
public:
	MidiPainterModeTab(MidiPainter *mp, Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, const ColorScheme &colors);

	void reset() override;
	void update() override;

	void draw_notes(Painter *c, const MidiNoteBuffer &midi) override;
	void draw_background(Painter *c, bool force = false) override;

    void draw_note(Painter *c, const MidiNote *n, MidiNoteState state);


    float string_to_screen(int string_no) const;
    int screen_to_string(float y) const;


	float string_dy = 0;
	float string_y0 = 0;
    float clef_line_width = 0;
};

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODETAB_H_ */
