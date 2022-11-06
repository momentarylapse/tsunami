/*
 * MidiPainter.h
 *
 *  Created on: 13.11.2018
 *      Author: michi
 */

#ifndef SRC_VIEW_PAINTER_MIDIPAINTER_H_
#define SRC_VIEW_PAINTER_MIDIPAINTER_H_

#include <functional>
#include "midi/MidiPainterModeClassical.h"
#include "midi/MidiPainterModeTab.h"
#include "midi/MidiPainterModeLinear.h"
#include "../../lib/base/base.h"
#include "../../lib/math/rect.h"
#include "../../data/Range.h"
#include "../../data/midi/Scale.h"

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
struct QuantizedNote;
class QuantizedNoteGroup;
class PluginManager;

class MidiKeyChange {
public:
	MidiKeyChange();
	MidiKeyChange(double pos, const Scale &key);
	double pos;
	Scale key;
};


class MidiPainter {
	friend class PluginManager;
	friend class MidiPainterModeClassical;
	friend class MidiPainterModeTab;
	friend class MidiPainterModeLinear;
public:
	MidiPainter(Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, const ColorScheme &colors);
	void __init__(Song *song, ViewPort *cam, SongSelection *sel, HoverData *hover, const ColorScheme &colors);


	static color pitch_color(int pitch);

	void set_synthesizer(Synthesizer *synth);
	Synthesizer *synth = nullptr;

private:
	void draw_pitch_grid(Painter *c);

	void draw_rhythm(Painter *c, const MidiNoteBuffer &midi, const Range &range, std::function<float(MidiNote*)> y_func);

	void draw_simple_note(Painter *c, float x1, float x2, float y, float rx, const color &col, const color &col_shadow, bool force_circle);

	void draw_note_flags(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y);

	void draw_low_detail_dummy(Painter *c, const MidiNoteBuffer &midi);
	void draw_low_detail_dummy_part(Painter *c, const Range &r, const MidiNoteBuffer &midi);

public:
	void _draw_notes(Painter *p, const MidiNoteBuffer &notes);
	void draw(Painter *c, const MidiNoteBuffer &midi);
	void draw_background(Painter *c, bool force = false);

	// some is exposed to rhythm...
	ViewPort *cam;
	Scale midi_scale;
	SongSelection *sel;
	HoverData *hover;
	Song *song;
	const ColorScheme &local_theme;
	rect area;
	bool is_playable;
	int shift;
	bool as_reference;
	bool allow_shadows;

public:
	void set_context(const rect &area, const Instrument &i, bool playable, MidiMode mode);
	void set_size_data(bool direct_size_mode, float s);
	void set_shift(int shift);
	void set_linear_range(float pitch_min, float pitch_max);
	void set_quality(float quality, bool antialiasing);
	void set_force_shadows(bool force);
	void set_key_changes(const Array<MidiKeyChange> &changes);



private:
	const Instrument *instrument;
	const Clef *clef;
	bool force_shadows;

private:
	struct {
		// configuration
		bool antialiasing;
		float rhythm_zoom_min;
		float notes_zoom_min;
		float shadow_threshold;
		float note_circle_threshold;
		float tab_text_threshold;
		int note_count_threshold;
		float factor;

		// current state
		bool _highest_details;
	} quality;

	MidiMode mode;
	MidiPainterModeClassical mode_classical;
	MidiPainterModeTab mode_tab;
	MidiPainterModeLinear mode_linear;
	MidiPainterMode *mmode = nullptr;
	float rr;
	float modifier_font_size;
	Range cur_range;


public:
	float clef_pos_to_screen(int pos) const;
	int screen_to_clef_pos(float y) const;
	float string_to_screen(int string) const;
	int screen_to_string(float y) const;

	float pitch2y(int pitch) const;
	int y2pitch(float y, NoteModifier modifier) const;
	int y2clef(float y, NoteModifier &mod) const;

	float note_r() const;
	float get_clef_dy() const;

private:
	float scale;
	float neck_length_single;
	float neck_length_group;
	float neck_width;
	float bar_distance;
	float bar_width;
	float flag_dx, flag_dy;
	void draw_single_ndata(Painter *c, QuantizedNote &d, bool neck_offset);
	void draw_group_ndata(Painter *c, const QuantizedNoteGroup &d, bool neck_offset);
	void draw_eigth_note_flag(Painter *c, const vec2 &p, float e);
};

#endif /* SRC_VIEW_PAINTER_MIDIPAINTER_H_ */
