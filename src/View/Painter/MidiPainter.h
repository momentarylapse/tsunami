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
#include "../../lib/math/rect.h"
#include "../../Data/Range.h"
#include "../../Data/Midi/Scale.h"

class AudioView;
class ViewPort;
class SongSelection;
class HoverData;
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
class ColorScheme;
enum class NoteModifier;
enum class MidiMode;

class MidiKeyChange {
public:
	MidiKeyChange();
	MidiKeyChange(double pos, const Scale &key);
	double pos;
	Scale key;
};

class MidiPainter {
public:
	MidiPainter(Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors);
	void __init__(Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, ColorScheme &colors);


	static const int PITCH_MIN_DEFAULT = 25;
	static const int PITCH_MAX_DEFAULT = 105;

	enum MidiNoteState {
		STATE_DEFAULT = 0,
		STATE_HOVER = 1,
		STATE_SELECTED = 2,
		STATE_REFERENCE = 4,
	};

	static color pitch_color(int pitch);

	void draw_pitch_grid(Painter *c, Synthesizer *synth);

private:
	void draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func);

	void draw_simple_note(Painter *c, float x1, float x2, float y, float rx, const color &col, const color &col_shadow, bool force_circle);
	void draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y);

	void draw_note_flags(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y);

	void draw_note_linear(Painter *c, const MidiNote &n, MidiNoteState state);
	void draw_linear(Painter *c, const MidiNoteBuffer &midi);
	void draw_note_tab(Painter *c, const MidiNote *n, MidiNoteState state);
	void draw_tab(Painter *c, const MidiNoteBuffer &midi);
	void draw_note_classical(Painter *c, const MidiNote *n, MidiNoteState state);
	void draw_classical(Painter *c, const MidiNoteBuffer &midi);

	void draw_low_detail_dummy(Painter *c, const MidiNoteBuffer &midi);
	void draw_low_detail_dummy_part(Painter *c, const Range &r, const MidiNoteBuffer &midi);

public:
	void draw_clef_tab(Painter *c);
	void draw_clef_classical(Painter *c);
	void draw_key_symbol(Painter *c, const MidiKeyChange &kc);
	void _draw_notes(Painter *p, const MidiNoteBuffer &notes);
	void draw(Painter *c, const MidiNoteBuffer &midi);
	void draw_background(Painter *c);

	ViewPort *cam;
	Scale midi_scale;
	SongSelection *sel;
	HoverData *hover;
	Song *song;
	ColorScheme &local_theme;

	void set_context(const rect &area, const Instrument &i, bool playable, MidiMode mode);
	void set_shift(int shift);
	void set_linear_range(float pitch_min, float pitch_max);
	void set_quality(float quality, bool antialiasing);
	rect area;
	const Instrument *instrument;
	const Clef *clef;
	bool is_playable;
	float pitch_min, pitch_max;
	int shift;
	bool as_reference;
	bool allow_shadows;
	bool force_shadows;
	void set_force_shadows(bool force);
	Array<MidiKeyChange> key_changes;
	void set_key_changes(const Array<MidiKeyChange> &changes);

	struct {
		bool antialiasing;
		float rhythm_zoom_min;
		float notes_zoom_min;
		float shadow_threshold;
		float note_circle_threshold;
		float tab_text_threshold;
		int note_count_threshold;
		float factor;
	} quality;

	float clef_dy;
	float clef_y0;

	float string_dy;
	float string_y0;
	float clef_line_width;
	MidiMode mode;
	float rr;
	float modifier_font_size;
	Range cur_range;


	float clef_pos_to_screen(int pos);
	int screen_to_clef_pos(float y);
	float string_to_screen(int string);
	int screen_to_string(float y);

	float pitch2y_classical(int pitch);
	float pitch2y_linear(float pitch);
	int y2pitch_classical(float y, NoteModifier modifier);
	int y2pitch_linear(float y);
	int y2clef_classical(float y, NoteModifier &mod);
	int y2clef_linear(float y, NoteModifier &mod);
};

#endif /* SRC_VIEW_PAINTER_MIDIPAINTER_H_ */
