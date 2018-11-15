/*
 * MidiPainter.h
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDIPAINTER_H_
#define SRC_VIEW_PAINTER_MIDIPAINTER_H_

#include <functional>
#include "../../lib/base/base.h"
#include "../../lib/math/math.h"

class AudioView;
class Painter;
class MidiNote;
class MidiNoteBuffer;
class Clef;
class Scale;
class Song;
class Range;
class color;
class Instrument;
class Synthesizer;
enum class NoteModifier;
enum class MidiMode;

class MidiPainter
{
public:
	MidiPainter(AudioView *view);


	static const int PITCH_MIN_DEFAULT = 25;
	static const int PITCH_MAX_DEFAULT = 105;

	enum MidiNoteState
	{
		STATE_DEFAULT = 0,
		STATE_HOVER = 1,
		STATE_SELECTED = 2,
		STATE_REFERENCE = 4,
	};

	static color pitch_color(int pitch);

	void draw_pitch_grid(Painter *c, Synthesizer *synth);

	void draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func);

	void draw_simple_note(Painter *c, float x1, float x2, float y, float r, float rx, const color &col, const color &col_shadow, bool force_circle);
	void draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y, float r);

	void draw_note_linear(Painter *c, const MidiNote &n, MidiNoteState state);
	void draw_linear(Painter *c, const MidiNoteBuffer &midi);
	void draw_clef_tab(Painter *c);
	void draw_note_tab(Painter *c, const MidiNote *n, MidiNoteState state);
	void draw_tab(Painter *c, const MidiNoteBuffer &midi);
	void draw_note_classical(Painter *c, const MidiNote *n, MidiNoteState state);
	void draw_clef_classical(Painter *c);
	void draw_classical(Painter *c, const MidiNoteBuffer &midi);

	void draw(Painter *c, const MidiNoteBuffer &midi);
	void draw_background(Painter *c);

	AudioView *view;
	Song *song;

	void set_context(const rect &area, const Instrument &i, const Scale &s, bool playable, MidiMode mode);
	void set_shift(int shift);
	void set_linear_range(int pitch_min, int pitch_max);
	rect area;
	const Instrument *instrument;
	const Clef *clef;
	const Scale *scale;
	bool is_playable;
	int pitch_min, pitch_max;
	int shift;
	bool as_reference;

	float clef_dy;
	float clef_y0;

	float string_dy;
	float string_y0;
	MidiMode mode;


	float clef_pos_to_screen(int pos);
	int screen_to_clef_pos(float y);
	float string_to_screen(int string);
	int screen_to_string(float y);

	float pitch2y_classical(int pitch);
	float pitch2y_linear(int pitch);
	int y2pitch_classical(float y, NoteModifier modifier);
	int y2pitch_linear(float y);
	int y2clef_classical(float y, NoteModifier &mod);
	int y2clef_linear(float y, NoteModifier &mod);
};

#endif /* SRC_VIEW_PAINTER_MIDIPAINTER_H_ */
