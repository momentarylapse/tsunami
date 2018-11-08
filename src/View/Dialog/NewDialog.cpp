/*
 * NewDialog.cpp
 *
 *  Created on: 24.03.2012
 *      Author: michi
 */

#include "NewDialog.h"
#include "../../Data/base.h"
#include "../../Data/Song.h"
#include "../../Action/ActionManager.h"
#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Session.h"

NewDialog::NewDialog(hui::Window *_parent):
	hui::Window("new_dialog", _parent)
{
	add_string("sample_rate", "22050");
	add_string("sample_rate", i2s(DEFAULT_SAMPLE_RATE));
	add_string("sample_rate", "48000");
	add_string("sample_rate", "96000");
	set_int("sample_rate", 1);
	hide_control("nd_g_metronome_params", true);

	check("new_track_type:audio-stereo", true);

	set_int("num_bars", 32);
	set_int("beats_per_bar", 4);
	set_int("sub_beats", 1);

	event("cancel", std::bind(&NewDialog::destroy, this));
	event("hui:close", std::bind(&NewDialog::destroy, this));
	event("ok", std::bind(&NewDialog::on_ok, this));
	event("metronome", std::bind(&NewDialog::on_metronome, this));
	event("new_track_type:midi", std::bind(&NewDialog::on_type_midi, this));
}

void NewDialog::on_ok()
{
	int sample_rate = get_string("sample_rate")._int();
	auto type = SignalType::AUDIO_MONO;
	if (is_checked("new_track_type:midi"))
		type = SignalType::MIDI;
	else if (is_checked("new_track_type:audio-stereo"))
		type = SignalType::AUDIO_STEREO;
	Session *session = tsunami->create_session();
	Song *song = session->song;
	song->sample_rate = sample_rate;
	song->action_manager->enable(false);
	if (is_checked("metronome")){
		Track *t = song->add_track(SignalType::BEATS, 0);
		int count = get_int("num_bars");
		for (int i=0; i<count; i++)
			song->add_bar(-1, get_float("beats_per_minute"), get_int("beats_per_bar"), get_int("sub_beats"), false);
	}
	song->add_track(type);

	song->add_tag("title", _("New Audio File"));
	song->add_tag("album", AppName);
	song->add_tag("artist", hui::Config.get_str("DefaultArtist", AppName));

	song->action_manager->enable(true);
	song->notify(song->MESSAGE_NEW);
	song->notify(song->MESSAGE_FINISHED_LOADING);
	destroy();
	session->win->show();
	session->win->activate("");
}

void NewDialog::on_metronome()
{
	hide_control("nd_g_metronome_params", !is_checked(""));
}

void NewDialog::on_type_midi()
{
	check("metronome", true);
	hide_control("nd_g_metronome_params", false);
}

