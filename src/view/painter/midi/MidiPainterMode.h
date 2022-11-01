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
enum class MidiNoteState;

class MidiPainterMode {
public:
	MidiPainterMode(MidiPainter *mp, Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors) :
        mp(mp), song(song), cam(cam), sel(sel), hover(hover), local_theme(colors)
    {}

	virtual void draw_notes(Painter *c, const MidiNoteBuffer &midi) {}//= 0;
	virtual void draw_background(Painter *c, bool force = false) {}//= 0;

	virtual void reset() {};
	virtual void update() {};


    MidiPainter *mp;
	ViewPort *cam;
	Song *song;
	SongSelection *sel;
	HoverData *hover;
	ColorScheme &local_theme;
};

#endif /* SRC_VIEW_PAINTER_MIDI_MIDIPAINTERMODE_H_ */
