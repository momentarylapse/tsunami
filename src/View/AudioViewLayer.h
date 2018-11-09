/*
 * AudioViewLayer.h
 *
 *  Created on: 25.10.2014
 *      Author: michi
 */

#ifndef SRC_VIEW_AUDIOVIEWLAYER_H_
#define SRC_VIEW_AUDIOVIEWLAYER_H_

#include "../lib/math/math.h"
#include "../Stuff/Observable.h"

class Track;
class TrackLayer;
class Painter;
class AudioView;
class AudioBuffer;
class SampleRef;
class MidiNoteBuffer;
class MidiNote;
class MidiEvent;
class TrackMarker;
class Clef;
class Scale;
class Range;
enum class NoteModifier;
enum class MidiMode;


class AudioViewLayer : public Observable<VirtualBase>
{
public:
	AudioViewLayer(AudioView *v, TrackLayer *l);
	virtual ~AudioViewLayer(){}


	enum MidiNoteState
	{
		STATE_DEFAULT = 0,
		STATE_HOVER = 1,
		STATE_REFERENCE = 2,
		STATE_SELECTED = 4,
	};

	color background_color();
	color background_selection_color();
	void draw_blank_background(Painter *c);

	void draw_midi_linear(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);
	void draw_midi_note_linear(Painter *c, const MidiNote &n, int shift, MidiNoteState state);
	void draw_midi_tab(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);
	void draw_midi_note_tab(Painter *c, const MidiNote *n, int shift, MidiNoteState state);
	void draw_midi_clef_tab(Painter *c);
	void draw_midi_classical(Painter *c, const MidiNoteBuffer &midi, bool as_reference, int shift);
	void draw_midi_clef_classical(Painter *c, const Clef &clef, const Scale &scale);
	void draw_midi_note_classical(Painter *c, const MidiNote *n, int shift, MidiNoteState state, const Clef &clef);

	void draw_track_buffers(Painter *c);
	void draw_buffer(Painter *c, AudioBuffer &b, double view_pos_rel, const color &col, float x0, float x1);
	void draw_buffer_selection(Painter *c, AudioBuffer &b, double view_pos_rel, const color &col, const Range &r);

	void draw_sample_frame(Painter *c, SampleRef *s, const color &col, int delay);
	void draw_sample(Painter *c, SampleRef *s);

	void draw_marker(Painter *c, const TrackMarker *marker, int index, bool hover);

	void draw_version_header(Painter *c);
	virtual void draw(Painter *c);

	void draw_complex_note(Painter *c, const MidiNote *n, MidiNoteState state, float x1, float x2, float y, float r);
	static void draw_simple_note(Painter *c, float x1, float x2, float y, float r, float rx, const color &col, const color &col_shadow, bool force_circle);

	bool on_screen();

	TrackLayer *layer;
	rect area;
	rect area_last, area_target;
	int height_wish, height_min;
	Array<rect> marker_areas;
	Array<rect> marker_label_areas;
	AudioView *view;
	void set_midi_mode(MidiMode wanted);
	MidiMode midi_mode;


	static color pitch_color(int pitch);
	static color marker_color(const TrackMarker *m);

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

	void set_edit_pitch_min_max(int pitch_min, int pitch_max);
	int pitch_min, pitch_max;
	int edit_pitch_min, edit_pitch_max;

	virtual bool is_playable();


	float clef_dy;
	float clef_y0;


	void set_solo(bool solo);
	bool solo;


	bool mouse_over();

	bool hidden;
	bool represents_imploded;
};

#endif /* SRC_VIEW_AUDIOVIEWLAYER_H_ */
