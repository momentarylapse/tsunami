/*
 * MidiEditorConsole.h
 *
 *  Created on: 09.09.2014
 *      Author: michi
 */

#ifndef MIDIEDITORCONSOLE_H_
#define MIDIEDITORCONSOLE_H_

#include "SideBar.h"

class TrackLayer;
class Device;
enum class MidiMode;
enum class NoteBaseLength;
enum class ChordType;
class ViewModeMidi;

class MidiEditorConsole : public SideBarConsole {
public:
	MidiEditorConsole(Session *session);
	virtual ~MidiEditorConsole();

	void on_enter() override;
	void on_leave() override;

	void on_layer_delete();
	void on_view_cur_layer_change();
	void on_view_vtrack_change();
	void on_settings_change();
	void update();
	void update_input_device_list();

	void on_base_length(NoteBaseLength l);
	void on_length_dotted();
	void on_length_triplet();
	void on_length_custom();
	NoteBaseLength get_base_length();
	void on_creation_mode();
	void on_interval();
	void on_chord_type(ChordType t);
	void on_chord_inversion(int i);
	void on_reference_tracks();
	void on_modifier(NoteModifier m);

	void on_input_active();
	void on_input_capture();
	void on_input_source();
	void on_input_volume(int mode);
	Array<Device*> input_sources;

	void on_quantize();
	void on_apply_string();
	void on_apply_hand_position();
	void on_apply_flags(int mask);
	void on_add_key_change();
	void on_apply_pitch_shift();

	void clear();
	void set_layer(TrackLayer *t);

	TrackLayer *layer;
	ViewModeMidi *mode;
};

#endif /* MIDIEDITORCONSOLE_H_ */
