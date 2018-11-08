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
enum class MidiMode;

class MidiEditorConsole : public SideBarConsole
{
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

	void on_scale();
	void on_beat_partition();
	void on_note_length();
	void on_creation_mode();
	void on_interval();
	void on_chord_type();
	void on_chord_inversion();
	void on_reference_tracks();
	void on_modifier_none();
	void on_modifier_sharp();
	void on_modifier_flat();
	void on_modifier_natural();

	void on_quantize();
	void on_apply_string();

	void on_edit_track();
	void on_edit_midi_fx();
	void on_edit_song();

	void clear();
	void set_layer(TrackLayer *t);


	string id_inner;

	TrackLayer *layer;
};

#endif /* MIDIEDITORCONSOLE_H_ */
