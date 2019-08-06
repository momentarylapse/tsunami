/*
 * ViewModeMidi.h
 *
 *  Created on: 12.11.2015
 *      Author: michi
 */

#ifndef SRC_VIEW_MODE_VIEWMODEMIDI_H_
#define SRC_VIEW_MODE_VIEWMODEMIDI_H_

#include "ViewModeDefault.h"
#include "../../lib/math/math.h"

class MidiNoteBuffer;
class MidiNote;
class MidiEvent;
class AudioOutput;
class Synthesizer;
class MidiPreview;
class TrackLayer;
class ScrollBar;
class MidiInput;
class Device;
class SignalChain;
class Scale;
enum class NoteModifier;
enum class ChordType;

class ViewModeMidi : public ViewModeDefault
{
public:
	ViewModeMidi(AudioView *view);
	virtual ~ViewModeMidi();

	void on_start() override;
	void on_end() override;

	void on_key_down(int k) override;
	void on_command(const string &id) override;
	float layer_suggested_height(AudioViewLayer *l) override;
	void on_cur_layer_change() override;

	void draw_layer_background(Painter *c, AudioViewLayer *l) override;
	void draw_post(Painter *c) override;

	HoverData get_hover_data(AudioViewLayer *vlayer, float mx, float my) override;
	SongSelection get_selection_for_rect(const Range &r, int y0, int y1) override;
	SongSelection get_selection_for_range(const Range &r) override;

	SongSelection get_select_in_edit_cursor();
	void select_in_edit_cursor();

	MidiNoteBuffer get_creation_notes(HoverData *sel, int pos0);
	void set_sub_beat_partition(int partition);
	void set_note_length(int length);

	Array<int> get_creation_pitch(int base_pitch);
	int suggest_move_cursor(int pos, bool forward) override;
	Range get_edit_range();
	Range get_backwards_range();
	void start_midi_preview(const Array<int> &pitch, float ttl);
	Scale cur_scale();

	void edit_add_pause();
	void edit_add_note_on_string(int hand_pos);
	void edit_add_note_by_urelative(int relative);
	void edit_backspace();
	void jump_string(int delta);
	void jump_octave(int delta);

	int sub_beat_partition;
	int note_length;
	ChordType chord_type;
	int chord_inversion;
	int midi_interval;

	void set_modifier(NoteModifier mod);
	NoteModifier modifier;

	void set_mode(MidiMode mode);
	MidiMode mode_wanted;

	enum class CreationMode {
		SELECT,
		NOTE,
		INTERVAL,
		CHORD
	};
	void set_creation_mode(CreationMode mode);
	CreationMode creation_mode;

	enum class InputMode {
		DEFAULT,
		NOTE_LENGTH,
		BEAT_PARTITION
	};
	void set_input_mode(InputMode mode);
	InputMode input_mode;

	MidiPreview *preview;
	Device *input_wanted_device;
	Device *input_device();
	bool input_wanted_active;
	bool input_capture;

	bool is_input_active();
	void activate_input(bool active);
	void set_input_capture(bool capture);
	void _start_input();
	void _stop_input();
	void set_input_device(Device *d);
	bool maximize_input_volume;

	void on_midi_input();
	void ri_insert();

	AudioViewLayer *cur_vlayer();
	TrackLayer *cur_layer();
	bool editing(AudioViewLayer *l);

	bool moving;
	Array<int> pre_moving_offsets;
	int mouse_pre_moving_pos;
	int string_no;
	int octave;

	void set_rep_key(int k);
	int rep_key_runner;
	int rep_key;
	int rep_key_num;

	void left_click_handle_void(AudioViewLayer *vlayer) override;
	string get_tip() override;
};

#endif /* SRC_VIEW_MODE_VIEWMODEMIDI_H_ */
